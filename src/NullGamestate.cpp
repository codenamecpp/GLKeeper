#include "pch.h"
#include "NullGamestate.h"
#include "GameMain.h"
#include "GuiManager.h"
#include "GuiButton.h"
#include "GuiPictureBox.h"
#include "GraphicsDevice.h"
#include "GuiPanel.h"
#include "GuiSlider.h"
#include "TexturesManager.h"

static GuiWidget* gRootWidget = nullptr;

class MyHandler: public GuiEventsHandler
{
public:
    void HandleClickEvent(GuiWidget* sender, GuiEvent* eventData) override
    {
        if (mBox)
        {
            mBox->SetVisible(!mBox->IsVisible());
        }
        if (mSlider)
        {
            mSlider->SetEnabled(!mSlider->IsEnabled());
        }
    }
    void HandleMouseEnter(GuiWidget* sender, GuiEvent* eventData) override
    {
        if (mBox)
        {
            mBox->SetSize(Point(30, 30), eGuiUnits_Percents, eGuiUnits_Percents);
        }
    }
    void HandleMouseLeave(GuiWidget* sender, GuiEvent* eventData) override
    {
        if (mBox)
        {
            mBox->SetSize(Point(20, 20), eGuiUnits_Percents, eGuiUnits_Percents);
        }
    }

public:
    GuiPictureBox* mBox = nullptr;
    GuiSlider* mSlider = nullptr;
};

static MyHandler ghandler;

void NullGamestate::HandleGamestateEnter()
{
    gRootWidget = new GuiWidget;
    gRootWidget->SetSize(gGraphicsDevice.mScreenResolution);

    {
        GuiPanel* panel = new GuiPanel;
        panel->SetSize(Point(100, 100), eGuiUnits_Percents, eGuiUnits_Percents);
        gRootWidget->AttachChild(panel);

        {
            GuiSlider* slider = new GuiSlider;
            slider->SetPosition(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
            slider->SetOrigin(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
            slider->SetSize(Point(600, 70));

            GuiAnchors anchors;
            anchors.mB = true; anchors.mT = true; anchors.mL = true; anchors.mR = true;
            slider->SetAnchors(anchors);

            ghandler.mSlider = slider;
            {
                GuiPictureBox* picbox = new GuiPictureBox;
                picbox->mName = "#slider";
                picbox->SetTexture(gTexturesManager.mBlackTexture);
                picbox->SetSizeMode(eGuiSizeMode_Scale);
                //picbox->SetSize(Point(60, 60));
                slider->AttachChild(picbox);
            }

            {
                GuiButton* left_button = new GuiButton;
                left_button->SetSize(Point(60, 60));
                left_button->SetPosition(Point(35, 50), eGuiUnits_Pixels, eGuiUnits_Percents);
                left_button->SetOrigin(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
                slider->AttachChild(left_button);
            }

            {
                GuiButton* right_button = new GuiButton;
                right_button->SetSize(Point(60, 60));
                right_button->SetPosition(Point(565, 50), eGuiUnits_Pixels, eGuiUnits_Percents);
                right_button->SetOrigin(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);

                GuiAnchors anchors;
                anchors.mB = false; anchors.mT = false; anchors.mL = false; anchors.mR = true;
                right_button->SetAnchors(anchors);
                slider->AttachChild(right_button);
            }

            panel->AttachChild(slider);
        }
    }

    gGuiManager.AttachWidget(gRootWidget);
}

void NullGamestate::HandleGamestateLeave()
{
}

void NullGamestate::HandleUpdateFrame()
{
    gRootWidget->SetSize(gGraphicsDevice.mScreenResolution);
}

void NullGamestate::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void NullGamestate::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void NullGamestate::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void NullGamestate::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void NullGamestate::HandleInputEvent(KeyCharEvent& inputEvent)
{
}