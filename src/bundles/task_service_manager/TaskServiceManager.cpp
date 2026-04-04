#include "TaskServiceManager.h"
#include "common/Logger.h"
#include "common/Misc.h"
#include "cppmicroservices/ServiceEvent.h"
#include "imgui_extend/component.h"
#include <QCoreApplication>
#include <QDir>
#include <fstream>

namespace detail
{
    static QDir
    getConfigSaveDir()
    {
        return QCoreApplication::applicationDirPath() + "/task_service_config";
    }
    static QString
    getConfigSaveFileName(service::ITaskServiceManager::ITaskServiceController const& controller)
    {
        auto dir = getConfigSaveDir();
        if (!dir.exists())
        {
            if (!dir.mkpath(dir.absolutePath()))
            {
                common::Log::warn("无法创建目录: {}", dir.absolutePath().toStdString());
            }
        }
        return dir.filePath(QString("%1.yaml").arg(QString::fromStdString(controller.serviceName())));
    }

    static void
    loadConfig(service::ITaskServiceManager::ITaskServiceController const& controller,
               std::shared_ptr<service::ITaskService::IBasicConfig> config)
    {
        auto fileName = getConfigSaveFileName(controller);

        try
        {
            YAML::Node yaml;
            if (QFile::exists(fileName))
            {
                yaml = YAML::LoadFile(fileName.toLocal8Bit().constData());
            }
            config->restore(yaml);
        }
        catch (std::exception const& e)
        {
            common::Log::warn("加载配置文件:{} 出现异常:{}", fileName.toUtf8(), common::misc::to_u8string(e.what()));
        }
    }
    static void
    saveConfig(service::ITaskServiceManager::ITaskServiceController const& controller,
               std::shared_ptr<service::ITaskService::IBasicConfig> config)
    {
        auto fileName = getConfigSaveFileName(controller);

        try
        {
            YAML::Node yaml;
            config->save(yaml);

            std::ofstream file(fileName.toLocal8Bit());
            file << yaml;
        }
        catch (std::exception const& e)
        {
            common::Log::warn("保存配置文件:{} 出现异常:{}", fileName.toUtf8(), common::misc::to_u8string(e.what()));
        }
    }

} // namespace detail

TaskServiceControllerImpl::TaskServiceControllerImpl(cppmicroservices::ServiceReference<service::ITaskService> ref,
                                                     std::shared_ptr<service::ITaskService> service)
    : m_ref(ref)
    , m_service(std::move(service))
{
}

TaskServiceControllerImpl::~TaskServiceControllerImpl() { stop(); }

std::string
TaskServiceControllerImpl::symbolicName() const
{
    try
    {
        auto b = m_ref.GetBundle();
        return b ? b.GetSymbolicName() : std::string();
    }
    catch (...)
    {
        return std::string();
    }
}

std::string
TaskServiceControllerImpl::serviceName() const
{
    return m_service->name();
}

service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus
TaskServiceControllerImpl::status() const
{
    return isRunning() ? TaskServiceStatus::Running : TaskServiceStatus::Stopped;
}

std::shared_ptr<service::ITaskService::IBasicConfig>
TaskServiceControllerImpl::createConfig() const
{
    auto conf = m_service->createConfig();
    detail::loadConfig(*this, conf);
    return conf;
}

void
TaskServiceControllerImpl::saveConfig(std::shared_ptr<service::ITaskService::IBasicConfig> config)
{
    detail::saveConfig(*this, config);
}

void
TaskServiceControllerImpl::setStatusCallback(TaskServiceStatusCallback callback)
{
    std::lock_guard lock(m_mutex);
    m_statusCallback = std::move(callback);
}

bool
TaskServiceControllerImpl::start()
{
    if (isRunning())
    {
        return true;
    }
    return Thread::startThread();
}

void
TaskServiceControllerImpl::stop()
{
    if (isRunning())
    {
        m_service->requestStop();
        Thread::stopThread();
    }
}

bool
TaskServiceControllerImpl::isSelf(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                                  std::shared_ptr<service::ITaskService> const& service) const
{
    return m_ref == reference && m_service == service;
}

std::string
TaskServiceControllerImpl::displayServiceName() const
{
    return m_service->displayName();
}

bool
TaskServiceControllerImpl::onThreadStart()
{
    if (!m_service->onThreadStart(createConfig()))
    {
        return false;
    }

    std::lock_guard lock(m_mutex);
    if (m_statusCallback)
    {
        m_statusCallback(TaskServiceStatus::Running);
    }

    return true;
}

bool
TaskServiceControllerImpl::onThreadRun()
{
    return m_service->onThreadRun();
}

void
TaskServiceControllerImpl::onThreadEnd()
{
    m_service->onThreadEnd();

    std::lock_guard lock(m_mutex);
    if (m_statusCallback)
    {
        m_statusCallback(TaskServiceStatus::Stopped);
    }
}

TaskServiceManagerImpl::TaskServiceManagerImpl(cppmicroservices::BundleContext context)
    : m_context(context)
    , m_Tracker(context, this)
{
    m_Tracker.Open();
}

TaskServiceManagerImpl::~TaskServiceManagerImpl()
{
    m_Tracker.Close();

    for (auto& entry : m_services)
    {
        entry->stop();
    }
}

