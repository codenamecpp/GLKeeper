#pragma once

// forwards
class DebugRenderer;
class RenderManager;
class RenderProgram;
class RenderMaterial;

class ModelsRenderData;

enum eRenderPass
{
    eRenderPass_Opaque,
    eRenderPass_Translucent,

    eRenderPass_Count,
};

decl_enum_strings(eRenderPass);

// render context
class SceneRenderContext
{
public:

    eRenderPass mCurrentPass;

};