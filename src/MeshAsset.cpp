#include "stdafx.h"
#include "MeshAsset.h"
#include "TextureManager.h"
#include "DK2KMFModel.h"
#include "DK2AssetLoader.h"

MeshAsset::MeshAsset(const std::string& meshName)
    : mMeshName(meshName)
{
}

void MeshAsset::Load()
{
    Purge();

    if (LoadMeshFromFile())
    {
        mIsLoadedFromFile = true;
    }
    else
    {
        InitDefault();
    }
    InitMaterials();
}

void MeshAsset::InitDefault()
{
    Purge();

    mInternalName = "fallback_mesh";

    cxx_assert(false); // todo

    mIsDefaultMesh = true;
}

void MeshAsset::Purge()
{
    mFramesBounds.clear();
    mSubMeshes.clear();
    mMaterials.clear();
    mInternalName.clear();
    mRenderMaterials.clear();
    mGpuVertexBuffer.reset();
    mGpuIndexBuffer.reset();

    mMaxBounds.set_to_zero();

    mTranslation = {};
    mBaseScale = 1.0f;
    mCubeScale = 1.0f;
    mFramesCount = 1;
    mAnimFrameFactorFunc = eAnimFrameFactorFunc::Clamp;

    mIsLoadedFromFile = false;
    mIsDefaultMesh = false; 
    mIsRenderDataInited = false;
}

void MeshAsset::InitRenderData()
{
    if (IsRenderDataInited())
        return;

    if (!IsInited())
    {
        cxx_assert(false);
        return;
    }

    mIsRenderDataInited = true;

    // materials
    for (const MaterialDesc& roller: mMaterials)
    {
        if (Texture* envMappingTexture = roller.mEnvMappingTexture)
        {
            envMappingTexture->InitRenderData();
        }
        
        for (Texture* diffuseTexture: roller.mDiffuseTextures)
        {
            diffuseTexture->InitRenderData();
        }
    }

    if (!InitVertexBuffer() || !InitIndexBuffer())
    {
        cxx_assert(false);
        // todo : init fallback
        gConsole.LogMessage(eLogLevel_Warning, "Cannot initialize mesh render data ('%s'), trying fallback", mMeshName.c_str());
    }
}

