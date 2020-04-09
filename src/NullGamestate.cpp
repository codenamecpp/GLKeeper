#include "pch.h"
#include "NullGamestate.h"
#include "GameMain.h"
#include "GuiManager.h"
#include "GuiButton.h"
#include "GuiPictureBox.h"
#include "GraphicsDevice.h"
#include "GuiPanel.h"

static GuiWidget* gRootWidget = nullptr;

void NullGamestate::HandleGamestateEnter()
{
    gRootWidget = new GuiWidget;
    gRootWidget->SetSize(gGraphicsDevice.mScreenResolution);

    {
        GuiPanel* panel = new GuiPanel;
        panel->SetSize(Point(700, 400));
        panel->SetClipChildren(true);
        gRootWidget->AttachChild(panel);

        {
            GuiButton* button = new GuiButton;
            button->SetPosition(Point(100, 50), eGuiAddressingMode_Relative, eGuiAddressingMode_Relative);
            button->SetSize(Point(60, 60), eGuiAddressingMode_Relative, eGuiAddressingMode_Relative);
            button->SetOrigin(Point(50, 50), eGuiAddressingMode_Relative, eGuiAddressingMode_Relative);
            //button->SetClipChildren(true);

            button->mDebugColorDisabled = Color32_GrimGray;
            button->mDebugColorHovered = Color32_Orange;

            {
                GuiPictureBox* picture = new GuiPictureBox;
                {
                    Texture2D_Image texture_image;
                    texture_image.CreateImage(eTextureFormat_RGBA8, Point(32, 32), 0, false);
                    texture_image.FillWithCheckerBoard();

                    Texture2D* texture = new Texture2D("dummy");
                    texture->CreateTexture(texture_image);

                    picture->SetTexture(texture);
                }
                picture->SetSizeMode(eGuiSizeMode_Scale);
                picture->SetSize(Point(10, 10), eGuiAddressingMode_Relative, eGuiAddressingMode_Relative);
                picture->SetPosition(Point(-40, 50), eGuiAddressingMode_Absolute, eGuiAddressingMode_Relative);
                button->AttachChild(picture);
            }

            panel->AttachChild(button);
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