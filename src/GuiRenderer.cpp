#include "pch.h"
#include "GuiRenderer.h"
#include "Console.h"
#include "GraphicsDevice.h"
#include "GraphicsDefs.h"
#include "GpuBuffer.h"
#include "Texture2D.h"
#include "TexturesManager.h"

#define ALLOCATE_VERTICES(numVerts, ptr) \
    { \
        debug_assert((numVerts) < MaxBatchVertices); \
        if (MaxBatchVertices < (mBatchVertexCount + (numVerts))) \
        { \
            FlushPendingDrawCalls(); \
        } \
        (ptr) = (mBatchVertices + mBatchVertexCount); \
        mBatchVertexCount += (numVerts); \
    }

bool GuiRenderer::Initialize()
{
    if (!mGuiRenderProgram.LoadProgram())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize ui render manager");

        Deinit();
        return false;
    }

    // allocate vertex buffer
    mVertexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
    debug_assert(mVertexBuffer);

    return true;
}

void GuiRenderer::Deinit()
{
    if (mVertexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mVertexBuffer);
        mVertexBuffer = nullptr;
    }

    mGuiRenderProgram.FreeProgram();
}

void GuiRenderer::RenderFrameBegin()
{
    RenderStates guiRenderStates = RenderStates::GetForUI();
    gGraphicsDevice.SetRenderStates(guiRenderStates);

    Rectangle screenRect;
    screenRect.x = 0;
    screenRect.y = 0;
    screenRect.w = gGraphicsDevice.mViewportRect.w;
    screenRect.h = gGraphicsDevice.mViewportRect.h;

    mProjectionMatrix2D = glm::ortho(screenRect.x * 1.0f, 
        (screenRect.x + screenRect.w) * 1.0f, 
        (screenRect.y + screenRect.h) * 1.0f, screenRect.y * 1.0f);

    mGuiRenderProgram.ActivateProgram();
    mGuiRenderProgram.SetViewProjectionMatrix(mProjectionMatrix2D);
    // set scissor box
    gGraphicsDevice.SetScissorRect(screenRect);

    mCurrentTransform = nullptr;
}

void GuiRenderer::RenderFrameEnd()
{
    FlushPendingDrawCalls();

    mGuiRenderProgram.DeactivateProgram();

    // restore scissor box
    Rectangle screenRect;
    screenRect.x = 0;
    screenRect.y = 0;
    screenRect.w = gGraphicsDevice.mViewportRect.w;
    screenRect.h = gGraphicsDevice.mViewportRect.h;
    gGraphicsDevice.SetScissorRect(screenRect);
}

void GuiRenderer::SetCurrentTransform(glm::mat4* matrix)
{
    mCurrentTransform = matrix;
}

void GuiRenderer::FillRect(const Rectangle& rect, Color32 fillColor)
{
    SetCurrentBatchTexture(gTexturesManager.mWhiteTexture);

    Vertex2D* vertices = nullptr;
    ALLOCATE_VERTICES(6, vertices);

    vertices[0].mColor = fillColor;
    vertices[0].mPosition.x = rect.x * 1.0f;
    vertices[0].mPosition.y = rect.y * 1.0f;
    vertices[1].mColor = fillColor;
    vertices[1].mPosition.x = vertices[0].mPosition.x;
    vertices[1].mPosition.y = (rect.y + rect.h) * 1.0f;
    vertices[2].mColor = fillColor;
    vertices[2].mPosition.x = (rect.x + rect.w) * 1.0f;
    vertices[2].mPosition.y = vertices[1].mPosition.y;
    vertices[3] = vertices[0];
    vertices[4] = vertices[2];
    vertices[5].mColor = fillColor;
    vertices[5].mPosition.x = vertices[2].mPosition.x;
    vertices[5].mPosition.y = vertices[0].mPosition.y;

    TransformVertices(vertices);
}

void GuiRenderer::DrawRect(const Rectangle& rect, Color32 lineColor)
{
    // todo
}

