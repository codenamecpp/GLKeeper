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
    , mCurrentSize()
    , mCurrentPosition()
    , mCurrentOrigin()
    , mUserData()
{
    debug_assert(mClass);
}

GuiWidget::GuiWidget(GuiWidget* copyWidget)
    : mClass(copyWidget->mClass)
    , mName(copyWidget->mName)
    , mUserData()
    , mAnchors(copyWidget->mAnchors)
    , mScale(copyWidget->mScale)
    , mTransform(copyWidget->mTransform)
    , mTransformInvalidated(copyWidget->mTransformInvalidated)
    , mOriginComponentX(copyWidget->mOriginComponentX)
    , mOriginComponentY(copyWidget->mOriginComponentY)
    , mPositionComponentX(copyWidget->mPositionComponentX)
    , mPositionComponentY(copyWidget->mPositionComponentY)
    , mSizeComponentW(copyWidget->mSizeComponentW)
    , mSizeComponentH(copyWidget->mSizeComponentH)
    , mCurrentPosition(copyWidget->mCurrentPosition)
    , mCurrentSize(copyWidget->mCurrentSize)
    , mCurrentOrigin(copyWidget->mCurrentOrigin)
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
    widget->ParentSizeChanged(mCurrentSize, mCurrentSize); // force update layout

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

void GuiWidget::SetOriginX(int posx, eGuiAddressingMode xAddressingMode)
{
    Point origin(posx, mOriginComponentY.mValue);

    SetOrigin(origin, xAddressingMode, mOriginComponentY.mAddressingMode);
}

void GuiWidget::SetOriginY(int posy, eGuiAddressingMode yAddressingMode)
{
    Point origin(mOriginComponentX.mValue, posy);

    SetOrigin(origin, mOriginComponentX.mAddressingMode, yAddressingMode);
}

void GuiWidget::SetOrigin(const Point& position, eGuiAddressingMode xAddressingMode, eGuiAddressingMode yAddressingMode)
{
    GuiPositionComponent componentx (xAddressingMode, position.x);
    GuiPositionComponent componenty (yAddressingMode, position.y);

    mOriginComponentX = componentx;
    mOriginComponentY = componenty;

    Point newOrigin;
    ComputeAbsoluteOrigin(newOrigin);

    if (newOrigin == mCurrentOrigin)
        return;

    mCurrentOrigin = newOrigin;
    InvalidateTransform();
}

void GuiWidget::SetOriginToCenter()
{
    Point position = mCurrentOrigin;
    if (mOriginComponentX.mAddressingMode == eGuiAddressingMode_Absolute)
    {
        position.x = mCurrentSize.x / 2;
    }
    else
    {
        position.x = 50;
    }

    if (mOriginComponentY.mAddressingMode == eGuiAddressingMode_Absolute)
    {
        position.y = mCurrentSize.y / 2;
    }
    else
    {
        position.y = 50;
    }

    SetOrigin(position, mOriginComponentX.mAddressingMode, mOriginComponentY.mAddressingMode);
}

void GuiWidget::SetPositionX(int posx, eGuiAddressingMode xAddressingMode)
{
    Point position(posx, mPositionComponentY.mValue);

    SetPosition(position, xAddressingMode, mPositionComponentY.mAddressingMode);
}

void GuiWidget::SetPositionY(int posy, eGuiAddressingMode yAddressingMode)
{
    Point position(mPositionComponentX.mValue, posy);

    SetPosition(position, mPositionComponentX.mAddressingMode, yAddressingMode);
}

void GuiWidget::SetPosition(const Point& position, eGuiAddressingMode xAddressingMode, eGuiAddressingMode yAddressingMode)
{
    GuiPositionComponent componentx (xAddressingMode, position.x);
    GuiPositionComponent componenty (yAddressingMode, position.y);

    mPositionComponentX = componentx;
    mPositionComponentY = componenty;

    Point newPosition;
    ComputeAbsolutePosition(newPosition);

    if (newPosition == mCurrentPosition)
        return;

    Point prevPosition = mCurrentPosition;
    mCurrentPosition = newPosition;

    SelfPositionChanged(prevPosition);
}

void GuiWidget::SetSizeW(int sizew, eGuiAddressingMode wAddressingMode)
{
    Point newsize(sizew, mSizeComponentH.mValue);

    SetSize(newsize, wAddressingMode, mSizeComponentH.mAddressingMode);
}

void GuiWidget::SetSizeH(int sizeh, eGuiAddressingMode hAddressingMode)
{
    Point newsize(mSizeComponentW.mValue, sizeh);

    SetSize(newsize, mSizeComponentW.mAddressingMode, hAddressingMode);
}

