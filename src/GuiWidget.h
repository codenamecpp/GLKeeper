#pragma once

#include "GuiDefs.h"
#include "GuiAction.h"
#include "GuiLayout.h"

// basic gui element class
class GuiWidget: public cxx::handled_object<GuiWidget>
{
    friend class GuiManager;
    friend class GuiHierarchy;
    friend class GuiAction;
    friend class GuiLayout;
    friend class GuiScreen;

public:
    
    static GuiWidgetMetaClass MetaClass; // base widget class

    cxx::unique_string mId; // user-defined identifier for widget
    GuiUserData mUserData; // user-defined data

    // colors
    Color32 mTintColor = Color32_White;
    Color32 mBackgroundColor = Color32_Gray;
    Color32 mBordersColor = Color32_White;

    // readonly
    cxx::unique_string mTemplateClassName; // specified for template widgets
    Point mOrigin; // pixels
    Point mPosition; // pixels
    Point mSize; // pixels
    Point mMinSize; // pixels
    Point mMaxSize; // pixels
    GuiWidgetMetaClass* mMetaClass; // cannot be null, cannot be changed once widget created
    GuiActionsHolder mActions;
    GuiActionContext* mActionsContext = nullptr;

public:
    // construct widget
    GuiWidget();
    virtual ~GuiWidget();

    // render widget and all children
    // @param renderContext: Gui render context
    void RenderFrame(GuiRenderer& renderContext);

    // process widget logic and all children
    // @param deltaTime: Time passed since previous update
    void UpdateFrame(float deltaTime);

    // force update widget layout
    void UpdateLayout();

    // read widget properties from json document
    void LoadProperties(cxx::json_node_object documentNode);

    // process input event
    // @param inputEvent: Event data
    void ProcessEvent(MouseButtonInputEvent& inputEvent);
    void ProcessEvent(MouseMovedInputEvent& inputEvent);
    void ProcessEvent(MouseScrollInputEvent& inputEvent);

    // set current position in local or screen space
    // @param position: Position absolute or relative [0-100] %
    // @param units_x, units_y: Units of x and y
    void SetPosition(const Point& position, eGuiUnits units_x = eGuiUnits_Pixels, eGuiUnits units_y = eGuiUnits_Pixels);
    void SetPositionX(int posx, eGuiUnits untis_x = eGuiUnits_Pixels);
    void SetPositionY(int posy, eGuiUnits units_y = eGuiUnits_Pixels);
    void SetScreenPosition(const Point& position)
    {
        Point pos = ScreenToLocal(position);
        SetPosition(pos, eGuiUnits_Pixels, eGuiUnits_Pixels);
    }

    // set current origin position in local or screen space
    // @param position: Position absolute or relative [0-100] %
    // @param units_x, units_y: Units of x and y
    void SetOrigin(const Point& position, eGuiUnits units_x = eGuiUnits_Pixels, eGuiUnits units_y = eGuiUnits_Pixels);
    void SetOriginX(int posx, eGuiUnits units_x = eGuiUnits_Pixels);
    void SetOriginY(int posy, eGuiUnits units_y = eGuiUnits_Pixels);
    void SetOriginScreenPosition(const Point& position)
    {
        Point pos = ScreenToLocal(position);
        SetOrigin(pos, eGuiUnits_Pixels, eGuiUnits_Pixels);
    }

    // set current size
    // @param size: Size absolute or relative [0-100] %
    // @param units_w, units_h: Units of w and h
    void SetSize(const Point& size, eGuiUnits units_w = eGuiUnits_Pixels, eGuiUnits units_h = eGuiUnits_Pixels);
    void SetSizeW(int sizew, eGuiUnits units_w = eGuiUnits_Pixels);
    void SetSizeH(int sizeh, eGuiUnits units_h = eGuiUnits_Pixels);

    // setup widget anchors
    void SetAnchors(const GuiAnchors& anchors);
    bool HasAnchors() const;

