#include "pch.h"
#include "GuiWidget.h"
#include "GuiManager.h"
#include "GuiRenderer.h"

// base widget class factory
static GuiWidgetFactory<GuiWidget> _BaseWidgetsFactory;

GuiWidgetClass gBaseWidgetClass("widget", &_BaseWidgetsFactory);

//////////////////////////////////////////////////////////////////////////

GuiWidget::GuiWidget(): GuiWidget(&gBaseWidgetClass)
{
}

GuiWidget::GuiWidget(GuiWidgetClass* widgetClass)
    : mClass(widgetClass)
    , mScale(1.0f)
    , mTransform(1.0f)
    , mOriginPoint()
    , mPosition()
    , mSize()
    , mOriginRelative()
    , mUserData()
{
    debug_assert(mClass);
}

GuiWidget::GuiWidget(GuiWidget* copyWidget)
    : mClass(copyWidget->mClass)
    , mName(copyWidget->mName)
    , mUserData()
    , mHorzAlignment(copyWidget->mHorzAlignment)
    , mVertAlignment(copyWidget->mVertAlignment)
    , mOriginMode(copyWidget->mOriginMode)
    , mOriginPoint(copyWidget->mOriginPoint)
    , mOriginRelative(copyWidget->mOriginRelative)
    , mAnchors(copyWidget->mAnchors)
    , mPosition(copyWidget->mPosition)
    , mSize(copyWidget->mSize)
    , mScale(copyWidget->mScale)
    , mTransform(copyWidget->mTransform)
    , mTransformInvalidated(copyWidget->mTransformInvalidated)
{
    debug_assert(mClass);
}

GuiWidget::~GuiWidget()
{
    if (mParent)
    {
        mParent->DetachChild(this);
    }
    DetachAndFreeChildren();
}

GuiWidget* GuiWidget::Clone()
{
    GuiWidget* selfClone = ConstructClone();
    return selfClone;
}

GuiWidget* GuiWidget::CloseDeep()
{
    GuiWidget* selfClone = Clone();

    for (GuiWidget* currChild = mFirstChild; currChild;
        currChild = currChild->mNextSibling)
    {
        GuiWidget* childClone = currChild->CloseDeep();
        selfClone->AttachChild(childClone);
    }
    return selfClone;
}

void GuiWidget::RenderFrame(GuiRenderer& renderContext)
{
    ComputeTransform();

    renderContext.SetCurrentTransform(&mTransform);
    HandleRenderSelf(renderContext);

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->RenderFrame(renderContext);
    }
}

void GuiWidget::UpdateFrame(float deltaTime)
{
    HandleUpdateSelf(deltaTime);

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->UpdateFrame(deltaTime);
    }
}

bool GuiWidget::AttachChild(GuiWidget* widget)
{
    if (widget == nullptr || widget == this || widget->mParent == this)
    {
        debug_assert(false);
        return false;
    }

    // detach from previous parent
    if (widget->mParent)
    {
        if (!widget->mParent->DetachChild(widget))
        {
            debug_assert(false);
        }
    }

    GuiWidget* tailWidget = mFirstChild;
    for (; tailWidget && tailWidget->mNextSibling; 
        tailWidget = tailWidget->mNextSibling)
    {
    }
    if (tailWidget == nullptr)
    {
        debug_assert(mFirstChild == nullptr);
        mFirstChild = widget;
    }
    else
    {
        tailWidget->mNextSibling = widget;
        widget->mPrevSibling = tailWidget;
    }
    widget->mParent = this;
    widget->InvalidateTransform();
    widget->SetPosition(widget->mPosition);

    return true;
}

bool GuiWidget::DetachChild(GuiWidget* widget)
{
    if (widget == nullptr || widget->mParent != this)
    {
        debug_assert(false);
        return false;
    }
    if (widget == mFirstChild)
    {
        mFirstChild = widget->mNextSibling;
    }
    if (widget->mPrevSibling)
    {
        widget->mPrevSibling->mNextSibling = widget->mNextSibling;
    }
    if (widget->mNextSibling)
    {
        widget->mNextSibling->mPrevSibling = widget->mPrevSibling;
    }
    widget->mParent = nullptr;
    return true;
}

void GuiWidget::DetachAndFreeChildren()
{
    while (mFirstChild)
    {
        GuiWidget* deleteWidget = mFirstChild;
        DetachChild(deleteWidget);
        SafeDelete(deleteWidget);
    }
}

