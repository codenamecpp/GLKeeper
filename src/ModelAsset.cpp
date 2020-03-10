#include "pch.h"
#include "ModelAsset.h"
#include "BinaryInputStream.h"
#include "TexturesManager.h"
#include "Console.h"
#include "FileSystem.h"

#define MAKE_HEADER_ID(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

static const unsigned int KMF_HEADER_IDENTIFIER         = MAKE_HEADER_ID('K','M','S','H');
static const unsigned int KMF_HEAD                      = MAKE_HEADER_ID('H','E','A','D');
static const unsigned int KMF_MATERIALS                 = MAKE_HEADER_ID('M','A','T','L');
static const unsigned int KMF_MATERIAL                  = MAKE_HEADER_ID('M','A','T','2');
static const unsigned int KMF_MESH                      = MAKE_HEADER_ID('M','E','S','H');
static const unsigned int KMF_MESH_CONTROL              = MAKE_HEADER_ID('C','T','R','L');
static const unsigned int KMF_MESH_SPRITES              = MAKE_HEADER_ID('S','P','R','S');
static const unsigned int KMF_MESH_SPRITES_HEADER       = MAKE_HEADER_ID('S','P','H','D');
static const unsigned int KMF_MESH_SPRITES_DATA_HEADER  = MAKE_HEADER_ID('S','P','R','S');
static const unsigned int KMF_MESH_GEOM                 = MAKE_HEADER_ID('G','E','O','M');
static const unsigned int KMF_ANIM                      = MAKE_HEADER_ID('A','N','I','M');
static const unsigned int KMF_ANIM_SPRITES_POLY_HEADER  = MAKE_HEADER_ID('P','O','L','Y');
static const unsigned int KMF_ANIM_SPRITES_VERT_HEADER  = MAKE_HEADER_ID('V','E','R','T');
static const unsigned int KMF_ANIM_SPRITES_ITAB_HEADER  = MAKE_HEADER_ID('I','T','A','B');
static const unsigned int KMF_ANIM_SPRITES_VGEO_HEADER  = MAKE_HEADER_ID('V','G','E','O');
static const unsigned int KMF_GROP                      = MAKE_HEADER_ID('G','R','O','P');
static const unsigned int KMF_GROP_ELEM                 = MAKE_HEADER_ID('E','L','E','M');

// for internal usage
struct KMFAnimGeom
{
    short mFrameBase;
    glm::vec3 mGeometry;
};

//////////////////////////////////////////////////////////////////////////

struct KMFHeader
{
    unsigned int mID;
    unsigned int mLength; // include size of KMFHeader
};

enum 
{ 
    KMF_CHUNK_HEADER_LENGTH = sizeof(KMFHeader),

    // limits
    KMF_MAX_STRING_LENGTH = 128
};

// material flags
enum
{
    KMF_MATERIAL_HAS_ALPHA = 0x01,
    KMF_MATERIAL_SHINYNESS = 0x02,
    KMF_MATERIAL_ALPHA_ADDITIVE = 0x04,
    KMF_MATERIAL_ENVIRONMENT_MAPPED = 0x0100,
};

enum KMF_MeshType: unsigned int
{
    DK2_MESH_TYPE_STATIC = 1,
    DK2_MESH_TYPE_ANIMATED = 2,
    DK2_MESH_TYPE_GROUP = 3
};

//////////////////////////////////////////////////////////////////////////

#define READ_FROM_BYTE_STREAM(thestream, theoutput) \
    { \
        if (thestream->ReadData(&theoutput, sizeof(theoutput)) != sizeof(theoutput)) \
        { \
            debug_assert(false); \
            return false; \
        } \
    }

#define SKIP_BYTES(thestream, numbytes) \
    { \
        if (!thestream->AdvanceCursorPosition(numbytes)) \
        { \
            debug_assert(false); \
        } \
    }

