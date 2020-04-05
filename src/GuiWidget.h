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
    void SetAnchors(const GuiAnchors& anchors);

    // set current origin mode
    void SetOriginMode(eGuiOriginMode originMode);

    // set current origin position in local or screen space
    void SetOriginPosition(const Point2D& position);
    void SetOriginScreenPosition(const Point2D& position)
    {
        Point2D pos = ScreenToLocal(position);
        SetOriginPosition(pos);
    }
    void SetOriginRelativeValue(const glm::vec2& value);

    // set current position in local or screen space
    void SetPosition(const Point2D& position);
    void SetScreenPosition(const Point2D& position)
    {
        Point2D pos = ScreenToLocal(position);
        SetPosition(pos);
    }

    // set current size
    void SetSize(const Size2D& size);

    // get current position and size
    // @param outputRect: Output local rect
    void GetLocalRect(Rect2D& outputRect) const
    {
        outputRect.mX = 0;
        outputRect.mY = 0;
        outputRect.mSizeX = mSize.x;
        outputRect.mSizeY = mSize.y;
    }

    bool AttachChild(GuiWidget* widget);
    bool DetachChild(GuiWidget* widget);

    // hierarchy inspection
    GuiWidget* GetParent() const { return mParent; }
    GuiWidget* GetChild() const { return mFirstChild; }
    GuiWidget* NextSibling() const { return mNextSibling; }
    GuiWidget* PrevSibling() const { return mPrevSibling; }

    // convert from local to screen space and vice versa
    // @param position: Point
    Point2D LocalToScreen(const Point2D& position) const;
    Point2D ScreenToLocal(const Point2D& position) const;

    // force update transformations or invalidate current state
    void ComputeTransform();
    void InvalidateTransform();

protected:
    void ParentPositionChanged(const Point2D& prevPosition);
    void ParentSizeChanged(const Size2D& prevSize, const Size2D& currSize);

    int ComputeHorzAlignmentPos() const;
    int ComputeVertAlignmentPos() const;

    void ComputeOriginPoint(Point2D& outputPoint) const;

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
    Point2D mOriginPoint; // local space
    glm::vec2 mOriginRelative; // relative to size [0-1]
    GuiAnchors mAnchors;

    // current location and dimensions, local space
    Point2D mPosition;
    Size2D mSize;

    glm::vec2 mScale;
    glm::mat4 mTransform; // current transformations matrix, screen space
    bool mTransformInvalidated = false; // transformations matrix dirty
};

// base widget class
extern GuiWidgetClass gBaseWidgetClass;