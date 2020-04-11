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
    , mOriginPercents()
    , mOrigin()
    , mPositionPercents()
    , mPosition()
    , mSizePercents()
    , mSize()
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
    , mOriginUnitsX(copyWidget->mOriginUnitsX)
    , mOriginUnitsY(copyWidget->mOriginUnitsY)
    , mOriginPercents(copyWidget->mOriginPercents)
    , mOrigin(copyWidget->mOrigin)
    , mPositionUnitsX(copyWidget->mPositionUnitsX)
    , mPositionUnitsY(copyWidget->mPositionUnitsY)
    , mPositionPercents(copyWidget->mPositionPercents)
    , mPosition(copyWidget->mPosition)
    , mSizeUnitsW(copyWidget->mSizeUnitsW)
    , mSizeUnitsH(copyWidget->mSizeUnitsH)
    , mSizePercents(copyWidget->mSizePercents)
    , mSize(copyWidget->mSize)
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
    widget->ParentSizeChanged(mSize, mSize); // force update layout
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
    Point origin(posx, (mOriginUnitsY == eGuiUnits_Percents) ? mOriginPercents.y : mOrigin.y);
    SetOrigin(origin, units_x, mOriginUnitsY);
}

void GuiWidget::SetOriginY(int posy, eGuiUnits units_y)
{
    Point origin((mOriginUnitsX == eGuiUnits_Percents) ? mOriginPercents.x : mOrigin.x, posy);
    SetOrigin(origin, mOriginUnitsX, units_y);
}

void GuiWidget::SetOrigin(const Point& position, eGuiUnits units_x, eGuiUnits units_y)
{
    Point prevOrigin = mOrigin;

    mOriginUnitsX = units_x;
    mOriginUnitsY = units_y;

    (mOriginUnitsX == eGuiUnits_Percents ? mOriginPercents.x : mOrigin.x) = position.x;
    (mOriginUnitsY == eGuiUnits_Percents ? mOriginPercents.y : mOrigin.y) = position.y;

    mOrigin = ComputeOriginPixels();
    if (mOrigin != prevOrigin)
    {
        InvalidateTransform();
    }
}

void GuiWidget::SetPositionX(int posx, eGuiUnits units_x)
{
    Point position(posx, (mPositionUnitsY == eGuiUnits_Percents) ? mPositionPercents.y : mPosition.y);
    SetPosition(position, units_x, mPositionUnitsY);
}

void GuiWidget::SetPositionY(int posy, eGuiUnits units_y)
{
    Point position((mPositionUnitsX == eGuiUnits_Percents ? mPositionPercents.x : mPosition.x), posy);
    SetPosition(position, mPositionUnitsX, units_y);
}

void GuiWidget::SetPosition(const Point& position, eGuiUnits units_x, eGuiUnits units_y)
{
    Point prevPosition = mPosition;

    mPositionUnitsX = units_x;
    mPositionUnitsY = units_y;

    (mPositionUnitsX == eGuiUnits_Percents ? mPositionPercents.x : mPosition.x) = position.x;
    (mPositionUnitsY == eGuiUnits_Percents ? mPositionPercents.y : mPosition.y) = position.y;

    mPosition = ComputePositionPixels();
    if (mPosition != prevPosition)
    {
        PositionChanged(prevPosition);
    }
}

void GuiWidget::SetSizeW(int sizew, eGuiUnits units_w)
{
    Point newsize(sizew, (mSizeUnitsH == eGuiUnits_Percents) ? mSizePercents.y : mSize.y);
    SetSize(newsize, units_w, mSizeUnitsH);
}

void GuiWidget::SetSizeH(int sizeh, eGuiUnits units_h)
{
    Point newsize((mSizeUnitsW == eGuiUnits_Percents) ? mSizePercents.x : mSize.x, sizeh);
    SetSize(newsize, mSizeUnitsW, units_h);
}