inline bool KMF_ReadCString(BinaryInputStream* theStream, std::string& outputString)
{
    char charsBuffer[KMF_MAX_STRING_LENGTH];
    
    int currentLength = 0;
    for (; currentLength < KMF_MAX_STRING_LENGTH; ++currentLength)
    {
        if (theStream->ReadData(&charsBuffer[currentLength], sizeof(char)) == 0)
        {
            debug_assert(false);
            return false;
        }
        if (charsBuffer[currentLength] == 0)
            break;
    }
    outputString.assign(charsBuffer);
    return true;
}

inline bool KMF_ReadSectionHeader(BinaryInputStream* theStream, KMFHeader& theHeader, unsigned int theExpectedID)
{
    theStream->ReadData(&theHeader.mID, sizeof(theHeader.mID));
    theStream->ReadData(&theHeader.mLength, sizeof(theHeader.mLength));

    debug_assert(theHeader.mID == theExpectedID);
    return theHeader.mID == theExpectedID;
}

inline bool KMF_ReadVector3f(BinaryInputStream* theStream, glm::vec3& theVector)
{
    float vector3f[3];
    READ_FROM_BYTE_STREAM(theStream, vector3f[0]);
    READ_FROM_BYTE_STREAM(theStream, vector3f[1]);
    READ_FROM_BYTE_STREAM(theStream, vector3f[2]);
    theVector.x = vector3f[0];
    theVector.y = -vector3f[2]; // correct axes
    theVector.z = vector3f[1];
    return true;
}

inline bool KMF_ReadTriangle(BinaryInputStream* theStream, glm::ivec3& theTriangle)
{
    unsigned char i0, i1, i2;
    READ_FROM_BYTE_STREAM(theStream, i0);
    READ_FROM_BYTE_STREAM(theStream, i1);
    READ_FROM_BYTE_STREAM(theStream, i2);
    theTriangle.z = i1;
    theTriangle.y = i2;
    theTriangle.x = i0;
    return true;
}

inline bool KMF_ReadTexcoord(BinaryInputStream* theStream, glm::vec2& theTexcoord)
{
    unsigned short uv[2];
    READ_FROM_BYTE_STREAM(theStream, uv[0]);
    READ_FROM_BYTE_STREAM(theStream, uv[1]);
    // decode texture coordinates
    theTexcoord.x = uv[0] / 32768.0f;
    theTexcoord.y = uv[1] / 32768.0f;
    return true;
}

inline void KMF_EncodeGeomITab(glm::vec3& thePosition, unsigned short theIndex)
{
    ::memcpy(&thePosition, &theIndex, sizeof(theIndex));
}

inline void KMF_DecodeGeomITab(glm::vec3& thePosition, unsigned short& theIndex)
{
    ::memcpy(&theIndex, &thePosition, sizeof(theIndex));
}

//////////////////////////////////////////////////////////////////////////

ModelAsset::ModelAsset(const std::string& resourceName)
    : mName(resourceName)
{
}

ModelAsset::~ModelAsset()
{
    Clear();
}

bool ModelAsset::Load()
{
    // open stream
    BinaryInputStream* theStream = gFileSystem.OpenDataFile(mName);
    if (theStream == nullptr)
    {
        debug_assert(false);
        return false;
    }

    Clear();

    bool isLoaded = LoadFromStream(theStream);
    if (isLoaded)
    {
        ComputeBounds();
    }
    gFileSystem.CloseFileStream(theStream);
    return isLoaded;
}