    // test whether screen space point is within widget rect
    // @param screenPosition: Test point
    bool IsScreenPointInsideRect(const Point& screenPosition) const;

    // get current position and size
    inline Rectangle GetLocalRect() const
    {
        Rectangle rcLocal (0, 0, mSize.x, mSize.y);
        return rcLocal;
    }

    // attach or detach child widget
    bool AttachChild(GuiWidget* widget);
    bool DetachChild(GuiWidget* widget);

    void DetachAndFreeChildren();

    // hierarchy inspection
    inline GuiWidget* GetParent() const { return mParent; }
    inline GuiWidget* GetChild() const { return mFirstChild; }
    inline GuiWidget* NextSibling() const { return mNextSibling; }
    inline GuiWidget* PrevSibling() const { return mPrevSibling; }

    // find visible and interactive child widget at specified screen coordinate
    // return self if no child can be picked
    // @param screenPosition: Screen coordinate
    GuiWidget* PickWidget(const Point& screenPosition);

    // get child widget by its name
    GuiWidget* GetChild(const cxx::unique_string& name) const;
    GuiWidget* GetChild(const std::string& name) const;
    GuiWidget* GetLastChild() const;
    // get first child withing sub-hierarchy with specific name
    GuiWidget* SearchForChild(const cxx::unique_string& name) const;
    GuiWidget* SearchForChild(const std::string& name) const;
    // get child widget by its index
    GuiWidget* GetChild(int index) const;

    // get current position in screen space
    inline Point GetScreenPosition() const
    {
        Point screenPosition = LocalToScreen(mPosition);
        return screenPosition;
    }

    // get current origin point in screen space
    inline Point GetOriginScreenPosition() const
    {
        Point screenPosition = LocalToScreen(mOrigin);
        return screenPosition;
    }

    void SetMinSize(const Point& minSize);
    void SetMaxSize(const Point& maxSize);

    // set current state
    void SetVisible(bool isVisible);
    void SetEnabled(bool isEnabled);

    // set clipping child widgets feature
    void SetClipChildren(bool isEnabled);

    // enable or disable borders and background draw
    void SetDrawBackground(bool isEnabled);
    void SetDrawBorders(bool isEnabled);

    // test whether widget is visible, enabled or hovered
    bool IsVisibleWithParent() const;
    bool IsEnabledWithParent() const;
    bool IsVisible() const { return mVisible; }
    bool IsEnabled() const { return mEnabled; }
    bool IsHovered() const { return mHovered; }
    bool IsPressed() const { return mPressed; }
    bool IsClipChildren() const { return mClipChildren; }

    // set extended context for widget actions
    // @param context: Context or null
    void SetActionsContext(GuiActionContext* context);

    // convert from local to screen space and vice versa
    // @param position: Point
    Point LocalToScreen(const Point& position) const;
    Point ScreenToLocal(const Point& position) const;

    // force update transformations or invalidate current state
    void ComputeTransform();
    void InvalidateTransform();

    // clone widget with or without its chindren
    GuiWidget* Clone();
    GuiWidget* CloneDeep();

    // try to cast base widget to specific widget class
    template<typename TargetWidgetClass>
    inline TargetWidgetClass* CastToWidgetClass()
    {
        TargetWidgetClass* resultWidget = nullptr;
        GuiWidgetMetaClass* target_metaclass = &TargetWidgetClass::MetaClass;
        for (GuiWidgetMetaClass* currMetaclass = mMetaClass; currMetaclass; 
            currMetaclass = currMetaclass->mParentClass)
        {
            if (target_metaclass == currMetaclass)
            {
                resultWidget = (TargetWidgetClass*) this;
                break;
            }
        }
        return resultWidget;
    }

protected:
    GuiWidget(GuiWidgetMetaClass* widgetClass);
    // copy properties
    GuiWidget(GuiWidget* copyWidget);

    void ParentPositionChanged(const Point& prevPosition);
    void ParentSizeChanged(const Point& prevSize, const Point& currSize);
    void ParentShownStateChanged();
    void ParentEnableStateChanged();

