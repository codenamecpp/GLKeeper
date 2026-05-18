#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GpuBuffer.h"
#include "GpuTexture2D.h"

//////////////////////////////////////////////////////////////////////////

class MeshAsset : public cxx::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////

    struct MaterialDesc
    {
    public:
        std::string mName;

        // list of texture variations including primary one (at index 0)
        std::vector<std::string> mDiffuseTextureNames;
        std::vector<Texture*> mDiffuseTextures;

        std::string mEnvMappingTextureName;
        Texture* mEnvMappingTexture = nullptr;

        // material flags
        union 
        {
            struct
            {
                // masked / alpha-test geometry (ALPHABLENDENABLE)
                // Usage: masked UI/arrows and some decals
                // Often combined with DOUBLE_SIDED
                bool mFlags_HasAlpha : 1;

                // disable back-face culling / surface modifier
                // Usage: thin geometry, blades, tails, some candles/bottles
                bool mFlags_DoubleSided : 1;

                // additive blending (FX/glows/slashes)
                // Usage: additive effects, slashes, falloff rays
                // ALPHABLENDENABLE/ZWRITE + SRC/DST blend factors switched to create additive/falloff
                bool mFlags_AlphaAdditive : 1;

                // translucent / ice-like material
                bool mFlags_Translucent : 1;

                // use specular value
                // Usage: metal/weapon/reflective surfaces, swords, correlated with `specular` and envMap
                bool mFlags_HasSpecular : 1;

                // use emissive value
                // Usage: lava, torches, portal gems, illuminated props
                bool mFlags_HasEmissive : 1;

                // invisible / placeholder / env-only
                // Usage: invisible placeholders like "InvisibleKnightBack"
                bool mFlags_Invisible : 1;
            };

            unsigned int mMatFlags = 0;
        };

        // emissive value, used with mFlags_HasEmissive
        // 0.2 dungeon heart, 0.4 portal, 0.6 gold, 0.75 torch/lava/fire, 1.0 firefly tail/Torture_Trough/Jack Box
        float mEmissive = 0.0f; 

        // specular value, used with mFlags_HasSpecular
        // 0.2 pickimpback/armor/mistress suit, 0.3 GuardFrnt/DarkAngel sword, 0.4/0.5 swords/blades/scythe, 1 128gem1/#mask#gembit/Brickex1
        float mSpecular = 0.0f;
    };

    //////////////////////////////////////////////////////////////////////////

    enum class eAnimFrameFactorFunc { Clamp, Wrap };

    //////////////////////////////////////////////////////////////////////////

    struct SubMesh
    {
    public:
        std::vector<glm::ivec3> mTris;
        std::vector<glm::vec3> mVertices; // contains vertices for all animation frames
        std::vector<glm::vec3> mNormals; // contains vertices for all animation frames
        std::vector<glm::vec2> mTexCoords; // texcoords does not changes between frames so we have single frame here

        int mFrameVerticesCount = 0; // num vertices per animation frame
        int mMaterialIndex = 0; // material index in mMaterials

        // extra render data, just for those meshes which will be drawn
        int mGpuVBufferOffset = 0;
        int mGpuIBufferOffset = 0;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    MeshAsset(const std::string& meshName);

    const std::string& GetMeshName() const { return mMeshName; }

    // loads mesh data from file; does not create render data automatically
    // will initialize default mesh if loading fails
    void Load();

    // init fallback mesh
    void InitDefault();

    // uploads system bits to backend; the mesh data must be inited beforehand
    // will init fallback render data on failure
    void InitRenderData();

    // reset system bits and destroy render data 
    void Purge();

    // returns true if either mesh is loaded from file or it is the default one
    inline bool IsInited() const { return mIsLoadedFromFile || mIsDefaultMesh; }
    inline bool IsLoadedFromFile() const { return mIsLoadedFromFile; }
    inline bool IsDefaultMesh() const { return mIsDefaultMesh; }
    inline bool IsRenderDataInited() const { return mIsRenderDataInited; }

    // Get frame vertices of specific submesh
    inline const glm::vec3* GetFrameVertices(int iSubMesh, int iFrame) const
    {
        const SubMesh& subMeshData = mSubMeshes[iSubMesh];
        return &subMeshData.mVertices[iFrame * subMeshData.mFrameVerticesCount];
    }

    // Get frame normals of specific submesh
    inline const glm::vec3* GetFrameNormals(int iSubMesh, int iFrame) const
    {
        const SubMesh& subMeshData = mSubMeshes[iSubMesh];
        return &subMeshData.mVertices[iFrame * subMeshData.mFrameVerticesCount];
    }

    // Get frame texcoords of specific submesh
    inline const glm::vec2* GetFrameTexcoords(int iSubMesh) const
    {
        const SubMesh& subMeshData = mSubMeshes[iSubMesh];
        return subMeshData.mTexCoords.data();
    }

    // Get triangles of specific submesh
    inline const glm::ivec3* GetTriangles(int iSubMesh) const
    {
        const SubMesh& subMeshData = mSubMeshes[iSubMesh];
        return subMeshData.mTris.data();
    }

    inline const std::vector<SubMesh>& GetSubMeshList() const { return mSubMeshes; }
    inline const std::vector<MaterialDesc>& GetMeterialsList() const { return mMaterials; }
    inline const MaterialDesc& GetMaterialByIndex(int materialIndex) const
    {
        return mMaterials[materialIndex];
    }

    inline int GetAnimFramesCount() const { return mFramesCount; }
    inline int GetSubMeshCount() const 
    { 
        return static_cast<int>(mSubMeshes.size());
    }
    inline int GetMaterialsCount() const
    {
        return static_cast<int>(mMaterials.size());
    }

    inline eAnimFrameFactorFunc GetAnimFrameFactorFunc() const
    {
        return mAnimFrameFactorFunc;
    }

    inline const std::vector<cxx::aabbox>& GetBoundsList() const { return mFramesBounds; }
    inline const cxx::aabbox& GetBounds(int animFrameIndex) const
    {
        return mFramesBounds[animFrameIndex];
    }
    inline const cxx::aabbox& GetBounds() const
    {
        return mFramesBounds.front();
    }
    inline const cxx::aabbox& GetMaxBounds() const { return mMaxBounds; }

    // accessing render data, make sure IsRenderDataInited true
    inline const std::vector<SurfaceMaterial>& GetRenderMaterialsList() const { return mRenderMaterials; }
    inline const SurfaceMaterial& GetRenderMaterial(int materialIndex) const
    {
        return mRenderMaterials[materialIndex];
    }
    inline GpuVertexBuffer* GetGpuVertexBuffer() const { return mGpuVertexBuffer.get(); }
    inline GpuIndexBuffer* GetGpuIndexBuffer() const { return mGpuIndexBuffer.get(); }

private:
    bool LoadMeshFromFile();
    bool InitVertexBuffer();
    bool InitIndexBuffer();
    void InitMaterials();

private:
    std::string mMeshName;
    std::string mInternalName;

    std::vector<cxx::aabbox> mFramesBounds;
    cxx::aabbox mMaxBounds; // combines all frames bounds
    std::vector<SubMesh> mSubMeshes;
    std::vector<MaterialDesc> mMaterials;
    glm::vec3 mTranslation;
    float mBaseScale = 1.0f;
    float mCubeScale = 1.0f;
    int mFramesCount = 1; // num animation frames, 1 for static models

    eAnimFrameFactorFunc mAnimFrameFactorFunc = eAnimFrameFactorFunc::Clamp;

    // render data
    std::vector<SurfaceMaterial> mRenderMaterials;
    std::unique_ptr<GpuVertexBuffer> mGpuVertexBuffer;
    std::unique_ptr<GpuIndexBuffer> mGpuIndexBuffer;

    bool mIsLoadedFromFile = false;
    bool mIsDefaultMesh = false; 
    bool mIsRenderDataInited = false;
};