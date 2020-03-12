#include "pch.h"
#include "AnimatingModel.h"
#include "ModelAsset.h"
#include "TexturesManager.h"
#include "SceneRenderList.h"

AnimatingModel::AnimatingModel()
{
}

AnimatingModel::~AnimatingModel()
{
}

void AnimatingModel::SetModelAsset(ModelAsset* modelAsset)
{
    if (modelAsset == nullptr || !modelAsset->IsModelLoaded())
    {
        debug_assert(false);
        return;
    }

    mModelAsset = modelAsset;

    // setup bounding box
    SetLocalBoundingBox(mModelAsset->mFramesBounds[0]);

    // setup materials
    mSubmeshMaterials.clear();

    for (const ModelAsset::SubMeshMaterial& currentSourceMaterial: modelAsset->mMaterialsArray)
    {
        RenderMaterial material;
        if (currentSourceMaterial.mTextures.empty())
        {
            debug_assert(false);

            mSubmeshMaterials.push_back(material); // dummy material
            continue;
        }

        material.mDiffuseTexture = gTexturesManager.GetTexture2D(currentSourceMaterial.mTextures[0]);
        if (currentSourceMaterial.mEnvMappingTexture.length())
        {
            material.mEnvMappingTexture = gTexturesManager.GetTexture2D(currentSourceMaterial.mEnvMappingTexture);
        }

        if (currentSourceMaterial.mFlagHasAlpha)
        {
            material.mRenderStates.mIsAlphaBlendEnabled = true;
            material.mRenderStates.mBlendingMode = eBlendingMode_Alpha;
        }

        if (currentSourceMaterial.mFlagAlphaAdditive)
        {
            material.mRenderStates.mIsAlphaBlendEnabled = true;
            material.mRenderStates.mBlendingMode = eBlendingMode_Additive;
            material.mRenderStates.mIsDepthWriteEnabled = false;
        }

        mSubmeshMaterials.push_back(material);
    }
}

void AnimatingModel::SetModelAssetNull()
{
    mModelAsset = nullptr;

    mSubmeshMaterials.clear();
}

void AnimatingModel::RegisterForRendering(SceneRenderList& renderList)
{
    // todo

    renderList.RegisterRenderable(eRenderPass_Opaque, this);
}
