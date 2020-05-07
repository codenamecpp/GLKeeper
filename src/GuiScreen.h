#pragma once

#include "GuiDefs.h"
#include "GuiHierarchy.h"
#include "GuiAction.h"
#include "GuiEvent.h"

// interactive screen layer
class GuiScreen: public cxx::noncopyable
    , public GuiEventsHandler
    , public GuiActionContext
{
public:
    // readonly
    GuiHierarchy mHier;

public:
    GuiScreen(const std::string& screenId, const std::string& contentPath);
    virtual ~GuiScreen();

    // load internal resources
    bool LoadScreen();

    // cleanup internal resources
    void CleanupScreen();

    // activate or deacrivate screen layer, it will receive render and update events
    bool ShowScreen();
    void HideScreen();

    // test whether screen layer is initialized and active now
    bool IsScreenLoaded() const;
    bool IsScreenShown() const;

    // render screen layer
    void RenderFrame(GuiRenderer& renderContext);

    // process single frame logic
    void UpdateFrame();
    
protected:
    // overridables
    virtual void HandleScreenRender(GuiRenderer& renderContext) {}
    virtual void HandleScreenUpdate() {}
    virtual void HandleScreenCleanup() {}
    virtual void HandleScreenLoad() {}
    virtual void HandleScreenShow() {}
    virtual void HandleScreenHide() {}

private:
    void SetScreenAttached(bool isAttached);

protected:
    std::string mScreenId;
    std::string mContentPath;
};