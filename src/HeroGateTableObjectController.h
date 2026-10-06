#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObjectController.h"

//////////////////////////////////////////////////////////////////////////

class HeroGateTableObjectController: public GameObjectController
    , protected I3DMapTableLevel
{
public:
    HeroGateTableObjectController() = default;

    // override GameObjectController
    void ConfigureInstance(GameObject* objectInstance) override;
    void SpawnInstance() override;
    void DespawnInstance() override;
    void UpdateLogic(float stepDeltaTime) override;
    void UpdateFrame(float deltaTime) override;
    // pool
    void OnRecycle() override;

    // override IFrontendTablePiece
    void SelectLevel() override;
    void DeselectLevel() override;

private:
    bool mIsLevelSelected = false;
    bool mIsElevationActive = false;
};

//////////////////////////////////////////////////////////////////////////