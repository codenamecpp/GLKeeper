#include "pch.h"
#include "NullGamestate.h"
#include "GameMain.h"
#include "GuiManager.h"
#include "GuiButton.h"
#include "GuiPictureBox.h"
#include "GraphicsDevice.h"

static GuiWidget* gRootWidget = nullptr;

void NullGamestate::HandleGamestateEnter()
{
    gRootWidget = new GuiWidget;
    gRootWidget->SetSize(gGraphicsDevice.mScreenResolution);

    {
        GuiButton* button = new GuiButton;
        button->SetPosition(Point(100, 100));
        button->SetSize(Point(100, 60));

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
            picture->SetSizeMode(eGuiSizeMode_KeepCentered);
            picture->SetSize(Point(100, 100), eGuiAddressingMode_Relative, eGuiAddressingMode_Relative);
            button->AttachChild(picture);
        }

        gRootWidget->AttachChild(button);
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