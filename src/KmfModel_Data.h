#pragma once

#include "GraphicsDefs.h"
#include "RenderMaterial.h"

// forwards
class BinaryInputStream;

// defines mesh level of details
struct KmfModel_SubMeshLOD
{
public:
    std::vector<glm::ivec3> mTriangleArray;
};

// defines mesh draw material properties
struct KmfModel_SubMeshMaterial
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
};

// defines single piece of model
struct KmfModel_SubMesh
{
public:
    std::vector<glm::vec3> mVertexPositionArray; // contains vertices for all animation frames
    std::vector<glm::vec3> mVertexNormalArray; // contains vertices for all animation frames
    std::vector<glm::vec2> mVertexTexCoordArray; // texcoords does not changes between frames so we have single frame here
    std::vector<KmfModel_SubMeshLOD> mLODsArray;
    int mMaterialIndex = 0;
    int mFrameVerticesCount = 0; // num vertices per animation frame
};

// kmf model data container
class KmfModel_Data: public cxx::noncopyable
{
public:
    std::string mName;

    std::vector<cxx::aabbox> mFramesBounds;
    std::vector<KmfModel_SubMesh> mMeshArray;
    std::vector<KmfModel_SubMeshMaterial> mMaterialsArray;
    glm::vec3 mPosition; // base position offset

    float mScale = 1.0f; // base scale
    float mCubeScale = 1.0f;
    int mFramesCount = 0; // num animation frames, 1 for static models

public:
    KmfModel_Data(const std::string& resourceName);
    ~KmfModel_Data();

    // load model data from binary stream
    // @return false on error
    bool LoadDataFromStream(BinaryInputStream* binaryStream);

    // clear model data
    void Clear();

    // swap kmf model data
    // @param sourceData: Source kmf model data to swap with
    void Exchange(KmfModel_Data& sourceData);

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

    // compute model bounds for each animation frame
    void ComputeBounds();

    // test whether model geometry data is loaded
    inline bool IsModelLoaded() const { return mFramesCount > 0; }

private:
    bool ReadAnimMesh(BinaryInputStream* theStream);
    bool ReadStaticMesh(BinaryInputStream* theStream);
    bool ReadStaticMeshGeometries(BinaryInputStream* theStream, int theNumGeometies);
    bool ReadAnimSprites(BinaryInputStream* theStream, int theNumSprites, int theNumLODs);
    bool ReadSprites(BinaryInputStream* theStream, int theNumSprites, int theNumLODs);
    bool ReadMaterials(BinaryInputStream* theStream);
};