#include "stdafx.h"
#include "DK2KMFModel.h"

//////////////////////////////////////////////////////////////////////////

#define MAKE_HEADER_ID(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////

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
    KMF_MATERIAL_DOUBLE_SIDED = 0x02,
    KMF_MATERIAL_ALPHA_ADDITIVE = 0x04,
    KMF_MATERIAL_TRANSLUCENT = 0x08,
    KMF_MATERIAL_HAS_SPECULAR = 0x40,
    KMF_MATERIAL_HAS_EMISSIVE = 0x80,
    KMF_MATERIAL_INVISIBLE = 0x0100,
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
        if (!thestream.read((char*) &theoutput, sizeof(theoutput))) \
        { \
            cxx_assert(false); \
            return false; \
        } \
    }

#define SKIP_BYTES(thestream, numbytes) \
    { \
        thestream.seekg(numbytes, std::ios::cur); \
    }

inline bool KMF_ReadCString(std::istream& theStream, std::string& outputString)
{
    char charsBuffer[KMF_MAX_STRING_LENGTH];

    if (cxx::read_cstring(theStream, charsBuffer, KMF_MAX_STRING_LENGTH))
    {
        outputString.assign(charsBuffer);
        return true; 
    }

    cxx_assert(false);
    return false;
}

inline bool KMF_ReadSectionHeader(std::istream& theStream, KMFHeader& theHeader, unsigned int theExpectedID)
{
    bool isSuccess = 
        theStream.read((char*)&theHeader.mID, sizeof(theHeader.mID)) && 
        theStream.read((char*)&theHeader.mLength, sizeof(theHeader.mLength));

    isSuccess = isSuccess && (theHeader.mID == theExpectedID);
    cxx_assert(isSuccess);
    return isSuccess;
}

inline bool KMF_ReadVector3f(std::istream& theStream, glm::vec3& theVector)
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

inline bool KMF_ReadTriangle(std::istream& theStream, glm::ivec3& theTriangle)
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

inline bool KMF_ReadTexcoord(std::istream& theStream, glm::vec2& theTexcoord)
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

void DK2KMFModel::Clear()
{
    mName.clear();
    mFramesBounds.clear();
    mMeshArray.clear();
    mMaterialsArray.clear();
    mTranslation = {};
    mScale = 1.0f;
    mCubeScale = 1.0f;
    mFramesCount = 0;
}

void DK2KMFModel::Swap(DK2KMFModel& rhs)
{
    if (this != &rhs)
    {
        std::swap(mName,            rhs.mName);
        std::swap(mFramesBounds,    rhs.mFramesBounds);
        std::swap(mMeshArray,       rhs.mMeshArray);
        std::swap(mMaterialsArray,  rhs.mMaterialsArray);
        std::swap(mTranslation,     rhs.mTranslation);
        std::swap(mScale,           rhs.mScale);
        std::swap(mCubeScale,       rhs.mCubeScale);
        std::swap(mFramesCount,     rhs.mFramesCount);
    }
}

//////////////////////////////////////////////////////////////////////////

static bool KMF_ReadMaterials(std::istream& theStream, DK2KMFModel& outputData)
{
    int numMaterials;
    READ_FROM_BYTE_STREAM(theStream, numMaterials);

    outputData.mMaterialsArray.resize(numMaterials);
    // read materials
    for (DK2KMFModel::MaterialDesc& currentMaterial: outputData.mMaterialsArray)
    {
        KMFHeader header;
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MATERIAL))
            return false;

        unsigned int materialChunkSize;
        READ_FROM_BYTE_STREAM(theStream, materialChunkSize);
        if (!KMF_ReadCString(theStream, currentMaterial.mName))
            return false;

        cxx::trim(currentMaterial.mName);
        cxx_assert(!currentMaterial.mName.empty());

        int numtextures = 0;
        READ_FROM_BYTE_STREAM(theStream, numtextures);

        cxx_assert(numtextures > 0);
        if (numtextures > 0)
        {
            // reading texture names
            currentMaterial.mDiffuseTextureNames.reserve(numtextures);
            for (int itexture = 0; itexture < numtextures; ++itexture)
            {
                if (!KMF_ReadCString(theStream, currentMaterial.mDiffuseTextureNames.emplace_back()))
                    return false;
            }
        }

        unsigned int materialFlags;
        READ_FROM_BYTE_STREAM(theStream, materialFlags);

        // setup material flags
        currentMaterial.mFlags_HasAlpha = (materialFlags & KMF_MATERIAL_HAS_ALPHA) > 0;
        currentMaterial.mFlags_DoubleSided = (materialFlags & KMF_MATERIAL_DOUBLE_SIDED) > 0;
        currentMaterial.mFlags_AlphaAdditive = (materialFlags & KMF_MATERIAL_ALPHA_ADDITIVE) > 0;
        currentMaterial.mFlags_Translucent = (materialFlags & KMF_MATERIAL_TRANSLUCENT) > 0;
        currentMaterial.mFlags_HasSpecular = (materialFlags & KMF_MATERIAL_HAS_SPECULAR) > 0;
        currentMaterial.mFlags_HasEmissive = (materialFlags & KMF_MATERIAL_HAS_EMISSIVE) > 0;
        currentMaterial.mFlags_Invisible = (materialFlags & KMF_MATERIAL_INVISIBLE) > 0;

        READ_FROM_BYTE_STREAM(theStream, currentMaterial.mEmissive);
        READ_FROM_BYTE_STREAM(theStream, currentMaterial.mSpecular);

        if (!KMF_ReadCString(theStream, currentMaterial.mEnvMappingTextureName))
            return false;
    }
    return true;
}

