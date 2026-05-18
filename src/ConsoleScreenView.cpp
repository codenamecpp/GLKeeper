#include "stdafx.h"
#include "ConsoleScreenView.h"
#include "UiTextBox.h"
#include "UiPanel.h"
#include "UiWidgetManager.h"
#include "UiRenderContext.h"
#include "UiManager.h"

//////////////////////////////////////////////////////////////////////////

#define CONSOLE_SEPARATOR_STR   L". . . . . . . . . . . . . . . . . . . . . . . . . . . . ."
#define UI_CONSOLE_FADE_SPEED   12.0f

enum 
{ 
    UI_CONSOLE_NUM_TEXTLINES = 21, 
    UI_CONSOLE_X_OFFSET = 6,
    UI_CONSOLE_CURSOR_BLINKING_TIME_MSECONDS = 1200,
    UI_CONSOLE_MAX_INPUT_HISTORY_RECORDS = 32,
};

//////////////////////////////////////////////////////////////////////////

ConsoleScreenView::ConsoleScreenView()
    : UiView(eUiViewLayer_Overlay)
{
}

void ConsoleScreenView::ToggleConsole()
{
    if (IsActive())
    {
        mShowStatus = CurrentShowStatus_FadeOut;
        return;
    }

    Activate();
}

bool ConsoleScreenView::LoadContent()
{
    if (IsHierarchyLoaded())
        return true;

    mConsoleFont = gUiManager.GetDefaultFont(eDefaultFont::Console);
    cxx_assert(mConsoleFont);
    if (mConsoleFont)
    {
        mConsoleTextInvalidated = true;
        mConsoleScroll = 0;
        mHistoryCursor = -1;

        const GameProfile::UserSettings& userSettings = gGameProfile.GetUserSettings();
        // setup console dimensions
        mConsoleRect.x = 0;
        mConsoleRect.y = 0;
        mConsoleRect.w = userSettings.mScreenResolution.x;
        mConsoleRect.h = (UI_CONSOLE_NUM_TEXTLINES + 1) * mConsoleFont->GetLineHeight();
    }
    return true;
}

void ConsoleScreenView::Cleanup()
{
    UiView::Cleanup();

    mHistoryTextBatch.clear();
}

void ConsoleScreenView::RenderFrame(UiRenderContext& renderContext)
{
    // animate with transform
    if (mShowStatus != CurrentShowStatus_Open)
    {
        glm::vec3 translation { 0.0f, (mFadeProgress * mConsoleRect.h) - mConsoleRect.h, 0.0f };
        glm::mat4 translation_matrix = glm::translate(translation);
        renderContext.SetTransform(&translation_matrix);
    }

    renderContext.FillRect(mConsoleRect, MAKE_RGBA(28, 28, 28, 190));
    renderContext.DrawTextQuads(mConsoleFont, mHistoryTextBatch);

    Rect2D frameRect = mConsoleRect;
    frameRect.y -= 10;
    frameRect.h += 10;
    renderContext.DrawRect(frameRect, MAKE_RGBA(176, 163, 95, 255));

    renderContext.SetTransform(nullptr);
}

void ConsoleScreenView::UpdateFrame(float deltaTime)
{
    if (mShowStatus == CurrentShowStatus_FadeIn)
    {
        mFadeProgress += UI_CONSOLE_FADE_SPEED * deltaTime;
        if (mFadeProgress >= 1.0f)
        {
            mFadeProgress = 1.0f;
            mShowStatus = CurrentShowStatus_Open;
        }
        return;
    }

    if (mShowStatus == CurrentShowStatus_FadeOut)
    {
        mFadeProgress -= UI_CONSOLE_FADE_SPEED * deltaTime;
        if (mFadeProgress <= 0.0f)
        {
            mFadeProgress = 0.0f;
            Deactivate();
        }
        return;
    }

    if (mConsoleTextInvalidated)
    {
        OnConsoleTextChange();
        mConsoleTextInvalidated = false;
    }
}

void ConsoleScreenView::InputEvent(KeyCharEvent& inputEvent)
{
    inputEvent.SetConsumed();
}

