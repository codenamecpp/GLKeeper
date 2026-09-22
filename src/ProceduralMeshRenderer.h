#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ProceduralMeshObject.h"
#include "ShaderProgram.h"

//////////////////////////////////////////////////////////////////////////

class ProceduralMeshRenderer: public ISceneObjectRenderer
{
public:
    bool Initialize();
    void Shutdown();

    // override ISceneObjectRenderer
    void BeginFrame() override;
    void EndFrame() override;
    void BeginBatch(Camera& camera) override;
    void EndBatch() override;
    void RenderInstance(eRenderPass currentPass, SceneObject* object) override;

private:
    ShaderProgram_StaticMesh* mShaderProgram = nullptr;

    size_t mFrameBatchCounter {};
};