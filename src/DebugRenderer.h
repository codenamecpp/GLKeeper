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

    void DrawGrid();

    // push bounding sphere to debug draw queue
    // @param depth_test: Enable or disable depth test for lines
    void DrawSphere(const cxx::bounding_sphere& sphere, unsigned int color, bool depth_test);
    void DrawSphere(const glm::vec3& center_point, float sphere_length, unsigned int color, bool depth_test);

    // push aabbox to debug draw queue
    // @param depth_test: Enable or disable depth test for lines
    void DrawAabb(const cxx::aabbox& aabox, unsigned int color, bool depth_test);

    // push transformation axes to debug draw queue
    // @param depth_test: Enable or disable depth test for lines
    void DrawAxes(const glm::mat4& transform_matrix, const glm::vec3& center_point, float axis_length, bool depth_test);

private:

private:
    DebugDrawRenderProgram mDebugDrawRenderProgram;
};

