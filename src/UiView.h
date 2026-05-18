#pragma once

#include "UiDefs.h"
#include "UiHierarchy.h"

//////////////////////////////////////////////////////////////////////////
// UiView
//////////////////////////////////////////////////////////////////////////

class UiView: public cxx::noncopyable
{
public:
    const eUiViewLayer mViewLayer; // cannot be changed

public:
    UiView(eUiViewLayer viewLayer = eUiViewLayer_Background);
    virtual ~UiView();

    // Attach or detach this layer to ui system, it will receive inputs and render/update events
    bool Activate();
    void Deactivate();

    // Load layer resources and prepare to interaction
    virtual bool LoadContent();

    // Cleanup layer content
    virtual void Cleanup();

    // Processing uilayer rendering after all widgets were drawn
    virtual void RenderFrame(UiRenderContext& renderContext);

    // Processing uilayer logic after all widgets were updated
    virtual void UpdateFrame(float deltaTime);

    // Process input events
    virtual void InputEvent(MouseMovedInputEvent& inputEvent) {}
    virtual void InputEvent(MouseScrollInputEvent& inputEvent) {}
    virtual void InputEvent(MouseButtonInputEvent& inputEvent) {}
    virtual void InputEvent(KeyInputEvent& inputEvent) {}
    virtual void InputEvent(KeyCharEvent& inputEvent) {}
    virtual void ResolutionChanged();

    // Test whether current uilayer is active now
    bool IsActive() const;

    // Test whether ui layer is loaded
    inline bool IsHierarchyLoaded() const
    {
        return mHierarchy.IsLoaded();
    }
    inline const UiHierarchy& GetHierarchy() const { return mHierarchy; }

protected:
    virtual void OnActivated() {}
    virtual void OnDeactivated() {}

protected:
    UiHierarchy mHierarchy;
};