void GuiWidget::SetAlignment(eGuiHorzAlignment horzAlignment, eGuiVertAlignment vertAlignment)
{
    mHorzAlignment = horzAlignment;
    mVertAlignment = vertAlignment;

    SetPosition(mPosition);
}

void GuiWidget::SetHorzAlignment(eGuiHorzAlignment horzAlignment)
{
    SetAlignment(horzAlignment, mVertAlignment);
}

void GuiWidget::SetVertAlignment(eGuiVertAlignment vertAlignment)
{
    SetAlignment(mHorzAlignment, vertAlignment);
}

void GuiWidget::SetAnchors(const GuiAnchorsStruct& anchors)
{
    if (mAnchors.mLeft == anchors.mLeft && mAnchors.mTop == anchors.mTop && 
        mAnchors.mRight == anchors.mRight && mAnchors.mBottom == anchors.mBottom)
    {
        return;
    }

    mAnchors = anchors;
    InvalidateTransform();
}

void GuiWidget::SetOriginMode(eGuiOriginMode originMode)
{
    if (mOriginMode == originMode)
        return;

    mOriginMode = originMode;
    if (mOriginMode == eGuiOrigin_Relative)
    {
        ComputeOriginPoint(mOriginPoint);
    }
    InvalidateTransform();
}

void GuiWidget::SetCurrentOriginPositionToCenter()
{
    if (mOriginMode == eGuiOrigin_Fixed)
    {
        Point2D localCenter (mSize.x / 2, mSize.y / 2);
        SetOriginPosition(localCenter);
    }

    if (mOriginMode == eGuiOrigin_Relative)
    {
        glm::vec2 originValue (0.5f, 0.5f);
        SetOriginRelativeValue(originValue);
    }
}

void GuiWidget::SetOriginPosition(const Point2D& originPoint)
{
    if (mOriginMode != eGuiOrigin_Fixed || mOriginPoint == originPoint)
        return;

    mOriginPoint = originPoint;
    InvalidateTransform();
}

void GuiWidget::SetOriginRelativeValue(const glm::vec2& value)
{
    if (mOriginMode != eGuiOrigin_Relative || mOriginRelative == value)
        return;

    mOriginRelative = value;
    ComputeOriginPoint(mOriginPoint);
    InvalidateTransform();
}

void GuiWidget::SetPosition(const Point2D& position)
{
    Point2D newPosition = position;
    if (mHorzAlignment != eGuiHorzAlignment_None)
    {
        newPosition.x = ComputeHorzAlignmentPos();
    }
    if (mVertAlignment != eGuiVertAlignment_None)
    {
        newPosition.y = ComputeVertAlignmentPos();
    }

    if (mPosition == newPosition)
        return;

    Point2D prevPosition = mPosition;
    mPosition = newPosition;
    InvalidateTransform();

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->ParentPositionChanged(prevPosition);
    }

    HandlePositionChanged(prevPosition);
}

void GuiWidget::SetSize(const Size2D& size)
{
    Size2D newSize = glm::max(size, 0);

    if (mSize == newSize)
        return;

    Size2D prevSize = mSize;
    mSize = newSize;

    // update origin point
    if (mOriginMode == eGuiOrigin_Relative)
    {
        ComputeOriginPoint(mOriginPoint);
        InvalidateTransform();
    }

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->ParentSizeChanged(prevSize, mSize);
    }

    HandleSizeChanged(prevSize);
}

Point2D GuiWidget::LocalToScreen(const Point2D& position) const
{
    GuiWidget* thisWidget = const_cast<GuiWidget*>(this);
    thisWidget->ComputeTransform();

    glm::vec4 localSpacePosition(position, 0.0f, 1.0f);
    glm::vec4 screenSpacePosition = mTransform * localSpacePosition;
    return Point2D(screenSpacePosition);
}

Point2D GuiWidget::ScreenToLocal(const Point2D& position) const
{
    GuiWidget* thisWidget = const_cast<GuiWidget*>(this);
    thisWidget->ComputeTransform();

    glm::mat4 inverseTransform = glm::inverse(mTransform);
    glm::vec4 screenSpacePosition(position, 0.0f, 1.0f);
    glm::vec4 localSpacePosition = inverseTransform * screenSpacePosition;
    return Point2D(localSpacePosition);
}

