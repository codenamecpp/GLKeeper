#pragma once

#include "GuiWidget.h"

// picture box widget
class GuiPictureBox: public GuiWidget
{
public:
    GuiPictureBox();
    GuiPictureBox(GuiWidgetClass* widgetClass);

    // setup size mode for current picture
    // @param sizeMode: New mode
    void SetSizeMode(eGuiSizeMode sizeMode);

    // setup texture
    // @param texture: New texture
    void SetTexture(Texture2D* texture);

    // copy properties
protected:
    GuiPictureBox(GuiPictureBox* copyWidget);

    // process drawable quads
    void InvalidateCache();
    void GenerateQuads();

    // override GuiWidget
    void HandleRenderSelf(GuiRenderer& renderContext) override;
    void HandleSizeChanged(const Size2D& prevSize) override;
    GuiPictureBox* ConstructClone() override;

protected:
    Texture2D* mTexture = nullptr;
    eGuiSizeMode mSizeMode = eGuiSizeMode_KeepCentered;

    std::vector<GuiQuadStruct> mQuadsCache;
};

// picturebox widget class
extern GuiWidgetClass gPictureBoxWidgetClass;