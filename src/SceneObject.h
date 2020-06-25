#pragma once

#include "GameDefs.h"
#include "SceneDefs.h"

// object on render scene
class SceneObject: public cxx::noncopyable
{
    // enable rtti support for scene objects
    decl_rtti_base(SceneObject)

    friend class SceneRenderList;

public:
    Color32 mDebugColor; // color used for debug draw

    float mDistanceToCameraSquared; // this value gets updated during scene rendition

    // readonly
    glm::vec3 mDirectionRight; // direction vector along x axis, should be normalized
    glm::vec3 mDirectionUpward; // direction vector along y axis, should be normalized
    glm::vec3 mDirectionForward; // direction vector along z axis, should be normalized
    glm::vec3 mPosition; // position within world
    float mScaling; // uniform scaling
    glm::mat4 mTransformation; // should be manually updated so make sure to do ComputeTransformation
                               // transformed bounds should be manually updated so make sure to ComputeTransformation
    cxx::aabbox mBoundsTransformed; // world space
    cxx::aabbox mBounds; // untransformed, used for culling and mouse tests

    Entity* mParentEntity = nullptr; // world entity which this render object is belongs

public:
    SceneObject();

    // issue draw on specific render pass
    // @param renderContext: Scene render context
    virtual void RenderFrame(SceneRenderContext& renderContext);

    // process update
    // @param deltaTime: Time since last frame
    virtual void UpdateFrame(float deltaTime);

    // prepare/unload renderable component mesh for rendering
    virtual void PrepareRenderResources();
    virtual void ReleaseRenderResources();

    // immediately destroy object instance
    virtual void DestroyObject();

    // enable or disable scene object
    // it will be rendered only if active state
    void SetActive(bool isActive);

    // test whether object is enabled
    bool IsActive() const;

    // set new parent entity or clear current
    void SetParentEntity(Entity* parentEntity);

    // register itself for rendering on current frame
    // @param renderList: Current frame render list
    void RegisterForRendering(SceneRenderList& renderList);

    // setup renderable component mesh materials
    void SetMeshMaterialsCount(int numMaterials);
    void SetMeshMaterial(int materialIndex, const MeshMaterial& meshMaterial);
    void ClearMeshMaterials();

    // access renderable component mesh materials by index
    int GetMaterialsCount() const;
    MeshMaterial* GetMeshMaterial(int materialIndex);
    MeshMaterial* GetMeshMaterial();
    // readonly acceess to materials by index
    const MeshMaterial* GetMeshMaterial(int materialIndex) const;
    const MeshMaterial* GetMeshMaterial() const;

    // test whether renderable component mesh has opaque or transparent parts
    bool HasOpaqueMeshParts() const;
    bool HasTransparentMeshParts() const;

    // whether renderdata must be renegerated next draw frame
    void InvalidateMesh();
    bool IsMeshInvalidated() const;

    // set object orienation vectors, expecting all normalized
    // @param directionRight: Vector X
    // @param directionForward: Vector Z
    // @param directionUpward: Vector Y
    void SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward, const glm::vec3& directionUpward);
    void SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward);

    // setup object orientation, look at point
    // @param point: Point world
    // @param upward: Up vector
    void OrientTowards(const glm::vec3& point);
    void OrientTowards(const glm::vec3& point, const glm::vec3& upward);

    // reset orientation or transformations to defaults
    void ResetOrientation();
    void ResetTransformation();

    // set position
    // @param position: Position
    void SetPosition(const glm::vec3& position);

    // set uniform scale
    // @param scaling: Scaling factor
    void SetScaling(float scaling);

    // rotate object around specified axis, note that it is local rotation
    // @param rotationAxis: Reference axis
    // @param rotationAngle: Angle specified in radians
    void Rotate(const glm::vec3& rotationAxis, float rotationAngle);

    // translate position
    // @param translation: Translation
    void Translate(const glm::vec3& translation);

    // compute transformation matrix and world space bounding box
    void ComputeTransformation();

    // set local space bounding volume of object
    // @param aabox: Bounds
    void SetLocalBoundingBox(const cxx::aabbox& aabox);

    // process object transform or bounds changed
    void InvalidateTransform(); 
    void InvalidateBounds();

protected:
    // destroy object instance with dedicated method DestroyObject
    virtual ~SceneObject();

    // setup renderable parts
    void SetDrawPartsCount(int partsCount);
    void SetDrawPart(int drawPart, int materialIndex, int vertexOffset, int indexOffset, int vertexCount, int triangleCount);
    void ClearDrawParts();

protected:
    // dirty flag indicates that geometry is invalid and must be reuploaded
    bool mMeshInvalidated = false;

    // defines single draw part
    struct DrawPart
    {
    public:
        // geometry offset within vertex/index buffers, in bytes
        int mVertexDataOffset = 0;
        int mIndexDataOffset = 0;
        // geometry elements count
        int mVertexCount = 0;
        int mTriangleCount = 0;
        // material index within mesh materials array
        int mMaterialIndex = 0;
    };

    std::vector<MeshMaterial> mMeshMaterials;
    std::vector<DrawPart> mDrawParts;

    // hardware vertex and index buffers aren't managed
    GpuBuffer* mVertexBuffer = nullptr;
    GpuBuffer* mIndexBuffer = nullptr;

    RenderProgram* mRenderProgram = nullptr;

private:
    // transform dirty flags
    bool mTransformDirty;
    bool mBoundingBoxDirty;
};