#include "stdafx.h"
#include "GameObjectController.h"

GameObjectController::~GameObjectController()
{

}

void GameObjectController::ConfigureInstance(GameObject* objectInstance)
{
    cxx_assert(mGameObject == nullptr);
    mGameObject = objectInstance;
    cxx_assert(mGameObject);
}

void GameObjectController::SpawnInstance()
{

}

void GameObjectController::DespawnInstance()
{

}

void GameObjectController::UpdateLogic(float stepDeltaTime)
{

}

void GameObjectController::ParentRoomChanged()
{

}

void GameObjectController::OnRecycle()
{
    DespawnInstance();
    mGameObject = nullptr;
}
