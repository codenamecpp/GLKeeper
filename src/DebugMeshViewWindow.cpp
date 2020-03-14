#include "pch.h"
#include "DebugMeshViewWindow.h"
#include "DebugGuiManager.h"
#include "FileSystem.h"
#include "Console.h"
#include "FileSystemArchive.h"
#include "imgui.h"
#include "ModelAsset.h"
#include "ModelAssetsManager.h"
#include "AnimatingModel.h"
#include "MeshViewGamestate.h"
#include "cvars.h"

DebugMeshViewWindow::DebugMeshViewWindow(): DebugGuiWindow("Models view")
{
    mWindowNoCloseButton = true;
    mInitialSize.x = 400;
    mInitialSize.y = 500;
    mBackgroundAlpha = 0.5f;

    mModelsListFilter = new ImGuiTextFilter;
}

DebugMeshViewWindow::~DebugMeshViewWindow()
{
    SafeDelete(mModelsListFilter);
}

void DebugMeshViewWindow::Setup(MeshViewGamestate* meshviewGamestate)
{
    mMeshViewGamestate = meshviewGamestate;

    LoadModelsList();
}

void DebugMeshViewWindow::LoadModelsList()
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

void DebugMeshViewWindow::DoUI(ImGuiIO& imguiContext)
{
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("Tabs", tab_bar_flags))
    {
        AnimatingModel* animModel = mMeshViewGamestate ? mMeshViewGamestate->mModelObject : nullptr;

        // curr model info
        if (animModel && ImGui::BeginTabItem("Model info"))
        {
            ModelAsset* modelAsset = animModel->mModelAsset;

            ImGui::Text("Asset name: %s", modelAsset->mName.c_str());
            ImGui::Separator();
            ImGui::Text("Num frame vertices: %d", modelAsset->GetFrameVerticesCount());

            if (!animModel->IsStatic())
            {
                ImGui::Text("Num frames: %d", modelAsset->mFramesCount);
            }
            int icurrentSubMesh = 0;
            int numLODs = 0;
            for (const auto& currMesh: modelAsset->mMeshArray)
            {
                if (icurrentSubMesh == 0)
                {
                    numLODs = (int)currMesh.mLODsArray.size();
                }

                if (ImGui::TreeNode((void*)(intptr_t) icurrentSubMesh, "Submesh %d", icurrentSubMesh))
                {
                    ImGui::Text("Num frame vertices: %d", currMesh.mFrameVerticesCount);

                    // material
                    const auto& currMaterial = modelAsset->mMaterialsArray[currMesh.mMaterialIndex];
                    ImGui::Text("Material name: %s", currMaterial.mInternalName.c_str());
                    // flags
                    ImGui::Text("Material Alpha: %s", currMaterial.mFlagHasAlpha ? "yes" : "no");
                    ImGui::Text("Material Shinyness: %s", currMaterial.mFlagShinyness ? "yes" : "no");
                    ImGui::Text("Material AlphaAdditive: %s", currMaterial.mFlagAlphaAdditive ? "yes" : "no");
                    ImGui::Text("Material EnvironmentMapped: %s", currMaterial.mFlagEnvironmentMapped ? "yes" : "no");

                    ImGui::Text("Textures:");

                    const ImVec2 textureSize(128.0f, 128.0f);

                    int icurrentTexture = 0;
                    for (const std::string& currTexture: currMaterial.mTextures)
                    {
                        if (ImGui::TreeNode((void*)(intptr_t) icurrentTexture, "Texture %s", currTexture.c_str()))
                        {
                            Texture2D* texture2d = animModel->mSubmeshMaterials[currMesh.mMaterialIndex].mDiffuseTexture;
                            ImGui::Image(texture2d, textureSize);
                            ImGui::TreePop();
                        }

                        ++icurrentTexture;
                    }

                    ImGui::TreePop();
                }

                ++icurrentSubMesh;
            }

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
                ImGui::Separator();

                bool enableBlendFrames = gCvarRender_EnableAnimBlendFrames.mValue;
                if (ImGui::Checkbox("Enable blend frames", &enableBlendFrames))
                {
                    gCvarRender_EnableAnimBlendFrames.SetValue(enableBlendFrames);
                }
                ImGui::Separator();

                int currentFrame = animModel->mAnimState.mFrame0;   
                if (ImGui::SliderInt("Frame", &currentFrame, animModel->mAnimState.mStartFrame, animModel->mAnimState.mFinalFrame))
                {
                    // todo
                    //animModel->SetFrame
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
}

void DebugMeshViewWindow::UpdateFilteredElementsList()
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

void DebugMeshViewWindow::ChangeModelAsset(const char* assetName)
{
    if (mMeshViewGamestate)
    {
        ModelAsset* modelAsset = gModelsManager.LoadModelAsset(assetName);
        if (modelAsset == nullptr)
        {
            return;
        }

        mMeshViewGamestate->mModelObject->SetModelAsset(modelAsset);
        mMeshViewGamestate->mModelObject->StartAnimation(24.0f, true);
    }
}
