#pragma once

#include "ShaderProgram.h"

//////////////////////////////////////////////////////////////////////////
// Drawing debug stuff using this
//////////////////////////////////////////////////////////////////////////
   
class DebugRenderer: public cxx::noncopyable
{
public:
    // First time rendered initialization
    // All shaders, buffers and other graphics resources might be loaded here
    // Return false on error
    bool Initialize();

    // Free internal render resources
    void Shutdown();

    // Prepare rendering
    void BeginFrame(Camera& camera);
    void EndFrame();

    // Render aabbox with specified color
    // @param boundingBox
    void RenderAABoundingBox(const cxx::aabbox& boundingBox);

    // Render bounding sphere with specified color
    // @param boundingSphere
    // @param shapeColor
    void RenderBoundingSphere(const cxx::bounding_sphere& boundingSphere);

    // Render axes
    // @param origin: Location
    // @param upDirection, lookDirection, rightDirection: Orientation vectors
    // @param axisLength: Length of lines
    void RenderOrientationAxes(const glm::vec3& origin,
        const glm::vec3& upDirection, 
        const glm::vec3& lookDirection, 
        const glm::vec3& rightDirection, float axisLength = 1.0f);

    // Render grid plane
    // @param origin: Position of the grid center
    // @param dimensions: Num grid cells per width and height
    // @param cellSize: Size of the single cell
    // @param shapeColor: Color of the grid lines
    void RenderGrid(const glm::vec3& origin, const Point2D& dimensions, const glm::vec2& cellSize);

    // Render single line with specific color
    // @param startPosition, endPosition
    // @param shapeColor
    void RenderLine(const glm::vec3& startPosition, const glm::vec3& endPosition);

    // Set current drawing color
    // @param geometryColor: Color
    void SetRenderColor(Color32 geometryColor)
    {
        mCurrentColor = geometryColor;
    }

private:
    // Actually renders all geometry on screen
    void Flush();

private:
    
    ShaderProgram_DebugDraw* mShaderProgram = nullptr;

    enum 
    {
        MAX_CACHED_VERTICES = 8192
    };

    std::unique_ptr<GpuVertexBuffer> mVertexBuffer;
    Color32 mCurrentColor = 0;
    unsigned int mNumCachedVertices = 0;
    unsigned int mBufferOffset = 0; // specified in vertices
    DebugVertex3D mCachedVertices[MAX_CACHED_VERTICES];
};

//////////////////////////////////////////////////////////////////////////

inline void DebugRenderer::RenderLine(const glm::vec3& startPosition, const glm::vec3& endPosition)
{
    if (mBufferOffset + mNumCachedVertices + 2 > MAX_CACHED_VERTICES)
    {
        Flush();
    }

    {
        DebugVertex3D& vertex = mCachedVertices[mBufferOffset + (mNumCachedVertices++)];
        vertex.mPosition = startPosition;
        vertex.mColor = mCurrentColor;
    }
    {
        DebugVertex3D& vertex = mCachedVertices[mBufferOffset + (mNumCachedVertices++)];
        vertex.mPosition = endPosition;
        vertex.mColor = mCurrentColor;
    }
}