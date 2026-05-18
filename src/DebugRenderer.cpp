#include "stdafx.h"
#include "DebugRenderer.h"
#include "GameRenderManager.h"
#include "GameWorld.h"
#include "ShadersManager.h"

static struct SphereShape
{
    SphereShape()
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

    Line lines[128] {};

} sg_sphere;

//////////////////////////////////////////////////////////////////////////

bool DebugRenderer::Initialize()
{
    mShaderProgram = gShadersManager.GetProgramOfType<ShaderProgram_DebugDraw>("debug");
    cxx_assert(mShaderProgram);
    if (mShaderProgram == nullptr)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load debug shader program");
        return false;
    }

    mShaderProgram->InitRenderData();

    // load shader program and create vertex buffer
    mVertexBuffer = gRenderDevice.CreateVertexBuffer(eBufferUsage_Dynamic, sizeof(mCachedVertices));
    if (!mVertexBuffer)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot allocate video memory for DebugRenderer");
        return false;
    }

    mVertexBuffer->ConfigureVertexDefinitions(Vertex3D_Debug_Definition::Get());

    mNumCachedVertices = 0;
    mBufferOffset = 0;
    return true;
}

void DebugRenderer::Shutdown()
{
    mVertexBuffer = nullptr;
    mShaderProgram = nullptr;
}

void DebugRenderer::BeginFrame(Camera& camera)
{
    mShaderProgram->SetViewProjectionMatrix(camera.mViewProjectionMatrix);
    mShaderProgram->BindProgram();
  
    // setup render state
    RenderStates defaultRenderState;
    gRenderDevice.SetRenderState(defaultRenderState);
}

void DebugRenderer::EndFrame()
{
    Flush();
}

void DebugRenderer::Flush()
{
    if (mNumCachedVertices == 0)
        return;

    bool success = mVertexBuffer->SubData(mBufferOffset * Sizeof_DebugVertex3D, mNumCachedVertices * Sizeof_DebugVertex3D, mCachedVertices + mBufferOffset);
    cxx_assert(success);
    if (success)
    {
        // bind attributes and render
        gRenderDevice.BindVertexBuffer(mVertexBuffer.get());
        gRenderDevice.RenderPrimitives(ePrimitiveType_Lines, mBufferOffset, mNumCachedVertices);
    }
    mBufferOffset += mNumCachedVertices;
    if (mBufferOffset >= MAX_CACHED_VERTICES)
    {
        mBufferOffset = 0;
    }
    mNumCachedVertices = 0;
}

void DebugRenderer::RenderAABoundingBox(const cxx::aabbox& box)
{
    const glm::vec3& min = box.mMin;
    const glm::vec3& max = box.mMax;

    // corners
    glm::vec3 v1(max.x, min.y, min.z);
    glm::vec3 v2(max.x, max.y, min.z);
    glm::vec3 v3(min.x, max.y, min.z);
    glm::vec3 v4(min.x, min.y, max.z);
    glm::vec3 v5(max.x, min.y, max.z);
    glm::vec3 v6(min.x, max.y, max.z);

    // push lines
    RenderLine(min, v1);
    RenderLine(v1, v2);
    RenderLine(v2, v3);
    RenderLine(v3, min);
    RenderLine(v4, v5);
    RenderLine(v5, max);
    RenderLine(max, v6);
    RenderLine(v6, v4);
    RenderLine(min, v4);
    RenderLine(v1, v5);
    RenderLine(v2, max);
    RenderLine(v3, v6);
}

void DebugRenderer::RenderBoundingSphere(const cxx::bounding_sphere& boundingSphere)
{
    for (const auto& sphereLine : sg_sphere.lines)
    {
        const glm::vec3 startp (
            sphereLine.p0.x * boundingSphere.mRadius + boundingSphere.mOrigin.x,
            sphereLine.p0.y * boundingSphere.mRadius + boundingSphere.mOrigin.y,
            sphereLine.p0.z * boundingSphere.mRadius + boundingSphere.mOrigin.z
        );
        const glm::vec3 endp (
            sphereLine.p1.x * boundingSphere.mRadius + boundingSphere.mOrigin.x,
            sphereLine.p1.y * boundingSphere.mRadius + boundingSphere.mOrigin.y,
            sphereLine.p1.z * boundingSphere.mRadius + boundingSphere.mOrigin.z
        );
        RenderLine(startp, endp);
    }
}

void DebugRenderer::RenderOrientationAxes(
    const glm::vec3& origin, 
    const glm::vec3& upDirection, 
    const glm::vec3& lookDirection, 
    const glm::vec3& rightDirection, float axisLength)
{
    SetRenderColor(COLOR_RED);
    RenderLine(origin, origin + (rightDirection * axisLength)); // x is red

    SetRenderColor(COLOR_GREEN);
    RenderLine(origin, origin + (upDirection * axisLength)); // y is green

    SetRenderColor(COLOR_BLUE);
    RenderLine(origin, origin + (lookDirection * axisLength)); // z is blue
}

void DebugRenderer::RenderGrid(const glm::vec3& origin, const Point2D& dimensions, const glm::vec2& cellSize)
{
    const glm::vec3 cornerTL = origin + glm::vec3 { 
        -(cellSize.x * dimensions.x) / 2.0f, 
        0.0f,
        -(cellSize.y * dimensions.y) / 2.0f
    };

    const glm::vec3 cornerTR = origin + glm::vec3 {
        (cellSize.x * dimensions.x) / 2.0f, 
        0.0f,
        -(cellSize.y * dimensions.y) / 2.0f
    };

    const glm::vec3 cornerBL = origin + glm::vec3 {
        -(cellSize.x * dimensions.x) / 2.0f,
        0.0f,
        (cellSize.y * dimensions.y) / 2.0f
    };

    for (int iy = 0; iy < dimensions.y + 1; ++iy)
    {
        const glm::vec3 voffset{0.0f, 0.0f, iy * cellSize.y};
        RenderLine(cornerTL + voffset, cornerTR + voffset);
    }
    for (int ix = 0; ix < dimensions.x + 1; ++ix)
    {
        const glm::vec3 voffset{ix * cellSize.x, 0.0f, 0.0f};
        RenderLine(cornerTL + voffset, cornerBL + voffset);
    }
}