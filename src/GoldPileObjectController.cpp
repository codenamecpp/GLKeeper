#include "stdafx.h"
#include "GoldPileObjectController.h"

void GoldPileObjectController::ConfigureInstance(GameObject* objectInstance)
{
    GameObjectController::ConfigureInstance(objectInstance);

    // wire components
    mMoneyComponent = GetGameObject().GetComponent<MoneyComponent>();
    cxx_assert(mMoneyComponent);

    // setup capabilities
    GetGameObject().SetCapability<GoldContainerCapability>(this);
}

void GoldPileObjectController::SpawnInstance()
{
    GameObjectController::SpawnInstance();
    SetMeshFromGoldAmount();
}

void GoldPileObjectController::DespawnInstance()
{
    GameObjectController::DespawnInstance();
}

void GoldPileObjectController::UpdateLogic(float stepDeltaTime)
{
    GameObjectController::UpdateLogic(stepDeltaTime);
}

void GoldPileObjectController::OnRecycle()
{
    GameObjectController::OnRecycle();

    mMoneyComponent = nullptr;
}

long GoldPileObjectController::GetStoredGoldAmount() const
{
    return mMoneyComponent->mAmount;
}

long GoldPileObjectController::GetStoredGoldCapacity() const
{
    return mMoneyComponent->mCapacity;
}

long GoldPileObjectController::StoreGold(long goldAmount)
{
    long maxCapacity = GetStoredGoldCapacity();

    long oldGoldAmount = mMoneyComponent->mAmount;
    long newGoldAmount = (oldGoldAmount + goldAmount);
    if (maxCapacity > 0)
    {
        newGoldAmount = std::min(maxCapacity, newGoldAmount);
    }
    if (newGoldAmount > oldGoldAmount)
    {
        mMoneyComponent->mAmount = newGoldAmount;
        SetMeshFromGoldAmount();
        return newGoldAmount - oldGoldAmount;
    }
    return 0;
}

long GoldPileObjectController::DisposeGold(long goldAmount)
{
    long oldGoldAmount = mMoneyComponent->mAmount;
    long newGoldAmount = std::max(0L, oldGoldAmount - goldAmount);
    if (newGoldAmount < oldGoldAmount)
    {
        mMoneyComponent->mAmount = newGoldAmount;
        SetMeshFromGoldAmount();
        return oldGoldAmount - newGoldAmount;
    }
    return 0;
}

void GoldPileObjectController::SetMeshFromGoldAmount()
{
    const eGameObjectMeshId animIds[] = {
        eGameObjectMeshId_Additional1,
        eGameObjectMeshId_Additional2,
        eGameObjectMeshId_Main};

    long numStages = CountOf(animIds);

    eGameObjectMeshId meshId = eGameObjectMeshId_Main;

    long goldPerStage = (mMoneyComponent->mCapacity / numStages);
    if (goldPerStage > 0)
    {
        long currStage = std::min((mMoneyComponent->mAmount / goldPerStage), numStages - 1);
        meshId = animIds[currStage];
    }
    GetGameObject().SetMeshResource(meshId);
}
