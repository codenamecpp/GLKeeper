#pragma once

//////////////////////////////////////////////////////////////////////////

#include "AnimatingMeshObject.h"
#include "ShaderProgram.h"

//////////////////////////////////////////////////////////////////////////

class AnimatingMeshRenderer: public ISceneObjectRenderer
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
    ShaderProgram_BlendFrames* mShaderProgram = nullptr;
    size_t mFrameBatchCounter {};
};

//////////////////////////////////////////////////////////////////////////