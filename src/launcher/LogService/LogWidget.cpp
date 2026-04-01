#include "LogWidget.h"

#include <QCheckBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QVBoxLayout>

LogWidget::LogWidget(QWidget* parent) : QWidget(parent)
{
    setupUI();
    setupConnections();

    // 初始化格式
    m_errorFormat.setForeground(QColor(220, 53, 69)); // 红色
    m_errorFormat.setFontWeight(QFont::Bold);

    m_warningFormat.setForeground(QColor(255, 193, 7)); // 黄色/橙色
    m_warningFormat.setFontWeight(QFont::Bold);

    m_infoFormat.setForeground(QColor(40, 167, 69)); // 绿色

    m_debugFormat.setForeground(QColor(108, 117, 125)); // 灰色

    m_traceFormat.setForeground(QColor(173, 181, 189)); // 浅灰色
    m_traceFormat.setFontItalic(true);

    m_auditFormat.setForeground(QColor(111, 66, 193)); // 紫色
    m_auditFormat.setFontWeight(QFont::Bold);
}

LogWidget::~LogWidget() = default;

void
LogWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // 搜索和过滤工具栏
    auto* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(4);

    // 搜索框
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("搜索日志..."));
    m_searchEdit->setClearButtonEnabled(true);
    toolbarLayout->addWidget(m_searchEdit, 1);

    m_searchNextBtn = new QPushButton(tr("下一个"), this);
    m_searchPrevBtn = new QPushButton(tr("上一个"), this);
    toolbarLayout->addWidget(m_searchNextBtn);
    toolbarLayout->addWidget(m_searchPrevBtn);

    toolbarLayout->addSpacing(16);

    // 级别过滤
    m_showErrorCheck = new QCheckBox(tr("错误"), this);
    m_showWarningCheck = new QCheckBox(tr("警告"), this);
    m_showInfoCheck = new QCheckBox(tr("信息"), this);
    m_showDebugCheck = new QCheckBox(tr("调试"), this);

    m_showErrorCheck->setChecked(true);
    m_showWarningCheck->setChecked(true);
    m_showInfoCheck->setChecked(true);
    m_showDebugCheck->setChecked(false);

    toolbarLayout->addWidget(new QLabel(tr("显示级别:"), this));
    toolbarLayout->addWidget(m_showErrorCheck);
    toolbarLayout->addWidget(m_showWarningCheck);
    toolbarLayout->addWidget(m_showInfoCheck);
    toolbarLayout->addWidget(m_showDebugCheck);

    toolbarLayout->addStretch();

    // 自动滚动和清空
    m_autoScrollCheck = new QCheckBox(tr("自动滚动"), this);
    m_autoScrollCheck->setChecked(true);
    toolbarLayout->addWidget(m_autoScrollCheck);

    m_clearBtn = new QPushButton(tr("清空"), this);
    toolbarLayout->addWidget(m_clearBtn);

    mainLayout->addLayout(toolbarLayout);

    // 日志显示区域
    m_logDisplay = new QTextEdit(this);
    m_logDisplay->setReadOnly(true);
    m_logDisplay->setLineWrapMode(QTextEdit::WidgetWidth);
    m_logDisplay->setFont(QFont("Consolas", 9));
    mainLayout->addWidget(m_logDisplay, 1);

    // 状态栏
    auto* statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel(tr("就绪"), this);
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    mainLayout->addLayout(statusLayout);
}

void
LogWidget::setupConnections()
{
    connect(m_searchEdit, &QLineEdit::textChanged, this, &LogWidget::onSearchTextChanged);
    connect(m_searchNextBtn, &QPushButton::clicked, this, &LogWidget::onSearchNext);
    connect(m_searchPrevBtn, &QPushButton::clicked, this, &LogWidget::onSearchPrevious);
    connect(m_clearBtn, &QPushButton::clicked, this, &LogWidget::onClearLog);

    connect(m_showErrorCheck, &QCheckBox::stateChanged, this, &LogWidget::onFilterChanged);
    connect(m_showWarningCheck, &QCheckBox::stateChanged, this, &LogWidget::onFilterChanged);
    connect(m_showInfoCheck, &QCheckBox::stateChanged, this, &LogWidget::onFilterChanged);
    connect(m_showDebugCheck, &QCheckBox::stateChanged, this, &LogWidget::onFilterChanged);
}

void
LogWidget::addLog(int level, QString const& bundleName, QString const& message)
{
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    entry.level = level;
    entry.bundleName = bundleName;
    entry.message = message;

    QString timestamp = QDateTime::fromMSecsSinceEpoch(entry.timestamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
    entry.fullText = QString("[%1] [%2] [%3] %4").arg(timestamp).arg(levelToString(level)).arg(bundleName).arg(message);

    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_logEntries.push_back(entry);

        // 限制最大行数
        if (static_cast<int>(m_logEntries.size()) > m_maxLogLines)
        {
            m_logEntries.erase(m_logEntries.begin());
        }
    }

    if (entryMatchesFilter(entry))
    {
        appendToDisplay(entry);
    }

    emit logEntryAdded(entry);

    // 更新状态
    m_statusLabel->setText(tr("共 %1 条日志").arg(m_logEntries.size()));
}

void
LogWidget::appendToDisplay(LogEntry const& entry)
{
    QTextCursor cursor(m_logDisplay->document());
    cursor.movePosition(QTextCursor::End);

    cursor.insertText(entry.fullText + "\n", getFormatForLevel(entry.level));

    if (m_autoScrollCheck->isChecked())
    {
        m_logDisplay->verticalScrollBar()->setValue(m_logDisplay->verticalScrollBar()->maximum());
    }
}

