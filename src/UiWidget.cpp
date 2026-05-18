#include "stdafx.h"
#include "UiWidget.h"
#include "UiRenderContext.h"
#include "UiWidgetManager.h"

//////////////////////////////////////////////////////////////////////////

UiWidget::ScopedChildrenLocker::ScopedChildrenLocker(UiWidget& widget)
    : mOwner(widget)
{
    ++mOwner.mChildrenLocksCount;
}

UiWidget::ScopedChildrenLocker::~ScopedChildrenLocker()
{
    --mOwner.mChildrenLocksCount;
    cxx_assert(mOwner.mChildrenLocksCount >= 0);
}

//////////////////////////////////////////////////////////////////////////

UiWidget::UiWidget() : UiWidget("dummy")
{
}

UiWidget::UiWidget(const std::string& widgetClassName)
    : mParent()
    , mVisible(true)
    , mEnabled(true)
    , mInteractive(true)
    , mPosition()
    , mSize()
    , mName()
    , mEnablePickChildren(true)
    , mEnableClipChildren()
    , mVisible_Inherited(true)
    , mEnabled_Inherited(true)
    , mUserData()
    , mClassName(widgetClassName)
    , mScale(1.0f)
{
}

UiWidget::UiWidget(const UiWidget& sourceWidget)
    : mParent() // don't copy
    , mVisible(sourceWidget.mVisible)
    , mEnabled(sourceWidget.mEnabled)
    , mInteractive(sourceWidget.mInteractive)
    , mPosition(sourceWidget.mPosition)
    , mSize(sourceWidget.mSize)
    , mName(sourceWidget.mName)
    , mEnablePickChildren(sourceWidget.mEnablePickChildren)
    , mEnableClipChildren(sourceWidget.mEnableClipChildren)
    , mVisible_Inherited(true) // force default
    , mEnabled_Inherited(true) // force default
    , mTransformInvalidated(true) // force default
    , mUserData() // don't copy
    , mClassName(sourceWidget.mClassName)
    , mScale(sourceWidget.mScale)
    , mRelativePivot(sourceWidget.mRelativePivot)
    , mPivotPoint(sourceWidget.mPivotPoint)
    , mAnchorMin(sourceWidget.mAnchorMin)
    , mAnchorMax(sourceWidget.mAnchorMax)
    , mAnchorPointMin(sourceWidget.mAnchorMin)
    , mAnchorPointMax(sourceWidget.mAnchorMax)
{
    InvalidateTransform();
}

UiWidget::~UiWidget()
{
    cxx_assert(mChildren.empty());
}

bool UiWidget::IsHovered() const
{
    return this == gWidgetManager.GetHoveredWidget();
}

bool UiWidget::IsFocused() const
{
    return this == gWidgetManager.GetFocusedWidget();
}

void UiWidget::Subscribe(UiEventListener* listener)
{
    cxx_assert(listener);
    mEventListeners.AddListener(listener);
}

void UiWidget::Unsubscribe(UiEventListener* listener)
{
    cxx_assert(listener);
    mEventListeners.RemoveListener(listener);
}

void UiWidget::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (!IsVisibleInHierarchy() || !mInteractive)
        return;

    if (!IsEnabledInHierarchy())
        return;

    HandleInputEvent(inputEvent);
}

void UiWidget::InputEvent(KeyInputEvent& inputEvent)
{
    if (!IsVisibleInHierarchy() || !mInteractive)
        return;

    if (!IsEnabledInHierarchy())
        return;

    HandleInputEvent(inputEvent);
}

void UiWidget::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (!IsVisibleInHierarchy() || !mInteractive)
        return;

    if (!IsEnabledInHierarchy())
        return;

    HandleInputEvent(inputEvent);
}

void UiWidget::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (!IsVisibleInHierarchy() || !mInteractive)
        return;

    if (!IsEnabledInHierarchy())
        return;

    HandleInputEvent(inputEvent);
}

void UiWidget::InputEvent(KeyCharEvent& inputEvent)
{
    if (!IsVisibleInHierarchy() || !mInteractive)
        return;

    if (!IsEnabledInHierarchy())
        return;

    HandleInputEvent(inputEvent);
}