void GuiWidget::SetSize(const Point& size, eGuiUnits units_w, eGuiUnits units_h)
{
    Point correctSize = glm::max(size, 0); // sanity check
    Point prevSize = mSize;

    mSizeUnitsW = units_w;
    mSizeUnitsH = units_h;

    (mSizeUnitsW == eGuiUnits_Percents ? mSizePercents.x : mSize.x) = correctSize.x;
    (mSizeUnitsH == eGuiUnits_Percents ? mSizePercents.y : mSize.y) = correctSize.y;

    mSize = ComputeSizePixels();
    if (mSize != prevSize)
    {
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

        mTransform = glm::translate(-glm::vec3(mOrigin, 0.0f)) *
            glm::translate(glm::vec3(mPosition, 0.0f)) * mParent->mTransform;
    }
    else
    {
        mTransform = glm::translate(-glm::vec3(mOrigin, 0.0f)) *
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

void GuiWidget::ParentPositionChanged(const Point& prevPosition)
{
    InvalidateTransform();
}

void GuiWidget::ParentSizeChanged(const Point& prevSize, const Point& currSize)
{
    int deltax = currSize.x - prevSize.x;
    int deltay = currSize.y - prevSize.y;

    Point newPosition = mPosition;
    Point newSize = mSize;

    if (mAnchors.mL && mAnchors.mR)
    {
        if (mSizeUnitsW == eGuiUnits_Pixels)
        {
            newSize.x = mSize.x + deltax;
        }
        else
        {
            // ignore
        }
    }
    else if (mPositionUnitsX == eGuiUnits_Pixels)
    {
        if (mAnchors.mR)
        {
            newPosition.x = mPosition.x + deltax;
        }
        else if (!mAnchors.mL)
        {
            newPosition.x = mPosition.x + deltax / 2;
        }
    }

    if (mAnchors.mT && mAnchors.mB)
    {
        if (mSizeUnitsH == eGuiUnits_Pixels)
        {
            newSize.y = mSize.y + deltay;
        }
        else
        {
            // ignore
        }
    }
    else if (mPositionUnitsY == eGuiUnits_Pixels)
    {
        if (mAnchors.mB)
        {
            newPosition.y = mPosition.y + deltay;
        }
        else if (!mAnchors.mT)
        {
            newPosition.y = mPosition.y + deltay / 2;
        }
    }

    // compute relative position
    if (mPositionUnitsX == eGuiUnits_Percents || mPositionUnitsY == eGuiUnits_Percents)
    {
        Point temporaryPoint = ComputePositionPixels();
        if (mPositionUnitsX == eGuiUnits_Percents)
        {
            newPosition.x = temporaryPoint.x;
        }
        if (mPositionUnitsY == eGuiUnits_Percents)
        {
            newPosition.y = temporaryPoint.y;
        }
    }

    if (newPosition != mPosition)
    {
        Point prevPosition = mPosition;
        mPosition = newPosition;
        PositionChanged(prevPosition);
    }

    // compute relative size
    if (mSizeUnitsW == eGuiUnits_Percents || mSizeUnitsH == eGuiUnits_Percents)
    {
        Point temporarySize = ComputeSizePixels();
        if (mSizeUnitsW == eGuiUnits_Percents)
        {
            newSize.x = temporarySize.x;
        }
        if (mSizeUnitsH == eGuiUnits_Percents)
        {
            newSize.y = temporarySize.y;
        }
    }

    Point correctSize = glm::max(newSize, 0); // sanity check
    if (correctSize != mSize)
    {
        Point prevSize = mSize;
        mSize = correctSize;
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
    if (mOriginUnitsX == eGuiUnits_Percents || mOriginUnitsY == eGuiUnits_Percents)
    {
        mOrigin = ComputeOriginPixels();
        InvalidateTransform();
    }

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->ParentSizeChanged(prevSize, mSize);
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

Point GuiWidget::ComputeOriginPixels() const
{
    Point outputPoint = mOrigin;

    if (mOriginUnitsX == eGuiUnits_Percents)
    {
        float valuex = mSize.x * (mOriginPercents.x * 1.0f / 100.0f);
        outputPoint.x = (int) valuex;
    }

    if (mOriginUnitsY == eGuiUnits_Percents)
    {
        float valuey = mSize.y * (mOriginPercents.y * 1.0f / 100.0f);
        outputPoint.y = (int) valuey;
    }

    return outputPoint;
}

Point GuiWidget::ComputePositionPixels() const
{
    Point outputPoint = mPosition;

    if (mParent == nullptr ||
        (mPositionUnitsX == eGuiUnits_Pixels && mPositionUnitsY == eGuiUnits_Pixels))
    {
        return outputPoint;
    }

    if (mPositionUnitsX == eGuiUnits_Percents)
    {
        float valuex = mParent->mSize.x * (mPositionPercents.x * 1.0f / 100.0f);
        outputPoint.x = (int) valuex;
    }

    if (mPositionUnitsY == eGuiUnits_Percents)
    {
        float valuey = mParent->mSize.y * (mPositionPercents.y * 1.0f / 100.0f);
        outputPoint.y = (int) valuey;
    }

    return outputPoint;
}

Point GuiWidget::ComputeSizePixels() const
{
    Point outputSize = mSize;

    if (mParent == nullptr ||
        (mSizeUnitsW == eGuiUnits_Pixels && mSizeUnitsH == eGuiUnits_Pixels))
    {
        return outputSize;
    }

    if (mSizeUnitsW == eGuiUnits_Percents)
    {
        float valuew = mParent->mSize.x * (mSizePercents.x * 1.0f / 100.0f);
        outputSize.x = (int) valuew;
    }

    if (mSizeUnitsH == eGuiUnits_Percents)
    {
        float valueh = mParent->mSize.y * (mSizePercents.y * 1.0f / 100.0f);
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