void ConsoleScreenView::InputEvent(KeyInputEvent& inputEvent)
{
    inputEvent.SetConsumed();

    if (!inputEvent.mPressed)
        return;

    if (inputEvent.mKeycode == KEYCODE_PAGE_UP)
    {
        ScrollConsole(1);
    }

    if (inputEvent.mKeycode == KEYCODE_PAGE_DOWN)
    {
        ScrollConsole(-1);
    }
}

void ConsoleScreenView::ScrollConsole(int delta)
{
    const int ConsoleNumLines = gConsole.mRecords.size();
    if (ConsoleNumLines <= UI_CONSOLE_NUM_TEXTLINES)
        return;

    int newScroll = mConsoleScroll + delta;
    if (newScroll < 0)
    {
        newScroll = 0;
    }

    if (newScroll > (ConsoleNumLines - UI_CONSOLE_NUM_TEXTLINES))
    {
        newScroll = ConsoleNumLines - UI_CONSOLE_NUM_TEXTLINES;
    }

    if (mConsoleScroll == newScroll)
        return;

    mConsoleScroll = newScroll;
    mConsoleTextInvalidated = true;
}

void ConsoleScreenView::OnActivated()
{
    gConsole.Subscribe(this);

    mConsoleScroll = 0;
    mHistoryCursor = -1;
    mShowStatus = CurrentShowStatus_FadeIn;
    mFadeProgress = 0.0f;

    OnConsoleTextChange();
    mConsoleTextInvalidated = false;
}

void ConsoleScreenView::OnDeactivated() 
{
    gConsole.Unsubscribe(this);
}

void ConsoleScreenView::OnConsoleMessagesAdded(Console* sender)
{
    mConsoleTextInvalidated = true;
}

void ConsoleScreenView::OnConsoleTextChange()
{
    mHistoryTextBatch.clear();

    int MaxLines = UI_CONSOLE_NUM_TEXTLINES;
    if (mConsoleScroll)
    {
        MaxLines = UI_CONSOLE_NUM_TEXTLINES - 1;
    }

    if (mConsoleFont == nullptr) 
        return;

    mBufferTextBatch.clear();
    mBufferTextBatch.reserve(1024);

    for (int iline = 0, consoleLines = gConsole.mRecords.size(); 
        iline < MaxLines && iline < consoleLines; ++iline)
    {
        int recordIndex = consoleLines - (iline + mConsoleScroll) - 1;
        if (mConsoleScroll)
        {
            recordIndex = consoleLines - (iline + mConsoleScroll) - 2;
        }

        const Console::Record& rec = gConsole.mRecords[recordIndex];

        Color32 textColor = COLOR_WHITE;
        switch (rec.mLogLevel)
        {
            case eLogLevel_Debug: textColor = COLOR_DARK_GRAY; break;
            case eLogLevel_Info: textColor = COLOR_LIGHT_GRAY; break;
            case eLogLevel_Warning: textColor = COLOR_YELLOW; break;
            case eLogLevel_Error: textColor = COLOR_RED; break;
        }
        int positiony = (UI_CONSOLE_NUM_TEXTLINES - iline - 1) * mConsoleFont->GetLineHeight();
        if (mConsoleScroll)
        {
            positiony = (UI_CONSOLE_NUM_TEXTLINES - iline - 2) * mConsoleFont->GetLineHeight();
        }
        mConsoleFont->BuildTextMesh(rec.mMessage, Point2D{ UI_CONSOLE_X_OFFSET, positiony }, textColor, mBufferTextBatch);

        // combine batch
        mHistoryTextBatch.insert(mHistoryTextBatch.end(), 
            mBufferTextBatch.begin(), 
            mBufferTextBatch.end());
        mBufferTextBatch.clear();
    }

    // add line separator
    if (mConsoleScroll)
    {
        int positiony = (UI_CONSOLE_NUM_TEXTLINES - 1) * mConsoleFont->GetLineHeight();
        mConsoleFont->BuildTextMesh(CONSOLE_SEPARATOR_STR, Point2D{ UI_CONSOLE_X_OFFSET, positiony }, COLOR_WHITE, mBufferTextBatch);

        // combine batch
        mHistoryTextBatch.insert(mHistoryTextBatch.end(), 
            mBufferTextBatch.begin(), 
            mBufferTextBatch.end());
        mBufferTextBatch.clear();
    }
}