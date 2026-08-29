#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GraphicsDefs.h"
#include "GameObjectDefs.h"
#include "RoomDefs.h"
#include "PlayerDefs.h"

//////////////////////////////////////////////////////////////////////////

class GameMain;
class GameEventBus;

//////////////////////////////////////////////////////////////////////////

// limits
enum
{
    MAX_DUNGEON_MAP_DIMENSIONS = 128, // do not change
    MAX_TILE_SELECTION_RECT_WIDE = 9,
};
//////////////////////////////////////////////////////////////////////////

enum class eGamestate
{
    None,
    TitleScreen,
    LoadingFrontend,
    Frontend,
    LoadingGameplay,
    Gameplay,
};

enum_serialize_decl(eGamestate);

//////////////////////////////////////////////////////////////////////////

using TextTableId = unsigned int;
// well known text table ids
enum
{
    TextTableId_Null                = 0,

    TextTableId_Level1_Briefing     = 1000,
    TextTableId_Level2_Briefing     = 1001,
    TextTableId_Level3_Briefing     = 1002,
    TextTableId_Level4_Briefing     = 1003,
    TextTableId_Level5_Briefing     = 1004,
    TextTableId_Level6A_Briefing    = 1005,
    TextTableId_Level6B_Briefing    = 1006,
    TextTableId_Level7_Briefing     = 1007,
    TextTableId_Level8_Briefing     = 1008,
    TextTableId_Level9_Briefing     = 1009,
    TextTableId_Level10_Briefing    = 1010,
    TextTableId_Level11A_Briefing   = 1011,
    TextTableId_Level11B_Briefing   = 1012,
    TextTableId_Level11C_Briefing   = 1013,
    TextTableId_Level12_Briefing    = 1014,
    TextTableId_Level13_Briefing    = 1015,
    TextTableId_Level14_Briefing    = 1016,
    TextTableId_Level15A_Briefing   = 1017,
    TextTableId_Level15B_Briefing   = 1018,
    TextTableId_Level16_Briefing    = 1019,
    TextTableId_Level17_Briefing    = 1020,
    TextTableId_Level18_Briefing    = 1021,
    TextTableId_Level19_Briefing    = 1022,
    TextTableId_Level20_Briefing    = 1023,

    TextTableId_Mpd1_Briefing       = 1024,
    TextTableId_Mpd2_Briefing       = 1025,
    TextTableId_Mpd3_Briefing       = 1026,
    TextTableId_Mpd4_Briefing       = 1027,
    TextTableId_Mpd5_Briefing       = 1028,
    TextTableId_Mpd6_Briefing       = 1029,
    TextTableId_Mpd7_Briefing       = 1030,

    TextTableId_Secret1_Briefing    = 1031,
    TextTableId_Secret2_Briefing    = 1032,
    TextTableId_Secret3_Briefing    = 1033,
    TextTableId_Secret4_Briefing    = 1034,
    TextTableId_Secret5_Briefing    = 1035,

    TextTableId_Level1              = 1056,
    TextTableId_Level2              = 1057,
    TextTableId_Level3              = 1058,
    TextTableId_Level4              = 1059,
    TextTableId_Level5              = 1060,
    TextTableId_Level6A             = 1061,
    TextTableId_Level6B             = 1062,
    TextTableId_Level7              = 1063,
    TextTableId_Level8              = 1064,
    TextTableId_Level9              = 1065,
    TextTableId_Level10             = 1066,
    TextTableId_Level11A            = 1067,
    TextTableId_Level11B            = 1068,
    TextTableId_Level11C            = 1069,
    TextTableId_Level12             = 1070,
    TextTableId_Level13             = 1071,
    TextTableId_Level14             = 1072,
    TextTableId_Level15A            = 1073,
    TextTableId_Level15B            = 1074,
    TextTableId_Level16             = 1075,
    TextTableId_Level17             = 1076,
    TextTableId_Level18             = 1077,
    TextTableId_Level19             = 1078,
    TextTableId_Level20             = 1079,

    TextTableId_Multiplayer         = 1080,

    TextTableId_MyPetDungeon        = 1081,

    TextTableId_Secret1             = 1082,
    TextTableId_Secret2             = 1083,
    TextTableId_Secret3             = 1084,
    TextTableId_Secret4             = 1085,
    TextTableId_Secret5             = 1086,

    TextTableId_Demo1               = 1087,
    TextTableId_Demo2               = 1088,
    TextTableId_Demo3               = 1089,

    TextTableId_Speech              = 1090,
    TextTableId_Main                = 1091,
};

//////////////////////////////////////////////////////////////////////////

// economy

enum eGameResource
{
    eGameResource_Gold,
    eGameResource_Mana,
    eGameResource_COUNT
};

//////////////////////////////////////////////////////////////////////////

class GameLoadingAware: public cxx::noncopyable
{
public:
    virtual ~GameLoadingAware()
    {
    }
    // interface
    virtual void UpdateLoadingProgress(float progress) = 0;
};

//////////////////////////////////////////////////////////////////////////

class ScreenResolutionChangeAware
{
public:
    virtual ~ScreenResolutionChangeAware()
    {
    }
    // interface
    virtual void ScreenResolutionChanged() = 0;
};

//////////////////////////////////////////////////////////////////////////
