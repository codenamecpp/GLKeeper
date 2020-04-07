#pragma once

#include "GuiDefs.h"

// basic gui element class
class GuiWidget: public cxx::handled_object<GuiWidget>
    // internals
    , private GuiDragDropHandler
{
    friend class GuiManager;

public:
    std::string mName; // user-defined identifier for widget
    GuiUserData mUserData; // user-defined data

#ifdef _DEBUG
    Color32 mDebugColorNormal = Color32_Gray;
    Color32 mDebugColorHovered = Color32_Gray;
    Color32 mDebugColorDisabled = Color32_Gray;
#endif

public:
    // construct widget
    GuiWidget(GuiWidgetClass* widgetClass);
    GuiWidget();
    virtual ~GuiWidget();

    // render widget and all children
    // @param renderContext: Gui render context
    void RenderFrame(GuiRenderer& renderContext);

    // process widget logic and all children
    // @param deltaTime: Time passed since previous update
    void UpdateFrame(float deltaTime);

    // set current position in local or screen space
    // @param position: Position absolute or relative [0-100] %
    // @param xAddressingMode, yAddressingMode: Addressing modes for x and y
    void SetPosition(const Point& position, 
        eGuiAddressingMode xAddressingMode = eGuiAddressingMode_Absolute, 
        eGuiAddressingMode yAddressingMode = eGuiAddressingMode_Absolute);
    void SetPositionX(int posx, eGuiAddressingMode xAddressingMode = eGuiAddressingMode_Absolute);
    void SetPositionY(int posy, eGuiAddressingMode yAddressingMode = eGuiAddressingMode_Absolute);
    void SetScreenPosition(const Point& position)
    {
        Point pos = ScreenToLocal(position);
        SetPosition(pos, eGuiAddressingMode_Absolute, eGuiAddressingMode_Absolute);
    }

    // set current origin position in local or screen space
    // @param position: Position absolute or relative [0-100] %
    // @param xAddressingMode, yAddressingMode: Addressing modes for x and y
    void SetOrigin(const Point& position, 
        eGuiAddressingMode xAddressingMode = eGuiAddressingMode_Absolute, 
        eGuiAddressingMode yAddressingMode = eGuiAddressingMode_Absolute);
    void SetOriginX(int posx, eGuiAddressingMode xAddressingMode = eGuiAddressingMode_Absolute);
    void SetOriginY(int posy, eGuiAddressingMode yAddressingMode = eGuiAddressingMode_Absolute);
    void SetOriginScreenPosition(const Point& position)
    {
        Point pos = ScreenToLocal(position);
        SetOrigin(pos, eGuiAddressingMode_Absolute, eGuiAddressingMode_Absolute);
    }
    void SetOriginToCenter();

    // set current size
    // @param size: Size absolute or relative [0-100] %
    // @param wAddressingMode, hAddressingMode: Addressing modes for w and h
    void SetSize(const Point& size, 
        eGuiAddressingMode wAddressingMode = eGuiAddressingMode_Absolute, 
        eGuiAddressingMode hAddressingMode = eGuiAddressingMode_Absolute);
    void SetSizeW(int sizew, eGuiAddressingMode wAddressingMode = eGuiAddressingMode_Absolute);
    void SetSizeH(int sizeh, eGuiAddressingMode hAddressingMode = eGuiAddressingMode_Absolute);

    // setup widget anchors
    void SetAnchors(const GuiAnchorsStruct& anchors);

    // test whether screen space point is within widget rect
    // @param screenPosition: Test point
    bool IsScreenPointInsideRect(const Point& screenPosition) const;

    // get current position and size
    // @param outputRect: Output local rect
    void GetLocalRect(Rectangle& outputRect) const
    {
        outputRect.x = 0;
        outputRect.y = 0;
        outputRect.w = mCurrentSize.x;
        outputRect.h = mCurrentSize.y;
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

    // get current position in local or screen space
    inline Point GetPosition() const { return mCurrentPosition; }
    inline Point GetScreenPosition() const
    {
        Point screenPosition = LocalToScreen(mCurrentPosition);
        return screenPosition;
    }

    // get current origin point in local or screen space
    inline Point GetOrigin() const { return mCurrentOrigin; }
    inline Point GetOriginScreenPosition() const
    {
        Point screenPosition = LocalToScreen(mCurrentOrigin);
        return screenPosition;
    }

    // get current size
    inline Point GetSize() const { return mCurrentSize; }

    // set current visibility state
    void SetVisible(bool isVisible);

    // set enabled or disabled state
    void SetEnabled(bool isEnabled);

    // set current hovered state
    void SetHovered(bool isHovered);

    // test whether widget is visible, enabled or hovered
    bool IsVisible() const;
    bool IsEnabled() const;
    bool IsHovered() const { return mHovered; }

    // start drag
    // @return false if widget is ignoring drag
    bool StartDrag(const Point& screenPoint);
    bool IsBeingDragged() const;

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

protected:
    // copy properties
    GuiWidget(GuiWidget* copyWidget);

    GuiWidget* GetLastChild() const;

    void ParentPositionChanged(const Point& prevPosition);
    void ParentSizeChanged(const Point& prevSize, const Point& currSize);
    void ParentShownStateChanged();
    void ParentEnableStateChanged();

    void PositionChanged(const Point& prevPosition);
    void SizeChanged(const Point& prevSize);
    void ShownStateChanged();
    void EnableStateChanged();
    void HoveredStateChanged();

    void ComputeAbsoluteOrigin(Point& outputPoint) const;
    void ComputeAbsolutePosition(Point& outputPoint) const;
    void ComputeAbsoluteSize(Point& outputSize) const;

protected:
    // overridable
    virtual void HandleRender(GuiRenderer& renderContext);
    virtual void HandleUpdate(float deltaTime);
    virtual void HandleSizeChanged(const Point& prevSize);
    virtual void HandlePositionChanged(const Point& prevPosition);

    virtual void HandleShownStateChanged();
    virtual void HandleEnableStateChanged();
    virtual void HandleHoveredStateChanged();

    virtual bool HasAttribute(eGuiWidgetAttribute attribute) const;

    virtual GuiWidget* ConstructClone();

protected:
    GuiWidgetClass* mClass; // cannot be null

    GuiWidget* mParent = nullptr;
    GuiWidget* mFirstChild = nullptr;
    GuiWidget* mNextSibling = nullptr;
    GuiWidget* mPrevSibling = nullptr;

    // layout params
    GuiAnchorsStruct mAnchors;
    // origin
    GuiPositionComponent mOriginComponentX;
    GuiPositionComponent mOriginComponentY;
    // position
    GuiPositionComponent mPositionComponentX;
    GuiPositionComponent mPositionComponentY;
    // size
    GuiSizeComponent mSizeComponentW;
    GuiSizeComponent mSizeComponentH;    

    // current location and dimensions, local space
    Point mCurrentPosition;
    Point mCurrentSize;
    Point mCurrentOrigin; // local space

    glm::vec2 mScale;
    glm::mat4 mTransform; // current transformations matrix, screen space

    // state flags
    bool mSelfEnabled = true;
    bool mSelfVisible = true;

    bool mTransformInvalidated = false; // transformations matrix dirty
    bool mHovered = false;
};

// base widget class
extern GuiWidgetClass gBaseWidgetClass;