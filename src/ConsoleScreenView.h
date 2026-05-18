#pragma once

#include "UiView.h"
#include "Font.h"

//////////////////////////////////////////////////////////////////////////
// UiDebugConsoleScreen
//////////////////////////////////////////////////////////////////////////

class ConsoleScreenView: public UiView
    , private IConsoleListener
{
public:
    ConsoleScreenView();

    // Show or hide console screen with animation
    void ToggleConsole();

    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void UpdateFrame(float deltaTime) override;
    void RenderFrame(UiRenderContext& renderContext) override;
    void InputEvent(KeyInputEvent& inputEvent) override;
    void InputEvent(KeyCharEvent& inputEvent) override;

private:
    void OnConsoleTextChange();
    void ScrollConsole(int delta);

    // override UiView
    void OnActivated() override;
    void OnDeactivated() override;

    // override IConsoleListener
    void OnConsoleMessagesAdded(Console* sender) override;

private:

    enum CurrentShowStatus
    {
        CurrentShowStatus_FadeIn,
        CurrentShowStatus_FadeOut,
        CurrentShowStatus_Open,
    };

    CurrentShowStatus mShowStatus;
    Font* mConsoleFont = nullptr;
    Rect2D mConsoleRect;
    std::vector<Quad2D> mHistoryTextBatch;
    std::vector<Quad2D> mBufferTextBatch;
    float mFadeProgress;
    int mConsoleScroll;
    int mHistoryCursor;
    bool mConsoleTextInvalidated;
};