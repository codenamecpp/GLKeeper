#pragma once

//////////////////////////////////////////////////////////////////////////

#include "AnimatingMeshObject.h"
#include "ShaderProgram.h"

//////////////////////////////////////////////////////////////////////////

class AnimatingMeshRenderer: public cxx::noncopyable
{
public:
    bool Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void BeginBatch(Camera& camera);
    void EndBatch();

    void RenderInstance(eRenderPass currentPass, AnimatingMeshObject& object);

private:
    ShaderProgram_BlendFrames* mShaderProgram = nullptr;
    size_t mFrameBatchCounter {};
};

//////////////////////////////////////////////////////////////////////////