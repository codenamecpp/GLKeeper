#include "pch.h"
#include "GuiWidget.h"
#include "GuiManager.h"
#include "GuiRenderer.h"

// base widget class factory
static GuiWidgetFactory<GuiWidget> _BaseWidgetsFactory;

GuiWidgetClass gBaseWidgetClass("widget", &_BaseWidgetsFactory, nullptr);

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
    , mOriginX(copyWidget->mOriginX)
    , mOriginY(copyWidget->mOriginY)
    , mPositionX(copyWidget->mPositionX)
    , mPositionY(copyWidget->mPositionY)
    , mSizeW(copyWidget->mSizeW)
    , mSizeH(copyWidget->mSizeH)
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
    gGuiManager.HandleWidgetDestroy(this);

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
        Rectangle rcClip = GetLocalRect();
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
    widget->SetAnchorPositions();
    HandleChildAttached(widget);
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

    HandleChildDetached(widget);
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

    // pick child widgets only if not pick target
    if (!HasAttribute(eGuiWidgetAttribute_PickTarget))
    {
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

GuiWidget* GuiWidget::GetChild(const std::string& name) const
{
    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        if (currChild->mName == name)
            return currChild;
    }
    return nullptr;
}

GuiWidget* GuiWidget::GetChild(int index) const
{
    debug_assert(index >= 0);
    int currIndex = 0;
    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        if (currIndex == index)
            return currChild;

        ++currIndex;
    }
    return nullptr;
}

void GuiWidget::SetAnchors(const GuiAnchors& anchors)
{
    if (mAnchors.mL == anchors.mL && mAnchors.mT == anchors.mT && 
        mAnchors.mR == anchors.mR && mAnchors.mB == anchors.mB)
    {
        return;
    }

    mAnchors = anchors;
    SetAnchorPositions();

    InvalidateTransform();
}

void GuiWidget::SetOriginX(int posx, eGuiUnits units_x)
{
    Point origin(posx, mCurrentOrigin.y);
    if (mOriginY.mUnits == eGuiUnits_Percents)
    {
        origin.y = mOriginY.mValue;
    }
    SetOrigin(origin, units_x, mOriginY.mUnits);
}

void GuiWidget::SetOriginY(int posy, eGuiUnits units_y)
{
    Point origin(mCurrentOrigin.x, posy);
    if (mOriginX.mUnits == eGuiUnits_Percents)
    {
        origin.x = mOriginX.mValue;
    }
    SetOrigin(origin, mOriginX.mUnits, units_y);
}

void GuiWidget::SetOrigin(const Point& position, eGuiUnits units_x, eGuiUnits units_y)
{
    mOriginX = GuiPositionValue(units_x, position.x);
    mOriginY = GuiPositionValue(units_y, position.y);

    Point computePosition = GetOriginPixels();
    if (computePosition != mCurrentOrigin)
    {
        Point prevOrigin = mCurrentOrigin;
        mCurrentOrigin = computePosition;
        InvalidateTransform();
    }
}

void GuiWidget::SetPositionX(int posx, eGuiUnits units_x)
{
    Point position(posx, mCurrentPosition.y);
    if (mPositionY.mUnits == eGuiUnits_Percents)
    {
        position.y = mPositionY.mValue;
    }
    SetPosition(position, units_x, mPositionY.mUnits);
}

void GuiWidget::SetPositionY(int posy, eGuiUnits units_y)
{
    Point position(mCurrentPosition.x, posy);
    if (mPositionX.mUnits == eGuiUnits_Percents)
    {
        position.x = mPositionX.mValue;
    }
    SetPosition(position, mPositionX.mUnits, units_y);
}

void GuiWidget::SetPosition(const Point& position, eGuiUnits units_x, eGuiUnits units_y)
{
    mPositionX = GuiPositionValue (units_x, position.x);
    mPositionY = GuiPositionValue (units_y, position.y);

    Point computePosition = GetPositionPixels();
    if (computePosition != mCurrentPosition)
    {
        Point prevPosition = mCurrentPosition;
        mCurrentPosition = computePosition;
        PositionChanged(prevPosition);
    }
}

void GuiWidget::SetSizeW(int sizew, eGuiUnits units_w)
{
    Point newsize(sizew, mCurrentSize.y);
    if (mSizeH.mUnits == eGuiUnits_Percents)
    {
        newsize.y = mSizeH.mValue;
    }
    SetSize(newsize, units_w, mSizeH.mUnits);
}

void GuiWidget::SetSizeH(int sizeh, eGuiUnits units_h)
{
    Point newsize(mCurrentSize.x, sizeh);
    if (mSizeW.mUnits == eGuiUnits_Percents)
    {
        newsize.x = mSizeW.mValue;
    }
    SetSize(newsize, mSizeW.mUnits, units_h);
}