static bool KMF_ReadSprites(std::istream& theStream, DK2KMFModel& outputData, int theNumSprites, int theNumLODs)
{
    cxx_assert(theNumSprites > 0 && theNumLODs > 0);

    KMFHeader header;

    // allocate sprites
    outputData.mMeshArray.resize(theNumSprites);

    // read headers
    for (DK2KMFModel::SubMesh& refSprite: outputData.mMeshArray)
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
    for (DK2KMFModel::SubMesh& refSprite: outputData.mMeshArray)
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
        for (DK2KMFModel::SubMeshLOD& currentLOD: refSprite.mLODsArray)
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
        gConsole.LogMessage(eLogLevel_Info, "Dummy sprites found in mesh '%s'", outputData.mName.c_str());
    }
    return true;
}

static bool KMF_ReadAnimSprites(std::istream& theStream, DK2KMFModel& outputData, int theNumSprites, int theNumLODs)
{
    cxx_assert(theNumSprites > 0 && theNumLODs > 0);

    KMFHeader header;
    // allocate sprites
    outputData.mMeshArray.resize(theNumSprites);

    // read headers
    for (DK2KMFModel::SubMesh& refSprite: outputData.mMeshArray)
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
    for (DK2KMFModel::SubMesh& refSprite: outputData.mMeshArray)
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
        for (DK2KMFModel::SubMeshLOD& currentLOD: refSprite.mLODsArray)
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

static bool KMF_ReadStaticMeshGeometries(std::istream& theStream, DK2KMFModel& theOutputMesh, int theNumGeometies)
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
    for (DK2KMFModel::SubMesh& refSprite: theOutputMesh.mMeshArray)
    {
        for (glm::vec3& thePosition: refSprite.mVertexPositionArray)
        {
            unsigned short geo;
            KMF_DecodeGeomITab(thePosition, geo);
            thePosition = geometries[geo]; // set vertex position
        } // for
    }
    return true;
}

static bool KMF_ReadMeshCtrl(std::istream& theStream, DK2KMFModel& outputData)
{
    struct control_t
    {
        unsigned short b1;
        unsigned short b2;
        unsigned int c;
    };

    unsigned int numControls;
    READ_FROM_BYTE_STREAM(theStream, numControls);

    if (numControls > 0)
    {
        auto currpos = theStream.tellg();

        static std::vector<control_t> controls;
        controls.resize(numControls);
        for (control_t& roller: controls)
        {
            READ_FROM_BYTE_STREAM(theStream, roller);
        }
        controls.clear();
    }
    return true;
}

static bool KMF_ReadStaticMesh(std::istream& theStream, DK2KMFModel& outputData)
{
    KMFHeader header;

    //KMSH/MESH/HEAD
    if (!KMF_ReadSectionHeader(theStream, header, KMF_HEAD))
        return false;

    if (!KMF_ReadCString(theStream, outputData.mName))
        return false;

    outputData.mFramesCount = 1; // single frame for static mesh

    unsigned int numSprites;
    unsigned int numGeometries;
    unsigned int numLODs;
    READ_FROM_BYTE_STREAM(theStream, numSprites);
    READ_FROM_BYTE_STREAM(theStream, numGeometries);
    
    if (!KMF_ReadVector3f(theStream, outputData.mTranslation))
        return false;

    float scale;
    READ_FROM_BYTE_STREAM(theStream, scale);
    READ_FROM_BYTE_STREAM(theStream, numLODs);
    outputData.mScale = scale;

    //Controls
    //KMSH/MATL/CTRL
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_CONTROL))
        return false;

    SKIP_BYTES(theStream, header.mLength - KMF_CHUNK_HEADER_LENGTH); // controls are unknown 

    //Sprites
    //KMSH/MESH/SPRS
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_SPRITES))
        return false;

    if (!KMF_ReadSprites(theStream, outputData, numSprites, numLODs))
        return false;

    //KMSH/MESH/GEOM
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_GEOM))
        return false;

    if (!KMF_ReadStaticMeshGeometries(theStream, outputData, numGeometries))
        return false;

    // success
    return true;
}

