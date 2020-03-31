#include "pch.h"
#include "MeshViewWindow.h"
#include "DebugUiManager.h"
#include "FileSystem.h"
#include "Console.h"
#include "FileSystemArchive.h"
#include "imgui.h"
#include "ModelAsset.h"
#include "ModelAssetsManager.h"
#include "MeshViewGamestate.h"
#include "cvars.h"
#include "AnimatingModelComponent.h"
#include "SceneObject.h"

MeshViewWindow::MeshViewWindow()
{
    mModelsListFilter = new ImGuiTextFilter;
}

MeshViewWindow::~MeshViewWindow()
{
    SafeDelete(mModelsListFilter);
}

void MeshViewWindow::Setup(MeshViewGamestate* meshviewGamestate)
{
    mMeshViewGamestate = meshviewGamestate;

    LoadModelsList();
}

void MeshViewWindow::LoadModelsList()
{
    mAllModelsList.clear();
    if (FileSystemArchive* archive = gFileSystem.FindResourceArchive("Meshes.WAD"))
    {
        for (const auto& currEntry: archive->mEtriesMap)
        {
            mAllModelsList.push_back(currEntry.first.c_str());
        }
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot locate meshes wad archive");
    }

    UpdateFilteredElementsList();
}

void MeshViewWindow::DoUI(ImGuiIO& imguiContext)
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    const ImVec2 distance { 10.0f, 10.0f };
    const ImVec2 initialSize { 400.0f, imguiContext.DisplaySize.y - distance.y * 2.0f };
    const ImVec2 initialPos { imguiContext.DisplaySize.x - initialSize.x - distance.x, distance.y };

    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::SetNextWindowSize(initialSize, ImGuiCond_Once);
    ImGui::SetNextWindowPos(initialPos, ImGuiCond_Once);

    if (!ImGui::Begin("Models view", nullptr, windowFlags))
    {
        ImGui::End();
        return;
    }
 
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("Tabs", tab_bar_flags))
    {
        AnimatingModelComponent* animModel = nullptr;
        ModelAsset* modelAsset = nullptr;
        if (mMeshViewGamestate && mMeshViewGamestate->mModelObject)
        {
            animModel = mMeshViewGamestate->mModelObject->GetAnimatingModelComponent();
            if (animModel)
            {
                modelAsset = animModel->mModelAsset;
            }
        }

        // curr model info
        if (animModel && modelAsset && ImGui::BeginTabItem("Model info"))
        {
            ImGui::Text("Asset name: %s", modelAsset->mInternalName.c_str());
            ImGui::Text("Num frame vertices: %d", modelAsset->GetFrameVerticesCount());

            if (!animModel->IsStatic())
            {
                ImGui::Text("Num frames: %d", modelAsset->mFramesCount);
            }
            ImGui::NewLine();

            int icurrentSubMesh = 0;
            int numLODs = 0;
            for (const auto& currMesh: modelAsset->mMeshArray)
            {
                if (icurrentSubMesh == 0)
                {
                    numLODs = (int)currMesh.mLODsArray.size();
                }

                if (ImGui::TreeNode((void*)(intptr_t) icurrentSubMesh, "Submesh %d", icurrentSubMesh + 1))
                {
                    ImGui::Text("Num frame vertices: %d", currMesh.mFrameVerticesCount);

                    // material
                    const auto& currMaterial = modelAsset->mMaterialsArray[currMesh.mMaterialIndex];
                    ImGui::Text("Material name: %s", currMaterial.mInternalName.c_str());
                    // flags
                    if (currMaterial.mFlagHasAlpha || currMaterial.mFlagShinyness || 
                        currMaterial.mFlagAlphaAdditive || currMaterial.mFlagEnvironmentMapped)
                    {
                        ImGui::Text("Material Flags:");
                        if (currMaterial.mFlagHasAlpha)
                        {
                            ImGui::BulletText("Alpha");
                        }
                        if (currMaterial.mFlagShinyness)
                        {
                            ImGui::BulletText("Shinyness");
                        }
                        if (currMaterial.mFlagAlphaAdditive)
                        {
                            ImGui::BulletText("AlphaAdditive");
                        }
                        if (currMaterial.mFlagEnvironmentMapped)
                        {
                            ImGui::BulletText("EnvironmentMapped");
                        }
                        ImGui::Separator();
                    }
                    ImGui::Text("Textures:");

                    const ImVec2 textureSize(128.0f, 128.0f);

                    int icurrentTexture = 0;
                    for (const std::string& currTexture: currMaterial.mTextures)
                    {
                        if (ImGui::TreeNode((void*)(intptr_t) icurrentTexture, "Texture %s", currTexture.c_str()))
                        {
                            Texture2D* texture2d = animModel->mSubmeshTextures[currMesh.mMaterialIndex][icurrentTexture];
                            ImGui::Image(texture2d, textureSize);
                            ImGui::TreePop();
                        }

                        ++icurrentTexture;
                    }

                    ImGui::TreePop();
                }

                ++icurrentSubMesh;
            }
            ImGui::NewLine();
            // set level of details
            int currentLOD = animModel->mPreferredLOD;
            if (numLODs > 0)
            {
                if (ImGui::SliderInt("LOD Level", &currentLOD, 0, numLODs - 1))
                {
                    animModel->SetPreferredLOD(currentLOD);
                }
            }
            ImGui::Text("Num LOD triangles: %d", modelAsset->GetTrianglesCount(currentLOD));
            ImGui::EndTabItem();
        }

        // anim control
        if (!animModel->IsStatic())
        {
            if (ImGui::BeginTabItem("Animation"))
            {
                ImGui::Text("Cycles counter: %d", animModel->mAnimState.mCyclesCount);
                ImGui::NewLine();

                bool enableBlendFrames = gCvarRender_EnableAnimBlendFrames.mValue;
                if (ImGui::Checkbox("Enable blend frames", &enableBlendFrames))
                {
                    gCvarRender_EnableAnimBlendFrames.SetValue(enableBlendFrames);
                }
                ImGui::NewLine();

                int currentFrame = animModel->mAnimState.mFrame0;   
                if (ImGui::SliderInt("Frame", &currentFrame, animModel->mAnimState.mStartFrame, animModel->mAnimState.mFinalFrame))
                {
                    // todo
                    //animModel->SetFrame
                }
                if (ImGui::Button("Pause/Unpause"))
                {
                    animModel->SetAnimationPaused(!animModel->IsAnimationPaused());
                }

                ImGui::EndTabItem();
            }
        }

        // model selection
        if (ImGui::BeginTabItem("All Models List"))
        {
            if (mModelsListFilter->Draw("Filter"))
            {
                UpdateFilteredElementsList();
            }

            if (ImGui::BeginChild("child_Frame"))
            {
                // list filtered models
                const int numElements = (int) mFilteredList.size();
                ImGui::PushItemWidth(-1);
                if (ImGui::ListBox("##", &mListSelection, mFilteredList.data(), numElements, numElements))
                {
                    ChangeModelAsset(mFilteredList[mListSelection]);
                }
                ImGui::PopItemWidth();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void MeshViewWindow::UpdateFilteredElementsList()
{
    mFilteredList.clear();
    mFilteredList.reserve(mAllModelsList.size());

    for (const char* currentString: mAllModelsList)
    {
        if (mModelsListFilter->PassFilter(currentString))
        {
            mFilteredList.push_back(currentString);
        }
    }
}

void MeshViewWindow::ChangeModelAsset(const char* assetName)
{
    if (mMeshViewGamestate)
    {
        std::string assetNameWithoutExt(assetName);
        cxx::path_remove_extension(assetNameWithoutExt);

        ModelAsset* modelAsset = gModelsManager.LoadModelAsset(assetNameWithoutExt);
        if (modelAsset == nullptr)
        {
            return;
        }

        AnimatingModelComponent* component = mMeshViewGamestate->mModelObject->GetAnimatingModelComponent();
        debug_assert(component);

        component->SetModelAsset(modelAsset);
        component->StartAnimation(24.0f, true);
    }
}
