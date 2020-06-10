#pragma once

#include "GenericRoom.h"

// temple room logic
class TempleRoom: public GenericRoom
{
public:
    TempleRoom(RoomDefinition* definition, ePlayerID owner, RoomInstanceID uid);
    ~TempleRoom();

private:
    // override GenericRoom
    void Reconfigure() override;

    // get room tiles for water pool
    void ScanWaterPoolTiles(TilesList& poolTiles);

private:
    Entity* mWaterPoolObject = nullptr;
};
