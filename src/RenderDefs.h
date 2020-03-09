#pragma once

#include "GraphicsDefs.h"

// forwards
class DebugRenderer;
class RenderManager;
class RenderProgram;
class RenderMaterial;

enum eRenderPass
{
    eRenderPass_Opaque,
    eRenderPass_Translucent,
};

decl_enum_strings(eRenderPass);