#pragma once

#include "GuiDefs.h"
#include "GuiHierarchy.h"

// interactive screen layer
class GuiScreen: public cxx::noncopyable
{
public:
    // readonly
    GuiHierarchy mHier;

public:
    virtual ~GuiScreen();

    // preload internal resources
    bool InitializeScreen();

    // cleanup internal resources
    void CleanupScreen();

    // test whether screen layer is initialized 
    bool IsScreenInitialized() const { return mIsInitialized; }

    // attach or detach screen layer to gui system, it will receive render and update events
    bool ActivateScreen();
    void DeactivateScreen();

    // test whether screen layer is active now
    bool IsScreenActive() const { return mIsActive; }

    // render screen layer
    void RenderFrame(GuiRenderer& renderContext);

    // process single frame logic
    void UpdateFrame();

protected:
    void SetScreenAttached(bool isAttached);
    
protected:
    // overridables
    virtual void HandleRenderScreen(GuiRenderer& renderContext) {}
    virtual void HandleUpdateScreen() {}
    virtual void HandleCleanupScreen() = 0;
    virtual bool HandleInitializeScreen() = 0;
    virtual void HandleStartScreen() {}
    virtual void HandleEndScreen() {}

protected:
    bool mIsInitialized = false;
    bool mIsActive = false;
};