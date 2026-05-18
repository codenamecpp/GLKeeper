#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObjectController.h"

//////////////////////////////////////////////////////////////////////////

class ChickenObjectController: public GameObjectController
{
public:
    ChickenObjectController() = default;

    // override GameObjectController
    void ConfigureInstance(GameObject* objectInstance) override;
    void SpawnInstance() override;
    void DespawnInstance() override;
    void UpdateLogic(float stepDeltaTime) override;
    // pool
    void OnRecycle() override;

protected:
    bool TrySelectRandomMovePoint(glm::vec2& nextMovePoint);
    bool TryStartMoveToNextMovePoint(const glm::vec2& nextMovePoint);

    void StartIdle();
    void StartPecking();
    void Die();

    void StartHatching();

    void InitEgg();
    void InitChicken();

protected:
    int mPeckingChangeDirectionCountdown = 0;
    int mPeckingChangeDirectionTicks = 0;
    int mLifetimeTicks = 0; // testing
    int mTicksBeforeHatch = 0;
};