bool ModelAsset::LoadFromStream(BinaryInputStream* theStream)
{
    KMFHeader header;
    if (!KMF_ReadSectionHeader(theStream, header, KMF_HEADER_IDENTIFIER))
        return false;

    unsigned int formatVersion = 0;
    READ_FROM_BYTE_STREAM(theStream, formatVersion);

    //KMSH/HEAD
    if (!KMF_ReadSectionHeader(theStream, header, KMF_HEAD))
        return false;

    KMF_MeshType meshType;
    READ_FROM_BYTE_STREAM(theStream, meshType); // mesh type

    bool meshTypeSupported = meshType == DK2_MESH_TYPE_STATIC || 
        meshType == DK2_MESH_TYPE_ANIMATED;// || 
                                           //meshType == DK2_MESH_TYPE_GROUP;

                                           // TODO : add groups support

    debug_assert(meshTypeSupported);
    if (!meshTypeSupported)
        return false;

    SKIP_BYTES(theStream, 4); // unknown data

                              //KMSH/MATL
    if (meshType != DK2_MESH_TYPE_GROUP)
    {
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MATERIALS))
            return false;

        if (!ReadMaterials(theStream))
            return false;
    }

    //KMSH/MESH
    if (meshType == DK2_MESH_TYPE_STATIC)
    {
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH)) 
            return false;

        if (!ReadStaticMesh(theStream))
            return false;
    }

    //KMSH/ANIM
    if (meshType == DK2_MESH_TYPE_ANIMATED)
    {
        if (!KMF_ReadSectionHeader(theStream, header, KMF_ANIM)) 
            return false;

        if (!ReadAnimMesh(theStream))
            return false;
    }

    return true;
}

void ModelAsset::Clear()
{
    mFramesBounds.clear();
    mMeshArray.clear();
    mMaterialsArray.clear();
    mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    mScale = 1.0f;
    mCubeScale = 1.0f;
    mFramesCount = 0;
}

void ModelAsset::Exchange(ModelAsset& sourceData)
{
    std::swap(mName, sourceData.mName);

    std::swap(mFramesBounds, sourceData.mFramesBounds);
    std::swap(mMeshArray, sourceData.mMeshArray);
    std::swap(mMaterialsArray, sourceData.mMaterialsArray);
    std::swap(mPosition, sourceData.mPosition);

    std::swap(mScale, sourceData.mScale);
    std::swap(mCubeScale, sourceData.mCubeScale);
    std::swap(mFramesCount, sourceData.mFramesCount);
}

void ModelAsset::ComputeBounds()
{
    mFramesBounds.resize(mFramesCount);

    // process animation frames
    for (int iAnimFrame = 0; iAnimFrame < mFramesCount; ++iAnimFrame)
    {
        cxx::aabbox& bounds = mFramesBounds[iAnimFrame];
        bounds.reset();
        for (const SubMesh& subMesh: mMeshArray)
        {
            const glm::vec3* verticesPos = &subMesh.mVertexPositionArray[iAnimFrame * subMesh.mFrameVerticesCount];
            for (int ivertex = 0; ivertex < subMesh.mFrameVerticesCount; ++ivertex)
            {
                bounds.extend(verticesPos[ivertex]);
            }
        }
    } // for
}

