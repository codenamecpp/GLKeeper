#include "pch.h"
#include "GuiWidget.h"
#include "GuiManager.h"
#include "GuiRenderer.h"
#include "InputsManager.h"
#include "GraphicsDevice.h"
#include "GuiHelpers.h"
#include "GuiLayout.h"

// base widget class factory
static GuiWidgetFactory<GuiWidget> _BaseWidgetsFactory;
GuiWidgetMetaClass GuiWidget::MetaClass ( cxx::unique_string("widget"), &_BaseWidgetsFactory, nullptr );

//////////////////////////////////////////////////////////////////////////

GuiWidget::GuiWidget(): GuiWidget(&MetaClass)
{
}

GuiWidget::GuiWidget(GuiWidgetMetaClass* widgetClass)
    : mMetaClass(widgetClass)
    , mTransform(1.0f)
    , mOriginPercents()
    , mOrigin()
    , mPositionPercents()
    , mPosition()
    , mSizePercents()
    , mSize()
    , mUserData()
    , mActions(this)
{
    debug_assert(mMetaClass);
}

GuiWidget::GuiWidget(GuiWidget* copyWidget)
    : mMetaClass(copyWidget->mMetaClass)
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
    , mTemplateClassName(copyWidget->mTemplateClassName)
    , mOnClickEvent(copyWidget->mOnClickEvent)
    , mOnMouseEnterEvent(copyWidget->mOnMouseEnterEvent)
    , mOnMouseLeaveEvent(copyWidget->mOnMouseLeaveEvent)
    , mOnMouseLButtonDownEvent(copyWidget->mOnMouseLButtonDownEvent)
    , mOnMouseRButtonDownEvent(copyWidget->mOnMouseRButtonDownEvent)
    , mOnMouseMButtonDownEvent(copyWidget->mOnMouseMButtonDownEvent)
    , mOnMouseLButtonUpEvent(copyWidget->mOnMouseLButtonUpEvent)
    , mOnMouseRButtonUpEvent(copyWidget->mOnMouseRButtonUpEvent)
    , mOnMouseMButtonUpEvent(copyWidget->mOnMouseMButtonUpEvent)
    , mTintColor(copyWidget->mTintColor)
    , mActions(this)
    , mHasInteractiveAttribute(copyWidget->mHasInteractiveAttribute)
    , mHasSelectableAttribute(copyWidget->mHasSelectableAttribute)
    , mHasDisablePickChildrenAttribute(copyWidget->mHasDisablePickChildrenAttribute)
    , mHasDrawBackgroundAttribute(copyWidget->mHasDrawBackgroundAttribute)
    , mHasDrawBordersAttribute(copyWidget->mHasDrawBordersAttribute)
    , mBordersColor(copyWidget->mBordersColor)
    , mBackgroundColor(copyWidget->mBackgroundColor)
{
    debug_assert(mMetaClass);

    mActions.CopyActions(copyWidget->mActions);

    if (copyWidget->mLayout)
    {
        mLayout = new GuiLayout(this);
        mLayout->CopyProperties(*copyWidget->mLayout);
    }
}

