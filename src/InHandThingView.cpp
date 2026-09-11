#include "stdafx.h"
#include "InHandThingView.h"
#include "GameObjectManager.h"
#include "CreatureManager.h"
#include "RenderView.h"
#include "GameRenderManager.h"
#include "Scene.h"
#include "MeshAssetManager.h"
#include "UiManager.h"

InHandThingView::~InHandThingView()
{

}

void InHandThingView::SetScreenPosition(const Point2D& screenPosition)
{
    // compute projection offset
    if (mRenderView && mRenderView->IsActive())
    {
        const Rect2D& screenRect = gUiManager.GetScreenRect();
        // ndc
        const glm::vec2 projectionOffset {
            (2.0f * screenPosition.x / screenRect.w) - 1.0f,
            1.0f - (2.0f * screenPosition.y / screenRect.h) };

        mRenderView->GetCamera().SetProjectionOffset(projectionOffset);
    }
}

void InHandThingView::SetHeldThing(EntityHandle entHandle)
{
    if (mEntityHandle == entHandle)
        return;

    mEntityHandle = entHandle;

    if (entHandle.IsGameObject())
    {
        if (GameObject* objectEntity = gGameObjectManager.GetObjectPtr(entHandle))
        {
            SetHeldThing(objectEntity->GetDefinition());
            return;
        }
    }

    if (entHandle.IsCreature())
    {
        if (Creature* creatureEntity = gCreatureManager.GetCreaturePtr(entHandle))
        {
            SetHeldThing(creatureEntity->GetDefinition());
            return;
        }
    }

    if (entHandle)
    {
        cxx_assert(false);
    }

    SetHeldNothing();
}

void InHandThingView::SetHeldNothing()
{
    mEntityHandle = {};
    mMeshAsset = nullptr;
    if (mRenderView)
    {
        mRenderView->SetActive(false);
    }
    if (mMeshObject)
    {
        mMeshObject->SetObjectActive(false);
    }
}

bool InHandThingView::HasHeldThing() const
{
    return mEntityHandle.WasSet();
}

bool InHandThingView::HasHeldThing(EntityHandle entHandle) const
{
    return mEntityHandle.WasSet() && (mEntityHandle == entHandle);
}

void InHandThingView::SetMeshAsset(const std::string& assetName)
{
    // init resource

    if (mRenderView == nullptr)
    {
        mRenderView = gGameRenderer.CreateRenderView();
        cxx_assert(mRenderView);
        if (mRenderView)
        {
            Camera& camera = mRenderView->GetCamera();
            Camera::ProjectionParams projParams;
            projParams.mFarDistance = 10.0f;
            projParams.mNearDistance = 0.1f;
            projParams.mFovy = 60.0f;
            camera.SetupProjection(projParams);
            camera.SetPosition({1.0f, 1.3f, 1.0f});
            camera.LookAt({0.0f, 0.0f, 0.0f}, WorldAxes::Y);
            camera.mRenderLayers.Set(eSceneRenderLayer_UiOverlay);
            mRenderView->SetActive(false); 
        }
    }

    if (mMeshObject == nullptr)
    {
        mMeshObject = gScene.CreateAnimatingMesh();
        cxx_assert(mMeshObject);
        if (mMeshObject)
        {
            mMeshObject->GetRenderLayers().Set(eSceneRenderLayer_UiOverlay);
            mMeshObject->SetObjectActive(false);
        }
    }

    if (mRenderView && mMeshObject)
    {
        MeshAsset* meshAsset = gMeshAssetManager.GetMesh(assetName);
        cxx_assert(meshAsset);

        if (meshAsset && (meshAsset != mMeshAsset))
        {
            mMeshAsset = meshAsset;
            AnimationParams animParams;
            {
                animParams.mLastFrame = mMeshAsset->GetAnimFramesCount() - 1;
                animParams.mLoopMode = eAnimationLoopMode_Repeat;
            }
            mMeshObject->Configure(mMeshAsset, animParams);
            mMeshObject->SetObjectActive(true);
            mRenderView->SetActive(true);
        }
        return;
    }
    cxx_assert(false);
    SetHeldNothing();
}

void InHandThingView::SetHeldThing(GameObjectDefinition* definition)
{
    cxx_assert(definition);
    if (definition)
    {
        const ArtResourceDefinition& inHandArtResource = definition->mResourceInHandMesh;
        if (inHandArtResource.HasResource(eArtResource_AnimatingMesh) ||
            (inHandArtResource.HasResource(eArtResource_Mesh)))
        {
            SetMeshAsset(inHandArtResource.mResourceName);
            return;
        }
    }
    SetHeldNothing();
}

void InHandThingView::SetHeldThing(CreatureDefinition* definition)
{
    cxx_assert(definition);
    if (definition)
    {
        const ArtResourceDefinition& swingArtResource = definition->mAnimationResources[CreatureAnimation_Swing];
        if (swingArtResource.HasResource(eArtResource_AnimatingMesh) ||
            (swingArtResource.HasResource(eArtResource_Mesh)))
        {
            SetMeshAsset(swingArtResource.mResourceName);
            return;
        }
    }
    SetHeldNothing();
}