bool ModelAsset::ReadAnimMesh(BinaryInputStream* theStream)
{
    KMFHeader header;

    //KMSH/ANIM/HEAD
    if (!KMF_ReadSectionHeader(theStream, header, KMF_HEAD))
        return false;

    if (!KMF_ReadCString(theStream, mInternalName))
        return false;

    unsigned int sprsCount;
    unsigned int frameCount;
    unsigned int indexCount;
    unsigned int geomCount;
    unsigned int frameFactorFunction;
    float cubeScale;
    float scale;
    unsigned int numLODs;

    // read data
    READ_FROM_BYTE_STREAM(theStream, sprsCount);
    READ_FROM_BYTE_STREAM(theStream, frameCount);
    READ_FROM_BYTE_STREAM(theStream, indexCount);
    READ_FROM_BYTE_STREAM(theStream, geomCount);
    READ_FROM_BYTE_STREAM(theStream, frameFactorFunction);

    mFramesCount = frameCount;

    if (!KMF_ReadVector3f(theStream, mPosition))
        return false;

    READ_FROM_BYTE_STREAM(theStream, cubeScale);
    READ_FROM_BYTE_STREAM(theStream, scale);
    mCubeScale = cubeScale;
    mScale = scale;

    READ_FROM_BYTE_STREAM(theStream, numLODs);

    //KMSH/ANIM/CTRL
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_CONTROL))
        return false;

    SKIP_BYTES(theStream, header.mLength - KMF_CHUNK_HEADER_LENGTH); // controls are unknown 

    //KMSH/ANIM/SPRS
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_SPRITES))
        return false;

    if (!ReadAnimSprites(theStream, sprsCount, numLODs))
        return false;

    //KMSH/ANIM/SPRS/ITAB
    if (!KMF_ReadSectionHeader(theStream, header, KMF_ANIM_SPRITES_ITAB_HEADER))
        return false;

    const unsigned int NumItabChunks = static_cast<unsigned int>(floorf((frameCount - 1) / 128.0f + 1.0f));

    // allocate memory for itab chunks
    std::vector<unsigned int> itabChunks(NumItabChunks * indexCount);

    for (unsigned int iChunk = 0; iChunk < NumItabChunks; ++iChunk)
    for (unsigned int iIndex = 0; iIndex < indexCount; ++iIndex)
    {
        READ_FROM_BYTE_STREAM(theStream, itabChunks[iChunk * indexCount + iIndex]);
    }

    //KMSH/ANIM/SPRS/GEOM
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_GEOM))
        return false;

    std::vector<KMFAnimGeom> geometries;
    geometries.resize(geomCount);

    for (KMFAnimGeom& animGeometry : geometries)
    {
        //10 _BITS_ per coordinate (Z, Y, X) = 30 bits (2 last bits can be thrown away)
        // ^ so read 4 bytes
        // + 1 byte for frame base

        int coordinates;
        READ_FROM_BYTE_STREAM(theStream, coordinates);

        const float x = (((coordinates >> 20) & 0x3ff) - 0x200) / 511.0f; // x 
        const float y = (((coordinates >> 10) & 0x3ff) - 0x200) / 511.0f; // y
        const float z = (((coordinates >> 0) & 0x3ff) - 0x200) / 511.0f; // z

        // prescale
        animGeometry.mGeometry.x = x * mScale;
        animGeometry.mGeometry.z = y * mScale;
        animGeometry.mGeometry.y = -z * mScale;
     
        // frame base
        unsigned char frameBase;
        READ_FROM_BYTE_STREAM(theStream, frameBase);
        animGeometry.mFrameBase = frameBase;
    }

    //KMSH/ANIM/SPRS/VGEO
    if (!KMF_ReadSectionHeader(theStream, header, KMF_ANIM_SPRITES_VGEO_HEADER))
        return false;

    std::vector<unsigned char> offsets(indexCount * frameCount);

    for (unsigned int iIndex = 0; iIndex < indexCount; ++iIndex)
    for (unsigned int iFrame = 0; iFrame < frameCount; ++iFrame)
    {
        READ_FROM_BYTE_STREAM(theStream, offsets[iIndex * frameCount + iFrame]);
    }

    // unpack animation frames
    for (SubMesh& submesh: mMeshArray)
    {
        std::vector<glm::vec3> frame0_normals;
        std::vector<glm::vec3> frame0_positions;

        frame0_normals.swap(submesh.mVertexNormalArray);
        frame0_positions.swap(submesh.mVertexPositionArray);

        submesh.mVertexNormalArray.resize(frameCount * submesh.mFrameVerticesCount);
        submesh.mVertexPositionArray.resize(frameCount * submesh.mFrameVerticesCount);

        for (unsigned int iframe = 0; iframe < frameCount; ++iframe)
        {

            for (int ivertex = 0; ivertex < submesh.mFrameVerticesCount; ++ivertex)
            {
                unsigned short itabIndex;
                KMF_DecodeGeomITab(frame0_positions[ivertex], itabIndex);

                // magic
                const int geomIndex = itabChunks[indexCount * (iframe >> 7) + itabIndex] + offsets[itabIndex * frameCount + iframe];

                //Simple interpolation must be used to discover the progression from the most
                //recent keyframe. The frameBase field of GEOM is used in this interpolation
                //to determine percentage progressed for a given frame, and the .next. geom
                //element is always geomIndex + 1. The offset value resets at each 128 frame
                //chunk. Calculation of the the interpolation geomFactor for a given geomIndex
                //is as follows:

                //frameBase = GEOM.geom[geomIndex].frameBase
                //nextFrameBase = GEOM.geom[geomIndex + 1].frameBase

                //geomFactor = ((frame&0x7f) ?frameBase)/(nextFrameBase ?frameBase)
                //This makes calculation of the interpolated position at a given geomIndex
                //as follows:
                //coord = GEOM.geom[geomIndex].coord
                //nextCoord = GEOM.geom[geomIndex + 1].coord
                //interpCoord = (nextCoord ? coord) ? geomFactor + coord

                const int geomIndexNext = (geomIndex + 1) < (int)geomCount ? (geomIndex + 1) : geomIndex;

                const KMFAnimGeom& currGeom = geometries[geomIndex];
                const KMFAnimGeom& nextGeom = geometries[geomIndexNext];

                float geomFactor = 1.0f;
                if (currGeom.mFrameBase != nextGeom.mFrameBase)
                {
                    geomFactor = (((iframe & 0x7F) - currGeom.mFrameBase) * 1.0f) / ((nextGeom.mFrameBase - currGeom.mFrameBase) * 1.0f);
                }

                int lastPose = ((iframe >> 7) * 128 + currGeom.mFrameBase);
                int nextPose = ((iframe >> 7) * 128 + nextGeom.mFrameBase);

                int idx = iframe * submesh.mFrameVerticesCount + ivertex;

                submesh.mVertexNormalArray[idx] = frame0_normals[ivertex];
                submesh.mVertexPositionArray[idx] = (nextGeom.mGeometry - currGeom.mGeometry) * geomFactor + currGeom.mGeometry;
            }

        }
    }
    return true;
}