void GuiWidget::LoadProperties(cxx::json_node_object documentNode)
{
    if (!documentNode)
    {
        debug_assert(false);
        return;
    }

    mName = cxx::unique_string(documentNode.get_element_name());
    if (mName.empty())
    {
        // generate unique name
        const char* name = cxx::va("%s_%p", mMetaClass->mClassName.c_str(), this);
        mName = cxx::unique_string(name);
    }

    bool is_visible = false;
    bool is_enabled = false;
    bool is_clip_children = false;
    
    if (cxx::json_get_attribute(documentNode, "visible", is_visible)) 
    {  
        SetVisible(is_visible);
    }
    if (cxx::json_get_attribute(documentNode, "enabled", is_enabled))
    {
        SetEnabled(is_enabled);
    }
    if (cxx::json_get_attribute(documentNode, "clip_children", is_clip_children))
    {
        SetClipChildren(is_clip_children);
    }
    // min size
    if (cxx::json_node_array prop_minsize = documentNode["min_size"])
    {
        Point minSize(0, 0);
        cxx::json_get_array_item(prop_minsize, 0, minSize.x);
        cxx::json_get_array_item(prop_minsize, 1, minSize.y);
        SetMinSize(minSize);
    }
    // max size
    if (cxx::json_node_array prop_maxsize = documentNode["max_size"])
    {
        Point maxSize(0, 0);
        cxx::json_get_array_item(prop_maxsize, 0, maxSize.x);
        cxx::json_get_array_item(prop_maxsize, 1, maxSize.y);
        SetMaxSize(maxSize);
    }    
    // origin
    {
        Point origin(0, 0);
        eGuiUnits unitsX = eGuiUnits_Pixels;
        eGuiUnits unitsY = eGuiUnits_Pixels;

        if (cxx::json_node_array prop_position = documentNode["origin"])
        {
            if (!GuiParsePixelsOrPercents(prop_position[0], unitsX, origin.x) ||
                !GuiParsePixelsOrPercents(prop_position[1], unitsY, origin.y))
            {
                debug_assert(false);
            }
        }
        SetOrigin(origin, unitsX, unitsY);
    }
    // position
    {
        Point position(0, 0);
        eGuiUnits unitsX = eGuiUnits_Pixels;
        eGuiUnits unitsY = eGuiUnits_Pixels;
        if (cxx::json_node_array prop_position = documentNode["pos"])
        {
            if (!GuiParsePixelsOrPercents(prop_position[0], unitsX, position.x) ||
                !GuiParsePixelsOrPercents(prop_position[1], unitsY, position.y))
            {
                debug_assert(false);
            }
        }
        SetPosition(position, unitsX, unitsY);
    }
    // size
    {
        Point size(0, 0);
        eGuiUnits unitsW = eGuiUnits_Pixels;
        eGuiUnits unitsH = eGuiUnits_Pixels;
        if (cxx::json_node_array prop_size = documentNode["size"])
        {
            if (!GuiParsePixelsOrPercents(prop_size[0], unitsW, size.x) ||
                !GuiParsePixelsOrPercents(prop_size[1], unitsH, size.y))
            {
                debug_assert(false);
            }
        }
        SetSize(size, unitsW, unitsH);
    }
    // anchors
    if (cxx::json_document_node prop_anchors = documentNode["anchors"])
    {
        GuiAnchors anchors;
        if (GuiParseAnchors(prop_anchors, anchors))
        {
            SetAnchors(anchors);
        }
        else
        {
            debug_assert(false);
        }
    }

    // custom events
    if (cxx::json_node_object events_node = documentNode["events"])
    {
        cxx::json_get_attribute(events_node, "on_click", mOnClickEvent);
        cxx::json_get_attribute(events_node, "on_mouse_enter", mOnMouseEnterEvent);
        cxx::json_get_attribute(events_node, "on_mouse_leave", mOnMouseLeaveEvent);
        cxx::json_get_attribute(events_node, "on_lbutton_down", mOnMouseLButtonDownEvent);
        cxx::json_get_attribute(events_node, "on_rbutton_down", mOnMouseRButtonDownEvent);
        cxx::json_get_attribute(events_node, "on_mbutton_down", mOnMouseMButtonDownEvent);
        cxx::json_get_attribute(events_node, "on_lbutton_up", mOnMouseLButtonUpEvent);
        cxx::json_get_attribute(events_node, "on_rbutton_up", mOnMouseRButtonUpEvent);
        cxx::json_get_attribute(events_node, "on_mbutton_up", mOnMouseMButtonUpEvent);
    }

    // colors
    GuiParseColor(documentNode["tint_color"], mTintColor);
    GuiParseColor(documentNode["background_color"], mBackgroundColor);
    GuiParseColor(documentNode["borders_color"], mBordersColor);

    // layout
    if (cxx::json_node_object layoutNode = documentNode["layout"])
    {
        debug_assert(mLayout == nullptr);

        // deserialize layout
        mLayout = new GuiLayout(this);
        mLayout->LoadProperties(layoutNode);
    }

    // actions
    if (cxx::json_document_node actions_node = documentNode["actions"])
    {
        LoadActions(actions_node);
    }

    // attributes
    cxx::json_get_attribute(documentNode, "interactive", mHasInteractiveAttribute);
    cxx::json_get_attribute(documentNode, "selectable", mHasSelectableAttribute);
    cxx::json_get_attribute(documentNode, "disable_pick_children", mHasDisablePickChildrenAttribute);
    cxx::json_get_attribute(documentNode, "draw_background", mHasDrawBackgroundAttribute);
    cxx::json_get_attribute(documentNode, "draw_borders", mHasDrawBordersAttribute);

    HandleLoadProperties(documentNode);
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
    SafeDelete(mLayout);
    if (mParent)
    {
        mParent->DetachChild(this);
    }

    while (mFirstChild)
    {
        GuiWidget* deleteWidget = mFirstChild;
        deleteWidget->SetDetached(); // doesn't invoke HandleChildDetached - it is pointless in destructor
        SafeDelete(deleteWidget);
    }
}

