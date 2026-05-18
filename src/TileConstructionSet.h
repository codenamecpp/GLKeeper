#pragma once

//////////////////////////////////////////////////////////////////////////

#include "TileConstructor.h"
#include "RoomTileConstructor.h"

//////////////////////////////////////////////////////////////////////////

class TileConstructionSet: public cxx::noncopyable
{
public:
    TileConstructionSet();

    void Initialize(const ScenarioDefinition& scenarioDef);
    void Cleanup();

    inline TileConstructor* GetBaseConstructor() const { return mBaseConstructor.get(); }

    RoomTileConstructor* GetRoomConstructor(RoomDefinition* roomDef) const;
    RoomTileConstructor* GetRoomConstructor(RoomTypeId roomTypeId) const;

private:
    void InitRoomConstructors();

    template<typename T>
    void RegisterRoomConstructor(eRoomTileConstruction constructionType);

    template<typename T>
    void RegisterRoomConstructor(RoomDefinition* roomDef);

private:
    std::unique_ptr<TileConstructor> mBaseConstructor;
    std::map<RoomDefinition*, std::unique_ptr<RoomTileConstructor>> mRoomConstructorsDefMap;
    std::map<eRoomTileConstruction, std::unique_ptr<RoomTileConstructor>> mRoomConstructorsTypeMap;
    std::unique_ptr<RoomTileConstructor> mFallbackRoomConstructor;
};