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

    void BeginFrame(Camera& camera);
    void EndFrame();

    void BeginBatch();
    void EndBatch();

    void RenderInstance(eRenderPass currentPass, AnimatingMeshObject& object);

private:
    ShaderProgram_BlendFrames* mShaderProgram = nullptr;
};

//////////////////////////////////////////////////////////////////////////