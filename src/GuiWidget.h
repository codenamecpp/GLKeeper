#pragma once

#include "GuiDefs.h"

// basic gui element class
class GuiWidget: public cxx::handled_object<GuiWidget>
{
    friend class GuiManager;
    friend class GuiHierarchy;

public:
    
    static GuiWidgetMetaClass MetaClass; // base widget class

    std::string mName; // user-defined identifier for widget
    GuiUserData mUserData; // user-defined data

#ifdef _DEBUG
    Color32 mDebugColorNormal = Color32_Gray;
    Color32 mDebugColorHovered = Color32_Gray;
    Color32 mDebugColorDisabled = Color32_Gray;
#endif

    // readonly
    std::string mTemplateClassName; // specified for template widgets

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

    // read widget properties from json document
    void LoadProperties(cxx::json_document_node documentNode);

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
    GuiWidget* GetChild(const std::string& name) const;

    // get child widget by its index
    GuiWidget* GetChild(int index) const;

    // get current position in local or screen space
    inline Point GetPosition() const { return mPosition; }
    inline Point GetScreenPosition() const
    {
        Point screenPosition = LocalToScreen(mPosition);
        return screenPosition;
    }

    // get current origin point in local or screen space
    inline Point GetOrigin() const { return mOrigin; }
    inline Point GetOriginScreenPosition() const
    {
        Point screenPosition = LocalToScreen(mOrigin);
        return screenPosition;
    }

    // get current size
    inline Point GetSize() const { return mSize; }

    void SetMinSize(const Point& minSize);
    void SetMaxSize(const Point& maxSize);

    // set current visibility state
    void SetVisible(bool isVisible);

    // set enabled or disabled state
    void SetEnabled(bool isEnabled);

    // set current hovered state
    void SetHovered(bool isHovered);

    // set clipping child widgets feature
    void SetClipChildren(bool isEnabled);

    // test whether widget is visible, enabled or hovered
    bool IsVisible() const;
    bool IsEnabled() const;
    bool IsHovered() const { return mHovered; }
    bool IsClipChildren() const { return mClipChildren; }

    bool IsMouseCaptured() const;

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

    template<typename TargetWidgetClass>
    inline TargetWidgetClass* CastToWidgetClass()
    {
        GuiWidgetMetaClass* target_metaclass = &TargetWidgetClass::MetaClass;
        for (GuiWidgetMetaClass* currentMetaclass = mMetaClass; currentMetaclass; 
            currentMetaclass = currentMetaclass->mParentClass)
        {
            if (target_metaclass == currentMetaclass)
                return static_cast<TargetWidgetClass*>(this);
        }
        return nullptr;
    }

protected:
    GuiWidget(GuiWidgetMetaClass* widgetClass);
    // copy properties
    GuiWidget(GuiWidget* copyWidget);

    GuiWidget* GetLastChild() const;

    void ParentPositionChanged(const Point& prevPosition);
    void ParentSizeChanged(const Point& prevSize, const Point& currSize);
    void ParentShownStateChanged();
    void ParentEnableStateChanged();

    void PositionChanged(const Point& prevPosition, bool setAnchors);
    void SizeChanged(const Point& prevSize, bool setAnchors);
    void ShownStateChanged();
    void EnableStateChanged();
    void HoveredStateChanged();

    Point ComputeOriginPixels() const;
    Point ComputePositionPixels() const;
    Point ComputeSizePixels() const;

    void SetAnchorPositions();
    void ReleaseMouseCapture();

protected:
    // overridables
    virtual void HandleLoadProperties(cxx::json_document_node documentNode);

    virtual void HandleRender(GuiRenderer& renderContext) {}
    virtual void HandleUpdate(float deltaTime) {}
    virtual void HandleSizeChanged(const Point& prevSize) {}
    virtual void HandlePositionChanged(const Point& prevPosition) {}

    virtual void HandleChildAttached(GuiWidget* childWidget) {}
    virtual void HandleChildDetached(GuiWidget* childWidget) {}

    virtual void HandleShownStateChanged() {}
    virtual void HandleEnableStateChanged() {}
    virtual void HandleHoveredStateChanged() {}

    virtual void HandleInputEvent(MouseButtonInputEvent& inputEvent) {}
    virtual void HandleInputEvent(MouseMovedInputEvent& inputEvent) {}
    virtual void HandleInputEvent(MouseScrollInputEvent& inputEvent) {}

    virtual void HandleClick() {}

    virtual bool HasAttribute(eGuiWidgetAttribute attribute) const;

    virtual GuiWidget* ConstructClone();

protected:
    GuiWidgetMetaClass* mMetaClass; // cannot be null

    GuiWidget* mParent = nullptr;
    GuiWidget* mFirstChild = nullptr;
    GuiWidget* mNextSibling = nullptr;
    GuiWidget* mPrevSibling = nullptr;

    // layout params
    GuiAnchors mAnchors;

    int mAnchorDistL = 0;
    int mAnchorDistT = 0;
    int mAnchorDistR = 0;
    int mAnchorDistB = 0;

    eGuiUnits mOriginUnitsX = eGuiUnits_Pixels;
    eGuiUnits mOriginUnitsY = eGuiUnits_Pixels;
    Point mOriginPercents;
    Point mOrigin; // pixels

    eGuiUnits mPositionUnitsX = eGuiUnits_Pixels;
    eGuiUnits mPositionUnitsY = eGuiUnits_Pixels;
    Point mPositionPercents;
    Point mPosition; // pixels

    eGuiUnits mSizeUnitsW = eGuiUnits_Pixels;
    eGuiUnits mSizeUnitsH = eGuiUnits_Pixels;
    Point mSizePercents;
    Point mSize;

    Point mMinSize;
    Point mMaxSize;

    glm::mat4 mTransform; // current transformations matrix, screen space

    // state flags
    bool mSelfEnabled = true;
    bool mSelfVisible = true;
    bool mClipChildren = false;

    bool mTransformInvalidated = true; // transformations matrix dirty
    bool mHovered = false;
};