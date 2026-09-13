#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ProceduralMeshObject.h"
#include "ShaderProgram.h"

//////////////////////////////////////////////////////////////////////////

class ProceduralMeshRenderer: public cxx::noncopyable
{
public:
    bool Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void BeginBatch(Camera& camera);
    void EndBatch();

    void RenderInstance(eRenderPass currentPass, ProceduralMeshObject& object);

private:
    ShaderProgram_StaticMesh* mShaderProgram = nullptr;

    size_t mFrameBatchCounter {};
};