void GuiWidget::SetSize(const Point& size, eGuiAddressingMode wAddressingMode, eGuiAddressingMode hAddressingMode)
{
    Point correctSize = glm::max(size, 0); // sanity check

    GuiSizeComponent componentw (wAddressingMode, correctSize.x);
    GuiSizeComponent componenth (hAddressingMode, correctSize.y);

    mSizeComponentW = componentw;
    mSizeComponentH = componenth;

    Point newSize;
    ComputeAbsoluteSize(newSize);

    if (newSize == mCurrentSize)
        return;

    Point prevSize = mCurrentSize;
    mCurrentSize = newSize;

    SelfSizeChanged(prevSize);
}

Point GuiWidget::LocalToScreen(const Point& position) const
{
    GuiWidget* thisWidget = const_cast<GuiWidget*>(this);
    thisWidget->ComputeTransform();

    glm::vec4 localSpacePosition(position, 0.0f, 1.0f);
    glm::vec4 screenSpacePosition = mTransform * localSpacePosition;
    return Point(screenSpacePosition);
}

Point GuiWidget::ScreenToLocal(const Point& position) const
{
    GuiWidget* thisWidget = const_cast<GuiWidget*>(this);
    thisWidget->ComputeTransform();

    glm::mat4 inverseTransform = glm::inverse(mTransform);
    glm::vec4 screenSpacePosition(position, 0.0f, 1.0f);
    glm::vec4 localSpacePosition = inverseTransform * screenSpacePosition;
    return Point(localSpacePosition);
}

bool GuiWidget::IsScreenPointInsideRect(const Point& screenPosition) const
{
    Point localPoint = ScreenToLocal(screenPosition);
    Rectangle localRect;
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

        mTransform = glm::translate(-glm::vec3(mCurrentOrigin, 0.0f)) * glm::scale(glm::vec3(mScale, 0.0f)) * 
            glm::translate(glm::vec3(mCurrentPosition, 0.0f)) * mParent->mTransform;
    }
    else
    {
        mTransform = glm::translate(-glm::vec3(mCurrentOrigin, 0.0f)) * glm::scale(glm::vec3(mScale, 0.0f)) *
            glm::translate(glm::vec3(mCurrentPosition, 0.0f));
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

void GuiWidget::ParentPositionChanged(const Point& prevPosition)
{
    InvalidateTransform();
}

void GuiWidget::ParentSizeChanged(const Point& prevSize, const Point& currSize)
{
    int deltax = currSize.x - prevSize.x;
    int deltay = currSize.y - prevSize.y;

    Point newPosition = mCurrentPosition;
    Point newSize = mCurrentSize;

    if (mPositionComponentX.mAddressingMode == eGuiAddressingMode_Absolute) // test anchors
    {
        if (mAnchors.mLeft && mAnchors.mRight)
        {
            if (mSizeComponentW.mAddressingMode == eGuiAddressingMode_Absolute)
            {
                newSize.x = mCurrentSize.x + deltax / 2;
            }
            else
            {
                // ignore
            }
        }
        else if (mAnchors.mRight)
        {
            newPosition.x = mCurrentPosition.x + deltax;
        }
        else if (!mAnchors.mLeft)
        {
            newPosition.x = mCurrentPosition.x + deltax / 2;
        }
    }

    if (mPositionComponentY.mAddressingMode == eGuiAddressingMode_Absolute) // test anchors
    {
        if (mAnchors.mTop && mAnchors.mBottom)
        {
            if (mSizeComponentH.mAddressingMode == eGuiAddressingMode_Absolute)
            {
                newSize.y = mCurrentSize.y + deltay / 2;
            }
            else
            {
                // ignore
            }
        }
        else if (mAnchors.mBottom)
        {
            newPosition.y = mCurrentPosition.y + deltay;
        }
        else if (!mAnchors.mTop)
        {
            newPosition.y = mCurrentPosition.y + deltay / 2;
        }
    }

    // compute relative position
    if (mPositionComponentX.mAddressingMode == eGuiAddressingMode_Relative ||
        mPositionComponentY.mAddressingMode == eGuiAddressingMode_Relative)
    {
        Point temporaryPoint;
        ComputeAbsolutePosition(temporaryPoint);
        if (mPositionComponentX.mAddressingMode == eGuiAddressingMode_Relative)
        {
            newPosition.x = temporaryPoint.x;
        }
        if (mPositionComponentY.mAddressingMode == eGuiAddressingMode_Relative)
        {
            newPosition.y = temporaryPoint.y;
        }
    }

    if (newPosition != mCurrentPosition)
    {
        Point prevPosition = mCurrentPosition;
        mCurrentPosition = newPosition;
        SelfPositionChanged(prevPosition);
    }

    // compute relative size
    if (mSizeComponentW.mAddressingMode == eGuiAddressingMode_Relative ||
        mSizeComponentH.mAddressingMode == eGuiAddressingMode_Relative)
    {
        Point temporarySize;
        ComputeAbsoluteSize(temporarySize);
        if (mSizeComponentW.mAddressingMode == eGuiAddressingMode_Relative)
        {
            newSize.x = temporarySize.x;
        }
        if (mSizeComponentH.mAddressingMode == eGuiAddressingMode_Relative)
        {
            newSize.y = temporarySize.y;
        }
    }

    Point correctSize = glm::max(newSize, 0); // sanity check
    if (correctSize != mCurrentSize)
    {
        Point prevSize = mCurrentSize;
        mCurrentSize = correctSize;
        SelfSizeChanged(prevSize);
    }
}

void GuiWidget::SelfPositionChanged(const Point& prevPosition)
{
    InvalidateTransform();

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->ParentPositionChanged(prevPosition);
    }

    HandlePositionChanged(prevPosition);
}