bool GuiWidget::IsScreenPointInsideRect(const Point2D& screenPosition) const
{
    Point2D localPoint = ScreenToLocal(screenPosition);
    Rect2D localRect;
    GetLocalRect(localRect);
    return localRect.PointWithin(localPoint);
}

void GuiWidget::ComputeTransform()
{
    if (!mTransformInvalidated)	
        return;
	
    if (mParent)
    {
        mParent->ComputeTransform();

        mTransform = glm::translate(-glm::vec3(mOriginPoint, 0.0f)) * glm::scale(glm::vec3(mScale, 0.0f)) * 
            glm::translate(glm::vec3(mPosition, 0.0f)) * mParent->mTransform;
    }
    else
    {
        mTransform = glm::translate(-glm::vec3(mOriginPoint, 0.0f)) * glm::scale(glm::vec3(mScale, 0.0f)) *
            glm::translate(glm::vec3(mPosition, 0.0f));
    }
    mTransformInvalidated = false;
}

void GuiWidget::InvalidateTransform()
{
    if (mTransformInvalidated)
        return;

    mTransformInvalidated = true;
    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->InvalidateTransform();
    }
}

void GuiWidget::ParentPositionChanged(const Point2D& prevPosition)
{
    InvalidateTransform();
}

void GuiWidget::ParentSizeChanged(const Size2D& prevSize, const Size2D& currSize)
{
    int deltax = currSize.x - prevSize.x;
    int deltay = currSize.y - prevSize.y;

    Point2D newPosition = mPosition;
    Size2D newSize = mSize;

    if (deltax)
    {
        if (mHorzAlignment == eGuiHorzAlignment_None) // test anchors
        {
            if (mAnchors.mLeft && mAnchors.mRight)
            {
                newSize.x = mSize.x + deltax / 2;
            }
            else if (mAnchors.mRight)
            {
                newPosition.x = mPosition.x + deltax;
            }
            else if (!mAnchors.mLeft)
            {
                newPosition.x = mPosition.x + deltax / 2;
            }
        }
    }

    if (deltay)
    {
        if (mVertAlignment == eGuiVertAlignment_None) // test anchors
        {
            if (mAnchors.mTop && mAnchors.mBottom)
            {
                newSize.y = mSize.y + deltay / 2;
            }
            else if (mAnchors.mBottom)
            {
                newPosition.y = mPosition.y + deltay;
            }
            else if (!mAnchors.mTop)
            {
                newPosition.y = mPosition.y + deltay / 2;
            }
        }
    }

    SetPosition(newPosition);
    SetSize(newSize);
}

int GuiWidget::ComputeHorzAlignmentPos() const
{
    switch (mHorzAlignment)
    {
        case eGuiHorzAlignment_Left: return 0;
        case eGuiHorzAlignment_Center:
            if (mParent)
            {
                return mParent->mSize.x / 2;
            }
        break;

        case eGuiHorzAlignment_Right:
            if (mParent)
            {
                return mParent->mSize.x;
            }
        break;
    }
    return mPosition.x;
}

int GuiWidget::ComputeVertAlignmentPos() const
{
    switch (mVertAlignment)
    {
        case eGuiVertAlignment_Top: return 0;
        case eGuiVertAlignment_Center:
            if (mParent)
            {
                return mParent->mSize.y / 2;
            }
        break;

        case eGuiVertAlignment_Bottom:
            if (mParent)
            {
                return mParent->mSize.y;
            }
        break;
    }
    return mPosition.y;
}

void GuiWidget::ComputeOriginPoint(Point2D& outputPoint) const
{
    outputPoint = mOriginPoint;
    if (mOriginMode == eGuiOrigin_Relative)
    {
        glm::vec2 originPoint ( mOriginRelative.x * mSize.x, mOriginRelative.y * mSize.y );
        outputPoint = originPoint;
    }
}

void GuiWidget::HandleRenderSelf(GuiRenderer& renderContext)
{
    // do nothing
}

void GuiWidget::HandleUpdateSelf(float deltaTime)
{
    // do nothing
}

void GuiWidget::HandleSizeChanged(const Size2D& prevSize)
{
    // do nothing
}

void GuiWidget::HandlePositionChanged(const Point2D& prevPosition)
{
    // do nothing
}

GuiWidget* GuiWidget::ConstructClone()
{
    GuiWidget* selfClone = new GuiWidget(this);
    return selfClone;
}
