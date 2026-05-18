#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiDefs.h"
#include "UiEvent.h"
#include "ListenersContainer.h"

//////////////////////////////////////////////////////////////////////////

class UiWidget: public cxx::noncopyable
{
    friend class UiWidgetManager;

public:
    const std::string mClassName;

public:
    UiWidget();

    // render widget and all children
    void RenderFrame(UiRenderContext& uiRenderContext);

    // process widget logic

    // Warning:
    //
    // During update widget should NOT (directly or indirectly):
    // - attach, detach, or delete itself, its siblings, or its ancestors
    // - change its own parent or the parent of its siblings or ancestors
    // - add new widgets as siblings or ancestors
    //
    // Use deferred events for safety
    void UpdateFrame(float deltaTime);

    // Attach or detach child widget, parent is responsible for deleting its children
    void AttachChild(UiWidget* widget);
    void DetachChild(UiWidget* widget);
    void DetachSelf();

    // mark widget for deletion
    // all attached child widgets will be deleted as well
    void DeleteWidget();

    // Set widget enabled and visibility state
    void SetEnabled(bool isEnabled);
    void SetVisible(bool isEnabled);

    // Test whether widget is visible and Enabled
    bool IsVisibleSelf() const { return mVisible; }
    bool IsEnabledSelf() const { return mEnabled; }
    bool IsVisibleInHierarchy() const { return mVisible && mVisible_Inherited; }
    bool IsEnabledInHierarchy() const { return mEnabled && mEnabled_Inherited; }

    // Test whether widget is hovered and focused
    bool IsHovered() const;
    bool IsFocused() const;

    inline bool IsInteractive() const { return mInteractive; }

    void Subscribe(UiEventListener* listener);
    void Unsubscribe(UiEventListener* listener);

    // Process widget inputs
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);

    // accessing user data
    inline UiUserData& UserData() { return mUserData; }
    inline const UiUserData& UserData() const { return mUserData; }

    // Load widget properties from json
    virtual void Deserialize(const JsonElement& jsonElement);

    // Find child widget at specified screen coordinate, ignores invisible widgets
    // Return self if no one child can be picked
    UiWidget* PickWidget(const Point2D& screenPosition);

    // Get next sibling widget
    UiWidget* NextSibling() const;
    UiWidget* PrevSibling() const;
    UiWidget* FirstChild() const;
    UiWidget* LastChild() const;
    UiWidget* GetParent() const;
    UiWidget* GetChild(const std::string_view& name) const;
    UiWidget* GetChild(int index) const;

    UiWidget* FindChildWithName(const std::string_view& name) const;

    // deep clone
    UiWidget* CloneWidget() const;

    // accessing children
    inline cxx::span<UiWidget*> GetChildren() const { return mChildren; }

    // Get widget position and dimensions
    inline const Point2D& GetPosition() const { return mPosition; }
    // get current position in screen space
    inline Point2D GetScreenPosition() const
    {
        Point2D screenPosition = LocalToScreen(mPosition);
        return screenPosition;
    }
    const Point2D& GetSize() const { return mSize; }

    inline Rect2D GetParentSpaceBounds() const
    {
        Rect2D rcBounds { mPosition.x, mPosition.y, mSize.x, mSize.y };
        return rcBounds;
    }

    inline Rect2D GetLocalBounds() const
    {
        Rect2D rcBounds (0, 0, mSize.x, mSize.y);
        return rcBounds;
    }

    // Set widget position relative to parent
    void SetPosition(const Point2D& position);

    // Set widget dimensions
    void SetSize(const Point2D& newSize);

    void SetScale(const glm::vec2& newScale);
    void SetScale(float uniformScale)
    {
        SetScale({uniformScale, uniformScale});
    }
    const glm::vec2& GetScale() const { return mScale; }

    // Enable or disable children widgets clipping feature
    void SetClipChildren(bool isEnabled);

    // get widget name and class name
    inline const std::string& GetName() const { return mName; }
    inline const std::string& GetClassName() const { return mClassName; }

    // test whether screen space point is within widget rect
    bool IsScreenPointInsideRect(const Point2D& screenPosition) const;

    // convert from local to screen space and vice versa
    Point2D LocalToScreen(const Point2D& position) const;
    Point2D ScreenToLocal(const Point2D& position) const;

    // force update transformations or invalidate current state
    void ComputeTransform();
    void InvalidateTransform();

    // temporary
    void FitLayoutToScreen(const Point2D& screenSize);

