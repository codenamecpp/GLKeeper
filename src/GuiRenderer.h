#pragma once

#include "GuiRenderProgram.h"
#include "GuiDefs.h"

class GuiRenderer: public cxx::noncopyable
{
public:
    // setup gui renderer internal resources
    // @returns false on error
    bool Initialize();
    void Deinit();
    
    // setup view matrix and initialize resources
    // it should be called before using any gui drawing methods
    void RenderFrameBegin();

    // will flush pending drawing operations and restore previous render state
    // it should be called at end of gui rendering
    void RenderFrameEnd();

    // set transformation matrix for drawing gui elements
    // @param matrix: Transformation matrix, null for reset current
    void SetCurrentTransform(glm::mat4* matrix);

    // draw without textures
    void FillRect(const Rectangle& rect, Color32 fillColor);
    void DrawRect(const Rectangle& rect, Color32 lineColor);
    
    // draw quads
    // @param texture: Current texture
    // @param quads: Quads buffer
    // @param quadsCount: Number of quads in buffer
    void DrawQuads(Texture2D* texture, const GuiQuadStruct* quads, int quadsCount);
    void DrawQuad(Texture2D* texture, const GuiQuadStruct& quad)
    {
        DrawQuads(texture, &quad, 1);
    }

    void PushClipRect(const Rectangle& rcLocal);
    void PopClipRect();

private:
    void FlushPendingDrawCalls();

    void SetCurrentBatchTexture(Texture2D* newTexutre);
    void TransformVertices(Vertex2D* vertices);
    void TransformClipRect(Rectangle& rectangle) const;

private:
    static const int MaxBatchVertices = 16384;

    GuiRenderProgram mGuiRenderProgram;
    glm::mat4 mProjectionMatrix2D;
    glm::mat4* mCurrentTransform = nullptr;
    std::vector<Rectangle> mClipRectsStack;
    GpuBuffer* mVertexBuffer = nullptr;
    // current batch data
    Texture2D* mCurrentTexture = nullptr;
    int mBatchVertexCount = 0;
    Vertex2D mBatchVertices[MaxBatchVertices];
};