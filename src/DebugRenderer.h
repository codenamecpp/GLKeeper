#pragma once

#include "GraphicsDefs.h"
#include "DebugDrawRenderProgram.h"

// debug geometry visualization manager
class DebugRenderer: public cxx::noncopyable
{
public:

    // setup debug renderer internal resources
    // @returns false on error
    bool Initialize();
    void Deinit();

    void RenderFrameBegin();
    void RenderFrameEnd();

    // push line to debug draw queue
    // @param depth_test: Enable or disable depth test for line
    void DrawLine(const glm::vec3& start_point, const glm::vec3& end_point, unsigned int color, bool depth_test);
    
    void DrawBox();

    // push grid to debug draw queue
    // @param depth_test: Enable or disable depth test for line
    void DrawGrid(const Size2D& cellCount, const glm::vec2& cellSize, unsigned int color, bool depth_test);

    // push bounding sphere to debug draw queue
    // @param depth_test: Enable or disable depth test for lines
    void DrawSphere(const cxx::bounding_sphere& sphere, unsigned int color, bool depth_test);
    void DrawSphere(const glm::vec3& center_point, float sphere_radius, unsigned int color, bool depth_test);

    // push aabbox to debug draw queue
    // @param depth_test: Enable or disable depth test for lines
    void DrawAabb(const cxx::aabbox& aabox, unsigned int color, bool depth_test);

    // push transformation axes to debug draw queue
    // @param depth_test: Enable or disable depth test for lines
    void DrawAxes(const glm::mat4& transform_matrix, const glm::vec3& center_point, float axis_length, bool depth_test);

private:

    struct DebugLineStruct;
    DebugLineStruct* try_allocate_lines(int numLines, bool depth_test);
    Vertex3D_Debug* try_allocate_debug_vertices(int numVertices);

    // push line vertices to draw buffer, returns false on out of memory
    bool PushLineVertices(const DebugLineStruct& lineStruct);

    void FlushDebugVertices(bool depth_test);
    void Flush();
    bool HasPendingDraws() const;

private:
    static const int MaxDebugLines = 32768;
    static const int MaxDebugVertices = 4096;

    // debug line draw struct
    struct DebugLineStruct
    {
        glm::vec3 mLineStart, mLineEnd; // start/end points
        unsigned int mColor;
    };

    // debug lines array divided into two halves:
    // growing from start - no depth test
    // growing fron end - with depth test enabled

    int mDebugLinesCount = 0;
    int mDebugLinesDepthTestCount = 0;
    int mDebugVerticesCount = 0;
    DebugLineStruct mDebugLinesArray[MaxDebugLines];
    Vertex3D_Debug mDebugVertices[MaxDebugVertices];

    GpuBuffer* mGpuVerticesBuffer = nullptr;
    DebugDrawRenderProgram mDebugDrawRenderProgram;
};