bool MeshAsset::LoadMeshFromFile()
{
    const std::string& meshName = mMeshName;

    if (meshName.empty())
    {
        cxx_assert(false);
        return false;
    }

    std::string realName = FSGetFileNameWithoutExtension(meshName); // override extension
    realName.append(".kmf");

    DK2KMFModel temporaryResource;
    if (!gDK2AssetLoader.LoadKMFModelData(realName, temporaryResource))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load mesh asset '%s'", meshName.c_str());
        return false;
    }

    // conversion

    std::swap(temporaryResource.mName, mInternalName);
    std::swap(temporaryResource.mFramesBounds, mFramesBounds);
    std::swap(temporaryResource.mTranslation, mTranslation);
    std::swap(temporaryResource.mScale, mBaseScale);
    std::swap(temporaryResource.mCubeScale, mCubeScale);
    std::swap(temporaryResource.mFramesCount, mFramesCount);

    // compute max bounds
    mMaxBounds.set_to_zero();
    if (!mFramesBounds.empty())
    {
        mMaxBounds = mFramesBounds.front();
        for (const cxx::aabbox& roller: mFramesBounds)
        {
            mMaxBounds.extend(roller);
        }
    }

    mAnimFrameFactorFunc = (temporaryResource.mFrameFactorFunction == 1) ? 
        eAnimFrameFactorFunc::Wrap : 
        eAnimFrameFactorFunc::Clamp;

    // geomery
    mSubMeshes.clear();
    mSubMeshes.reserve(temporaryResource.mMeshArray.size());
    for (DK2KMFModel::SubMesh& srcSubMesh: temporaryResource.mMeshArray)
    {
        SubMesh& dstSubMesh = mSubMeshes.emplace_back();

        std::swap(srcSubMesh.mVertexPositionArray, dstSubMesh.mVertices);
        std::swap(srcSubMesh.mVertexNormalArray, dstSubMesh.mNormals);
        std::swap(srcSubMesh.mVertexTexCoordArray, dstSubMesh.mTexCoords);
        std::swap(srcSubMesh.mFrameVerticesCount, dstSubMesh.mFrameVerticesCount);
        std::swap(srcSubMesh.mMaterialIndex, dstSubMesh.mMaterialIndex);

        dstSubMesh.mTris.clear();
        if (!srcSubMesh.mLODsArray.empty())
        {
            std::swap(srcSubMesh.mLODsArray[0].mTriangleArray, dstSubMesh.mTris);
        }
    }

    // handle materials
    mMaterials.clear();
    mMaterials.reserve(temporaryResource.mMaterialsArray.size());
    for (DK2KMFModel::MaterialDesc& srcMaterial: temporaryResource.mMaterialsArray)
    {
        MaterialDesc& dstMaterial = mMaterials.emplace_back();
        {
            std::swap(srcMaterial.mName, dstMaterial.mName);
            std::swap(srcMaterial.mDiffuseTextureNames, dstMaterial.mDiffuseTextureNames);
            std::swap(srcMaterial.mEnvMappingTextureName, dstMaterial.mEnvMappingTextureName);
            // flags
            dstMaterial.mFlags_HasAlpha = srcMaterial.mFlags_HasAlpha;
            dstMaterial.mFlags_DoubleSided = srcMaterial.mFlags_DoubleSided;
            dstMaterial.mFlags_AlphaAdditive = srcMaterial.mFlags_AlphaAdditive;
            dstMaterial.mFlags_Translucent = srcMaterial.mFlags_Translucent;
            dstMaterial.mFlags_HasSpecular = srcMaterial.mFlags_HasSpecular;
            dstMaterial.mFlags_HasEmissive = srcMaterial.mFlags_HasEmissive;
            dstMaterial.mFlags_Invisible = srcMaterial.mFlags_Invisible;
            // params
            dstMaterial.mEmissive = srcMaterial.mEmissive;
            dstMaterial.mSpecular = srcMaterial.mSpecular;
        }

        // get or load textures
        dstMaterial.mDiffuseTextures.clear();
        for (const std::string& roller: dstMaterial.mDiffuseTextureNames)
        {
            Texture* texture = gTextureManager.GetTexture(roller);
            cxx_assert(texture);
            dstMaterial.mDiffuseTextures.push_back(texture);
        }

        dstMaterial.mEnvMappingTexture = {};
        if (!dstMaterial.mEnvMappingTextureName.empty())
        {
            dstMaterial.mEnvMappingTexture = gTextureManager.GetTexture(dstMaterial.mEnvMappingTextureName);
            cxx_assert(dstMaterial.mEnvMappingTexture);
        }
    }
    cxx_assert(mFramesCount > 0);
    return true;
}

