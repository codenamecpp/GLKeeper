#pragma once

#include "GuiWidget.h"

// picture box widget
class GuiPictureBox: public GuiWidget
{
public:
    GuiPictureBox();

    // setup size mode for current picture
    // @param sizeMode: New mode
    void SetSizeMode(eGuiSizeMode sizeMode);

    // setup texture
    // @param texture: New texture
    void SetTexture(Texture2D* texture);

protected:
    GuiPictureBox(GuiWidgetClass* widgetClass);
    GuiPictureBox(GuiPictureBox* copyWidget);

    // process drawable quads
    void InvalidateCache();
    void GenerateQuads();

    // override GuiWidget
    void HandleRender(GuiRenderer& renderContext) override;
    void HandleSizeChanged(const Point& prevSize) override;
    GuiPictureBox* ConstructClone() override;

protected:
    Texture2D* mTexture = nullptr;
    eGuiSizeMode mSizeMode = eGuiSizeMode_KeepCentered;

    std::vector<GuiQuadStruct> mQuadsCache;
};

// picturebox widget class
extern GuiWidgetClass gPictureBoxWidgetClass;