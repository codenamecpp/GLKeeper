#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObjectController.h"

//////////////////////////////////////////////////////////////////////////

class GoldPileObjectController: public GameObjectController
    , private GoldContainerCapability
{
public:
    GoldPileObjectController() = default;

    // override GameObjectController
    void ConfigureInstance(GameObject* objectInstance) override;
    void SpawnInstance() override;
    void DespawnInstance() override;
    void UpdateLogic(float stepDeltaTime) override;

    // pool
    void OnRecycle() override;

private:
    // override GoldContainerCapability
    long GetStoredGoldAmount() const override;
    long GetStoredGoldCapacity() const override;
    long StoreGold(long goldAmount) override;
    long DisposeGold(long goldAmount) override;

    void SetMeshFromGoldAmount();
    
private:
    MoneyComponent* mMoneyComponent = nullptr;
};