bool ModelAsset::ReadStaticMesh(BinaryInputStream* theStream)
{
    KMFHeader header;

    //KMSH/MESH/HEAD
    if (!KMF_ReadSectionHeader(theStream, header, KMF_HEAD))
        return false;

    if (!KMF_ReadCString(theStream, mInternalName))
        return false;

    mFramesCount = 1; // single frame for static mesh

    unsigned int numSprites;
    unsigned int numGeometries;
    unsigned int numLODs;
    READ_FROM_BYTE_STREAM(theStream, numSprites);
    READ_FROM_BYTE_STREAM(theStream, numGeometries);
    
    if (!KMF_ReadVector3f(theStream, mPosition))
        return false;

    float scale;
    READ_FROM_BYTE_STREAM(theStream, scale);
    READ_FROM_BYTE_STREAM(theStream, numLODs);
    mScale = scale;

    //Controls
    //KMSH/MATL/CTRL
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_CONTROL))
        return false;

    SKIP_BYTES(theStream, header.mLength - KMF_CHUNK_HEADER_LENGTH); // controls are unknown 

    //Sprites
    //KMSH/MESH/SPRS
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_SPRITES))
        return false;

    if (!ReadSprites(theStream, numSprites, numLODs))
        return false;

    //KMSH/MESH/GEOM
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_GEOM))
        return false;

    if (!ReadStaticMeshGeometries(theStream, numGeometries))
        return false;

    // success
    return true;
}

bool ModelAsset::ReadStaticMeshGeometries(BinaryInputStream* theStream, int theNumGeometies)
{
    std::vector<glm::vec3> geometries;
    geometries.resize(theNumGeometies);

    // read geometries
    for (glm::vec3& refGeom: geometries)
    {
        if (!KMF_ReadVector3f(theStream, refGeom))
            return false;
    }

    // decode geometries
    for (SubMesh& refSprite: mMeshArray)
        for (glm::vec3& thePosition: refSprite.mVertexPositionArray)
        {
            unsigned short geo;
            KMF_DecodeGeomITab(thePosition, geo);
            thePosition = geometries[geo]; // set vertex position
        } // for
    return true;
}