bool MeshAsset::InitVertexBuffer()
{
    int numVerticesPerFrame = 0;
    for (SubMesh& currentSubMesh: mSubMeshes)
    {
        numVerticesPerFrame += currentSubMesh.mFrameVerticesCount;
    }

    int numVerticesTotal = numVerticesPerFrame * mFramesCount;
    int vbufferLengthBytes = (numVerticesTotal * (sizeof(glm::vec3) + sizeof(glm::vec3))) + (numVerticesPerFrame * sizeof(glm::vec2));
    cxx_assert(vbufferLengthBytes > 0);

    if (vbufferLengthBytes < 1)
        return false;

    mGpuVertexBuffer = gRenderDevice.CreateVertexBuffer(eBufferUsage_Static, vbufferLengthBytes);
    cxx_assert(mGpuVertexBuffer);

    if (mGpuVertexBuffer == nullptr)
        return false;

    // upload vertex attributes
    unsigned char* vbufferptr = (unsigned char*)mGpuVertexBuffer->Lock(BufferAccess_UnsynchronizedWrite);
    cxx_assert(vbufferptr);

    if (vbufferptr)
    {
        // attributes layout:
        // submesh #0 - [texture coords] [positions of all animation frames] [normals of all animation frames]
        // submesh #1 - [texture coords] [positions of all animation frames] [normals of all animation frames]
        // and so on

        int currentBufferOffset = 0;
        for (SubMesh& currentSubMesh: mSubMeshes)
        {
            currentSubMesh.mGpuVBufferOffset = currentBufferOffset;

            int texcoordsDataLength = currentSubMesh.mTexCoords.size() * sizeof(glm::vec2);
            ::memcpy(vbufferptr + currentBufferOffset, currentSubMesh.mTexCoords.data(), texcoordsDataLength);
            currentBufferOffset += texcoordsDataLength;

            int positionsDataLength = currentSubMesh.mVertices.size() * sizeof(glm::vec3);
            ::memcpy(vbufferptr + currentBufferOffset, currentSubMesh.mVertices.data(), positionsDataLength);
            currentBufferOffset += positionsDataLength;

            int normalsDataLength = currentSubMesh.mNormals.size() * sizeof(glm::vec3);
            ::memcpy(vbufferptr + currentBufferOffset, currentSubMesh.mNormals.data(), normalsDataLength);
            currentBufferOffset += normalsDataLength;
        }

        // finish
        if (mGpuVertexBuffer->Unlock())
            return true;

        cxx_assert(false);
    }
    return false;
}

bool MeshAsset::InitIndexBuffer()
{
    int numTriangles = 0;
    for (SubMesh& currentSubMesh: mSubMeshes)
    {
        numTriangles += static_cast<int>(currentSubMesh.mTris.size());
    }
    // ibuffer may be empty
    if (numTriangles < 1)
        return true;

    int ibufferLengthByets = numTriangles * sizeof(glm::ivec3);
    cxx_assert(ibufferLengthByets > 0);

    mGpuIndexBuffer = gRenderDevice.CreateIndexBuffer(eBufferUsage_Static, ibufferLengthByets);
    cxx_assert(mGpuIndexBuffer);

    if (mGpuIndexBuffer == nullptr)
        return false;

    // upload index data
    unsigned char* ibufferptr = (unsigned char*)mGpuIndexBuffer->Lock(BufferAccess_UnsynchronizedWrite);
    cxx_assert(ibufferptr);

    if (ibufferptr)
    {
        int currentBufferOffset = 0;
        for (SubMesh& currentSubMesh: mSubMeshes)
        {
            currentSubMesh.mGpuIBufferOffset = currentBufferOffset;

            int indexDataLength = currentSubMesh.mTris.size() * sizeof(glm::ivec3);
            if (indexDataLength < 1)
                continue;

            ::memcpy(ibufferptr + currentBufferOffset, currentSubMesh.mTris.data(), indexDataLength);
            currentBufferOffset += indexDataLength;
        }

        // finish
        if (mGpuIndexBuffer->Unlock())
            return true;
            
        cxx_assert(false);
    }
    return false;
}

void MeshAsset::InitMaterials()
{
    int materialsCount = static_cast<int>(mMaterials.size());

    mRenderMaterials.resize(materialsCount);
    for (int icounter = 0; icounter < materialsCount; ++icounter)
    {
        const MaterialDesc& desc = mMaterials[icounter];

        SurfaceMaterial& renderMaterial = mRenderMaterials[icounter];
        renderMaterial.Clear();

        renderMaterial.mDiffuseTexture = desc.mDiffuseTextures.front();
        renderMaterial.mEnvMappingTexture = desc.mEnvMappingTexture;

        // render flags
        if (desc.mFlags_HasAlpha)
        {
            renderMaterial.mRenderStates.EnableAlphaBlend(RENDER_STATES_BLENDMODE_ALPHA);
            renderMaterial.mRenderStates.mIsDepthWriteEnabled = true;
        }
        if (desc.mFlags_AlphaAdditive)
        {
            renderMaterial.mRenderStates.EnableAlphaBlend(RENDER_STATES_BLENDMODE_ALPHA_ADDITIVE);
            renderMaterial.mRenderStates.mIsDepthWriteEnabled = false;
        }
    }
}
