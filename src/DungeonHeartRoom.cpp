#include "pch.h"
#include "DungeonHeartRoom.h"

DungeonHeartRoom::DungeonHeartRoom(RoomDefinition* definition, ePlayerID owner, RoomInstanceID uid)
    : GenericRoom(definition, owner, uid)
{
}

DungeonHeartRoom::~DungeonHeartRoom()
{

}

void DungeonHeartRoom::Reconfigure()
{
    //if (mHeartAnimation == nullptr)
    //{
    //    mHeartAnimation = gEntityManager.CreateEntity();

    //    glm::vec3 centerPosition;
    //    Point centerTile (mOccupationArea.x + 2, mOccupationArea.y + 2);
    //    GetTerrainBlockCenter(centerTile, centerPosition);

    //    centerPosition.y = TERRAIN_BLOCK_HALF_SIZE;
    //    if (Entity* entity = gObjectsManager.CreateObjectInstance(OBJECT_NAME_DUNGEON_HEART))
    //    {
    //        entity->EnterGameWorld();
    //        entity->SetPosition(heartCentrePosition);
    //        mRoomObjects.push_back(entity);
    //    }
    //}
}