void UiWidget::Deserialize(const JsonElement& jsonElement)
{
    if (!jsonElement) return;

    JsonQuery(jsonElement, "id", mName);

    mSize = {0, 0};
    JsonQuery(jsonElement, "size", mSize);

    mPosition = {0, 0};
    JsonQuery(jsonElement, "pos", mPosition);

    mScale = {1.0f, 1.0f};
    JsonQuery(jsonElement, "scale", mScale);

    mRelativePivot = {0.0f, 0.0f};
    JsonQuery(jsonElement, "pivot", mRelativePivot);

    mAnchorMin = {0.0f, 0.0f};
    mAnchorMax = {0.0f, 0.0f};
    JsonQuery(jsonElement, "anchor_min", mAnchorMin);
    JsonQuery(jsonElement, "anchor_max", mAnchorMax);
    
    mAnchorMax = glm::max(mAnchorMin, mAnchorMax);

    mVisible = true;
    JsonQuery(jsonElement, "visible", mVisible);
    
    mEnabled = true;
    JsonQuery(jsonElement, "enabled", mEnabled);

    bool isEnablePickChildren = true;
    JsonQuery(jsonElement, "pick_children", isEnablePickChildren);

    bool isEnableClipChildren = false;
    JsonQuery(jsonElement, "clip_children", isEnableClipChildren);

    mEnablePickChildren = isEnablePickChildren;
    mEnableClipChildren = isEnableClipChildren;

    JsonQuery(jsonElement, "interactive", mInteractive);

    RecomputeLocalPivotPoint();
    
    mAnchorPointMin = {};
    mAnchorPointMax = {};
}

UiWidget* UiWidget::PickWidget(const Point2D& screenPosition)
{
    if (!IsVisibleInHierarchy()) return nullptr;

    if (mEnablePickChildren)
    {
        ScopedChildrenLocker childrenLocker{*this};

        // process in reversed oreder
        for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it)
        {
            UiWidget* roller = *it;
            // is point within widget and visible
            if (!roller->IsVisibleInHierarchy() || !roller->IsScreenPointInsideRect(screenPosition))
                continue;
        
            if (UiWidget* picked = roller->PickWidget(screenPosition))
                return picked;
        }
    }
    // return self if pickable
    return IsInteractive() ? this : nullptr;
}

void UiWidget::RenderFrame(UiRenderContext& uiRenderContext)
{
    ComputeTransform();

    if (!IsVisibleInHierarchy()) return;

    uiRenderContext.SetTransform(&mTransform);

    if (mEnableClipChildren)
    {
        Rect2D rcLocal = GetLocalBounds();
        if (!uiRenderContext.EnterChildClipArea(rcLocal))
            return;
    }

    RenderSelf(uiRenderContext);

    // render debug info
    if (gDebug.mDrawUiBounds)
    {
        // todo
    }

    // process children
    if (!mChildren.empty())
    {
        ScopedChildrenLocker childrenLocker{*this};
        for (UiWidget* childWidget: mChildren)
        {
            childWidget->RenderFrame(uiRenderContext);
        }
    }

    if (mEnableClipChildren)
    {
        uiRenderContext.LeaveChildClipArea();
    }
}

void UiWidget::UpdateFrame(float deltaTime)
{
    PreUpdateSelf(deltaTime);

    if (!mChildren.empty())
    {
        ScopedChildrenLocker childrenLocker{*this};
        // using indexing instead of range-for for stability
        for (size_t i = 0; i < mChildren.size(); ++i) 
        {
            UiWidget* child = mChildren[i];
            child->UpdateFrame(deltaTime);
        }
    }

    UpdateSelf(deltaTime);
}

void UiWidget::SetParentWidget(UiWidget* newParent)
{
    if (mParent == newParent) return;

    cxx_assert(newParent != this);
    if (newParent == this) return;
    // change parent
    if (mParent)
    {
        mParent->CheckChildrenUnLocked();

        if (!cxx::erase(mParent->mChildren, this))
        {
            cxx_assert(false);
        }
        mParent->HandleChildDetached(this);
    }

    mParent = newParent;
    if (mParent)
    {
        mParent->CheckChildrenUnLocked();

        mParent->mChildren.push_back(this);
    }

    InvalidateTransform();

    UpdateInheritedVisibilityState();
    UpdateInheritedEnabledState();

    if (mParent)
    {
        Point2D newSize = mParent->GetSize();
        ParentSizeChanged(newSize);
        mParent->HandleChildAttached(this);
    }
}

