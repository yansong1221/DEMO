#pragma once
#include <QString>

class QWidget;
namespace demo {
class IWidgetService
{
public:
    virtual QWidget* widget()  = 0;
    virtual QString uniqueName() const = 0;
};
class IWidgetPlus : public IWidgetService
{
public:
    virtual void hello() = 0;
};
} // namespace demo