void GuiWidget::SetSize(const Point& size, eGuiUnits units_w, eGuiUnits units_h)
{
    Point correctSize = glm::max(size, 0); // sanity check

    mSizeW = GuiSizeValue (units_w, correctSize.x);
    mSizeH = GuiSizeValue (units_h, correctSize.y);

    Point computeSize = GetSizePixels();
    if (computeSize != mCurrentSize)
    {
        Point prevSize = mCurrentSize;
        mCurrentSize = computeSize;
        SizeChanged(prevSize);
    }
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

    gGuiManager.SetDragHandler(this, screenPoint);
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
    Rectangle localRect = GetLocalRect();
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

    if (mAnchors.mL && mAnchors.mR)
    {
        if (mSizeW.mUnits == eGuiUnits_Pixels)
        {
            newSize.x = mCurrentSize.x + deltax;
        }
        else
        {
            // ignore
        }
    }
    else if (mPositionX.mUnits == eGuiUnits_Pixels)
    {
        if (mAnchors.mR)
        {
            newPosition.x = mCurrentPosition.x + deltax;
        }
        else if (!mAnchors.mL)
        {
            newPosition.x = mCurrentPosition.x + deltax / 2;
        }
    }

    if (mAnchors.mT && mAnchors.mB)
    {
        if (mSizeH.mUnits == eGuiUnits_Pixels)
        {
            newSize.y = mCurrentSize.y + deltay;
        }
        else
        {
            // ignore
        }
    }
    else if (mPositionY.mUnits == eGuiUnits_Pixels)
    {
        if (mAnchors.mB)
        {
            newPosition.y = mCurrentPosition.y + deltay;
        }
        else if (!mAnchors.mT && mPositionY.mUnits == eGuiUnits_Pixels)
        {
            newPosition.y = mCurrentPosition.y + deltay / 2;
        }
    }

    // compute relative position
    if (mPositionX.mUnits == eGuiUnits_Percents ||
        mPositionY.mUnits == eGuiUnits_Percents)
    {
        Point temporaryPoint = GetPositionPixels();
        if (mPositionX.mUnits == eGuiUnits_Percents)
        {
            newPosition.x = temporaryPoint.x;
        }
        if (mPositionY.mUnits == eGuiUnits_Percents)
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
    if (mSizeW.mUnits == eGuiUnits_Percents ||
        mSizeH.mUnits == eGuiUnits_Percents)
    {
        Point temporarySize = GetSizePixels();
        if (mSizeW.mUnits == eGuiUnits_Percents)
        {
            newSize.x = temporarySize.x;
        }
        if (mSizeH.mUnits == eGuiUnits_Percents)
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
    if (!mSelfVisible)
        return;

    ShownStateChanged();
}

void GuiWidget::ParentEnableStateChanged()
{
    if (!mSelfEnabled)
        return;

    EnableStateChanged();
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
    if (mOriginX.mUnits == eGuiUnits_Percents || 
        mOriginY.mUnits == eGuiUnits_Percents)
    {
        mCurrentOrigin = GetOriginPixels();
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
            gGuiManager.SetDragHandler(nullptr, Point());
        }

        if (IsHovered()) // force cancel hover
        {
            mHovered = false;
            HoveredStateChanged();
        }
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
            gGuiManager.SetDragHandler(nullptr, Point()); 
        }

        if (IsHovered()) // force cancel hover
        {
            mHovered = false;
            HoveredStateChanged();
        }
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
    if (IsHovered())
    {
        GuiEvent ev (this, eGuiEvent_MouseEnter);
        gGuiManager.PostGuiEvent(&ev);

    }
    else
    {
        GuiEvent ev (this, eGuiEvent_MouseLeave);
        gGuiManager.PostGuiEvent(&ev);
    }

    HandleHoveredStateChanged();
}

Point GuiWidget::GetOriginPixels() const
{
    Point outputPoint(mOriginX.mValue, mOriginY.mValue);

    if (mOriginX.mUnits == eGuiUnits_Percents)
    {
        float valuex = mCurrentSize.x * (outputPoint.x * 1.0f / 100.0f);
        outputPoint.x = (int) valuex;
    }

    if (mOriginY.mUnits == eGuiUnits_Percents)
    {
        float valuey = mCurrentSize.y * (outputPoint.y * 1.0f / 100.0f);
        outputPoint.y = (int) valuey;
    }

    return outputPoint;
}

Point GuiWidget::GetPositionPixels() const
{
    Point outputPoint(mPositionX.mValue, mPositionY.mValue);

    if (mParent == nullptr ||
        (mPositionX.mUnits == eGuiUnits_Pixels && mPositionY.mUnits == eGuiUnits_Pixels))
    {
        return outputPoint;
    }

    if (mPositionX.mUnits == eGuiUnits_Percents)
    {
        float valuex = mParent->mCurrentSize.x * (outputPoint.x * 1.0f / 100.0f);
        outputPoint.x = (int) valuex;
    }

    if (mPositionY.mUnits == eGuiUnits_Percents)
    {
        float valuey = mParent->mCurrentSize.y * (outputPoint.y * 1.0f / 100.0f);
        outputPoint.y = (int) valuey;
    }

    return outputPoint;
}

Point GuiWidget::GetSizePixels() const
{
    Point outputSize(mSizeW.mValue, mSizeH.mValue);

    if (mParent == nullptr ||
        (mSizeW.mUnits == eGuiUnits_Pixels && mSizeH.mUnits == eGuiUnits_Pixels))
    {
        return outputSize;
    }

    if (mSizeW.mUnits == eGuiUnits_Percents)
    {
        float valuew = mParent->mCurrentSize.x * (outputSize.x * 1.0f / 100.0f);
        outputSize.x = (int) valuew;
    }

    if (mSizeH.mUnits == eGuiUnits_Percents)
    {
        float valueh = mParent->mCurrentSize.y * (outputSize.y * 1.0f / 100.0f);
        outputSize.y = (int) valueh;
    }

    return outputSize;
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

void GuiWidget::SetAnchorPositions()
{

}