void UiWidget::AttachChild(UiWidget* widget)
{
    if ((widget == nullptr) || (widget == this) || (widget->mParent == this))
    {
        cxx_assert(false);
        return;
    }
    CheckChildrenUnLocked();
    widget->SetParentWidget(this);
}

void UiWidget::DetachChild(UiWidget* widget)
{
    if ((widget == nullptr) || (widget->mParent != this))
    {
        cxx_assert(false);
        return;
    }
    CheckChildrenUnLocked();
    widget->SetParentWidget(nullptr);
}

void UiWidget::DetachSelf()
{
    SetParentWidget(nullptr);
}

void UiWidget::DeleteWidget()
{
    DetachSelf();
    gWidgetManager.MarkWidgetForDelete(this);

    while (!mChildren.empty())
    {
        UiWidget* childWidget = mChildren.back();
        childWidget->DeleteWidget();
    }
}

UiWidget* UiWidget::CloneSelf() const
{
    UiWidget* cloneWidget = new UiWidget(*this);
    return cloneWidget;
}

void UiWidget::PositionChanged(const Point2D& prevPosition)
{
    InvalidateTransform();
    for (UiWidget* child: mChildren)
    {
        child->InvalidateTransform();
    }
    HandlePositionChanged(prevPosition);
}

void UiWidget::SizeChanged(const Point2D& prevSize)
{
    InvalidateTransform();

    RecomputeLocalPivotPoint();

    if (!mChildren.empty())
    {
        ScopedChildrenLocker childrenLocker{*this};
        // using indexing instead of range-for for stability
        for (size_t i = 0; i < mChildren.size(); ++i) 
        {
            UiWidget* child = mChildren[i];
            child->ParentSizeChanged(mSize);
        }
    }
    HandleSizeChanged(prevSize);
}

void UiWidget::ScaleChanged(const glm::vec2& prevScale)
{
    InvalidateTransform();
}

void UiWidget::ParentSizeChanged(const Point2D& newParentSize)
{
    bool anchorPointsChanged = RecomputeAnchorPoints(newParentSize);
    if (anchorPointsChanged)
    {
        InvalidateTransform();

        // actualize position and size
        Point2D prevSize = mSize;
        Point2D newSize = ComputeNewSize(prevSize);
        if (newSize != prevSize)
        {
            mSize = newSize;
            SizeChanged(prevSize);
        }
        Point2D prevPosition = mPosition;
        Point2D newPosition = ComputeNewPosition(prevPosition);
        if (newPosition != prevPosition)
        {
            mPosition = newPosition;
            PositionChanged(prevPosition);
        }
    }
}

bool UiWidget::RecomputeLocalPivotPoint()
{
    Point2D newPivotPoint = Point2D
    {
        static_cast<int>(mRelativePivot.x * mSize.x),
        static_cast<int>(mRelativePivot.y * mSize.y)
    };
    bool changed = (newPivotPoint != mPivotPoint);
    if (changed)
    {
        mPivotPoint = newPivotPoint;
    }
    return changed;
}

bool UiWidget::RecomputeAnchorPoints(const Point2D& parentSize)
{
    Point2D newAnchorPointMin;
    Point2D newAnchorPointMax;

    glm::vec2 parentSizef {parentSize};
    newAnchorPointMin = Point2D { parentSizef * mAnchorMin };
    newAnchorPointMax = Point2D { parentSizef * mAnchorMax };
    newAnchorPointMax = glm::max(newAnchorPointMin, newAnchorPointMax);

    bool changed = (newAnchorPointMin != mAnchorPointMin) || (newAnchorPointMax != mAnchorPointMax);
    if (changed)
    {
        mAnchorPointMin = newAnchorPointMin;
        mAnchorPointMax = newAnchorPointMax;
    }
    return changed;
}

Point2D UiWidget::ComputeNewPosition(const Point2D& desiredPosition) const
{
    const Point2D correctedPosition
    {
        StretchingHorz() ? mAnchorPointMin.x : desiredPosition.x,
        StretchingVert() ? mAnchorPointMin.y : desiredPosition.y,
    };
    return correctedPosition;
}

Point2D UiWidget::ComputeNewSize(const Point2D& desiredSize) const
{
    const Point2D correctedSize
    {
        StretchingHorz() ? (mAnchorPointMax.x - mAnchorPointMin.x) : desiredSize.x,
        StretchingVert() ? (mAnchorPointMax.y - mAnchorPointMin.y) : desiredSize.y,
    };
    return correctedSize;
}

