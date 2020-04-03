#pragma once

// forwards
class GuiRenderer;
class GuiWidget;

// Typedefs
using GuiWidgetUserData = void*; // any data

// creates widget instance of specific class
class GuiWidgetFactoryInterface
{
public:
    virtual ~GuiWidgetFactoryInterface()
    {
    }

    virtual GuiWidget* ConstructWidget() = 0;
};

// typical widget factory implementation
template<typename TWidgetClass>
class GuiWidgetFactory final: public GuiWidgetFactoryInterface
{
public:
    // Construct widget instance on heap
    inline TWidgetClass* ConstructWidget() override
    {
        return new TWidgetClass;
    }
};

// widget class descriptor, unique for each type of widgets
class GuiWidgetClass
{
public:
    // widget class does not changes during runtime
    const std::string mClassName;

    // factory is responsible for constructing widgets of this class
    // it should not be null
    GuiWidgetFactoryInterface* mFactory;

public:
    GuiWidgetClass(std::string&& className, GuiWidgetFactoryInterface* factory)
        : mClassName(className)
        , mFactory(factory)
    {
        debug_assert(mFactory);
    }
};