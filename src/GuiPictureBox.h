#pragma once

#include "GuiWidget.h"

// picture box widget
class GuiPictureBox: public GuiWidget
{
public:
    GuiPictureBox();
    GuiPictureBox(GuiWidgetClass* widgetClass);

    // set picture stretch mode
    // @param stretchMode: New mode
    void SetStretchMode(eGuiStretchMode stretchMode);

    // set picture texture
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
    eGuiStretchMode mPictureStretch = eGuiStretchMode_KeepCentered;

    std::vector<GuiQuadStruct> mQuadsCache;
};

// picturebox widget class
extern GuiWidgetClass gPictureBoxWidgetClass;