void UiWidget::SetEnabled(bool isEnabled)
{
    if (mEnabled == isEnabled)
        return;

    bool wasEnabledInHierarchy = IsEnabledInHierarchy();
    mEnabled = isEnabled;
    if (wasEnabledInHierarchy != IsEnabledInHierarchy())
    {
        if (!mChildren.empty())
        {
            ScopedChildrenLocker childrenLocker{*this};
            // using indexing instead of range-for for stability
            for (size_t i = 0; i < mChildren.size(); ++i) 
            {
                UiWidget* child = mChildren[i];
                child->UpdateInheritedEnabledState();
            }
        }
        HandleEnableStateChanged();
    }
}

void UiWidget::SetVisible(bool isEnabled)
{
    if (mVisible == isEnabled)
        return;

    bool wasVisibleInHierarchy = IsVisibleInHierarchy();
    mVisible = isEnabled;
    if (wasVisibleInHierarchy != IsVisibleInHierarchy())
    {
        if (!mChildren.empty())
        {
            ScopedChildrenLocker childrenLocker{*this};
            // using indexing instead of range-for for stability
            for (size_t i = 0; i < mChildren.size(); ++i) 
            {
                UiWidget* child = mChildren[i];
                child->UpdateInheritedVisibilityState();
            }
        }
        HandleVisibilityChanged();
    }
}

void UiWidget::SetPosition(const Point2D& position)
{
    const Point2D correctedPosition = ComputeNewPosition(position);

    bool positionChanged = (correctedPosition != mPosition);
    if (positionChanged)
    {
        Point2D prevPosition = mPosition;
        mPosition = correctedPosition;
        PositionChanged(prevPosition);
    }    
}

void UiWidget::SetSize(const Point2D& newSize)
{
    // cancelling out stratching axis
    const Point2D correctedSize = ComputeNewSize(newSize);

    bool sizeChanged = (correctedSize != mSize);
    if (sizeChanged)
    {
        Point2D prevSize = mSize;
        mSize = correctedSize;
        SizeChanged(prevSize);
    }
}

void UiWidget::SetClipChildren(bool isEnabled)
{
    mEnableClipChildren = isEnabled;
}

void UiWidget::SetScale(const glm::vec2& newScale)
{
    bool scaleChanged = (newScale != mScale);
    if (scaleChanged)
    {
        glm::vec2 prevScale = mScale;
        mScale = newScale;
        ScaleChanged(prevScale);
    }
}

UiWidget* UiWidget::NextSibling() const
{
    if (mParent == nullptr) return nullptr;

    if (mParent->mChildren.empty())
    {
        cxx_assert(false);
        return nullptr;
    }

    if (mParent->mChildren.back() == this) 
        return nullptr;

    int index = cxx::get_item_index(mParent->mChildren, this);
    cxx_assert(index != -1);
    if (index == -1) return nullptr;

    return mParent->mChildren[index + 1];
}

UiWidget* UiWidget::PrevSibling() const
{
    if (mParent == nullptr) return nullptr;

    if (mParent->mChildren.empty())
    {
        cxx_assert(false);
        return nullptr;
    }

    if (mParent->mChildren.front() == this) 
        return nullptr;

    int index = cxx::get_item_index(mParent->mChildren, this);
    cxx_assert(index != -1);
    if (index == -1) return nullptr;

    return mParent->mChildren[index - 1];
}

UiWidget* UiWidget::FirstChild() const
{
    if (mChildren.empty()) return nullptr;
    return mChildren.front();
}

UiWidget* UiWidget::LastChild() const
{
    if (mChildren.empty()) return nullptr;
    return mChildren.back();
}

UiWidget* UiWidget::GetParent() const
{
    return mParent;
}

UiWidget* UiWidget::GetChild(const std::string_view& name) const
{
    for (UiWidget* roller: mChildren)
    {
        if (roller->mName == name) return roller;
    }
    return nullptr;
}

UiWidget* UiWidget::GetChild(int index) const
{
    if ((index > -1) && (index < static_cast<int>(mChildren.size())))
    {
        return mChildren[index];
    }
    return nullptr;
}

UiWidget* UiWidget::FindChildWithName(const std::string_view& name) const
{
    for (UiWidget* roller: mChildren)
    {
        if (roller->mName == name) return roller;
    }
    // scan in depth
    for (UiWidget* roller: mChildren)
    {
        if (UiWidget* child = roller->FindChildWithName(name))
            return child;
    }
    return nullptr;
}