std::vector<service::ITaskServiceManager::ControllerPtr>
TaskServiceManagerImpl::listTaskControllers() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<service::ITaskServiceManager::ControllerPtr> controllers;
    for (auto const& entry : m_services)
    {
        controllers.push_back(entry);
    }
    return controllers;
}

void
TaskServiceManagerImpl::setControllerEventCallback(ControllerEventCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_controllerEventCallback = std::move(callback);
}

void
TaskServiceManagerImpl::drawImGui()
{
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(tr("UITaskManager").toUtf8()))
    {
        this->draw();
        if (restartMsgBox_.Draw() == 1)
        {
            if (currentController_)
            {
                currentController_->stop();
                currentController_->start();
            }
        }
    }
    ImGui::End();
}

int
TaskServiceManagerImpl::columnCount() const
{
    return 4;
}

std::string
TaskServiceManagerImpl::headerLable(int column) const
{
    switch (column)
    {
        case ColName:
            return tr("Service Name").toStdString();
        case ColBundle:
            return tr("Bundle Name").toStdString();
        case ColStatus:
            return tr("Status").toStdString();
        case ColActions:
            return tr("Actions").toStdString();
        default:
            break;
    }
    return {};
}

int
TaskServiceManagerImpl::rowCount() const
{
    return m_services.size();
}

std::shared_ptr<service::ITaskService>
TaskServiceManagerImpl::AddingService(cppmicroservices::ServiceReference<service::ITaskService> const& reference)
{
    auto svc = m_context.GetService(reference);
    if (!svc)
    {
        return nullptr;
    }
    auto controller = std::make_shared<TaskServiceControllerImpl>(reference, svc);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_services.push_back(controller);
    if (m_controllerEventCallback)
    {
        m_controllerEventCallback(controller, ControllerEvent::Registered);
    }
    tableReloadRows();
    return svc;
}

void
TaskServiceManagerImpl::ModifiedService(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                                        std::shared_ptr<service::ITaskService> const& service)
{
}

void
TaskServiceManagerImpl::RemovedService(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                                       std::shared_ptr<service::ITaskService> const& service)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_services.begin(),
                           m_services.end(),
                           [&reference, &service](std::shared_ptr<TaskServiceControllerImpl> const& entry)
                           { return entry->isSelf(reference, service); });
    if (it == m_services.end())
    {
        return;
    }

    // 清空配置变量
    if (currentController_ && currentController_->isSelf(reference, service))
    {
        currentController_.reset();
        currentConfig_.reset();
    }

    (*it)->stop();

    if (m_controllerEventCallback)
    {
        m_controllerEventCallback(*it, ControllerEvent::Unregistered);
    }
    tableReloadRows();
    m_services.erase(it);
}

void
TaskServiceManagerImpl::drawCell(int row, int column)
{
    auto controller = m_services.at(row);

    switch (column)
    {
        case ColName:
        {
            ImGui::TextUnformatted(controller->serviceName().c_str());
        }
        break;
        case ColBundle:
        {
            ImGui::TextUnformatted(controller->symbolicName().c_str());
        }
        break;
        case ColStatus:
        {
            ImGui::TextUnformatted(
                controller->status() == service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus::Running
                    ? tr("Running").toUtf8()
                    : tr("Stopped").toUtf8());
        }
        break;
        case ColActions:
        {
            switch (controller->status())
            {
                case service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus::Running:
                {
                    if (ImGui::extend::DeleteButton(tr("Stop").toUtf8()))
                    {
                        controller->stop();
                    }
                }
                break;
                case service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus::Stopped:
                {
                    if (ImGui::Button(tr("Start").toUtf8()))
                    {
                        controller->start();
                    }
                }
                break;
                default:
                    break;
            }
            ImGui::SameLine();

            if (ImGui::Button(tr("Setup").toUtf8()))
            {
                currentConfig_ = controller->createConfig();
                currentController_ = controller;

                ImGui::OpenPopup(tr("Setup UI").toUtf8().constData(), 0);
                ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
            }
            if (currentConfig_ && currentController_ && ImGui::BeginPopupModal(tr("Setup UI").toUtf8(), NULL))
            {
                ImGui::extend::BeginChild(
                    [&]()
                    {
                        if (ImGui::Button(tr("Save").toUtf8()))
                        {
                            currentController_->saveConfig(currentConfig_);

                            ImGui::CloseCurrentPopup();

                            restartMsgBox_.Init(tr("Start task").toStdString(),
                                                "",
                                                tr("Do you want to start/restart the task immediately?").toStdString(),
                                                { tr("Yes").toStdString(), tr("NO").toStdString() });

                            restartMsgBox_.Open();
                        }
                        ImGui::SameLine();
                        if (ImGui::extend::DeleteButton(tr("Cancel").toUtf8()))
                        {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SeparatorText(tr("Setup Items").toUtf8());

                        ImGui::extend::BeginChild([&]() { currentConfig_->draw(); }, "##BeginChildDrawConfigUI");
                    },
                    "##配置界面");

                ImGui::EndPopup();
            }
        }
        break;
        default:
            break;
    }
}
