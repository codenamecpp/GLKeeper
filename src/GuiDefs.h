#pragma once

#include "Texture2D.h"

// forwards
class GuiRenderer;
class GuiWidget;

// alias
using GuiUserData = void*; // widget custom data

enum eGuiVertAlignment
{
    eGuiVertAlignment_None,
    eGuiVertAlignment_Top,
    eGuiVertAlignment_Center,
    eGuiVertAlignment_Bottom,
};
decl_enum_strings(eGuiVertAlignment);

enum eGuiHorzAlignment
{
    eGuiHorzAlignment_None,
    eGuiHorzAlignment_Left,
    eGuiHorzAlignment_Center,
    eGuiHorzAlignment_Right,
};
decl_enum_strings(eGuiHorzAlignment);

enum eGuiOriginMode
{
    eGuiOrigin_Relative,
    eGuiOrigin_Fixed,
};
decl_enum_strings(eGuiOriginMode);

enum eGuiStretchMode 
{
    eGuiStretchMode_Scale,
    eGuiStretchMode_ProportionalScale,
    eGuiStretchMode_Tile,
    eGuiStretchMode_Keep,
    eGuiStretchMode_KeepCentered,
    eGuiStretchMode_TileVertical,
    eGuiStretchMode_TileHorizontal,
};
decl_enum_strings(eGuiStretchMode);

// gui widget anchors struct
struct GuiAnchorsStruct
{
public:
    GuiAnchorsStruct() = default;
    GuiAnchorsStruct(bool anchorLeft, bool anchorTop, bool anchorRight, bool anchorBottom)
        : mLeft(anchorLeft)
        , mTop(anchorTop)
        , mRight(anchorRight)
        , mBottom(anchorBottom)
    {
    }
public:
    bool mLeft = true, mTop = true;
    bool mRight = false, mBottom = false;
};

// gui textured quad
struct GuiQuadStruct
{
public:
    inline void SetupVertices(Texture2D* texture, const Rect2D& rcSrc, const Rect2D& rcDest, Color32 color)
    {
        //debug_assert(texture);

        //const float invx = 1.0f / texDims.x;
        //const float invy = 1.0f / texDims.y;

        //// setup quad vertices in specific order
        //mPoints[0].mColor       = theColor;
        //mPoints[0].mTexcoord[0] = rcSrc.x * invx;
        //mPoints[0].mTexcoord[1] = rcSrc.y * invy;
        //mPoints[0].mXyz.x       = rcDest.x * 1.0f;
        //mPoints[0].mXyz.y       = rcDest.y * 1.0f;
        //mPoints[1].mColor       = theColor;
        //mPoints[1].mTexcoord[0] = mPoints[0].mTexcoord[0];
        //mPoints[1].mTexcoord[1] = (rcSrc.y + rcSrc.h) * invy;
        //mPoints[1].mXyz.x       = mPoints[0].mXyz.x;
        //mPoints[1].mXyz.y       = (rcDest.y + rcDest.h) * 1.0f;
        //mPoints[2].mColor       = theColor;
        //mPoints[2].mTexcoord[0] = (rcSrc.x + rcSrc.w) * invx;
        //mPoints[2].mTexcoord[1] = mPoints[1].mTexcoord[1];
        //mPoints[2].mXyz.x       = (rcDest.x + rcDest.w) * 1.0f;
        //mPoints[2].mXyz.y       = mPoints[1].mXyz.y;
        //mPoints[3].mColor       = theColor;
        //mPoints[3].mTexcoord[0] = mPoints[2].mTexcoord[0];
        //mPoints[3].mTexcoord[1] = mPoints[0].mTexcoord[1];
        //mPoints[3].mXyz.x       = mPoints[2].mXyz.x;
        //mPoints[3].mXyz.y       = mPoints[0].mXyz.y;
    }
    inline void SetupVertices(Texture2D* texture, const Rect2D& rcQuad, Color32 color)
    {
        debug_assert(texture);

        // setup quad vertices in specific order
        mPoints[0].mColor       = color;
        mPoints[0].mTexcoord[0] = 0.0f;
        mPoints[0].mTexcoord[1] = 0.0f;
        mPoints[0].mPosition.x  = rcQuad.mX * 1.0f;
        mPoints[0].mPosition.y  = rcQuad.mY * 1.0f;
        mPoints[1].mColor       = color;
        mPoints[1].mTexcoord[0] = mPoints[0].mTexcoord[0];
        mPoints[1].mTexcoord[1] = texture->mTextureDesc.mMaxV;
        mPoints[1].mPosition.x  = mPoints[0].mPosition.x;
        mPoints[1].mPosition.y  = (rcQuad.mY + rcQuad.mSizeY) * 1.0f;
        mPoints[2].mColor       = color;
        mPoints[2].mTexcoord[0] = texture->mTextureDesc.mMaxU;
        mPoints[2].mTexcoord[1] = mPoints[1].mTexcoord[1];
        mPoints[2].mPosition.x  = (rcQuad.mX + rcQuad.mSizeX) * 1.0f;
        mPoints[2].mPosition.y  = mPoints[1].mPosition.y;
        mPoints[3].mColor       = color;
        mPoints[3].mTexcoord[0] = mPoints[2].mTexcoord[0];
        mPoints[3].mTexcoord[1] = mPoints[0].mTexcoord[1];
        mPoints[3].mPosition.x  = mPoints[2].mPosition.x;
        mPoints[3].mPosition.y  = mPoints[0].mPosition.y;
    }
public:
    // vertices has specific order:
    // 0 - TOP LEFT
    // 1 - BOTTOM LEFT
    // 2 - BOTTOM RIGHT
    // 3 - TOP RIGHT
    Vertex2D mPoints[4];
};

// creates widget instance of specific class
class GuiWidgetFactoryInterface
{
public:
    virtual ~GuiWidgetFactoryInterface()
    {
    }

    virtual GuiWidget* ConstructWidget() = 0;
};

// typical widget factory implementation
template<typename TWidgetClass>
class GuiWidgetFactory final: public GuiWidgetFactoryInterface
{
public:
    // Construct widget instance on heap
    inline TWidgetClass* ConstructWidget() override
    {
        return new TWidgetClass;
    }
};

// widget class descriptor, unique for each type of widgets
class GuiWidgetClass
{
public:
    // widget class does not changes during runtime
    const std::string mClassName;

    // factory is responsible for constructing widgets of this class
    // it should not be null
    GuiWidgetFactoryInterface* mFactory;

public:
    GuiWidgetClass(std::string&& className, GuiWidgetFactoryInterface* factory)
        : mClassName(className)
        , mFactory(factory)
    {
        debug_assert(mFactory);
    }
};