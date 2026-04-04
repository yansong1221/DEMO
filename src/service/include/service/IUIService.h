#pragma once
#include <string>

class QWidget;
namespace service
{
    class IImGuiDrawService
    {
      public:
        virtual ~IImGuiDrawService() = default;
        virtual void drawImGui() = 0;
    };
    class IWidgetService
    {
      public:
        virtual QWidget* widget() = 0;
        virtual std::string uniqueName() const = 0;
    };
} // namespace service