    void PositionChanged(const Point& prevPosition, bool setAnchors);
    void SizeChanged(const Point& prevSize, bool setAnchors);
    void ShownStateChanged();
    void EnableStateChanged();
    void HoveredStateChanged();
    void PressedStateChanged();

    Point ComputeOriginPixels() const;
    Point ComputePositionPixels() const;
    Point ComputeSizePixels() const;

    void SetDetached();
    void SetupAnchorsOffsets();

    // mouse capture
    void GetMouseCapture();
    void ReleaseMouseCapture();

    // post widget event to gui system
    void DispatchEvent(const GuiEvent& eventData);

    bool ResolveCondition(const cxx::unique_string& name, bool& isTrue) const;

    // load widget actions from json document node
    void LoadActions(cxx::json_node_array actionsNode);

    // gui notifications
    void NotifyHoverStateChange(bool isHovered);
    void NotifyMouseCaptureStateChange(bool isMouseCapture);

protected:
    // overridables
    virtual void HandleLoadProperties(cxx::json_node_object documentNode) {}
    virtual bool HandleResolveCondition(const cxx::unique_string& name, bool& isTrue) const
    {
        return false;
    }

    virtual void HandleRender(GuiRenderer& renderContext) {}
    virtual void HandleUpdate(float deltaTime) {}
    virtual void HandleSizeChanged(const Point& prevSize) {}
    virtual void HandlePositionChanged(const Point& prevPosition) {}

    virtual void HandleChildAttached(GuiWidget* childWidget) {}
    virtual void HandleChildDetached(GuiWidget* childWidget) {}

    virtual void HandleShownStateChanged() {}
    virtual void HandleEnableStateChanged() {}
    virtual void HandleHoveredStateChanged() {}
    virtual void HandlePressedStateChanged() {}

    virtual void HandleInputEvent(MouseButtonInputEvent& inputEvent) {}
    virtual void HandleInputEvent(MouseMovedInputEvent& inputEvent) {}
    virtual void HandleInputEvent(MouseScrollInputEvent& inputEvent) {}

    virtual void HandleClick(eMouseButton mouseButton) {}

    virtual GuiWidget* CreateClone();

protected:
    GuiWidget* mParent = nullptr;
    GuiWidget* mFirstChild = nullptr;
    GuiWidget* mNextSibling = nullptr;
    GuiWidget* mPrevSibling = nullptr;

    GuiLayout mLayout;

    // layout params
    GuiAnchors mAnchors;

    int mAnchorDistL = 0;
    int mAnchorDistT = 0;
    int mAnchorDistR = 0;
    int mAnchorDistB = 0;

    eGuiUnits mOriginUnitsX = eGuiUnits_Pixels;
    eGuiUnits mOriginUnitsY = eGuiUnits_Pixels;
    Point mOriginPercents;

    eGuiUnits mPositionUnitsX = eGuiUnits_Pixels;
    eGuiUnits mPositionUnitsY = eGuiUnits_Pixels;
    Point mPositionPercents;

    eGuiUnits mSizeUnitsW = eGuiUnits_Pixels;
    eGuiUnits mSizeUnitsH = eGuiUnits_Pixels;
    Point mSizePercents;

    glm::mat4 mTransform; // current transformations matrix, screen space

    // attributes
    bool mHasInteractiveAttribute = false; // widget receiving mouse inputs and can be pressed or hovered
    bool mHasDisablePickChildrenAttribute = false; // cannot pick child widgets
    bool mHasDrawBackgroundAttribute = false; // fill widget rectangle with background color
    bool mHasDrawBordersAttribute = false; // draw borders

    bool mEnabled = true;
    bool mVisible = true;
    bool mClipChildren = false;

    // state flags
    bool mTransformInvalidated = true; // transformations matrix dirty
    bool mHovered = false;
    bool mPressed = false;

    eMouseButton mPressMouseButton = eMouseButton_null;
};