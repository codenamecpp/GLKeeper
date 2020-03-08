#pragma once

#include "ConsoleDefs.h"

class DebugRenderer;
class RenderManager;
class RenderProgram;

enum eRenderPass
{
    eRenderPass_Opaque,
    eRenderPass_Translucent,
};

decl_enum_strings(eRenderPass);

// render common cvars

extern CvarBoolean gCvarRender_DebugDrawEnabled;