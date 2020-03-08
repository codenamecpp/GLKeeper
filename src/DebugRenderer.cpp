#include "pch.h"
#include "Console.h"
#include "DebugRenderer.h"

//////////////////////////////////////////////////////////////////////////

static struct DebugSpherePrecomp
{
    DebugSpherePrecomp()
    {
        auto PointOnSphere = [](float theta, float phi) -> glm::vec3
        {
            glm::vec3 point;
            theta = glm::radians(theta);
            phi = glm::radians(phi);
            point.x = sinf(theta) * sinf(phi);
            point.y = cosf(phi);
            point.z = cosf(theta) * sinf(phi);
            return point;
        };

        int iline = 0;
        for (float j = 0.0f; j < 180.0f; j += 45.0f)
        {
            for (float i = 0.0f; i < 360.0f; i += 45.0f)
            {
                glm::vec3 p1 = PointOnSphere(i, j);
                glm::vec3 p2 = PointOnSphere(i + 45.0f, j);
                glm::vec3 p3 = PointOnSphere(i, j + 45.0f);
                glm::vec3 p4 = PointOnSphere(i + 45.0f, j + 45.0f);

                lines[iline++] = { p1, p2 };
                lines[iline++] = { p3, p4 };
                lines[iline++] = { p1, p3 };
                lines[iline++] = { p2, p4 };
            }
        }
    }

    struct Line
    {
        glm::vec3 p0;
        glm::vec3 p1;
    };

    Line lines[128];

} sg_sphere;

//////////////////////////////////////////////////////////////////////////

bool DebugRenderer::Initialize()
{
    return true;
}

void DebugRenderer::Deinit()
{

}

void DebugRenderer::RenderFrameBegin()
{
}

void DebugRenderer::RenderFrameEnd()
{
}

void DebugRenderer::DrawLine(const glm::vec3& start_point, const glm::vec3& end_point, unsigned int color, bool depth_test)
{
}

void DebugRenderer::DrawSphere(const cxx::bounding_sphere& sphere, unsigned int color, bool depth_test)
{
}

void DebugRenderer::DrawSphere(const glm::vec3& center_point, float sphere_length, unsigned int color, bool depth_test)
{
}

void DebugRenderer::DrawAabb(const cxx::aabbox& aabox, unsigned int color, bool depth_test)
{
}

void DebugRenderer::DrawAxes(const glm::mat4& transform_matrix, const glm::vec3& center_point, float axis_length, bool depth_test)
{
}
