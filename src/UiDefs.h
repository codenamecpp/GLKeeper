#pragma once

//////////////////////////////////////////////////////////////////////////

class UiRenderContext;
class UiWidget;
class UiPicture;
class UiTextBox;
class UiGridLayout;
class UiButton;
class UiPanel;
class UiWidgetManager;
template<typename TEventArgs> class UiEvent_Listener;
template<typename TEventArgs> class UiEvent;
class UiPropertyContext;
class UiPropertyLink;
class UiView;
class UiDialogManager;
class UiDialog;

//////////////////////////////////////////////////////////////////////////

struct UiUserData
{
public:
    UiUserData() = default;
    template<typename T> 
    inline T GetValue() const { return (T) mRawValue; }
    template<typename T>
    inline void SetValue(T value) 
    {
        static_assert(sizeof(T) <= sizeof(mRawValue), "Raw value data overflow");
        mRawValue = (uint64_t) value; 
    }
public:
    uint64_t mRawValue = 0;
};

//////////////////////////////////////////////////////////////////////////

enum class eDefaultFont
{
    FpsBlock,
    Console,
    Debug
};

enum eTextHorzAlignment
{
    eTextHorzAlignment_Left,
    eTextHorzAlignment_Center,
    eTextHorzAlignment_Right,
    eTextHorzAlignment_Justify,
};
enum_serialize_decl(eTextHorzAlignment);

enum eTextVertAlignment
{
    eTextVertAlignment_Top,
    eTextVertAlignment_Center,
    eTextVertAlignment_Bottom,
};

enum eUiRenderEffect
{
    eUiRenderEffect_None,
    eUiRenderEffect_Grayscale,
};

enum eUiStretchMode 
{
    eUiStretchMode_Scale,
    eUiStretchMode_Tile,
    eUiStretchMode_Keep,
    eUiStretchMode_KeepCentered,
    eUiStretchMode_TileVertical,
    eUiStretchMode_TileHorizontal,
};
enum_serialize_decl(eUiStretchMode);

enum eUiButtonState
{
    eUiButtonState_Normal,
    eUiButtonState_Hovered,
    eUiButtonState_Disabled,
    eUiButtonState_Pressed,
    eUiButtonState_COUNT
};
enum_serialize_decl(eUiButtonState);

// Grid Layout orientation
enum eUiLayoutOrientation
{
    eUiLayoutOrientation_Horizontal,
    eUiLayoutOrientation_Vertical,
    eUiLayoutOrientation_COUNT
};
enum_serialize_decl(eUiLayoutOrientation);

// Interactive layers drawing and processing order
enum eUiViewLayer
{
    eUiViewLayer_Background, // drawing at bottom
    eUiViewLayer_Foreground,
    eUiViewLayer_Loadscreen,
    eUiViewLayer_Overlay, // drawing on top
};