bool UiWidget::IsScreenPointInsideRect(const Point2D& screenPosition) const
{
    Point2D localPoint = ScreenToLocal(screenPosition);
    Rect2D localRect = GetLocalBounds();
    return localRect.PointWithin(localPoint);
}

Point2D UiWidget::LocalToScreen(const Point2D& position) const
{
    UiWidget* thisWidget = const_cast<UiWidget*>(this);
    thisWidget->ComputeTransform();

    glm::vec4 localSpacePosition(position, 0.0f, 1.0f);
    glm::vec4 screenSpacePosition = mTransform * localSpacePosition;
    return Point2D(screenSpacePosition);
}

Point2D UiWidget::ScreenToLocal(const Point2D& position) const
{
    UiWidget* thisWidget = const_cast<UiWidget*>(this);
    thisWidget->ComputeTransform();

    glm::mat4 inverseTransform = glm::inverse(mTransform);
    glm::vec4 screenSpacePosition(position, 0.0f, 1.0f);
    glm::vec4 localSpacePosition = inverseTransform * screenSpacePosition;
    return Point2D(localSpacePosition);
}

void UiWidget::ComputeTransform()
{
    if (!mTransformInvalidated)	
        return;

    mTransformInvalidated = false;

    const Point2D position {
        mPosition.x + (StretchingHorz() ? mPivotPoint.x : mAnchorPointMin.x),
        mPosition.y + (StretchingVert() ? mPivotPoint.y : mAnchorPointMin.y)
    };

    if (mParent)
    {
        mParent->ComputeTransform();

        mTransform = mParent->mTransform *
            glm::translate(glm::vec3(position, 0.0f)) * 
            glm::scale(glm::vec3(mScale.x, mScale.y, 1.0f)) *
            glm::translate(-glm::vec3(mPivotPoint, 0.0f));
    }
    else
    {
        mTransform = 
            glm::translate(glm::vec3(position, 0.0f)) *
            glm::scale(glm::vec3(mScale.x, mScale.y, 1.0f)) *
            glm::translate(-glm::vec3(mPivotPoint, 0.0f));
    }
}

void UiWidget::InvalidateTransform()
{
    if (mTransformInvalidated)
        return;

    mTransformInvalidated = true;
    for (UiWidget* roller: mChildren)
    {
        roller->InvalidateTransform();
    }
}

void UiWidget::FitLayoutToScreen(const Point2D& screenSize)
{
    if (mParent == nullptr)
    {
        ParentSizeChanged(screenSize);
    }
}

UiWidget* UiWidget::CloneWidget() const
{
    UiWidget* clone = CloneSelf();
    // clone children
    for (UiWidget* child: mChildren)
    {
        UiWidget* childClone = child->CloneSelf();
        clone->AttachChild(childClone);
    }
    return clone;
}

void UiWidget::UpdateInheritedVisibilityState()
{
    bool prevInheritedState = mVisible_Inherited;

    mVisible_Inherited = (mParent == nullptr) || mParent->IsVisibleInHierarchy();
    if (prevInheritedState == mVisible_Inherited)
        return;

    if (IsVisibleSelf())
    {
        if (!mChildren.empty())
        {
            ScopedChildrenLocker childrenLocker{*this};
            // using indexing instead of range-for for stability
            for (size_t i = 0; i < mChildren.size(); ++i) 
            {
                UiWidget* child = mChildren[i];
                child->UpdateInheritedVisibilityState();
            }
        }
        HandleVisibilityChanged();
    }
}

void UiWidget::UpdateInheritedEnabledState()
{
    bool prevInheritedState = mEnabled_Inherited;

    mEnabled_Inherited = (mParent == nullptr) || mParent->IsEnabledInHierarchy();
    if (prevInheritedState == mEnabled_Inherited)
        return;
    
    if (IsEnabledSelf())
    {
        if (!mChildren.empty())
        {
            ScopedChildrenLocker childrenLocker{*this};
            // using indexing instead of range-for for stability
            for (size_t i = 0; i < mChildren.size(); ++i) 
            {
                UiWidget* child = mChildren[i];
                child->UpdateInheritedEnabledState();
            }
        }
        HandleEnableStateChanged();
    }
}

