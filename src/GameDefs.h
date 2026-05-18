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
    LoadingScenario,
    Gameplay,
};

enum_serialize_decl(eGamestate);

//////////////////////////////////////////////////////////////////////////

using TextTableId = unsigned int;
// well known text table ids
enum
{
    TextTableId_Null    = 0,
    TextTableId_Main    = 1091,
    TextTableId_Speech  = 1090,
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
