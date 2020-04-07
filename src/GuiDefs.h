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

enum eGuiAddressingMode
{
    eGuiAddressingMode_Absolute,
    eGuiAddressingMode_Relative,
};
decl_enum_strings(eGuiAddressingMode);

enum eGuiWidgetAttribute
{
    eGuiWidgetAttribute_Interactive, // widget receiving mouse inputs and can be pressed or hovered
    eGuiWidgetAttribute_DragDrop, // widget supports drag and drop
};

// position along one of the axes
struct GuiPositionComponent
{
public:
    GuiPositionComponent() = default;
    GuiPositionComponent(eGuiAddressingMode addressingMode, int value)
        : mAddressingMode(addressingMode)
        , mValue(value)
    {
    }
    inline bool operator == (const GuiPositionComponent& other) const
    {
        return mAddressingMode == other.mAddressingMode && mValue == other.mValue;
    }
    inline bool operator != (const GuiPositionComponent& other) const
    {
        return mAddressingMode != other.mAddressingMode || mValue != other.mValue;
    }
public:
    eGuiAddressingMode mAddressingMode = eGuiAddressingMode_Absolute;
    int mValue = 0;
};
// alias
using GuiSizeComponent = GuiPositionComponent;

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

// drag and drop events listener
class GuiDragDropHandler
{
public:
    virtual ~GuiDragDropHandler()
    {
    }
    // on start drag
    void HandleDragStart(const Point& screenPoint)
    {
    }
    // drag interrupted
    void HandleDragCancel()
    {
    }
    // dropped
    void HandleDragDrop(const Point& screenPoint)
    {
    }
    // process drag
    void HandleDrag(const Point& screenPoint)
    {
    }
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