#pragma once

// forwards
class GuiRenderer;
class GuiWidget;

// alias
using GuiUserData = void*; // widget custom data

enum eGuiVertAlignment
{
    eGuiVertAlignment_None,
    eGuiVertAlignment_Top,
    eGuiVertAlignment_Center,
    eGuiVertAlignment_Bottom,
};
decl_enum_strings(eGuiVertAlignment);

enum eGuiHorzAlignment
{
    eGuiHorzAlignment_None,
    eGuiHorzAlignment_Left,
    eGuiHorzAlignment_Center,
    eGuiHorzAlignment_Right,
};
decl_enum_strings(eGuiHorzAlignment);

enum eGuiOriginMode
{
    eGuiOrigin_Relative,
    eGuiOrigin_Fixed,
};
decl_enum_strings(eGuiOriginMode);

// gui widget anchors struct
struct GuiAnchors
{
public:
    GuiAnchors() = default;
    GuiAnchors(bool anchorLeft, bool anchorTop, bool anchorRight, bool anchorBottom)
        : mLeft(anchorLeft)
        , mTop(anchorTop)
        , mRight(anchorRight)
        , mBottom(anchorBottom)
    {
    }
public:
    bool mLeft = true, mTop = true;
    bool mRight = false, mBottom = false;
};

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