bool ModelAsset::ReadAnimSprites(BinaryInputStream* theStream, int theNumSprites, int theNumLODs)
{
    debug_assert(theNumSprites > 0 && theNumLODs > 0);

    KMFHeader header;
    // allocate sprites
    mMeshArray.resize(theNumSprites);

    // read headers
    for (SubMesh& refSprite: mMeshArray)
    {
        //KMSH/ANIM/SPRS/SPHD
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_SPRITES_HEADER))
            return false;

        // allocate lods for each submesh
        if (theNumLODs > 0)
        {
            refSprite.mLODsArray.resize(theNumLODs);
        }

        // read triangles count for each lod
        for (int iCurrentLOD = 0; iCurrentLOD < theNumLODs; ++iCurrentLOD)
        {
            int numTriangles = 0;
            READ_FROM_BYTE_STREAM(theStream, numTriangles);

            // allocate triangles for each lod
            if (numTriangles > 0)
            {
                refSprite.mLODsArray[iCurrentLOD].mTriangleArray.resize(numTriangles);
            }
        }

        // geom info
        int numVertices;
        float mmFactor;
        READ_FROM_BYTE_STREAM(theStream, numVertices);
        READ_FROM_BYTE_STREAM(theStream, mmFactor);

        refSprite.mFrameVerticesCount = numVertices;
        if (numVertices > 0)
        {
            refSprite.mVertexPositionArray.resize(numVertices);
            refSprite.mVertexNormalArray.resize(numVertices);
            refSprite.mVertexTexCoordArray.resize(numVertices);
        }
    }

    // read geometries
    for (SubMesh& refSprite: mMeshArray)
    {
        //KMSH/ANIM/SPRS/SPRS
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_SPRITES_DATA_HEADER))
            return false;

        int materialIndex;
        READ_FROM_BYTE_STREAM(theStream, materialIndex);
        refSprite.mMaterialIndex = materialIndex;

        //KMSH/ANIM/SPRS/SPRS/POLY
        if (!KMF_ReadSectionHeader(theStream, header, KMF_ANIM_SPRITES_POLY_HEADER))
            return false;

        // read triangles for each lod
        for (SubMeshLOD& currentLOD: refSprite.mLODsArray)
        {
            for (glm::ivec3& refTriangle: currentLOD.mTriangleArray)
            {
                KMF_ReadTriangle(theStream, refTriangle);
            }
        }

        //KMSH/ANIM/SPRS/SPRS/VERT
        if (!KMF_ReadSectionHeader(theStream, header, KMF_ANIM_SPRITES_VERT_HEADER))
            return false;

        // read vertices
        for (int ivertex = 0; ivertex < refSprite.mFrameVerticesCount; ++ivertex)
        {
            KMF_ReadTexcoord(theStream, refSprite.mVertexTexCoordArray[ivertex]);
            KMF_ReadVector3f(theStream, refSprite.mVertexNormalArray[ivertex]);

            unsigned short geometryIndex;
            READ_FROM_BYTE_STREAM(theStream, geometryIndex);
            // hack to save geometry index in position vector
            KMF_EncodeGeomITab(refSprite.mVertexPositionArray[ivertex], geometryIndex);
        }
    }
    return true;
}