protected:
    UiWidget(const std::string& className);
    UiWidget(const UiWidget& sourceWidget); // clone props
    virtual ~UiWidget();

    virtual UiWidget* CloneSelf() const;

    // internals
    void SetParentWidget(UiWidget* theParentWidget);
    void PositionChanged(const Point2D& prevPosition);
    void SizeChanged(const Point2D& prevSize);
    void ScaleChanged(const glm::vec2& prevScale);
    void ParentSizeChanged(const Point2D& newParentSize);

    bool RecomputeLocalPivotPoint();
    bool RecomputeAnchorPoints(const Point2D& parentSize);

    inline bool StretchingHorz() const { return mAnchorPointMax.x > mAnchorPointMin.x; }
    inline bool StretchingVert() const { return mAnchorPointMax.y > mAnchorPointMin.y; }

    Point2D ComputeNewPosition(const Point2D& desiredPosition) const;
    Point2D ComputeNewSize(const Point2D& desiredSize) const;

    // overridable
    virtual void RenderSelf(UiRenderContext& uiRenderContext) {}
    virtual void PreUpdateSelf(float deltaTime) {}
    virtual void UpdateSelf(float deltaTime) {}
    virtual void HandleChildAttached(UiWidget* widget) {}
    virtual void HandleChildDetached(UiWidget* widget) {}
    virtual void HandleEnableStateChanged() {}
    virtual void HandleVisibilityChanged() {}
    virtual void HandlePositionChanged(const Point2D& prevPosition) {}
    virtual void HandleSizeChanged(const Point2D& prevSize) {}
    virtual void HandleFocusGained() {}
    virtual void HandleFocusLost() {}
    virtual void HandleMouseEnter() {}
    virtual void HandleMouseLeave() {}
    virtual void HandleInputEvent(MouseButtonInputEvent& inputEvent) {}
    virtual void HandleInputEvent(KeyInputEvent& inputEvent) {}
    virtual void HandleInputEvent(MouseMovedInputEvent& inputEvent) {}
    virtual void HandleInputEvent(MouseScrollInputEvent& inputEvent) {}
    virtual void HandleInputEvent(KeyCharEvent& inputEvent) {}

private:
    void UpdateInheritedVisibilityState();
    void UpdateInheritedEnabledState();

    //////////////////////////////////////////////////////////////////////////
    struct ScopedChildrenLocker
    {
    public:
        ScopedChildrenLocker(UiWidget& widget);
        ~ScopedChildrenLocker();
    public:
        UiWidget& mOwner;
    };

    inline void CheckChildrenUnLocked()
    {
        cxx_assert(mChildrenLocksCount == 0);
    }
    //////////////////////////////////////////////////////////////////////////
        
protected:
    ListenersContainer<UiEventListener> mEventListeners;

    std::string mName;

    UiUserData mUserData;

    UiWidget* mParent = nullptr;
    std::vector<UiWidget*> mChildren;

    Point2D mSize;
    Point2D mPosition;

    Point2D mPivotPoint {};     // computed
    Point2D mAnchorPointMin {}; // computed
    Point2D mAnchorPointMax {}; // computed

    glm::vec2 mScale;
    glm::vec2 mRelativePivot {};
    glm::vec2 mAnchorMin {};
    glm::vec2 mAnchorMax {};

    glm::mat4 mTransform; // current transformations matrix, screen space

    bool mVisible;
    bool mVisible_Inherited;
    bool mEnabled;
    bool mEnabled_Inherited;
    bool mEnablePickChildren; // prevent children widgets from mouse interaction
    bool mEnableClipChildren; // clipping enabled
    bool mTransformInvalidated = true; // transformations matrix dirty
    bool mInteractive = true; // process input events

    int mChildrenLocksCount = 0;
};
