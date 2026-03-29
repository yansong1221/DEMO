#pragma once

class QWidget;
namespace demo {
class IWidgetService
{
public:
    virtual QWidget* createWidget(QWidget* parent) = 0;
    virtual void destroyWidget(QWidget* widget)    = 0;
};
class IWidgetPlus : public IWidgetService
{
public:
    virtual void hello() = 0;
};
} // namespace demo