bool ModelAsset::ReadSprites(BinaryInputStream* theStream, int theNumSprites, int theNumLODs)
{
    debug_assert(theNumSprites > 0 && theNumLODs > 0);

    KMFHeader header;

    // allocate sprites
    mMeshArray.resize(theNumSprites);

    // read headers
    for (SubMesh& refSprite: mMeshArray)
    {
        //KMSH/MESH/SPRS/SPHD
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_SPRITES_HEADER))
            return false;

        // allocate lods for each submesh
        if (theNumLODs > 0)
        {
            refSprite.mLODsArray.resize(theNumLODs);
        }

        // read triangles count for each lod
        for (int iCurrentLOD = 0; iCurrentLOD < theNumLODs; ++iCurrentLOD)
        {
            int numTriangles = 0;
            READ_FROM_BYTE_STREAM(theStream, numTriangles);

            // allocate triangles for each lod
            if (numTriangles > 0)
            {
                refSprite.mLODsArray[iCurrentLOD].mTriangleArray.resize(numTriangles);
            }
        }
       
        // geom info
        int numVertices;
        float mmFactor;
        READ_FROM_BYTE_STREAM(theStream, numVertices);
        READ_FROM_BYTE_STREAM(theStream, mmFactor);

        refSprite.mFrameVerticesCount = numVertices; 
        if (numVertices > 0)
        {
            refSprite.mVertexPositionArray.resize(numVertices);
            refSprite.mVertexNormalArray.resize(numVertices);
            refSprite.mVertexTexCoordArray.resize(numVertices);
        }

    } // for

    bool hasDummySprites = false;

    // read geometries
    for (SubMesh& refSprite: mMeshArray)
    {
        //KMSH/MESH/SPRS/SPRS
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_SPRITES_DATA_HEADER))
            return false;

        if (refSprite.mLODsArray.empty())
        {
            hasDummySprites = true;
            SKIP_BYTES(theStream, header.mLength - KMF_CHUNK_HEADER_LENGTH);
            continue;
        }

        int materialIndex;
        READ_FROM_BYTE_STREAM(theStream, materialIndex);
        refSprite.mMaterialIndex = materialIndex;

        // read triangles for each lod
        for (SubMeshLOD& currentLOD: refSprite.mLODsArray)
        {
            for (glm::ivec3& refTriangle: currentLOD.mTriangleArray)
            {
                KMF_ReadTriangle(theStream, refTriangle);
            }
        }

        // read vertices
        for (int ivertex = 0; ivertex < refSprite.mFrameVerticesCount; ++ivertex)
        {
            unsigned short geometryIndex;
            READ_FROM_BYTE_STREAM(theStream, geometryIndex);
            // hack to save geometry index in position vector
            KMF_EncodeGeomITab(refSprite.mVertexPositionArray[ivertex], geometryIndex);
            KMF_ReadTexcoord(theStream, refSprite.mVertexTexCoordArray[ivertex]);
            KMF_ReadVector3f(theStream, refSprite.mVertexNormalArray[ivertex]);
        }

    } // for sprite
    if (hasDummySprites)
    {
        gConsole.LogMessage(eLogMessage_Debug, "Dummy sprites found in model '%s'", mName.c_str());
    }
    return true;
}

bool ModelAsset::ReadMaterials(BinaryInputStream* theStream)
{
    int numMaterials;
    READ_FROM_BYTE_STREAM(theStream, numMaterials);

    mMaterialsArray.resize(numMaterials);
    // read materials
    for (SubMeshMaterial& currentMaterial: mMaterialsArray)
    {
        KMFHeader header;
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MATERIAL))
            return false;

        unsigned int materialChunkSize;
        READ_FROM_BYTE_STREAM(theStream, materialChunkSize);
        if (!KMF_ReadCString(theStream, currentMaterial.mInternalName))
            return false;

        int numtextures = 0;
        READ_FROM_BYTE_STREAM(theStream, numtextures);
        // reading texture names
        currentMaterial.mTextures.resize(numtextures);
        for (int itexture = 0; itexture < numtextures; ++itexture)
        {
            if (!KMF_ReadCString(theStream, currentMaterial.mTextures[itexture]))
                return false;
        }

        unsigned int materialFlags;
        READ_FROM_BYTE_STREAM(theStream, materialFlags);
        READ_FROM_BYTE_STREAM(theStream, currentMaterial.mBrightness);
        READ_FROM_BYTE_STREAM(theStream, currentMaterial.mGamma);

        // setup material flags
        currentMaterial.mFlagHasAlpha = (materialFlags & KMF_MATERIAL_HAS_ALPHA) > 0;
        currentMaterial.mFlagAlphaAdditive = (materialFlags & KMF_MATERIAL_ALPHA_ADDITIVE) > 0;
        currentMaterial.mFlagShinyness = (materialFlags & KMF_MATERIAL_SHINYNESS) > 0;
        currentMaterial.mFlagEnvironmentMapped = (materialFlags & KMF_MATERIAL_ENVIRONMENT_MAPPED) > 0;

        if (!KMF_ReadCString(theStream, currentMaterial.mEnvMappingTexture))
            return false;
    }
    return true;
}