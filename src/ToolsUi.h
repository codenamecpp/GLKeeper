#pragma once

//////////////////////////////////////////////////////////////////////////

// forwards
struct ImGuiIO;

//////////////////////////////////////////////////////////////////////////

class ToolsUi: public cxx::noncopyable
{
public:
    virtual ~ToolsUi()
    {
    }

    // process window logic
    // @param imguiContext: Internal imgui context
    virtual void DoUI(ImGuiIO& imguiContext, float deltaTime) = 0;

    // show/hide ui
    inline bool IsActive() const { return mIsActive; }
    void SetActive(bool isActive) 
    {
        if (mIsActive == isActive) return;
        mIsActive = isActive;
        if (mIsActive)
        {
            OnActivateUi();
        }
        else
        {
            OnDeactivateUi();
        }
    }

protected:
    // overridables
    virtual void OnActivateUi() {}
    virtual void OnDeactivateUi() {}

private:
    bool mIsActive = false;
};