void
LogWidget::refreshDisplay()
{
    m_logDisplay->clear();

    std::lock_guard<std::mutex> lock(m_logMutex);
    for (auto const& entry : m_logEntries)
    {
        if (entryMatchesFilter(entry))
        {
            appendToDisplay(entry);
        }
    }
}

QTextCharFormat
LogWidget::getFormatForLevel(int level) const
{
    switch (level)
    {
        case 0:
            return m_errorFormat;
        case 1:
            return m_warningFormat;
        case 2:
            return m_infoFormat;
        case 3:
            return m_debugFormat;
        case 4:
            return m_traceFormat;
        case 5:
            return m_auditFormat;
        default:
            return m_infoFormat;
    }
}

QString
LogWidget::levelToString(int level) const
{
    switch (level)
    {
        case 0:
            return tr("ERROR");
        case 1:
            return tr("WARN");
        case 2:
            return tr("INFO");
        case 3:
            return tr("DEBUG");
        case 4:
            return tr("TRACE");
        case 5:
            return tr("AUDIT");
        default:
            return tr("UNKNOWN");
    }
}

QColor
LogWidget::levelToColor(int level) const
{
    switch (level)
    {
        case 0:
            return QColor(220, 53, 69);
        case 1:
            return QColor(255, 193, 7);
        case 2:
            return QColor(40, 167, 69);
        case 3:
            return QColor(108, 117, 125);
        case 4:
            return QColor(173, 181, 189);
        case 5:
            return QColor(111, 66, 193);
        default:
            return QColor(0, 0, 0);
    }
}

bool
LogWidget::entryMatchesFilter(LogEntry const& entry) const
{
    switch (entry.level)
    {
        case 0:
            return m_showErrorCheck->isChecked();
        case 1:
            return m_showWarningCheck->isChecked();
        case 2:
            return m_showInfoCheck->isChecked();
        case 3:
            return m_showDebugCheck->isChecked();
        case 4:
            return m_showDebugCheck->isChecked(); // Trace 跟随 Debug
        case 5:
            return true; // Audit 始终显示
        default:
            return true;
    }
}

void
LogWidget::onSearchTextChanged(QString const& text)
{
    m_currentSearchText = text;
    m_currentSearchIndex = -1;
    highlightSearchResults();
}

void
LogWidget::highlightSearchResults()
{
    // 简单的文本高亮实现
    if (m_currentSearchText.isEmpty())
    {
        return;
    }

    // 刷新显示以清除之前的高亮
    refreshDisplay();
}

void
LogWidget::onSearchNext()
{
    if (m_currentSearchText.isEmpty())
    {
        return;
    }

    QString text = m_logDisplay->toPlainText();
    int pos = text.indexOf(m_currentSearchText, m_currentSearchIndex + 1, Qt::CaseInsensitive);

    if (pos >= 0)
    {
        m_currentSearchIndex = pos;
        QTextCursor cursor(m_logDisplay->document());
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, m_currentSearchText.length());
        m_logDisplay->setTextCursor(cursor);
        m_logDisplay->ensureCursorVisible();
    }
    else
    {
        // 从头开始搜索
        pos = text.indexOf(m_currentSearchText, 0, Qt::CaseInsensitive);
        if (pos >= 0)
        {
            m_currentSearchIndex = pos;
            QTextCursor cursor(m_logDisplay->document());
            cursor.setPosition(pos);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, m_currentSearchText.length());
            m_logDisplay->setTextCursor(cursor);
            m_logDisplay->ensureCursorVisible();
        }
    }
}

void
LogWidget::onSearchPrevious()
{
    if (m_currentSearchText.isEmpty())
    {
        return;
    }

    QString text = m_logDisplay->toPlainText();
    int startPos = (m_currentSearchIndex < 0) ? text.length() : m_currentSearchIndex;
    int pos = text.lastIndexOf(m_currentSearchText, startPos - 1, Qt::CaseInsensitive);

    if (pos >= 0)
    {
        m_currentSearchIndex = pos;
        QTextCursor cursor(m_logDisplay->document());
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, m_currentSearchText.length());
        m_logDisplay->setTextCursor(cursor);
        m_logDisplay->ensureCursorVisible();
    }
}

void
LogWidget::onClearLog()
{
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_logEntries.clear();
    }
    m_logDisplay->clear();
    m_currentSearchIndex = -1;
    m_statusLabel->setText(tr("已清空"));
}

void
LogWidget::onFilterChanged()
{
    refreshDisplay();
}

void
LogWidget::onAutoScrollToggled(bool checked)
{
    Q_UNUSED(checked)
    // 自动滚动状态已更改
}

void
LogWidget::clearLog()
{
    onClearLog();
}

QString
LogWidget::getAllLogText() const
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    QString result;
    for (auto const& entry : m_logEntries)
    {
        result += entry.fullText + "\n";
    }
    return result;
}

void
LogWidget::setMaxLogLines(int maxLines)
{
    m_maxLogLines = maxLines;

    std::lock_guard<std::mutex> lock(m_logMutex);
    if (static_cast<int>(m_logEntries.size()) > m_maxLogLines)
    {
        m_logEntries.erase(m_logEntries.begin(), m_logEntries.begin() + (m_logEntries.size() - m_maxLogLines));
        refreshDisplay();
    }
}
