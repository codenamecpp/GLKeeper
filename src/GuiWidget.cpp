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
    , mSelfVisible(copyWidget->mSelfVisible)
    , mSelfEnabled(copyWidget->mSelfEnabled)
    , mClipChildren(copyWidget->mClipChildren)
{
    debug_assert(mClass);
}

GuiWidget* GuiWidget::GetLastChild() const
{
    for (GuiWidget* currWidget = mFirstChild; currWidget; 
        currWidget = currWidget->mNextSibling)
    {
        if (currWidget->mNextSibling == nullptr)
            return currWidget;
    }
    return mFirstChild;
}

GuiWidget::~GuiWidget()
{
    if (IsBeingDragged())
    {
        gGuiManager.ClearDrag(); // clear silently
    }

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

GuiWidget* GuiWidget::CloneDeep()
{
    GuiWidget* selfClone = Clone();

    for (GuiWidget* currChild = mFirstChild; currChild;
        currChild = currChild->mNextSibling)
    {
        GuiWidget* childClone = currChild->CloneDeep();
        selfClone->AttachChild(childClone);
    }
    return selfClone;
}

void GuiWidget::RenderFrame(GuiRenderer& renderContext)
{
    ComputeTransform();

    if (!IsVisible())
        return;

    renderContext.SetCurrentTransform(&mTransform);

    bool isClipChildren = mClipChildren;
    if (isClipChildren)
    {
        Rectangle rcClip;
        GetLocalRect(rcClip);
        if (!renderContext.EnterChildClipArea(rcClip))
            return;
    }

    HandleRender(renderContext);

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->RenderFrame(renderContext);
    }

    if (isClipChildren)
    {
        renderContext.LeaveChildClipArea();
    }
}

void GuiWidget::UpdateFrame(float deltaTime)
{
    HandleUpdate(deltaTime);

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->UpdateFrame(deltaTime);
    }
}

void GuiWidget::ProcessEvent(MouseButtonInputEvent& inputEvent)
{
    if (!IsVisible() || !HasAttribute(eGuiWidgetAttribute_Interactive))
        return;

    inputEvent.SetConsumed(true);
    if (!IsEnabled())
        return;

    HandleInputEvent(inputEvent);
}

void GuiWidget::ProcessEvent(MouseMovedInputEvent& inputEvent)
{
    if (!IsVisible() || !HasAttribute(eGuiWidgetAttribute_Interactive))
        return;

    inputEvent.SetConsumed(true);
    if (!IsEnabled())
        return;

    HandleInputEvent(inputEvent);
}

