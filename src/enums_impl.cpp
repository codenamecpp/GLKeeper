#include "stdafx.h"
#include "RenderDevice.h"
#include "GameDefs.h"
#include "ScenarioDefs.h"
#include "UiDefs.h"

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(CreatureAnimationID)
{
    {CreatureAnimation_Walk, "walk"},
    {CreatureAnimation_Run, "run"},
    {CreatureAnimation_Dragged_Pose, "dragged_pose"},
    {CreatureAnimation_Recoil_Forwards, "recoil_forwards"},
    {CreatureAnimation_Melee, "melee"},
    {CreatureAnimation_Magic, "magic"},
    {CreatureAnimation_Die, "die"},
    {CreatureAnimation_Happy, "happy"},
    {CreatureAnimation_Angry, "angry"},
    {CreatureAnimation_Stunned_Pose, "stunned_pose"},
    {CreatureAnimation_Swing, "swing"},
    {CreatureAnimation_Sleep_Pose, "sleep_pose"},
    {CreatureAnimation_Eat, "eat"},
    {CreatureAnimation_Research, "research"},
    {CreatureAnimation_Null_NotUsed1, "unused1"},
    {CreatureAnimation_Dejected_Pose, "dejected_pose"},
    {CreatureAnimation_Torture, "torture"},
    {CreatureAnimation_Null_NotUsed2, "unused2"},
    {CreatureAnimation_Drink, "drink"},
    {CreatureAnimation_Idle1, "idle1"},
    {CreatureAnimation_Recoil_Backwards, "recoil_backwards"},
    {CreatureAnimation_Building, "building"},
    {CreatureAnimation_Pray, "pray"},
    {CreatureAnimation_Fallback, "fallback"},
    {CreatureAnimation_Elec, "elec"},
    {CreatureAnimation_Electrocute, "electrocute"},
    {CreatureAnimation_Getup, "getup"},
    {CreatureAnimation_Dance, "dance"},
    {CreatureAnimation_Drunk1, "drunk1"},
    {CreatureAnimation_Entrance, "entrance"},
    {CreatureAnimation_Idle2, "idle2"},
    {CreatureAnimation_Special1, "special1"},
    {CreatureAnimation_Special2, "special2"},
    {CreatureAnimation_Drunk2, "drunk2"},
    {CreatureAnimation_Special3, "special3"},
    {CreatureAnimation_Null_NotUsed3, "unused3"},
    {CreatureAnimation_DrunkIdle, "drunk_idle"},
    {CreatureAnimation_Melee2, "melee2"},
    {CreatureAnimation_Special4, "special4"},
    {CreatureAnimation_Special5, "special5"},
    {CreatureAnimation_Special6, "special6"},
    {CreatureAnimation_Special7, "special7"},
    {CreatureAnimation_Special8, "special8"},
    {CreatureAnimation_WalkBack, "walk_back"},
    {CreatureAnimation_Pose_Frame, "pose_frame"},
    {CreatureAnimation_Walk2, "walk2"},
    {CreatureAnimation_Die_Pose, "die_pose"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(eGameObjectCategory)
{
    {eGameObjectCategory_Normal, "normal"},
    {eGameObjectCategory_Special, "special"},
    {eGameObjectCategory_SpellBook, "spell_book"},
    {eGameObjectCategory_Crate, "crate"},
    {eGameObjectCategory_Lair, "lair"},
    {eGameObjectCategory_Gold, "gold"},
    {eGameObjectCategory_Food, "food"},
    {eGameObjectCategory_LevelGem, "level_gem"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(ePlayerID)
{
    {ePlayerID_Null, "null"},
    {ePlayerID_Good, "good"},
    {ePlayerID_Neutral, "neutral"},
    {ePlayerID_Keeper1, "keeper1"},
    {ePlayerID_Keeper2, "keeper2"},
    {ePlayerID_Keeper3, "keeper3"},
    {ePlayerID_Keeper4, "keeper4"},
    {ePlayerID_Keeper5, "keeper5"},
    {ePlayerID_Keeper6, "keeper6"},
    {ePlayerID_Keeper7, "keeper7"},
    {ePlayerID_Keeper8, "keeper8"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(ePlayerType)
{
    {ePlayerType_Null, "null"},
    {ePlayerType_Human, "human"},
    {ePlayerType_AI, "ai"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(eUiLayoutOrientation)
{
    {eUiLayoutOrientation_Horizontal, "horizontal"},
    {eUiLayoutOrientation_Vertical, "vertical"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(eTextHorzAlignment)
{
    {eTextHorzAlignment_Left, "left"},
    {eTextHorzAlignment_Center, "center"},
    {eTextHorzAlignment_Right, "right"},
    {eTextHorzAlignment_Justify, "justify"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(eUiButtonState)
{
    {eUiButtonState_Normal, "state_normal"},
    {eUiButtonState_Hovered, "state_hover"},
    {eUiButtonState_Disabled, "state_disabled"},
    {eUiButtonState_Pressed, "state_pressed"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(eUiStretchMode)
{
    {eUiStretchMode_Scale, "scale"},
    {eUiStretchMode_Tile, "tile"},
    {eUiStretchMode_Keep, "keep"},
    {eUiStretchMode_KeepCentered, "keep_centered"},
    {eUiStretchMode_TileVertical, "tile_vertical"},
    {eUiStretchMode_TileHorizontal, "tile_horizontal"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(eLogLevel)
{
    {eLogLevel_Debug, "debug"},
    {eLogLevel_Info, "info"},
    {eLogLevel_Warning, "warning"},
    {eLogLevel_Error, "error"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(eArtResource)
{
    {eArtResource_Null, "null"},
    {eArtResource_Sprite, "sprite"},
    {eArtResource_Alpha, "alpha"},
    {eArtResource_AdditiveAlpha, "additive alpha"},
    {eArtResource_TerrainMesh, "terrain mesh"},
    {eArtResource_Mesh, "mesh"},
    {eArtResource_AnimatingMesh, "animating mesh"},
    {eArtResource_ProceduralMesh, "procedural mesh"},
    {eArtResource_MeshCollection, "mesh collection"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(eGamestate)
{
    {eGamestate::None, "none"},
    {eGamestate::LoadingFrontend, "loading_frontend"},
    {eGamestate::Frontend, "frontend"},
    {eGamestate::LoadingScenario, "loading_scenario"},
    {eGamestate::Gameplay, "gameplay"},
};

//////////////////////////////////////////////////////////////////////////

enum_serialize_impl(ePixelFormat)
{
    {ePixelFormat_Null, "null"},
    {ePixelFormat_R8, "r8"},
    {ePixelFormat_R8_G8, "r8_g8"},
    {ePixelFormat_RGB8, "rgb8"},
    {ePixelFormat_RGBA8, "rgba8"},
    {ePixelFormat_R8UI, "r8ui"},
    {ePixelFormat_RGBA8UI, "rgba8ui"},
    {ePixelFormat_R16UI, "r16ui"},
};

enum_serialize_impl(eVertexAttributeFormat)
{
    {eVertexAttributeFormat_Null, "null"},
    {eVertexAttributeFormat_2F, "2f"},
    {eVertexAttributeFormat_3F, "3f"},
    {eVertexAttributeFormat_4F, "4f"},
    {eVertexAttributeFormat_4UB, "4ub"},
    {eVertexAttributeFormat_1US, "1us"},
    {eVertexAttributeFormat_2US, "2us"},
    {eVertexAttributeFormat_4US, "4us"},
};

enum_serialize_impl(eVertexAttribute)
{
    {eVertexAttribute_Position0, "in_position0"},
    {eVertexAttribute_Position1, "in_position1"},
    {eVertexAttribute_Texcoord0, "in_texcoord0"},
    {eVertexAttribute_Texcoord1, "in_texcoord1"},
    {eVertexAttribute_Normal0, "in_normal0"},
    {eVertexAttribute_Normal1, "in_normal1"},
    {eVertexAttribute_Color0, "in_color0"},
    {eVertexAttribute_Color1, "in_color1"},
    {eVertexAttribute_TileCoord, "in_tile_coord"},
};

//////////////////////////////////////////////////////////////////////////