void GuiWidget::SelfSizeChanged(const Point& prevSize)
{
    // update origin point
    if (mOriginComponentX.mAddressingMode == eGuiAddressingMode_Relative || 
        mOriginComponentY.mAddressingMode == eGuiAddressingMode_Relative)
    {
        ComputeAbsoluteOrigin(mCurrentOrigin);
        InvalidateTransform();
    }

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->ParentSizeChanged(prevSize, mCurrentSize);
    }

    HandleSizeChanged(prevSize);
}

void GuiWidget::ComputeAbsoluteOrigin(Point& outputPoint) const
{
    outputPoint.x = mOriginComponentX.mValue;
    outputPoint.y = mOriginComponentY.mValue;

    if (mOriginComponentX.mAddressingMode == eGuiAddressingMode_Relative)
    {
        float valuex = mCurrentSize.x * (outputPoint.x * 1.0f / 100.0f);
        outputPoint.x = (int) valuex;
    }

    if (mOriginComponentY.mAddressingMode == eGuiAddressingMode_Relative)
    {
        float valuey = mCurrentSize.y * (outputPoint.y * 1.0f / 100.0f);
        outputPoint.y = (int) valuey;
    }
}

void GuiWidget::ComputeAbsolutePosition(Point& outputPoint) const
{
    outputPoint.x = mPositionComponentX.mValue;
    outputPoint.y = mPositionComponentY.mValue;

    if (mPositionComponentX.mAddressingMode == eGuiAddressingMode_Relative)
    {
        if (mParent)
        {
            float valuex = mParent->mCurrentSize.x * (outputPoint.x * 1.0f / 100.0f);
            outputPoint.x = (int) valuex;
        }
        else
        {
            outputPoint.x = 0;
        }
    }

    if (mPositionComponentY.mAddressingMode == eGuiAddressingMode_Relative)
    {
        if (mParent)
        {
            float valuey = mParent->mCurrentSize.y * (outputPoint.y * 1.0f / 100.0f);
            outputPoint.y = (int) valuey;
        }
        else
        {
            outputPoint.y = 0;
        }
    }
}

void GuiWidget::ComputeAbsoluteSize(Point& outputSize) const
{
    outputSize.x = mSizeComponentW.mValue;
    outputSize.y = mSizeComponentH.mValue;

    if (mSizeComponentW.mAddressingMode == eGuiAddressingMode_Relative)
    {
        if (mParent)
        {
            float valuew = mParent->mCurrentSize.x * (outputSize.x * 1.0f / 100.0f);
            outputSize.x = (int) valuew;
        }
        else
        {
            outputSize.x = 0;
        }
    }

    if (mSizeComponentH.mAddressingMode == eGuiAddressingMode_Relative)
    {
        if (mParent)
        {
            float valueh = mParent->mCurrentSize.y * (outputSize.y * 1.0f / 100.0f);
            outputSize.y = (int) valueh;
        }
        else
        {
            outputSize.y = 0;
        }
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

void GuiWidget::HandleSizeChanged(const Point& prevSize)
{
    // do nothing
}

void GuiWidget::HandlePositionChanged(const Point& prevPosition)
{
    // do nothing
}

bool GuiWidget::HandleDragStart(const Point& screenPoint)
{
    // do nothing
    return false;
}

void GuiWidget::HandleDragCancel()
{
    // do nothing
    debug_assert(false);
}

void GuiWidget::HandleDragDrop(const Point& screenPoint)
{
    // do nothing
    debug_assert(false);
}

void GuiWidget::HandleDrag(const Point& screenPoint)
{
    // do nothing
    debug_assert(false);
}

GuiWidget* GuiWidget::ConstructClone()
{
    GuiWidget* selfClone = new GuiWidget(this);
    return selfClone;
}
