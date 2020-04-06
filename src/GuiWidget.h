#pragma once

#include "GuiDefs.h"

// basic gui element class
class GuiWidget: public cxx::handled_object<GuiWidget>
{
    friend class GuiManager;

public:
    std::string mName; // user-defined identifier for widget
    GuiUserData mUserData; // user-defined data

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

    // setup vertical and horizontal alignment
    // alignment has precedence over anchors
    void SetHorzAlignment(eGuiHorzAlignment horzAlignment);
    void SetVertAlignment(eGuiVertAlignment vertAlignment);
    void SetAlignment(eGuiHorzAlignment horzAlignment, eGuiVertAlignment vertAlignment);

    // setup widget anchors
    void SetAnchors(const GuiAnchorsStruct& anchors);

    // set current origin mode
    void SetOriginMode(eGuiOriginMode originMode);
    void SetCurrentOriginPositionToCenter();

    // set current origin position in local or screen space
    void SetOriginPosition(const Point& position);
    void SetOriginScreenPosition(const Point& position)
    {
        Point pos = ScreenToLocal(position);
        SetOriginPosition(pos);
    }
    void SetOriginRelativeValue(const glm::vec2& value);

    // set current position in local or screen space
    void SetPosition(const Point& position);
    void SetScreenPosition(const Point& position)
    {
        Point pos = ScreenToLocal(position);
        SetPosition(pos);
    }

    // set current size
    void SetSize(const Point& size);

    // test whether screen space point is within widget rect
    // @param screenPosition: Test point
    bool IsScreenPointInsideRect(const Point& screenPosition) const;

    // get current position and size
    // @param outputRect: Output local rect
    void GetLocalRect(Rectangle& outputRect) const
    {
        outputRect.x = 0;
        outputRect.y = 0;
        outputRect.w = mSize.x;
        outputRect.h = mSize.y;
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
    inline Point GetPosition() const { return mPosition; }
    inline Point GetScreenPosition() const
    {
        Point screenPosition = LocalToScreen(mPosition);
        return screenPosition;
    }

    // get current origin point in local or screen space
    inline Point GetOriginPosition() const { return mOriginPoint; }
    inline Point GetOriginScreenPosition() const
    {
        Point screenPosition = LocalToScreen(mOriginPoint);
        return screenPosition;
    }

    // get current size
    inline Point GetSize() const { return mSize; }

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

    int ComputeHorzAlignmentPos() const;
    int ComputeVertAlignmentPos() const;

    void ComputeOriginPoint(Point& outputPoint) const;

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
    eGuiHorzAlignment mHorzAlignment = eGuiHorzAlignment_None;
    eGuiVertAlignment mVertAlignment = eGuiVertAlignment_None;
    eGuiOriginMode mOriginMode = eGuiOrigin_Fixed;
    Point mOriginPoint; // local space
    glm::vec2 mOriginRelative; // relative to size [0-1]
    GuiAnchorsStruct mAnchors;

    // current location and dimensions, local space
    Point mPosition;
    Point mSize;

    glm::vec2 mScale;
    glm::mat4 mTransform; // current transformations matrix, screen space
    bool mTransformInvalidated = false; // transformations matrix dirty
};

// base widget class
extern GuiWidgetClass gBaseWidgetClass;