#pragma once

#include "RenderProgram.h"
#include "GpuProgram.h"

// render program intended for debug draw
class DebugDrawRenderProgram: public RenderProgram
{
public:
    DebugDrawRenderProgram();

private:
    void HandleProgramLoad() override;
    void HandleProgramFree() override;
};

//////////////////////////////////////////////////////////////////////////

// terrain mesh shader
class TerrainRenderProgram: public RenderProgram
{
public:
    TerrainRenderProgram();

private:
    void HandleProgramLoad() override;
    void HandleProgramFree() override;
};

//////////////////////////////////////////////////////////////////////////

// procedural mesh shader
class StaticMeshRenderProgram: public RenderProgram
{
public:
    StaticMeshRenderProgram();

private:
    void HandleProgramLoad() override;
    void HandleProgramFree() override;
};

//////////////////////////////////////////////////////////////////////////

// terrain mesh shader
class WaterLavaRenderProgram: public RenderProgram
{
public:
    WaterLavaRenderProgram();

    void SetWaveParams(float waveTime, float waveWidth, float waveHeight, float waterLine);

private:
    void HandleProgramLoad() override;
    void HandleProgramFree() override;

private:
    GpuVariableLocation mUniformID_wave_params = GpuVariable_NULL;
};

//////////////////////////////////////////////////////////////////////////

// blend frames animation shader
class MorphAnimRenderProgram: public RenderProgram
{
public:
    MorphAnimRenderProgram();

    void SetMixFrames(float mixFrames);

private:
    void HandleProgramLoad() override;
    void HandleProgramFree() override;

private:
    GpuVariableLocation mUniformID_mix_frames = GpuVariable_NULL;
};

//////////////////////////////////////////////////////////////////////////

// render program intended for gui draw
class GuiRenderProgram: public RenderProgram
{
public:
    GuiRenderProgram();

private:
    void HandleProgramLoad() override;
    void HandleProgramFree() override;
};