GuiWidget* GuiWidget::Clone()
{
    GuiWidget* selfClone = CreateClone();
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
        Rectangle rcLocal = GetLocalRect();
        if (!renderContext.EnterChildClipArea(rcLocal))
            return;
    }

    if (mHasDrawBackgroundAttribute || mHasDrawBordersAttribute)
    {
        Rectangle rcLocal = GetLocalRect();
        if (mHasDrawBackgroundAttribute)
        {
            renderContext.FillRect(rcLocal, mBackgroundColor);
        }
        if (mHasDrawBordersAttribute)
        {
            renderContext.DrawRect(rcLocal, mBordersColor);
        }
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
    if (!IsVisible() || !mHasInteractiveAttribute)
        return;

    inputEvent.SetConsumed(true);
    if (!IsEnabled())
        return;

    // post event
    if (inputEvent.mPressed)
    {
        GuiEvent eventData = GuiEvent::MouseDownEvent(this, inputEvent.mButton, gInputsManager.mCursorPosition);
        mActions.EmitEvent(eventData.mEventId);
        gGuiManager.BroadcastEvent(eventData);
    }
    else
    {
        GuiEvent eventData = GuiEvent::MouseUpEvent(this, inputEvent.mButton, gInputsManager.mCursorPosition);
        mActions.EmitEvent(eventData.mEventId);
        gGuiManager.BroadcastEvent(eventData);
    }

    // custom event
    {
        cxx::unique_string customEventId;
        switch (inputEvent.mButton)
        {
            case eMouseButton_Left: customEventId = inputEvent.mPressed ? mOnMouseLButtonDownEvent : mOnMouseLButtonUpEvent; break;
            case eMouseButton_Right: customEventId = inputEvent.mPressed ? mOnMouseRButtonDownEvent : mOnMouseRButtonUpEvent; break;
            case eMouseButton_Middle: customEventId = inputEvent.mPressed ? mOnMouseMButtonDownEvent : mOnMouseMButtonUpEvent; break;
        }
        if (!customEventId.empty())
        {
            GuiEvent customEvent = GuiEvent::CustomEvent(this, customEventId);
            mActions.EmitEvent(customEvent.mEventId);
            gGuiManager.BroadcastEvent(customEvent);
        }
    }
    
    bool hasBeenClicked = false;
    // process clicks
    if (inputEvent.mButton == eMouseButton_Left && mHasSelectableAttribute)
    {
        if (inputEvent.mPressed)
        {
            if (IsHovered())
            {
                gGuiManager.SetSelectedWidget(this);
            }
        }
        else
        {
            hasBeenClicked = IsHovered() && IsSelected();
            Deselect();
        }
    }

    HandleInputEvent(inputEvent);

    if (hasBeenClicked)
    {
        // post event
        {
            GuiEvent eventData = GuiEvent::ClickEvent(this, gInputsManager.mCursorPosition);
            mActions.EmitEvent(eventData.mEventId);
            gGuiManager.BroadcastEvent(eventData);
        }

        // custom event
        if (!mOnClickEvent.empty())
        {
            GuiEvent customEvent = GuiEvent::CustomEvent(this, mOnClickEvent);
            mActions.EmitEvent(customEvent.mEventId);
            gGuiManager.BroadcastEvent(customEvent);
        }

        HandleClick();
    }
}

void GuiWidget::ProcessEvent(MouseMovedInputEvent& inputEvent)
{
    if (!IsVisible() || !mHasInteractiveAttribute)
        return;

    inputEvent.SetConsumed(true);
    if (!IsEnabled())
        return;

    HandleInputEvent(inputEvent);
}

