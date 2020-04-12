#include "pch.h"
#include "LayoutsEditGamestate.h"
#include "GameMain.h"
#include "GuiManager.h"
#include "GuiButton.h"
#include "GuiPictureBox.h"
#include "GraphicsDevice.h"
#include "GuiPanel.h"
#include "GuiSlider.h"
#include "TexturesManager.h"
#include "ToolsUIManager.h"
#include "Console.h"

void LayoutsEditGamestate::HandleGamestateEnter()
{
    gToolsUIManager.AttachWindow(&mLayoutsEditWindow);

    gGuiManager.AttachWidgets(&mHier);

    mHier.mRootWidget = new GuiWidget;
    mHier.mRootWidget->mName = "root";
    
    {
        GuiButton* button = new GuiButton;
        button->mName = "button_0";
        button->mDebugColorHovered = Color32_DarkGray;

        button->SetPosition(Point(200, 200));
        button->SetSize(Point(100, 100));
        Subscribe(button, eGuiEvent_Click);
        Subscribe(button, eGuiEvent_MouseDown);
        Subscribe(button, eGuiEvent_MouseUp);
        Subscribe(button, eGuiEvent_MouseEnter);
        Subscribe(button, eGuiEvent_MouseLeave);
        mHier.mRootWidget->AttachChild(button);

        {
            GuiPictureBox* picbox = new GuiPictureBox;
            picbox->mName = "picbox_0";
            picbox->SetSize(Point(85, 85), eGuiUnits_Percents, eGuiUnits_Percents);
            picbox->SetOrigin(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
            picbox->SetPosition(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
            picbox->SetTexture(gTexturesManager.mWhiteTexture);
            picbox->SetScaleMode(eGuiSizeMode_Scale);
            button->AttachChild(picbox);
        }
    }

    {
        GuiSlider* slider = new GuiSlider;
        slider->mName = "slider_0";
        slider->SetPosition(Point(400, 200));
        slider->SetSize(Point(400, 30));

        {
            GuiButton* slider_thumb = new GuiButton;
            slider_thumb->mName = "#slider";
            slider_thumb->mDebugColorHovered = Color32_DarkGray;
            slider->AttachChild(slider_thumb);
        }

        {
            GuiButton* slider_min = new GuiButton;
            slider_min->mName = "#min";
            slider_min->mDebugColorHovered = Color32_DarkGray;
            slider->AttachChild(slider_min);
        }

        {
            GuiButton* slider_max = new GuiButton;
            slider_max->mName = "#max";
            slider_max->mDebugColorHovered = Color32_DarkGray;
            slider->AttachChild(slider_max);
        }

        mHier.mRootWidget->AttachChild(slider);
    }
}

void LayoutsEditGamestate::HandleGamestateLeave()
{
    gToolsUIManager.DetachWindow(&mLayoutsEditWindow);

    UnsubscribeAll();

    gGuiManager.DetachWidgets(&mHier);
    mHier.Clear();
}

void LayoutsEditGamestate::HandleUpdateFrame()
{
}

void LayoutsEditGamestate::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleClickEvent(GuiWidget* sender)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_click %s", sender->mName.c_str());
}

void LayoutsEditGamestate::HandleMouseEnter(GuiWidget* sender)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_enter %s", sender->mName.c_str());
}

void LayoutsEditGamestate::HandleMouseLeave(GuiWidget* sender)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_leave %s", sender->mName.c_str());
}

void LayoutsEditGamestate::HandleMouseDown(GuiWidget* sender, eMouseButton mbutton)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_down %s", sender->mName.c_str());
}

void LayoutsEditGamestate::HandleMouseUp(GuiWidget* sender, eMouseButton mbutton)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_up %s", sender->mName.c_str());
}