static bool KMF_ReadAnimMesh(std::istream& theStream, DK2KMFModel& outputData)
{
    KMFHeader header;

    //KMSH/ANIM/HEAD
    if (!KMF_ReadSectionHeader(theStream, header, KMF_HEAD))
        return false;

    if (!KMF_ReadCString(theStream, outputData.mName))
        return false;

    unsigned int sprsCount;
    unsigned int frameCount;
    unsigned int indexCount;
    unsigned int geomCount;
    float cubeScale;
    float scale;
    unsigned int numLODs;

    // read data
    READ_FROM_BYTE_STREAM(theStream, sprsCount);
    READ_FROM_BYTE_STREAM(theStream, frameCount);
    READ_FROM_BYTE_STREAM(theStream, indexCount);
    READ_FROM_BYTE_STREAM(theStream, geomCount);

    unsigned int frameFactorFunction;
    READ_FROM_BYTE_STREAM(theStream, frameFactorFunction);
    cxx_assert((frameFactorFunction == 0) || (frameFactorFunction == 1));
    outputData.mFrameFactorFunction = frameFactorFunction;

    outputData.mFramesCount = frameCount;

    if (!KMF_ReadVector3f(theStream, outputData.mTranslation))
        return false;

    READ_FROM_BYTE_STREAM(theStream, cubeScale);
    READ_FROM_BYTE_STREAM(theStream, scale);
    outputData.mCubeScale = cubeScale;
    outputData.mScale = scale;

    READ_FROM_BYTE_STREAM(theStream, numLODs);

    //KMSH/ANIM/CTRL
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_CONTROL))
        return false;

    if (!KMF_ReadMeshCtrl(theStream, outputData))
        return false;

    //KMSH/ANIM/SPRS
    if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH_SPRITES))
        return false;

    if (!KMF_ReadAnimSprites(theStream, outputData, sprsCount, numLODs))
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
        const float z = (((coordinates >>  0) & 0x3ff) - 0x200) / 511.0f; // z

        // prescale
        animGeometry.mGeometry.x = x * outputData.mScale;
        animGeometry.mGeometry.z = y * outputData.mScale;
        animGeometry.mGeometry.y = -z * outputData.mScale;
     
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
    for (DK2KMFModel::SubMesh& submesh: outputData.mMeshArray)
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

static void KMF_ComputeBounds(DK2KMFModel& outputData)
{
    outputData.mFramesBounds.resize(outputData.mFramesCount);

    // process animation frames
    for (int iAnimFrame = 0; iAnimFrame < outputData.mFramesCount; ++iAnimFrame)
    {
        cxx::aabbox& bounds = outputData.mFramesBounds[iAnimFrame];
        bounds.reset();
        for (const DK2KMFModel::SubMesh& subMesh: outputData.mMeshArray)
        {
            const glm::vec3* verticesPos = &subMesh.mVertexPositionArray[iAnimFrame * subMesh.mFrameVerticesCount];
            for (int ivertex = 0; ivertex < subMesh.mFrameVerticesCount; ++ivertex)
            {
                bounds.extend(verticesPos[ivertex]);
            }
        }
    } // for
}

//////////////////////////////////////////////////////////////////////////

bool DK2_KMF_LoadFromStream(std::istream& theStream, DK2KMFModel& outputData)
{
    outputData.Clear();

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

    if (!meshTypeSupported)
        return false;

    SKIP_BYTES(theStream, 4); // unknown data

    //KMSH/MATL
    if (meshType != DK2_MESH_TYPE_GROUP)
    {
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MATERIALS))
            return false;

        if (!KMF_ReadMaterials(theStream, outputData))
            return false;
    }

    //KMSH/MESH
    if (meshType == DK2_MESH_TYPE_STATIC)
    {
        if (!KMF_ReadSectionHeader(theStream, header, KMF_MESH)) 
            return false;

        if (!KMF_ReadStaticMesh(theStream, outputData))
            return false;
    }

    //KMSH/ANIM
    if (meshType == DK2_MESH_TYPE_ANIMATED)
    {
        if (!KMF_ReadSectionHeader(theStream, header, KMF_ANIM)) 
            return false;

        if (!KMF_ReadAnimMesh(theStream, outputData))
            return false;
    }

    KMF_ComputeBounds(outputData);
    return true;
}
