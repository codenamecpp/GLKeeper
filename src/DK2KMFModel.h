#pragma once

//////////////////////////////////////////////////////////////////////////

struct DK2KMFModel
{
public:
    
    //////////////////////////////////////////////////////////////////////////

    struct MaterialDesc
    {
    public:
        std::string mName;
        std::string mEnvMappingTextureName;
        std::vector<std::string> mDiffuseTextureNames;

        float mEmissive = 0.0f;
        float mSpecular = 0.0f;

        // flags
        bool mFlags_HasAlpha : 1;
        bool mFlags_DoubleSided : 1;
        bool mFlags_AlphaAdditive : 1;
        bool mFlags_Translucent : 1;
        bool mFlags_HasSpecular : 1;
        bool mFlags_HasEmissive : 1;
        bool mFlags_Invisible : 1;
    };

    //////////////////////////////////////////////////////////////////////////

    // defines mesh level of details
    struct SubMeshLOD
    {
    public:
        std::vector<glm::ivec3> mTriangleArray;
    };

    //////////////////////////////////////////////////////////////////////////

    // piece of geometry that shares single material
    struct SubMesh
    {
    public:
        std::vector<glm::vec3> mVertexPositionArray; // contains vertices for all animation frames
        std::vector<glm::vec3> mVertexNormalArray; // contains vertices for all animation frames
        std::vector<glm::vec2> mVertexTexCoordArray; // texcoords does not changes between frames so we have single frame here

        std::vector<SubMeshLOD> mLODsArray;

        int mFrameVerticesCount = 0; // num vertices per animation frame
        int mMaterialIndex = 0;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    DK2KMFModel() = default;

    void Clear();
    void Swap(DK2KMFModel& rhs);

public:
    std::string mName;

    std::vector<cxx::aabbox> mFramesBounds;

    std::vector<SubMesh> mMeshArray;
    std::vector<MaterialDesc> mMaterialsArray;

    glm::vec3 mTranslation; // base position offset

    float mScale = 1.0f; // base scale
    float mCubeScale = 1.0f;

    int mFrameFactorFunction = 0; // clamp, wrap
    int mFramesCount = 0; // num animation frames, 1 for static models
};

//////////////////////////////////////////////////////////////////////////

bool DK2_KMF_LoadFromStream(std::istream& theStream, DK2KMFModel& outputData);

//////////////////////////////////////////////////////////////////////////