void GuiWidget::ProcessEvent(MouseScrollInputEvent& inputEvent)
{
    if (!IsVisible() || !HasAttribute(eGuiWidgetAttribute_Interactive))
        return;

    inputEvent.SetConsumed(true);
    if (!IsEnabled())
        return;

    HandleInputEvent(inputEvent);
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
    for (; tailWidget && tailWidget->mNextSibling; tailWidget = tailWidget->mNextSibling)
    {
        // iterate
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

GuiWidget* GuiWidget::PickWidget(const Point& screenPosition)
{
    GuiWidget* resultWidget = nullptr;

    if (!IsVisible())
        return nullptr;

    // process in reversed oreder
    for (GuiWidget* currChild = GetLastChild(); currChild;
        currChild = currChild->mPrevSibling)
    {
        // is point within widget and visible
        if (!currChild->IsVisible() || !currChild->IsScreenPointInsideRect(screenPosition))
            continue;

        GuiWidget* currPicked = currChild->PickWidget(screenPosition);
        if (currPicked)
        {
            resultWidget = currPicked;
            break;
        }
    }

    if (resultWidget == nullptr)
    {
        if (HasAttribute(eGuiWidgetAttribute_Interactive))
        {
            resultWidget = this;
        }
    }

    return resultWidget;
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

    PositionChanged(prevPosition);
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

    SizeChanged(prevSize);
}

bool GuiWidget::IsVisible() const
{
    if (mParent)
        return mSelfVisible && mParent->IsVisible();

    return mSelfVisible;
}

bool GuiWidget::IsEnabled() const
{
    if (mParent)
        return mSelfEnabled && mParent->IsEnabled();

    return mSelfEnabled;
}

void GuiWidget::SetHovered(bool isHovered)
{
    if (!IsEnabled() || !HasAttribute(eGuiWidgetAttribute_Interactive))
        return;

    if (mHovered == isHovered)
        return;

    mHovered = isHovered;
    HoveredStateChanged();
}

void GuiWidget::SetClipChildren(bool isEnabled)
{
    mClipChildren = isEnabled;
}

void GuiWidget::SetVisible(bool isVisible)
{
    if (mSelfVisible == isVisible)
        return;

    mSelfVisible = isVisible;
    if (mParent) // check inherited state
    {
        if (!mParent->IsVisible())
            return;
    }
    ShownStateChanged();
}

void GuiWidget::SetEnabled(bool isEnabled)
{
    if (mSelfEnabled == isEnabled)
        return;

    mSelfEnabled = isEnabled;
    if (mParent) // check inherited state
    {
        if (!mParent->IsEnabled())
            return;
    }
    EnableStateChanged();
}

bool GuiWidget::StartDrag(const Point& screenPoint)
{
    if (!IsEnabled() || !IsVisible() || !HasAttribute(eGuiWidgetAttribute_DragDrop))
        return false;

    gGuiManager.StartDrag(this, screenPoint);
    return true;
}

bool GuiWidget::IsBeingDragged() const
{
    return gGuiManager.mCurrentDragHandler == this;
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

        mTransform = glm::translate(-glm::vec3(mCurrentOrigin, 0.0f)) *
            glm::translate(glm::vec3(mCurrentPosition, 0.0f)) * mParent->mTransform;
    }
    else
    {
        mTransform = glm::translate(-glm::vec3(mCurrentOrigin, 0.0f)) *
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
        PositionChanged(prevPosition);
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
        SizeChanged(prevSize);
    }
}

void GuiWidget::ParentShownStateChanged()
{
    // do nothing
}

void GuiWidget::ParentEnableStateChanged()
{
    // do nothing
}

void GuiWidget::PositionChanged(const Point& prevPosition)
{
    InvalidateTransform();

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->ParentPositionChanged(prevPosition);
    }

    HandlePositionChanged(prevPosition);
}

void GuiWidget::SizeChanged(const Point& prevSize)
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

void GuiWidget::ShownStateChanged()
{
    if (!IsVisible()) 
    {
        if (IsBeingDragged()) // force cancel drag
        {
            gGuiManager.CancelDrag();
        }

        // reset hovered state
        mHovered = false;
    }

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->ParentShownStateChanged();
    }

    HandleShownStateChanged();
}

void GuiWidget::EnableStateChanged()
{
    if (!IsEnabled())
    {
        if (IsBeingDragged()) // force cancel drag
        {
            gGuiManager.CancelDrag(); 
        }

        // reset hovered state
        mHovered = false;
    }

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->ParentEnableStateChanged();
    }

    HandleEnableStateChanged();
}

void GuiWidget::HoveredStateChanged()
{
    HandleHoveredStateChanged();
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

void GuiWidget::HandleRender(GuiRenderer& renderContext)
{
    // do nothing
}

void GuiWidget::HandleUpdate(float deltaTime)
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

void GuiWidget::HandleShownStateChanged()
{
    // do nothing
}

void GuiWidget::HandleEnableStateChanged()
{
    // do nothing
}

void GuiWidget::HandleHoveredStateChanged()
{
    // do nothing
}

void GuiWidget::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    // do nothing
}

void GuiWidget::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    // do nothing
}

void GuiWidget::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    // do nothing
}

bool GuiWidget::HasAttribute(eGuiWidgetAttribute attribute) const
{
    switch (attribute)
    {
        case eGuiWidgetAttribute_Interactive: return false;
        case eGuiWidgetAttribute_DragDrop: return false;
    }
    return false;
}

GuiWidget* GuiWidget::ConstructClone()
{
    GuiWidget* selfClone = new GuiWidget(this);
    return selfClone;
}
