#include "stdafx.h"
#include "TileConstructionSet.h"
#include "RoomTileConstructor_3_by_3.h"
#include "RoomTileConstructor_5_by_5.h"
#include "RoomTileConstructor_DoubleQuad.h"
#include "RoomTileConstructor_Quad.h"
#include "RoomTileConstructor_Normal.h"
#include "RoomTileConstructor_HeroGateFrontend.h"
#include "RoomTileConstructor_DungeonHeart.h"
#include "RoomTileConstructor_HeroGate_2_by_2.h"
#include "RoomTileConstructor_HeroGate_3_by_1.h"

//////////////////////////////////////////////////////////////////////////

TileConstructionSet::TileConstructionSet()
{

}

void TileConstructionSet::Initialize(const ScenarioDefinition& scenarioDef)
{
    mBaseConstructor = std::make_unique<TileConstructor>(scenarioDef);
    InitRoomConstructors();
}

void TileConstructionSet::Cleanup()
{
    mBaseConstructor.reset();
    mRoomConstructorsDefMap.clear();
    mRoomConstructorsTypeMap.clear();
    mFallbackRoomConstructor.reset();
}

template<typename T>
void TileConstructionSet::RegisterRoomConstructor(eRoomTileConstruction constructionType)
{
    mRoomConstructorsTypeMap[constructionType] = std::make_unique<T>(*mBaseConstructor);
}

template<typename T>
void TileConstructionSet::RegisterRoomConstructor(RoomDefinition* roomDef)
{
    mRoomConstructorsDefMap[roomDef] = std::make_unique<T>(*mBaseConstructor);
}


void TileConstructionSet::InitRoomConstructors()
{
    // fallback
    mFallbackRoomConstructor = std::make_unique<RoomTileConstructor>(*mBaseConstructor);

    // tile construction types
    RegisterRoomConstructor<RoomTileConstructor_5_by_5>(eRoomTileConstruction_5_by_5_Rotated);
    RegisterRoomConstructor<RoomTileConstructor_3_by_3>(eRoomTileConstruction_3_by_3);
    RegisterRoomConstructor<RoomTileConstructor_Quad>(eRoomTileConstruction_Quad);
    RegisterRoomConstructor<RoomTileConstructor_Normal>(eRoomTileConstruction_Normal);
    RegisterRoomConstructor<RoomTileConstructor_DoubleQuad>(eRoomTileConstruction_DoubleQuad);
    RegisterRoomConstructor<RoomTileConstructor_HeroGateFrontend>(eRoomTileConstruction_HeroGateFrontend);
    RegisterRoomConstructor<RoomTileConstructor_HeroGate_2_by_2>(eRoomTileConstruction_HeroGate_2_by_2);
    RegisterRoomConstructor<RoomTileConstructor_HeroGate_3_by_1>(eRoomTileConstruction_HeroGate_3_by_1);
    // todo types:

    //eRoomTileConstruction_Complete
    //eRoomTileConstruction_3_by_3_Rotated
    //eRoomTileConstruction_CenterPool
    //eRoomTileConstruction_HeroGate
    //eRoomTileConstruction_HeroGateTile

    // specific rooms
    RoomDefinition* dungeonHeartDef = mBaseConstructor->GetScenarioDefinition().GetRoomDefinition(RoomTypeId_DungeonHeart);
    RegisterRoomConstructor<RoomTileConstructor_DungeonHeart>(dungeonHeartDef);
}

RoomTileConstructor* TileConstructionSet::GetRoomConstructor(RoomDefinition* roomDef) const
{
    if (roomDef)
    {
        // search within registered room definitions
        auto defs_it = mRoomConstructorsDefMap.find(roomDef);
        if (defs_it != mRoomConstructorsDefMap.end())
        {
            return defs_it->second.get();
        }
    
        // search within registered room construction types
        auto types_it = mRoomConstructorsTypeMap.find(roomDef->mTileConstruction);
        if (types_it != mRoomConstructorsTypeMap.end())
        {
            return types_it->second.get();
        }
    }
    // fallback
    return mFallbackRoomConstructor.get();
}

RoomTileConstructor* TileConstructionSet::GetRoomConstructor(RoomTypeId roomTypeId) const
{
    const ScenarioDefinition& scenarioDef = mBaseConstructor->GetScenarioDefinition();
    return GetRoomConstructor(scenarioDef.GetRoomDefinition(roomTypeId));
}