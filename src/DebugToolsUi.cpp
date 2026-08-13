#include "stdafx.h"
#include "DebugToolsUi.h"
#include "imgui.h"
#include "GameWorld.h"
#include "GameRenderManager.h"
#include "MeshAssetManager.h"
#include "GameMain.h"
#include "GameObjectManager.h"
#include "Scene.h"
#include "GameSession.h"
#include "EconomyService.h"

void DebugToolsUi::DoUI(ImGuiIO& imguiContext, float deltaTime)
{
    const ImVec2 initialSize { 400.0f, 200.0f };

    ImGui::SetNextWindowSize(initialSize, ImGuiCond_Once);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);

    if (!ImGui::Begin("Debug Tools UI"))
    {
        DisableMeshPreview();

        ImGui::End();
        return;
    }

    EnableMeshPreview();

    if (ImGui::BeginTabBar("Tabs"))
    {
        if (ImGui::BeginTabItem("Cheats"))
        {
            DoCheatsTab(imguiContext);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Defs"))
        {

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Objects"))
        {
            DoObjectDefsTab(imguiContext);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Creatures"))
        {
            DoCreatueDefsTab(imguiContext);
            ImGui::EndTabItem();
        }

        if (mPreviewMeshObject && 
            mPreviewMeshObject->IsObjectActive() && 
            mPreviewMeshObject->HasAnimation())
        {
            if (ImGui::BeginTabItem("Preview"))
            {
                DoMeshPreviewTab(imguiContext);
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    UpdateMeshPreview(deltaTime);
}

void DebugToolsUi::OnActivateUi()
{
    EnableMeshPreview();
    SetupScenarioDefs();
    DisableMeshPreview();
}

void DebugToolsUi::OnDeactivateUi()
{
    DisableMeshPreview(true);
    ClearScenarioDefs();
}

void DebugToolsUi::DoCheatsTab(ImGuiIO& imguiContext)
{
    ImGui::PushID("cheats");

    if (ImGui::CollapsingHeader("Scene"))
    {
        ImGui::Checkbox("Show objects Aabb", &gDebug.mDrawSceneAabbTree);
        ImGui::Checkbox("Hide objects", &gDebug.mNoDrawSceneObjects);
    }

    if (ImGui::CollapsingHeader("Economy"))
    {
        if (ImGui::Button("+100 Gold"))
        {
            ChangeGoldAmount(100);
        }
        if (ImGui::Button("+1.000 Gold"))
        {
            ChangeGoldAmount(1000);
        }

        if (ImGui::Button("-100 Gold"))
        {
            ChangeGoldAmount(-100);
        }
        if (ImGui::Button("-1.000 Gold"))
        {
            ChangeGoldAmount(-1000);
        }
    }

    if (ImGui::Button("Spawn Chicken"))
    {
        EntityHandle objectHandle = gGameObjectManager.CreateObject(GameObjectClassId_Chicken);
        if (GameObject* gameObject = gGameObjectManager.GetObjectPtr(objectHandle))
        {
            gameObject->SetPosition({4.0f, 1.0f, 18.0f});
        }
        gGameObjectManager.ActivateObject(objectHandle);
    }

    
    ImGui::PopID();
}

void DebugToolsUi::EnableMeshPreview()
{
    if (mIsMeshPreviewActive) return;

    mIsMeshPreviewActive = true;

    // create mesh preview object
    if (mPreviewMeshObject == nullptr)
    {
        mPreviewMeshObject = gScene.CreateAnimatingMesh();
        cxx_assert(mPreviewMeshObject);
        mPreviewMeshObject->SetObjectActive(true);
    }

    // create render view 
    if (mPreviewMeshRenderView == nullptr)
    {
        mPreviewMeshRenderView = gGameRenderer.CreateRenderView();
        cxx_assert(mPreviewMeshRenderView);
    }

    if ((mPreviewMeshObject == nullptr) || (mPreviewMeshRenderView == nullptr)) 
    {
        cxx_assert(false);
        return;
    }

    mPreviewMeshObject->SetPosition({0.0f, 0.0f, 0.0f});
    mPreviewMeshObject->SetRenderLayers(RenderLayer_MeshPreview);
    mPreviewMeshObject->SetObjectActive(true);

    Camera& previewCamera = mPreviewMeshRenderView->GetCamera();
    ConfigurePreviewCameraProjection(previewCamera);
    previewCamera.SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
    previewCamera.LookAt(mPreviewMeshObject->GetPosition(), WorldAxes::Y);
    previewCamera.Translate(glm::vec3{0.0f, 0.3f, 0.0f});
    previewCamera.mRenderLayersMask = RenderLayer_MeshPreview;
    mPreviewMeshRenderView->SetActive(true);
}

void DebugToolsUi::DisableMeshPreview(bool forceUnloadResources)
{
    if (forceUnloadResources)
    {
        // destroy mesh preview object
        mPreviewMeshObject.reset();

        if (mPreviewMeshRenderView)
        {
            gGameRenderer.DestroyRenderView(mPreviewMeshRenderView);
            mPreviewMeshRenderView = nullptr;
        }
    }

    if (!mIsMeshPreviewActive) return;

    mIsMeshPreviewActive = false;

    if (mPreviewMeshRenderView)
    {
        mPreviewMeshRenderView->SetActive(false);
    }
}

void DebugToolsUi::SetMeshPreview(const ArtResourceDefinition& def, MeshAnimType animType)
{
    EnableMeshPreview();

    mPreviewMeshRotation = {};

    if ((def.mResourceType == eArtResource_Mesh) ||
        (def.mResourceType == eArtResource_TerrainMesh) ||
        (def.mResourceType == eArtResource_AnimatingMesh))
    {
        MeshAsset* meshAsset;
        switch (animType)
        {
            case MeshAnimType_Start:
                meshAsset = gMeshAssetManager.GetMesh(def.mResourceName + "start");
            break;
            case MeshAnimType_End:
                meshAsset = gMeshAssetManager.GetMesh(def.mResourceName + "end");
            break;
            default:
                meshAsset = gMeshAssetManager.GetMesh(def.mResourceName);
            break;
        }
        cxx_assert(meshAsset);

        cxx_assert(mPreviewMeshObject);
        if (def.mResourceType == eArtResource_AnimatingMesh)
        {
            AnimationParams animParams;
            {
                animParams.mFirstFrame = 0;
                animParams.mFramesPerSecond = def.mAnimationDesc.mFps * 1.0f;
                if (animType == MeshAnimType_Default)
                {
                    animParams.mLastFrame = def.mAnimationDesc.mFrames - 1;
                    animParams.mLoopMode = eAnimationLoopMode_Repeat;
                }
                else
                {
                    animParams.mLastFrame = meshAsset->GetAnimFramesCount() - 1;
                }
            }
            mPreviewMeshObject->Configure(meshAsset, animParams);
        }
        else
        {
            mPreviewMeshObject->Configure(meshAsset);
        }
    }
    else
    {
        DisableMeshPreview();
    }
}

void DebugToolsUi::DoCreatueDefsTab(ImGuiIO& imguiContext)
{
    ImGui::PushID("creature_defs");

    int creaturesCounter = 0;
    for (const CreatureDefinition& roller: mCreatureDefsList)
    {
        int creatureIndex = creaturesCounter++;

        if (roller.mCreatureName.empty()) continue; // skip dummy

        ImGui::PushID(creatureIndex);
        if (ImGui::CollapsingHeader(cxx::va("%s [#%d]", roller.mCreatureName.c_str(), creatureIndex)))
        {
            if (ImGui::TreeNode("Props"))
            {
                ImGui::Value("Shuffle speed", roller.mShuffleSpeed); 
                ImGui::Value("Height", roller.mHeight); 
                ImGui::Value("Speed", roller.mSpeed); 
                ImGui::Value("Run Speed", roller.mRunSpeed);
                ImGui::Value("Hunger Rate", roller.mHungerRate);
                ImGui::Value("Lair object", roller.mLairObjectId);
                ImGui::Value("Clone creature", roller.mCloneCreatureTypeId);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Flags"))
            {
                if (roller.mIsWorker) { ImGui::BulletText("Worker"); } 
                if (roller.mCanBePickedUp) { ImGui::BulletText("Can Be Picked Up"); } 
                if (roller.mCanBeSlapped) { ImGui::BulletText("Can Be Slapped"); } 
                if (roller.mAlwaysFlee) { ImGui::BulletText("Always Flee"); } 
                if (roller.mCanWalkOnLava) { ImGui::BulletText("Can Walk On Lava"); } 
                if (roller.mCanWalkOnWater) { ImGui::BulletText("Can Walk On Water"); } 
                if (roller.mCanDisarmTraps) { ImGui::BulletText("Can Disarm Traps"); }
                if (roller.mIsEvil) { ImGui::BulletText("Evil"); }
                if (roller.mIsImmuneToTurncoat) { ImGui::BulletText("Immune To Turncoat"); }
                if (roller.mAvailableViaPortal) { ImGui::BulletText("Available Via Portal"); }
                if (roller.mCanFly) { ImGui::BulletText("Can Fly"); }
                if (roller.mIsHorny) { ImGui::BulletText("Horny"); }
                if (roller.mLeavesCorpse) { ImGui::BulletText("Leaves Corpse"); }
                if (roller.mCanBeHypnotized) { ImGui::BulletText("Can Be Hypnotized"); }
                if (roller.mIsImmuneToChicken) { ImGui::BulletText("Immune To Chicken"); }
                if (roller.mIsFearless) { ImGui::BulletText("Fearless"); }
                if (roller.mCanBeElectrocuted) { ImGui::BulletText("Can Be Electrocuted"); }
                if (roller.mNeedBodyForFightIdle) { ImGui::BulletText("Need Body For Fight Idle"); }
                if (roller.mNotTrainWhenIdle) { ImGui::BulletText("Not Train When Idle"); }
                if (roller.mOnlyAttackableByHorny) { ImGui::BulletText("Only Attackable By Horny"); }
                if (roller.mCanBeResurrected) { ImGui::BulletText("Can Be Resurrected"); }
                if (roller.mDoesntGetAngryWithEnemies) { ImGui::BulletText("Doesnt Get Angry With Enemies"); }
                if (roller.mFreesFriendsOnJailbreak) { ImGui::BulletText("Frees Friends On Jailbreak"); }
                if (roller.mRevealsAdjacentTraps) { ImGui::BulletText("Reveals Adjacent Traps"); }
                if (roller.mIsUnique) { ImGui::BulletText("Unique"); }
                if (roller.mIsMale) { ImGui::BulletText(roller.mIsMale ? "Male" : "Female"); }
                ImGui::TreePop();
            }
            // resources
            for (int ianim = 0; ianim < CreatureAnimation_COUNT; ++ianim)
            {
                CreatureAnimationID animId = static_cast<CreatureAnimationID>(ianim);
                if (roller.mAnimationResources[ianim].IsDefined() && ImGui::TreeNode(cxx::va("Animation: %s", cxx::enum_to_string(animId))))
                {
                    DoArtResourceDef(imguiContext, roller.mAnimationResources[ianim]);
                    ImGui::TreePop();
                }
            }
        }
        ImGui::PopID();
    }

    ImGui::PopID();
}

void DebugToolsUi::DoObjectDefsTab(ImGuiIO& imguiContext)
{
    ImGui::PushID("obj_defs");

    int objectsCounter = 0;
    for (const GameObjectDefinition& roller: mObjectDefsList)
    {
        int objectIndex = objectsCounter++;

        if (roller.mObjectName.empty()) continue; // skip dummy

        ImGui::PushID(objectIndex);

        if (ImGui::CollapsingHeader(cxx::va("%s [#%d]", roller.mObjectName.c_str(), objectIndex)))
        {
            if (ImGui::TreeNode("Physics props"))
            {
                ImGui::Value("Width", roller.mWidth); 
                ImGui::Value("Height", roller.mHeight);
                ImGui::Value("Mass", roller.mMass); 
                ImGui::Value("Speed", roller.mSpeed);
                ImGui::Value("Air Friction", roller.mAirFriction);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Object flags"))
            {
                if (roller.mDieOverTime) { ImGui::BulletText("Die over time"); }
                if (roller.mDieOverTimeIfNotInRoom) { ImGui::BulletText("Die over time if not in room"); }

                ImGui::BulletText("Category: %s", cxx::enum_to_string(roller.mObjectCategory));

                if (roller.mCanBePickedUp) { ImGui::BulletText("Can be picked up"); }
                if (roller.mCanBeSlapped) { ImGui::BulletText("Can be slapped"); }
                if (roller.mDieWhenSlapped) { ImGui::BulletText("Die when slapped"); }
                if (roller.mCanBeDroppedOnAnyLand) { ImGui::BulletText("Can be dropped on any land"); }
                if (roller.mObstacle) { ImGui::BulletText("Obstacle"); }
                if (roller.mBounce) { ImGui::BulletText("Bounce"); }
                if (roller.mBoulderCanRollThrough) { ImGui::BulletText("Boulder can roll through"); }
                if (roller.mBoulderDestroys) { ImGui::BulletText("Boulder destroys"); }
                if (roller.mIsPillar) { ImGui::BulletText("Pillar"); }
                if (roller.mDoorKey) { ImGui::BulletText("Door key"); }
                if (roller.mIsDamageable) { ImGui::BulletText("Damageable"); }
                if (roller.mHighlightable) { ImGui::BulletText("Highlightable"); }
                if (roller.mPlaceable) { ImGui::BulletText("Placeable"); }
                if (roller.mFirstPersonObstacle) { ImGui::BulletText("First person obstacle"); }
                if (roller.mSolidObstacle) { ImGui::BulletText("Solid obstacle"); }
                if (roller.mCastShadows) { ImGui::BulletText("Cast shadows"); }
                ImGui::TreePop();
            }

            if (roller.mResourceMesh.IsDefined() && ImGui::TreeNode("Resource: Mesh"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceMesh);
                ImGui::TreePop();
            }

            if (roller.mResourceGuiIcon.IsDefined() && ImGui::TreeNode("Resource: Gui Icon"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceGuiIcon);
                ImGui::TreePop();
            }

            if (roller.mResourceInHandIcon.IsDefined() && ImGui::TreeNode("Resource: In Hand Icon"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceInHandIcon);
                ImGui::TreePop();
            }

            if (roller.mResourceInHandMesh.IsDefined() && ImGui::TreeNode("Resource: In Hand Mesh"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceInHandMesh);
                ImGui::TreePop();
            }

            if (roller.mResourceUnknown.IsDefined() && ImGui::TreeNode("Resource: Unknown"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceUnknown);
                ImGui::TreePop();
            }

            if (roller.mResourceAdditional1.IsDefined() && ImGui::TreeNode("Resource: Additional 1"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceAdditional1);
                ImGui::TreePop();
            }

            if (roller.mResourceAdditional2.IsDefined() && ImGui::TreeNode("Resource: Additional 2"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceAdditional2);
                ImGui::TreePop();
            }

            if (roller.mResourceAdditional3.IsDefined() && ImGui::TreeNode("Resource: Additional 3"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceAdditional3);
                ImGui::TreePop();
            }

            if (roller.mResourceAdditional4.IsDefined() && ImGui::TreeNode("Resource: Additional 4"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceAdditional4);
                ImGui::TreePop();
            }
        }

        ImGui::PopID();
    }

    ImGui::PopID();
}

void DebugToolsUi::DoArtResourceDef(ImGuiIO& imguiContext, const ArtResourceDefinition& def)
{
    if (!def.IsDefined()) return;

    ImGui::Text("Resource type: %s", cxx::enum_to_string(def.mResourceType));
    if ((def.mResourceType == eArtResource_Mesh) ||
        (def.mResourceType == eArtResource_TerrainMesh) ||
        (def.mResourceType == eArtResource_AnimatingMesh))
    {
        ImGui::SameLine();
        if (ImGui::Button("Preview"))
        {
            SetMeshPreview(def);
        }
    }
    ImGui::Text("Resource name: %s", def.mResourceName.c_str());

    if ((def.mResourceType == eArtResource_Sprite) ||
        (def.mResourceType == eArtResource_Alpha) ||
        (def.mResourceType == eArtResource_AdditiveAlpha))
    {
        ImGui::Text("Image Frames: %d", def.mImageDesc.mFrames);
        ImGui::Text("Image Size: %f x %f", def.mImageDesc.mWidth, def.mImageDesc.mHeight);
    }

    if ((def.mResourceType == eArtResource_Mesh) ||
        (def.mResourceType == eArtResource_MeshCollection))
    {
        ImGui::Text("Mesh Frames: %d", def.mMeshDesc.mFrames);
        ImGui::Text("Mesh Scale: %f", def.mMeshDesc.mScale);
    }

    if (def.mResourceType == eArtResource_TerrainMesh)
    {
        ImGui::Text("Terrain Frames: %d", def.mTerrainDesc.mFrames);
    }

    if (def.mResourceType == eArtResource_ProceduralMesh)
    {
        ImGui::Text("Proc Id: %d", def.mProcDesc.mId);
    }

    if (def.mResourceType == eArtResource_AnimatingMesh)
    {
        ImGui::Text("Anim Frames: %d", def.mAnimationDesc.mFrames);
        ImGui::Text("Anim Fps: %d", def.mAnimationDesc.mFps);
        if (def.mHasStartAnimation || def.mHasEndAnimation) 
        { 
            ImGui::Text("Start / End AF: %d / %d", def.mStartAF, def.mEndAF);
            ImGui::Text("Start / End: "); 
            if (def.mHasStartAnimation)
            {
                ImGui::SameLine();
                if (ImGui::Button("Start##start_anim"))
                {
                    SetMeshPreview(def, MeshAnimType_Start);
                }
            }
            if (def.mHasEndAnimation) 
            { 
                ImGui::SameLine();
                if (ImGui::Button("End##end_anim"))
                {
                    SetMeshPreview(def, MeshAnimType_End);
                }
            }
        }
    }

    if (ImGui::TreeNode("Flags"))
    {
        if (def.mPlayerColoured) { ImGui::BulletText("Player coloured"); }
        if (def.mAnimatingTexture) { ImGui::BulletText("Animating texture"); }
        if (def.mRandomStartFrame) { ImGui::BulletText("Random start frame"); }
        if (def.mOriginAtBottom) { ImGui::BulletText("Origin at bottom"); }
        if (def.mDoesntLoop) { ImGui::BulletText("Doesnt loop"); }
        if (def.mFlat) { ImGui::BulletText("Flat"); }
        if (def.mDoesntUseProgressiveMesh) { ImGui::BulletText("Doesnt use progressive mesh"); }
        if (def.mUseAnimatingTextureForSelection) { ImGui::BulletText("Use animating texture for selection"); }
        if (def.mPreload) { ImGui::BulletText("Preload"); }
        if (def.mBlood) { ImGui::BulletText("Blood"); }
        ImGui::TreePop();
    }
}

void DebugToolsUi::SetupScenarioDefs()
{
    const ScenarioDefinition& scenarioData = gGameSession.GetScenarioDefinition();
    mObjectDefsList = scenarioData.mGameObjectDefs;
    mCreatureDefsList = scenarioData.mCreatureDefs;
}

void DebugToolsUi::ClearScenarioDefs()
{
    mObjectDefsList.clear();
    mCreatureDefsList.clear();
}

void DebugToolsUi::ConfigurePreviewCameraProjection(Camera& camera)
{
    Camera::ProjectionParams cameraParams;
    cameraParams.mFarDistance = 10.0f;
    cameraParams.mNearDistance = 0.01f;
    cameraParams.mFovy = 30.0f;
    camera.SetupProjection(cameraParams);
}

void DebugToolsUi::ChangeGoldAmount(long amount)
{
    if (amount == 0) return;

    if (amount > 0)
    {
        gEconomyService.GiveResource(gGameSession.GetLocalPlayer(), eGameResource_Gold, amount);
    }
    else
    {
        gEconomyService.TakeResource(gGameSession.GetLocalPlayer(), eGameResource_Gold, -amount);
    }
}

void DebugToolsUi::DoMeshPreviewTab(ImGuiIO& imguiContext)
{
    ImGui::PushID("mesh_preview");

    ImGui::Checkbox("Rotate mesh", &mRotatePreviewMesh);

    ImGui::Text("Set rotation: ");
    ImGui::SameLine();
    if (ImGui::Button("0##degs"))
    {
        mRotatePreviewMesh = false;
        SetMeshRotation(cxx::angle_t(0.0f, cxx::angle_t::units::degrees));
    }
    ImGui::SameLine();
    if (ImGui::Button("90##degs"))
    {
        mRotatePreviewMesh = false;
        SetMeshRotation(cxx::angle_t(90.0f, cxx::angle_t::units::degrees));
    }
    ImGui::SameLine();
    if (ImGui::Button("-90##degs"))
    {
        mRotatePreviewMesh = false;
        SetMeshRotation(cxx::angle_t(-90.0f, cxx::angle_t::units::degrees));
    }
    ImGui::SameLine();
    if (ImGui::Button("180##degs"))
    {
        mRotatePreviewMesh = false;
        SetMeshRotation(cxx::angle_t(180.0f, cxx::angle_t::units::degrees));
    }

    bool isPaused = mPreviewMeshObject->IsAnimationPaused();
    if (ImGui::Checkbox("Paused", &isPaused))
    {
        if (isPaused)
        {
            mPreviewMeshObject->PauseAnimation();
        }
        else
        {
            mPreviewMeshObject->ResumeAnimation();
        }
    }

    ImGui::Text("Animation Frame: %d/%d", mPreviewMeshObject->GetAnimationFrame() + 1, mPreviewMeshObject->GetAnimationFrameCount());

    if (ImGui::Button("<<##to_first_frame"))
    {
        mPreviewMeshObject->PauseAnimation();
        mPreviewMeshObject->SetAnimationFrame(0);
    }
    ImGui::SameLine();
    if (ImGui::Button("<##prev_frame"))
    {
        mPreviewMeshObject->PauseAnimation();
        mPreviewMeshObject->SetAnimationFrame(mPreviewMeshObject->GetAnimationFrame() - 1);
    }
    ImGui::SameLine();
    if (ImGui::Button(">##next_frame"))
    {
        mPreviewMeshObject->PauseAnimation();
        mPreviewMeshObject->SetAnimationFrame(mPreviewMeshObject->GetAnimationFrame() + 1);
    }
    ImGui::SameLine();
    if (ImGui::Button(">>##to_last_frame"))
    {
        mPreviewMeshObject->PauseAnimation();
        mPreviewMeshObject->SetAnimationFrame(mPreviewMeshObject->GetAnimationFrameCount());
    }

    ImGui::PopID();
}

void DebugToolsUi::UpdateMeshPreview(float deltaTime)
{
    if (!mRotatePreviewMesh) return;
    if (mPreviewMeshObject && mPreviewMeshObject->IsObjectActive())
    {
        SetMeshRotation(mPreviewMeshRotation + cxx::angle_t::from_degrees(-30.0f * deltaTime));
    }
}

void DebugToolsUi::SetMeshRotation(cxx::angle_t meshRotation)
{
    mPreviewMeshRotation = meshRotation;
    if (mPreviewMeshObject && mPreviewMeshObject->IsObjectActive())
    {
        mPreviewMeshObject->ResetOrientation();
        mPreviewMeshObject->RotateAroundAxis(WorldAxes::Y, mPreviewMeshRotation);
    }
}