void GuiWidget::ProcessEvent(MouseScrollInputEvent& inputEvent)
{
    if (!IsVisible() || !mHasInteractiveAttribute)
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
    widget->SetupAnchorsOffsets();
    widget->ParentSizeChanged(mSize, mSize);
    UpdateLayout();
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

    widget->SetDetached();

    UpdateLayout();
    HandleChildDetached(widget);
    return true;
}

void GuiWidget::DetachAndFreeChildren()
{
    while (mFirstChild)
    {
        GuiWidget* deleteWidget = mFirstChild;
        SafeDelete(deleteWidget);
    }
}

void GuiWidget::SetDetached()
{
    if (mParent == nullptr)
    {
        debug_assert(mPrevSibling == nullptr);
        debug_assert(mNextSibling == nullptr);

        return;
    }

    if (this == mParent->mFirstChild)
    {
        mParent->mFirstChild = mNextSibling;
    }
    if (mPrevSibling)
    {
        mPrevSibling->mNextSibling = mNextSibling;
    }
    if (mNextSibling)
    {
        mNextSibling->mPrevSibling = mPrevSibling;
    }
    mParent = nullptr;
}

GuiWidget* GuiWidget::PickWidget(const Point& screenPosition)
{
    GuiWidget* resultWidget = nullptr;

    if (!IsVisible())
        return nullptr;

    // pick child widgets only if not pick target
    if (!mHasDisablePickChildrenAttribute)
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
        if (IsScreenPointInsideRect(screenPosition) && mHasInteractiveAttribute)
        {
            resultWidget = this;
        }
    }

    return resultWidget;
}

GuiWidget* GuiWidget::GetChild(const cxx::unique_string& name) const
{
    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        if (currChild->mName == name)
            return currChild;
    }
    return nullptr;
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

GuiWidget* GuiWidget::SearchForChild(const cxx::unique_string& name) const
{
    GuiWidget* child = GetChild(name);
    if (child == nullptr)
    {
        // fullscan sub-hierarchy
        for (GuiWidget* currChild = mFirstChild; currChild; 
            currChild = currChild->mNextSibling)
        {
            child = currChild->SearchForChild(name);
            if (child)
                break;
            
        }
    }
    return child;
}

GuiWidget* GuiWidget::SearchForChild(const std::string& name) const
{
    GuiWidget* child = GetChild(name);
    if (child == nullptr)
    {
        // fullscan sub-hierarchy
        for (GuiWidget* currChild = mFirstChild; currChild; 
            currChild = currChild->mNextSibling)
        {
            child = currChild->SearchForChild(name);
            if (child)
                break;

        }
    }
    return child;
}

void GuiWidget::SetAnchors(const GuiAnchors& anchors)
{
    if (mAnchors.mL == anchors.mL && mAnchors.mT == anchors.mT && 
        mAnchors.mR == anchors.mR && mAnchors.mB == anchors.mB)
    {
        return;
    }

    mAnchors = anchors;
    SetupAnchorsOffsets();

    InvalidateTransform();
}

