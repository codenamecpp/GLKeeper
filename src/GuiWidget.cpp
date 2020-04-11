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
    if (mAnchors.mLeft == anchors.mLeft && mAnchors.mTop == anchors.mTop && 
        mAnchors.mRight == anchors.mRight && mAnchors.mBottom == anchors.mBottom)
    {
        return;
    }

    mAnchors = anchors;
    InvalidateTransform();
}

void GuiWidget::SetOriginX(int posx, eGuiUnits units_x)
{
    Point origin(posx, mOriginComponentY.mValue);

    SetOrigin(origin, units_x, mOriginComponentY.mValueUnits);
}

void GuiWidget::SetOriginY(int posy, eGuiUnits units_y)
{
    Point origin(mOriginComponentX.mValue, posy);

    SetOrigin(origin, mOriginComponentX.mValueUnits, units_y);
}

void GuiWidget::SetOrigin(const Point& position, eGuiUnits units_x, eGuiUnits units_y)
{
    GuiPositionComponent componentx (units_x, position.x);
    GuiPositionComponent componenty (units_y, position.y);

    mOriginComponentX = componentx;
    mOriginComponentY = componenty;

    Point newOrigin;
    ComputeAbsoluteOrigin(newOrigin);

    if (newOrigin == mCurrentOrigin)
        return;

    mCurrentOrigin = newOrigin;
    InvalidateTransform();
}

void GuiWidget::SetPositionX(int posx, eGuiUnits units_x)
{
    Point position(posx, mPositionComponentY.mValue);

    SetPosition(position, units_x, mPositionComponentY.mValueUnits);
}

void GuiWidget::SetPositionY(int posy, eGuiUnits units_y)
{
    Point position(mPositionComponentX.mValue, posy);

    SetPosition(position, mPositionComponentX.mValueUnits, units_y);
}

void GuiWidget::SetPosition(const Point& position, eGuiUnits units_x, eGuiUnits units_y)
{
    GuiPositionComponent componentx (units_x, position.x);
    GuiPositionComponent componenty (units_y, position.y);

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

void GuiWidget::SetSizeW(int sizew, eGuiUnits units_w)
{
    Point newsize(sizew, mSizeComponentH.mValue);

    SetSize(newsize, units_w, mSizeComponentH.mValueUnits);
}

void GuiWidget::SetSizeH(int sizeh, eGuiUnits units_h)
{
    Point newsize(mSizeComponentW.mValue, sizeh);

    SetSize(newsize, mSizeComponentW.mValueUnits, units_h);
}

void GuiWidget::SetSize(const Point& size, eGuiUnits units_w, eGuiUnits units_h)
{
    Point correctSize = glm::max(size, 0); // sanity check

    GuiSizeComponent componentw (units_w, correctSize.x);
    GuiSizeComponent componenth (units_h, correctSize.y);

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

    if (mAnchors.mLeft && mAnchors.mRight)
    {
        if (mSizeComponentW.mValueUnits == eGuiUnits_Pixels)
        {
            newSize.x = mCurrentSize.x + deltax;
        }
        else
        {
            // ignore
        }
    }
    else if (mPositionComponentX.mValueUnits == eGuiUnits_Pixels)
    {
        if (mAnchors.mRight)
        {
            newPosition.x = mCurrentPosition.x + deltax;
        }
        else if (!mAnchors.mLeft)
        {
            newPosition.x = mCurrentPosition.x + deltax / 2;
        }
    }

    if (mAnchors.mTop && mAnchors.mBottom)
    {
        if (mSizeComponentH.mValueUnits == eGuiUnits_Pixels)
        {
            newSize.y = mCurrentSize.y + deltay;
        }
        else
        {
            // ignore
        }
    }
    else if (mPositionComponentY.mValueUnits == eGuiUnits_Pixels)
    {
        if (mAnchors.mBottom)
        {
            newPosition.y = mCurrentPosition.y + deltay;
        }
        else if (!mAnchors.mTop && mPositionComponentY.mValueUnits == eGuiUnits_Pixels)
        {
            newPosition.y = mCurrentPosition.y + deltay / 2;
        }
    }

    // compute relative position
    if (mPositionComponentX.mValueUnits == eGuiUnits_Percents ||
        mPositionComponentY.mValueUnits == eGuiUnits_Percents)
    {
        Point temporaryPoint;
        ComputeAbsolutePosition(temporaryPoint);
        if (mPositionComponentX.mValueUnits == eGuiUnits_Percents)
        {
            newPosition.x = temporaryPoint.x;
        }
        if (mPositionComponentY.mValueUnits == eGuiUnits_Percents)
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
    if (mSizeComponentW.mValueUnits == eGuiUnits_Percents ||
        mSizeComponentH.mValueUnits == eGuiUnits_Percents)
    {
        Point temporarySize;
        ComputeAbsoluteSize(temporarySize);
        if (mSizeComponentW.mValueUnits == eGuiUnits_Percents)
        {
            newSize.x = temporarySize.x;
        }
        if (mSizeComponentH.mValueUnits == eGuiUnits_Percents)
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
    if (mOriginComponentX.mValueUnits == eGuiUnits_Percents || 
        mOriginComponentY.mValueUnits == eGuiUnits_Percents)
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

void GuiWidget::ComputeAbsoluteOrigin(Point& outputPoint) const
{
    outputPoint.x = mOriginComponentX.mValue;
    outputPoint.y = mOriginComponentY.mValue;

    if (mOriginComponentX.mValueUnits == eGuiUnits_Percents)
    {
        float valuex = mCurrentSize.x * (outputPoint.x * 1.0f / 100.0f);
        outputPoint.x = (int) valuex;
    }

    if (mOriginComponentY.mValueUnits == eGuiUnits_Percents)
    {
        float valuey = mCurrentSize.y * (outputPoint.y * 1.0f / 100.0f);
        outputPoint.y = (int) valuey;
    }
}

void GuiWidget::ComputeAbsolutePosition(Point& outputPoint) const
{
    outputPoint.x = mPositionComponentX.mValue;
    outputPoint.y = mPositionComponentY.mValue;

    if (mPositionComponentX.mValueUnits == eGuiUnits_Percents)
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

    if (mPositionComponentY.mValueUnits == eGuiUnits_Percents)
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

    if (mSizeComponentW.mValueUnits == eGuiUnits_Percents)
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

    if (mSizeComponentH.mValueUnits == eGuiUnits_Percents)
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