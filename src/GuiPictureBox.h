#pragma once

#include "GuiWidget.h"

// picture box widget
class GuiPictureBox: public GuiWidget
{
public:

    static GuiWidgetMetaClass MetaClass; // picturebox widget class

public:
    GuiPictureBox();

    // setup size mode for current picture
    // @param sizeMode: New mode
    void SetScaleMode(eGuiSizeMode sizeMode);

    // setup texture
    // @param texture: New texture
    void SetTexture(Texture2D* texture);

protected:
    GuiPictureBox(GuiWidgetMetaClass* widgetClass);
    GuiPictureBox(GuiPictureBox* copyWidget);

    // process drawable quads
    void InvalidateCache();
    void GenerateQuads();

    // override GuiWidget
    void HandleLoadProperties(cxx::json_node_object documentNode) override;
    void HandleRender(GuiRenderer& renderContext) override;
    void HandleSizeChanged(const Point& prevSize) override;
    GuiPictureBox* CreateClone() override;

protected:
    Texture2D* mTexture = nullptr;
    eGuiSizeMode mSizeMode = eGuiSizeMode_KeepCentered;

    std::vector<GuiQuadStruct> mQuadsCache;
};