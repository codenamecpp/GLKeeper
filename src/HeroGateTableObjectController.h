#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObjectController.h"

//////////////////////////////////////////////////////////////////////////

class HeroGateTableObjectController: public GameObjectController
{
public:
    HeroGateTableObjectController() = default;

    // override GameObjectController
    void ConfigureInstance(GameObject* objectInstance) override;
    void SpawnInstance() override;
    void DespawnInstance() override;
    void UpdateLogic(float stepDeltaTime) override;

    // pool
    void OnRecycle() override;

private:

};

//////////////////////////////////////////////////////////////////////////