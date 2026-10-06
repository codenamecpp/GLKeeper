#include "stdafx.h"
#include "HeroGateTableObjectController.h"

//////////////////////////////////////////////////////////////////////////

static const float PieceElevation = 0.02f;

//////////////////////////////////////////////////////////////////////////

void HeroGateTableObjectController::ConfigureInstance(GameObject* objectInstance)
{
    GameObjectController::ConfigureInstance(objectInstance);

    objectInstance->AddCapability<I3DMapTableLevel>(this);
}

void HeroGateTableObjectController::SpawnInstance()
{
    GameObjectController::SpawnInstance();
}

void HeroGateTableObjectController::DespawnInstance()
{
    GameObjectController::DespawnInstance();
}

void HeroGateTableObjectController::UpdateLogic(float stepDeltaTime)
{
    GameObjectController::UpdateLogic(stepDeltaTime);
}

void HeroGateTableObjectController::UpdateFrame(float deltaTime)
{
    GameObjectController::UpdateFrame(deltaTime);

    if (mIsElevationActive)
    {
        // update piece elevation
        if (AnimatingMeshObject* mesh = GetGameObject().GetMeshObject())
        {
            const glm::vec3& currentMeshPosition = mesh->GetPosition();
            // compute elevation height
            const float elevationHeight = mIsLevelSelected ? PieceElevation : 0.0f;
            const float elevationSpeed = mIsLevelSelected ? (PieceElevation * 4.0f) : (PieceElevation * 0.5f);

            glm::vec3 targetMeshPosition = GetGameObject().GetPosition();
            targetMeshPosition.y += elevationHeight;
            glm::vec3 newPosition = cxx::move_towards(currentMeshPosition, targetMeshPosition, deltaTime * elevationSpeed);
            
            if (cxx::eps_equals_zero(glm::distance(targetMeshPosition, newPosition)))
            {
                mIsElevationActive = false;
                mesh->SetPosition(targetMeshPosition);
            }
            else
            {
                mesh->SetPosition(newPosition);
            }
        }
        else
        {
            mIsElevationActive = false;
        }
    }
}

void HeroGateTableObjectController::OnRecycle()
{
    GameObjectController::OnRecycle();
    mIsLevelSelected = {};
    mIsElevationActive = {};
}

void HeroGateTableObjectController::SelectLevel()
{
    if (mIsLevelSelected)
        return;

    mIsLevelSelected = true;
    mIsElevationActive = true;
}

void HeroGateTableObjectController::DeselectLevel()
{
    if (!mIsLevelSelected)
        return;

    mIsLevelSelected = false;
    mIsElevationActive = true;
}
