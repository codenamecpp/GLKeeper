#pragma once

// forwards
class BinaryInputStream;

// kmf model data container
class ModelAsset: public cxx::noncopyable
{
public:

    // defines mesh level of details
    struct SubMeshLOD
    {
    public:
        std::vector<glm::ivec3> mTriangleArray;
    };

    // defines mesh draw material properties
    struct SubMeshMaterial
    {
    public:
        std::string mInternalName;
        std::vector<std::string> mTextures;
        std::string mEnvMappingTexture;
        float mBrightness;
        float mGamma;
        // material flags
        bool mFlagHasAlpha : 1;
        bool mFlagShinyness : 1;
        bool mFlagAlphaAdditive : 1;
        bool mFlagEnvironmentMapped : 1;
    };

    // defines single piece of model
    struct SubMesh
    {
    public:
        std::vector<glm::vec3> mVertexPositionArray; // contains vertices for all animation frames
        std::vector<glm::vec3> mVertexNormalArray; // contains vertices for all animation frames
        std::vector<glm::vec2> mVertexTexCoordArray; // texcoords does not changes between frames so we have single frame here
        std::vector<SubMeshLOD> mLODsArray;
        int mMaterialIndex = 0;
        int mFrameVerticesCount = 0; // num vertices per animation frame
    };

    std::string mName; // unique model name
    std::string mInternalName;

    std::vector<cxx::aabbox> mFramesBounds;
    std::vector<SubMesh> mMeshArray;
    std::vector<SubMeshMaterial> mMaterialsArray;
    glm::vec3 mPosition; // base position offset

    float mScale = 1.0f; // base scale
    float mCubeScale = 1.0f;
    int mFramesCount = 0; // num animation frames, 1 for static models

public:
    ModelAsset(const std::string& resourceName);
    ~ModelAsset();

    // load model data
    // @return false on error
    bool Load();

    // clear model data
    void Clear();

    // swap kmf model data
    // @param sourceData: Source kmf model data to swap with
    void Exchange(ModelAsset& sourceData);

    // get vertices of specific frame and submesh
    // @param iSubMesh: Submesh index
    // @param iFrame: Animation frame index
    inline const glm::vec3* GetFrameVertices(int iSubMesh, int iFrame) const
    {
        debug_assert(iFrame < mFramesCount);
        return &mMeshArray[iSubMesh].mVertexPositionArray[iFrame * mMeshArray[iSubMesh].mFrameVerticesCount];
    }

    // get normals of specific frame and submesh
    // @param iSubMesh: Submesh index
    // @param iFrame: Animation frame index
    inline const glm::vec3* GetFrameNormals(int iSubMesh, int iFrame) const
    {
        debug_assert(iFrame < mFramesCount);
        return &mMeshArray[iSubMesh].mVertexNormalArray[iFrame * mMeshArray[iSubMesh].mFrameVerticesCount];
    }

    // get texcoords of specific frame and submesh
    // @param iSubMesh: Submesh index
    // @param iFrame: Animation frame index
    inline const glm::vec2* GetFrameTexcoords(int iSubMesh) const
    {
        return &mMeshArray[iSubMesh].mVertexTexCoordArray[0];
    }

    // get triangles of specific submesh
    // @param iSubMesh: Submesh index
    // @param iLOD: Level of detail
    inline const glm::ivec3* GetTriangles(int iSubMesh, int iLOD) const
    {
        return &mMeshArray[iSubMesh].mLODsArray[iLOD].mTriangleArray[0];
    }

    // get all submesh vertices count for single animation frame
    int GetFrameVerticesCount() const;

    // get all submesh triangles count for specific lod level
    // @param lodIndex: Level of details
    int GetTrianglesCount(int lodIndex) const;

    // compute model bounds for each animation frame
    void ComputeBounds();

    // test whether model geometry data is loaded
    inline bool IsModelLoaded() const { return mFramesCount > 0; }

private:
    bool LoadFromStream(BinaryInputStream* theStream);
    bool ReadAnimMesh(BinaryInputStream* theStream);
    bool ReadStaticMesh(BinaryInputStream* theStream);
    bool ReadStaticMeshGeometries(BinaryInputStream* theStream, int theNumGeometies);
    bool ReadAnimSprites(BinaryInputStream* theStream, int theNumSprites, int theNumLODs);
    bool ReadSprites(BinaryInputStream* theStream, int theNumSprites, int theNumLODs);
    bool ReadMaterials(BinaryInputStream* theStream);
};