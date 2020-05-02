#pragma once

#include "Texture2D.h"

// forwards
class GuiHierarchy;
class GuiRenderer;
class GuiWidget;
class GuiButton;
class GuiPictureBox;
class GuiPanel;
class GuiLayout;
class GuiScreen;
using GuiWidgetHandle = cxx::handle<GuiWidget>;
struct GuiEvent;

using GuiUserData = void*; // widget custom data

enum eGuiLayout
{
    eGuiLayout_None,
    eGuiLayout_SimpleGrid,
};
decl_enum_strings(eGuiLayout);

enum eGuiLayoutOrientation
{
    eGuiLayoutOrientation_Horizontal,
    eGuiLayoutOrientation_Vertical,
};
decl_enum_strings(eGuiLayoutOrientation);

enum eGuiVertAlignment
{
    eGuiVertAlignment_None,
    eGuiVertAlignment_Top,
    eGuiVertAlignment_Center,
    eGuiVertAlignment_Bottom,
};
decl_enum_strings(eGuiVertAlignment);

enum eGuiSizeMode 
{
    eGuiSizeMode_Scale,
    eGuiSizeMode_ProportionalScale,
    eGuiSizeMode_Tile,
    eGuiSizeMode_Keep,
    eGuiSizeMode_KeepCentered,
    eGuiSizeMode_TileVertical,
    eGuiSizeMode_TileHorizontal,
};
decl_enum_strings(eGuiSizeMode);

enum eGuiUnits
{
    eGuiUnits_Pixels,
    eGuiUnits_Percents,
};
decl_enum_strings(eGuiUnits);

enum eGuiAnimationStatus
{
    eGuiAnimationStatus_Stopped,
    eGuiAnimationStatus_PlayingForward,
    eGuiAnimationStatus_PlayingBackward,
};
decl_enum_strings(eGuiAnimationStatus);

enum eGuiAnimationLoop
{
    eGuiAnimationLoop_None,
    eGuiAnimationLoop_PingPong,
    eGuiAnimationLoop_FromStart,
};
decl_enum_strings(eGuiAnimationLoop);

// gui widget anchors struct
struct GuiAnchors
{
public:
    GuiAnchors() = default;
    GuiAnchors(bool anchorLeft, bool anchorTop, bool anchorRight, bool anchorBottom)
        : mL(anchorLeft)
        , mT(anchorTop)
        , mR(anchorRight)
        , mB(anchorBottom)
    {
    }
public:
    bool mL = true, mT = true;
    bool mR = false, mB = false;
};

// gui textured quad
struct GuiQuadStruct
{
public:
    inline void SetupVertices(Texture2D* texture, const Rectangle& rcSrc, const Rectangle& rcQuad, Color32 color)
    {
        debug_assert(texture);

        const float invx = 1.0f / texture->mTextureDesc.mDimensions.x; // use real texture dims
        const float invy = 1.0f / texture->mTextureDesc.mDimensions.y; // use real texture dims

        // setup quad vertices in specific order
        mPoints[0].mColor       = color;
        mPoints[0].mTexcoord[0] = rcSrc.x * invx;
        mPoints[0].mTexcoord[1] = rcSrc.y * invy;
        mPoints[0].mPosition.x  = rcQuad.x * 1.0f;
        mPoints[0].mPosition.y  = rcQuad.y * 1.0f;
        mPoints[1].mColor       = color;
        mPoints[1].mTexcoord[0] = mPoints[0].mTexcoord[0];
        mPoints[1].mTexcoord[1] = (rcSrc.y + rcSrc.h) * invy;
        mPoints[1].mPosition.x  = mPoints[0].mPosition.x;
        mPoints[1].mPosition.y  = (rcQuad.y + rcQuad.h) * 1.0f;
        mPoints[2].mColor       = color;
        mPoints[2].mTexcoord[0] = (rcSrc.x + rcSrc.w) * invx;
        mPoints[2].mTexcoord[1] = mPoints[1].mTexcoord[1];
        mPoints[2].mPosition.x  = (rcQuad.x + rcQuad.w) * 1.0f;
        mPoints[2].mPosition.y  = mPoints[1].mPosition.y;
        mPoints[3].mColor       = color;
        mPoints[3].mTexcoord[0] = mPoints[2].mTexcoord[0];
        mPoints[3].mTexcoord[1] = mPoints[0].mTexcoord[1];
        mPoints[3].mPosition.x  = mPoints[2].mPosition.x;
        mPoints[3].mPosition.y  = mPoints[0].mPosition.y;
    }
    inline void SetupVertices(Texture2D* texture, const Rectangle& rcQuad, Color32 color)
    {
        debug_assert(texture);

        // setup quad vertices in specific order
        mPoints[0].mColor       = color;
        mPoints[0].mTexcoord[0] = 0.0f;
        mPoints[0].mTexcoord[1] = 0.0f;
        mPoints[0].mPosition.x  = rcQuad.x * 1.0f;
        mPoints[0].mPosition.y  = rcQuad.y * 1.0f;
        mPoints[1].mColor       = color;
        mPoints[1].mTexcoord[0] = mPoints[0].mTexcoord[0];
        mPoints[1].mTexcoord[1] = texture->mTextureDesc.mMaxV;
        mPoints[1].mPosition.x  = mPoints[0].mPosition.x;
        mPoints[1].mPosition.y  = (rcQuad.y + rcQuad.h) * 1.0f;
        mPoints[2].mColor       = color;
        mPoints[2].mTexcoord[0] = texture->mTextureDesc.mMaxU;
        mPoints[2].mTexcoord[1] = mPoints[1].mTexcoord[1];
        mPoints[2].mPosition.x  = (rcQuad.x + rcQuad.w) * 1.0f;
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

    virtual GuiWidget* CreateWidget() = 0;
};

// typical widget factory implementation
template<typename TWidgetClass>
class GuiWidgetFactory final: public GuiWidgetFactoryInterface
{
public:
    // construct widget instance on heap
    inline TWidgetClass* CreateWidget() override
    {
        return new TWidgetClass;
    }
};

// widget class descriptor, unique for each type of widgets
class GuiWidgetMetaClass
{
public:
    // widget class does not changes during runtime
    cxx::unique_string mClassName;

    // factory is responsible for constructing widgets of this class
    // it should not be null
    GuiWidgetFactoryInterface* mFactory;

    // used for rtti
    GuiWidgetMetaClass* mParentClass;

public:
    GuiWidgetMetaClass(cxx::unique_string className, GuiWidgetFactoryInterface* factory, GuiWidgetMetaClass* parentClass)
        : mClassName(className)
        , mFactory(factory)
        , mParentClass(parentClass)
    {
        debug_assert(mFactory);
    }
};