bool GuiWidget::HasAnchors() const
{
    return mAnchors.mB || mAnchors.mL || mAnchors.mR || mAnchors.mT;
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
        PositionChanged(prevPosition, true);
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
    Point correctSize = glm::max(size, mMinSize); // sanity check
    Point prevSize = mSize;

    mSizeUnitsW = units_w;
    mSizeUnitsH = units_h;

    (mSizeUnitsW == eGuiUnits_Percents ? mSizePercents.x : mSize.x) = correctSize.x;
    (mSizeUnitsH == eGuiUnits_Percents ? mSizePercents.y : mSize.y) = correctSize.y;

    mSize = ComputeSizePixels();
    if (mSize != prevSize)
    {
        SizeChanged(prevSize, true);
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
    if (!IsEnabled() || !mHasInteractiveAttribute)
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

void GuiWidget::SetMinSize(const Point& minSize)
{
    mMinSize = minSize;
}

void GuiWidget::SetMaxSize(const Point& maxSize)
{
    mMaxSize = maxSize;
}

void GuiWidget::SetDrawBackground(bool isEnabled)
{
    mHasDrawBackgroundAttribute = isEnabled;
}

void GuiWidget::SetDrawBorders(bool isEnabled)
{
    mHasDrawBordersAttribute = isEnabled;
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
    Point newPosition = mPosition;
    Point newSize = mSize;

    if (mAnchors.mL && mAnchors.mR)
    {
        newSize.x = currSize.x - (mAnchorDistL + mAnchorDistR);
        if (newSize.x < 0)
        {
            newSize.x = 0;
        }
    }
    else
    {
        if (mAnchors.mR)
        {
            newPosition.x = currSize.x - mAnchorDistR;
        }
        else if (!mAnchors.mL)
        {
            newPosition.x = (currSize.x - (mAnchorDistL + mAnchorDistR)) / 2;
        }
    }

    if (mAnchors.mT && mAnchors.mB)
    {
        newSize.y = currSize.y - (mAnchorDistB + mAnchorDistT);
        if (newSize.y < 0)
        {
            newSize.y = 0;
        }
    }
    else
    {
        if (mAnchors.mB)
        {
            newPosition.y = currSize.y - mAnchorDistB;
        }
        else if (!mAnchors.mT)
        {
            newPosition.y = (currSize.y - (mAnchorDistT + mAnchorDistB)) / 2;
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
        PositionChanged(prevPosition, false);
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

    Point correctSize = glm::max(newSize, mMinSize); // sanity check
    if (correctSize != mSize)
    {
        Point prevSize = mSize;
        mSize = correctSize;
        SizeChanged(prevSize, false);
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

void GuiWidget::PositionChanged(const Point& prevPosition, bool setAnchors)
{
    InvalidateTransform();

    for (GuiWidget* currChild = mFirstChild; currChild; 
        currChild = currChild->mNextSibling)
    {
        currChild->ParentPositionChanged(prevPosition);
    }

    if (setAnchors)
    {
        SetupAnchorsOffsets();
    }

    HandlePositionChanged(prevPosition);
}

void GuiWidget::SizeChanged(const Point& prevSize, bool setAnchors)
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

    if (setAnchors)
    {
        SetupAnchorsOffsets();
    }
    UpdateLayout();
    HandleSizeChanged(prevSize);
}

void GuiWidget::ShownStateChanged()
{
    if (!IsVisible()) 
    {
        Deselect();

        if (IsHovered()) // force cancel hover
        {
            mHovered = false;
            HoveredStateChanged();
        }

        GuiEvent eventData = GuiEvent::HideEvent(this);
        mActions.EmitEvent(eventData.mEventId);
        gGuiManager.BroadcastEvent(eventData);
    }
    else
    {
        GuiEvent eventData = GuiEvent::ShowEvent(this);
        mActions.EmitEvent(eventData.mEventId);
        gGuiManager.BroadcastEvent(eventData);
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
        Deselect();

        if (IsHovered()) // force cancel hover
        {
            mHovered = false;
            HoveredStateChanged();
        }

        GuiEvent eventData = GuiEvent::DisableEvent(this);
        mActions.EmitEvent(eventData.mEventId);
        gGuiManager.BroadcastEvent(eventData);
    }
    else
    {
        GuiEvent eventData = GuiEvent::EnableEvent(this);
        mActions.EmitEvent(eventData.mEventId);
        gGuiManager.BroadcastEvent(eventData);
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
        GuiEvent eventData = GuiEvent::MouseEnterEvent(this, gInputsManager.mCursorPosition);
        mActions.EmitEvent(eventData.mEventId);
        gGuiManager.BroadcastEvent(eventData);

        // custom event
        if (!mOnMouseEnterEvent.empty())
        {
            GuiEvent customEvent = GuiEvent::CustomEvent(this, mOnMouseEnterEvent);
            mActions.EmitEvent(customEvent.mEventId);
            gGuiManager.BroadcastEvent(customEvent);
        }
    }
    else
    {
        GuiEvent eventData = GuiEvent::MouseLeaveEvent(this, gInputsManager.mCursorPosition);
        mActions.EmitEvent(eventData.mEventId);
        gGuiManager.BroadcastEvent(eventData);

        // custom event
        if (!mOnMouseLeaveEvent.empty())
        {
            GuiEvent customEvent = GuiEvent::CustomEvent(this, mOnMouseLeaveEvent);
            mActions.EmitEvent(customEvent.mEventId);
            gGuiManager.BroadcastEvent(customEvent);
        }
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

    if (mPositionUnitsX == eGuiUnits_Pixels && mPositionUnitsY == eGuiUnits_Pixels)
    {
        return outputPoint;
    }

    const Point& parentSize = mParent ? mParent->mSize : gGraphicsDevice.mScreenResolution;

    if (mPositionUnitsX == eGuiUnits_Percents)
    {
        float valuex = parentSize.x * (mPositionPercents.x * 1.0f / 100.0f);
        outputPoint.x = (int) valuex;
    }

    if (mPositionUnitsY == eGuiUnits_Percents)
    {
        float valuey = parentSize.y * (mPositionPercents.y * 1.0f / 100.0f);
        outputPoint.y = (int) valuey;
    }

    return outputPoint;
}

Point GuiWidget::ComputeSizePixels() const
{
    Point outputSize = mSize;

    if (mSizeUnitsW == eGuiUnits_Pixels && mSizeUnitsH == eGuiUnits_Pixels)
    {
        return outputSize;
    }

    const Point& parentSize = mParent ? mParent->mSize : gGraphicsDevice.mScreenResolution;

    if (mSizeUnitsW == eGuiUnits_Percents)
    {
        float valuew = parentSize.x * (mSizePercents.x * 1.0f / 100.0f);
        outputSize.x = (int) valuew;
    }

    if (mSizeUnitsH == eGuiUnits_Percents)
    {
        float valueh = parentSize.y * (mSizePercents.y * 1.0f / 100.0f);
        outputSize.y = (int) valueh;
    }

    return outputSize;
}

GuiWidget* GuiWidget::CreateClone()
{
    GuiWidget* selfClone = new GuiWidget(this);
    return selfClone;
}

void GuiWidget::SetupAnchorsOffsets()
{
    mAnchorDistL = 0;
    mAnchorDistT = 0;
    mAnchorDistR = 0;
    mAnchorDistB = 0;

    const Point& parentSize = mParent ? mParent->mSize : gGraphicsDevice.mScreenResolution;

    mAnchorDistL = mPosition.x - mOrigin.x;
    mAnchorDistT = mPosition.y - mOrigin.y;
    mAnchorDistR = parentSize.x - (mAnchorDistL + mSize.x);
    mAnchorDistB = parentSize.y - (mAnchorDistT + mSize.y);
}

void GuiWidget::UpdateLayout()
{
    if (mLayout)
    {
        mLayout->LayoutElements();
    }
}

void GuiWidget::Deselect()
{
    if (IsSelected())
    {
        gGuiManager.SetSelectedWidget(nullptr);
    }
}

void GuiWidget::SetActionsContext(GuiActionContext* context)
{
    mActionsContext = context;
}

bool GuiWidget::IsSelected() const
{
    return gGuiManager.mSelectedWidget == this;
}

void GuiWidget::LoadActions(cxx::json_node_object actionsNode)
{
    // iterate events
    for (cxx::json_node_array currNode = actionsNode.first_child(); currNode;
        currNode = currNode.next_sibling())
    {
        cxx::unique_string eventId = cxx::unique_string(currNode.get_element_name());

        // iterate actions
        for (cxx::json_node_object currActionNode = currNode.first_child(); currActionNode;
            currActionNode = currActionNode.next_sibling())
        {
            GuiAction* widgetAction = gGuiActionsFactory.DeserializeAction(currActionNode);
            if (widgetAction == nullptr)
            {
                debug_assert(false);
                continue;
            }

            mActions.AddAction(eventId, widgetAction);
        }
    }    
}

bool GuiWidget::ResolveCondition(const cxx::unique_string& name, bool& isTrue) const
{
    if (HandleResolveCondition(name, isTrue))
        return true;

    static cxx::unique_string id_pressed("pressed");
    static cxx::unique_string id_hovered("hovered");
    static cxx::unique_string id_enabled("enabled");
    static cxx::unique_string id_visible("visible");

    if (name == id_pressed)
    {
        isTrue = IsSelected();
        return true;
    }

    if (name == id_hovered)
    {
        isTrue = IsHovered();
        return true;
    }

    if (name == id_enabled)
    {
        isTrue = IsEnabled();
        return true;
    }

    if (name == id_visible)
    {
        isTrue = IsVisible();
        return true;
    }

    // examine ext contexts up to the root
    for (const GuiWidget* currWidget = this; currWidget; 
        currWidget = currWidget->mParent)
    {
        GuiActionContext* actionContext = currWidget->mActionsContext;
        if (actionContext && actionContext->ResolveCondition(currWidget, name, isTrue))
            return true;
    }

    debug_assert(false);
    return false;
}
