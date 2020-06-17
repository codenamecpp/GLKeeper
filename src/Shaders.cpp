#include "pch.h"
#include "Shaders.h"


DebugDrawRenderProgram::DebugDrawRenderProgram(): RenderProgram("shaders/debug.glsl")
{
}

void DebugDrawRenderProgram::HandleProgramLoad()
{
    // configure input layout
    mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
    mGpuProgram->BindAttribute(eVertexAttribute_Color0, "in_color");
}

void DebugDrawRenderProgram::HandleProgramFree()
{
}

//////////////////////////////////////////////////////////////////////////

TerrainRenderProgram::TerrainRenderProgram(): RenderProgram("shaders/terrain.glsl")
{
}

void TerrainRenderProgram::HandleProgramLoad()
{
    // configure input layout
    mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
    //mGpuProgram->BindAttribute(eVertexAttribute_Normal0, "in_normal");
    mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
    mGpuProgram->BindAttribute(eVertexAttribute_TerrainTilePosition, "in_tile_coord");
}

void TerrainRenderProgram::HandleProgramFree()
{
}

//////////////////////////////////////////////////////////////////////////

ProcMeshRenderProgram::ProcMeshRenderProgram(): RenderProgram("shaders/mesh.glsl")
{
}

void ProcMeshRenderProgram::HandleProgramLoad()
{
    // configure input layout
    mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
    mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
    mGpuProgram->BindAttribute(eVertexAttribute_Color0, "in_color");
}

void ProcMeshRenderProgram::HandleProgramFree()
{
}

//////////////////////////////////////////////////////////////////////////

WaterLavaRenderProgram::WaterLavaRenderProgram(): RenderProgram("shaders/water_lava.glsl")
{
}

void WaterLavaRenderProgram::SetWaveParams(float waveTime, float waveWidth, float waveHeight, float waterLine)
{
    debug_assert(IsProgramLoaded());

    glm::vec4 waveParams(waveTime, waveWidth, waveHeight, waterLine);
    mGpuProgram->SetUniformParam(mUniformID_wave_params, waveParams);
}


void WaterLavaRenderProgram::HandleProgramLoad()
{
    mUniformID_wave_params = mGpuProgram->QueryUniformLocation("wave_params");
    debug_assert(mUniformID_wave_params != GpuVariable_NULL);

    // configure input layout
    mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
    mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
}

void WaterLavaRenderProgram::HandleProgramFree()
{
    mUniformID_wave_params = GpuVariable_NULL;
}

//////////////////////////////////////////////////////////////////////////

MorphAnimRenderProgram::MorphAnimRenderProgram(): RenderProgram("shaders/anim_blend_frames.glsl")
{
}

void MorphAnimRenderProgram::SetMixFrames(float mixFrames)
{
    debug_assert(IsProgramLoaded());
    mGpuProgram->SetUniformParam(mUniformID_mix_frames, mixFrames);
}

void MorphAnimRenderProgram::HandleProgramLoad()
{
    mUniformID_mix_frames = mGpuProgram->QueryUniformLocation("mix_frames");
    debug_assert(mUniformID_mix_frames != GpuVariable_NULL);

    // configure input layout
    mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos_frame0");
    mGpuProgram->BindAttribute(eVertexAttribute_Position1, "in_pos_frame1");
    mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
}

void MorphAnimRenderProgram::HandleProgramFree()
{
    mUniformID_mix_frames = GpuVariable_NULL;
}

//////////////////////////////////////////////////////////////////////////

GuiRenderProgram::GuiRenderProgram(): RenderProgram("shaders/gui.glsl")
{
}

void GuiRenderProgram::HandleProgramLoad()
{
    // configure input layout
    mGpuProgram->BindAttribute(eVertexAttribute_Position0, "in_pos");
    mGpuProgram->BindAttribute(eVertexAttribute_Color0, "in_color");
    mGpuProgram->BindAttribute(eVertexAttribute_Texcoord0, "in_texcoord");
}

void GuiRenderProgram::HandleProgramFree()
{
}