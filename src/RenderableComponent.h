#pragma once

#include "GameObjectComponent.h"

// base class for renderable objects
class RenderableComponent: public GameObjectComponent
{
    decl_rtti(RenderableComponent, GameObjectComponent)

    friend class SceneRenderList;

public:
    RenderableComponent(GameObject* sceneObject);

    // register itself for rendering on current frame
    // @param renderList: Current frame render list
    void RegisterForRendering(SceneRenderList& renderList);

    // access renderable component mesh materials by index
    int GetMaterialsCount() const;
    MeshMaterial* GetMeshMaterial(int materialIndex);
    MeshMaterial* GetMeshMaterial();

    // access renderable component mesh parts
    int GetMeshPartsCount() const;
    MeshMaterial* GetMeshPartMaterial(int meshPartIndex);

    // test whether renderable component mesh has opaque or transparent parts
    bool HasOpaqueMeshParts() const;
    bool HasTransparentMeshParts() const;

    // prepare renderable component mesh for rendering
    virtual void PrepareRenderResources();

    // unload graphical resources (renderdata)
    virtual void ReleaseRenderResources();

    // issue draw on specific render pass
    // @param renderContext: Scene render context
    virtual void RenderFrame(SceneRenderContext& renderContext);

    // destroy renderable component instance
    void DestroyComponent() override;

protected:
    // setup renderable component mesh materials
    void SetMeshMaterialsCount(int numMaterials);
    void SetMeshMaterial(int materialIndex, const MeshMaterial& meshMaterial);
    void ClearMeshMaterials();

    // setup renderable component mesh parts
    void SetMeshPartsCount(int numMeshParts);
    void SetMeshPart(int meshPartIndex, int materialIndex, int vertexOffset, int indexOffset, int vertexCount, int triangleCount);
    void ClearMeshParts();

protected:

    // defines piece of geometry with single material within mesh
    struct MeshPartStruct
    {
    public:
        // material index within mesh materials array
        int mMaterialIndex = 0;

        // geometry offset within vertex/index buffers, in bytes
        int mVertexDataOffset = 0;
        int mIndexDataOffset = 0;

        // geometry elements count
        int mVertexCount = 0;
        int mTriangleCount = 0;
    };

    std::vector<MeshMaterial> mMeshMaterials;
    std::vector<MeshPartStruct> mMeshParts;

    GpuBuffer* mVertexBuffer = nullptr;
    GpuBuffer* mIndexBuffer = nullptr;
    RenderProgram* mRenderProgram = nullptr;
};