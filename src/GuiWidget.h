#pragma once

#include "GuiDefs.h"

// basic gui element class
class GuiWidget: public cxx::handled_object<GuiWidget>
{
    friend class GuiManager;

public:
    std::string mName; // user-defined identifier for widget
    GuiUserData mUserData; // user-defined data

    Color32 mDebugColor = Color32_Gray;

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

    // convert from local to screen space and vice versa
    // @param position: Point
    Point LocalToScreen(const Point& position) const;
    Point ScreenToLocal(const Point& position) const;

    // force update transformations or invalidate current state
    void ComputeTransform();
    void InvalidateTransform();

    // clone widget with or without its chindren
    GuiWidget* Clone();
    GuiWidget* CloseDeep();

protected:
    // copy properties
    GuiWidget(GuiWidget* copyWidget);

    void ParentPositionChanged(const Point& prevPosition);
    void ParentSizeChanged(const Point& prevSize, const Point& currSize);
    void SelfPositionChanged(const Point& prevPosition);
    void SelfSizeChanged(const Point& prevSize);

    void ComputeAbsoluteOrigin(Point& outputPoint) const;
    void ComputeAbsolutePosition(Point& outputPoint) const;
    void ComputeAbsoluteSize(Point& outputSize) const;

protected:
    // overridable
    virtual void HandleRenderSelf(GuiRenderer& renderContext);
    virtual void HandleUpdateSelf(float deltaTime);
    virtual void HandleSizeChanged(const Point& prevSize);
    virtual void HandlePositionChanged(const Point& prevPosition);

    virtual bool HandleDragStart(const Point& screenPoint);
    virtual void HandleDragCancel();
    virtual void HandleDragDrop(const Point& screenPoint);
    virtual void HandleDrag(const Point& screenPoint);

    virtual GuiWidget* ConstructClone();

protected:
    GuiWidgetClass* mClass; // cannot be null

    GuiWidget* mParent = nullptr;
    GuiWidget* mFirstChild = nullptr;
    GuiWidget* mNextSibling = nullptr;
    GuiWidget* mPrevSibling = nullptr;

    // layout params

    GuiPositionComponent mOriginComponentX;
    GuiPositionComponent mOriginComponentY;

    GuiPositionComponent mPositionComponentX;
    GuiPositionComponent mPositionComponentY;

    GuiSizeComponent mSizeComponentW;
    GuiSizeComponent mSizeComponentH;

    GuiAnchorsStruct mAnchors;

    // current location and dimensions, local space
    Point mCurrentPosition;
    Point mCurrentSize;
    Point mCurrentOrigin; // local space

    glm::vec2 mScale;
    glm::mat4 mTransform; // current transformations matrix, screen space
    bool mTransformInvalidated = false; // transformations matrix dirty
};

// base widget class
extern GuiWidgetClass gBaseWidgetClass;