void GuiRenderer::DrawQuads(Texture2D* texture, const GuiQuadStruct* quads, int quadsCount)
{
    if (quads == nullptr || quadsCount < 1)
    {
        debug_assert(false);
        return;
    }

    SetCurrentBatchTexture(texture);

    Vertex2D* vertices = nullptr;
    ALLOCATE_VERTICES(6 * quadsCount, vertices);

    // push all quad vertices to vertex cache
    for (int iquad = 0; iquad < quadsCount; ++iquad)
    {
        vertices[iquad * 6 + 0] = quads[iquad].mPoints[0];
        vertices[iquad * 6 + 1] = quads[iquad].mPoints[1];
        vertices[iquad * 6 + 2] = quads[iquad].mPoints[2];
        vertices[iquad * 6 + 3] = quads[iquad].mPoints[0];
        vertices[iquad * 6 + 4] = quads[iquad].mPoints[2];
        vertices[iquad * 6 + 5] = quads[iquad].mPoints[3];
    }
    TransformVertices(vertices);
}

void GuiRenderer::FlushPendingDrawCalls()
{
    if (mBatchVertexCount < 1)
        return;

    const int VertexBufferSize = mBatchVertexCount * Sizeof_Vertex2D;
    if (!mVertexBuffer->Setup(eBufferUsage_Stream, VertexBufferSize, nullptr))
    {
        debug_assert(false);
    }

    void* verticesPoiner = mVertexBuffer->Lock(BufferAccess_UnsynchronizedWrite);
    debug_assert(verticesPoiner);

    // upload vertices data
    if (verticesPoiner)
    {
        ::memcpy(verticesPoiner, mBatchVertices, VertexBufferSize);
        if (!mVertexBuffer->Unlock())
        {
            debug_assert(false);
        }
    }

    // draw
    gGraphicsDevice.BindIndexBuffer(nullptr);
    gGraphicsDevice.BindVertexBuffer(mVertexBuffer, Vertex2D_Format::Get());
    gGraphicsDevice.RenderPrimitives(ePrimitiveType_Triangles, 0, mBatchVertexCount);

    mBatchVertexCount = 0;
}

void GuiRenderer::SetCurrentBatchTexture(Texture2D* newTexutre)
{
    if (mCurrentTexture != newTexutre)
    {
        FlushPendingDrawCalls();
        mCurrentTexture = newTexutre;
    }

    if (mCurrentTexture)
    {
        mCurrentTexture->ActivateTexture(eTextureUnit_0);
    }
}

void GuiRenderer::TransformVertices(Vertex2D* vertices)
{
    if (mCurrentTransform == nullptr || vertices == nullptr)
        return;

    Vertex2D* vertices_end = mBatchVertices + mBatchVertexCount;
    for (; vertices != vertices_end; ++vertices)
    {
        vertices->mPosition = glm::vec2(*mCurrentTransform * glm::vec4(vertices->mPosition, 0.0f, 1.0f));
    }
}

void GuiRenderer::TransformClipRect(Rectangle& rectangle) const
{
    if (mCurrentTransform)
    {
        glm::vec2 pos = glm::vec2(*mCurrentTransform * glm::vec4(rectangle.x * 1.0, rectangle.y * 1.0f, 0.0f, 1.0f));
        glm::vec2 dim = glm::vec2(*mCurrentTransform * glm::vec4(rectangle.w * 1.0, rectangle.h * 1.0f, 0.0f, 0.0f));

        rectangle.w = (int) dim.x;
        rectangle.h = (int) dim.y;

        rectangle.x = (int) pos.x;
        rectangle.y = (int) pos.y;

        rectangle.y = gGraphicsDevice.mViewportRect.h - (rectangle.y + rectangle.h);
    }
}

void GuiRenderer::PushClipRect(const Rectangle& rcLocal)
{
    Rectangle cliprect = rcLocal;

    TransformClipRect(cliprect);
    gGraphicsDevice.SetScissorRect(cliprect);
}

void GuiRenderer::PopClipRect()
{
}