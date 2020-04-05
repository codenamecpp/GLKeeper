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

    Rect2D screenRect;
    screenRect.mX = 0;
    screenRect.mY = 0;
    screenRect.mSizeX = gGraphicsDevice.mViewportRect.mSizeX;
    screenRect.mSizeY = gGraphicsDevice.mViewportRect.mSizeY;

    mProjectionMatrix2D = glm::ortho(screenRect.mX * 1.0f, 
        (screenRect.mX + screenRect.mSizeX) * 1.0f, 
        (screenRect.mY + screenRect.mSizeY) * 1.0f, screenRect.mY * 1.0f);

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
    Rect2D screenRect;
    screenRect.mX = 0;
    screenRect.mY = 0;
    screenRect.mSizeX = gGraphicsDevice.mViewportRect.mSizeX;
    screenRect.mSizeY = gGraphicsDevice.mViewportRect.mSizeY;
    gGraphicsDevice.SetScissorRect(screenRect);
}

void GuiRenderer::SetCurrentTransform(glm::mat4* matrix)
{
    mCurrentTransform = matrix;
}

void GuiRenderer::FillRect(const Rect2D& rect, Color32 fillColor)
{
    SetCurrentBatchTexture(gTexturesManager.mWhiteTexture);

    Vertex2D* vertices = nullptr;
    ALLOCATE_VERTICES(6, vertices);

    vertices[0].mColor = fillColor;
    vertices[0].mPosition.x = rect.mX * 1.0f;
    vertices[0].mPosition.y = rect.mY * 1.0f;
    vertices[1].mColor = fillColor;
    vertices[1].mPosition.x = vertices[0].mPosition.x;
    vertices[1].mPosition.y = (rect.mY + rect.mSizeY) * 1.0f;
    vertices[2].mColor = fillColor;
    vertices[2].mPosition.x = (rect.mX + rect.mSizeX) * 1.0f;
    vertices[2].mPosition.y = vertices[1].mPosition.y;
    vertices[3] = vertices[0];
    vertices[4] = vertices[2];
    vertices[5].mColor = fillColor;
    vertices[5].mPosition.x = vertices[2].mPosition.x;
    vertices[5].mPosition.y = vertices[0].mPosition.y;

    if (mCurrentTransform)
    {
        for (int icurr = 0; icurr < 6; ++icurr)
        {
            vertices[icurr].mPosition = glm::vec2(*mCurrentTransform * glm::vec4(vertices[icurr].mPosition, 0.0f, 1.0f));
        }
    }
}

void GuiRenderer::DrawRect(const Rect2D& rect, Color32 lineColor)
{
    // todo
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
