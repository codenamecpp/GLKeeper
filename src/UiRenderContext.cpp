#include "stdafx.h"
#include "UiRenderContext.h"
#include "GameRenderManager.h"
#include "ShadersManager.h"
#include "TextureManager.h"
#include "UiManager.h"

//////////////////////////////////////////////////////////////////////////

#define ALLOCATE_VERTICES(numVerts, ptr) \
    { \
        cxx_assert((numVerts) < MaxBatchVertices); \
        if (MaxBatchVertices < (mBatchVertexCount + (numVerts))) \
        { \
            FlushDeferred(); \
        } \
        (ptr) = (mBatchVertices + mBatchVertexCount); \
        mBatchVertexCount += (numVerts); \
    }

//////////////////////////////////////////////////////////////////////////

static const Point2D ZeroDimensions (0, 0);
static const TextureRegion IdentTextureRegion = TextureRegion::GetIdent();

//////////////////////////////////////////////////////////////////////////

bool UiRenderContext::Initialize()
{
    mShaderProgram = gShadersManager.GetProgramOfType<ShaderProgram_UI>("ui");
    cxx_assert(mShaderProgram);
    if (mShaderProgram == nullptr)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load ui shader program");
        return false;
    }

    mShaderProgram->InitRenderData();

    mBatchVertexCount = 0;
    mVertexBufferCursor = 0;

    mWhiteTexture = gTextureManager.GetWhiteTexture();
    cxx_assert(mWhiteTexture);

    if (mWhiteTexture && !mWhiteTexture->IsRenderDataInited())
    {
        mWhiteTexture->InitRenderData();
    }

    mCurrentTextureIsA8 = false;

    // allocate vertex buffer
    unsigned int vertexBufferLength = MaxBatchVertices * Sizeof_Vertex2D;
    mVertexBuffer = gRenderDevice.CreateVertexBuffer(eBufferUsage_Dynamic, vertexBufferLength);
    cxx_assert(mVertexBuffer);

    if (!mVertexBuffer)
        return false;

    mVertexBuffer->ConfigureVertexDefinitions(Vertex2D_Format::Get());
    return true;
}

void UiRenderContext::Deinit()
{
    mVertexBuffer.reset();

    mShaderProgram = nullptr;
}

void UiRenderContext::BeginFrame()
{
    mRestoreRenderStates = gRenderDevice.GetCurrentRenderStates();

    RenderStates guiRenderStates = RenderStates::GetUIStates();
    gRenderDevice.SetRenderState(guiRenderStates);

    mCurrentBlendingMode = guiRenderStates.mBlendingMode;
    mScreenRect = gUiManager.GetScreenRect();

    mProjectionMatrix2D = glm::ortho(mScreenRect.x * 1.0f, 
        (mScreenRect.x + mScreenRect.w) * 1.0f, 
        (mScreenRect.y + mScreenRect.h) * 1.0f, mScreenRect.y * 1.0f);

    mShaderProgram->BindProgram();
    mShaderProgram->SetViewProjectionMatrix(mProjectionMatrix2D);
    mShaderProgram->SetTextureAlpha(mCurrentTextureIsA8);
    // set scissor box
    gRenderDevice.SetScissorRect(mScreenRect);

    SetTransform(nullptr);
}

void UiRenderContext::EndFrame()
{
    FlushDeferred();

    mCurrentTexture = nullptr;

    gRenderDevice.SetRenderState(mRestoreRenderStates);
    // restore scissor box
    gRenderDevice.ResetScissorRect();

    SetTransform(nullptr);
}

void UiRenderContext::SetTransform(glm::mat4* matrix)
{
    mCurrentTransform = matrix;
}

void UiRenderContext::FillRect(const Rect2D& rect, Color32 fillColor)
{
    SwitchTexture(nullptr);

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

void UiRenderContext::DrawRect(const Rect2D& rect, Color32 lineColor, int lineWidth)
{
    Rect2D rc;
    // left
    rc.x = rect.x;
    rc.y = rect.y;
    rc.w = lineWidth;
    rc.h = rect.h;
    FillRect(rc, lineColor);

    // right
    rc.x = (rect.x + rect.w) - lineWidth;
    FillRect(rc, lineColor);

    // top
    rc.x = rect.x + lineWidth;
    rc.w = rect.w - (lineWidth * 2);
    rc.h = lineWidth;
    FillRect(rc, lineColor);

    // bottom
    rc.y = (rect.y + rect.h) - lineWidth;
    FillRect(rc, lineColor);
}

void UiRenderContext::DrawTexture(Texture* texture, Color32 theColor, const Rect2D& theDest, const Rect2D& theSrc)
{
    GpuTexture2D* textureRenderData = texture ? texture->AcquireTextureRenderData() : nullptr;
    
    Quad2D quad;
    quad.BuildTextureQuad(texture ? texture->GetTextureRegion() : IdentTextureRegion, theSrc, theDest, theColor);
    DrawQuads(textureRenderData, &quad, 1);
}

void UiRenderContext::DrawTexture(Texture* texture, Color32 theColor, const Point2D& theDest, const Rect2D& theSrc)
{
    const Rect2D dstRect { theDest.x, theDest.y, theSrc.w, theSrc.h };
    DrawTexture(texture, theColor, dstRect, theSrc);
}

void UiRenderContext::DrawQuads(Texture* texture, const Quad2D* quads, int quadsCount)
{
    GpuTexture2D* textureRenderData = texture ? texture->AcquireTextureRenderData() : nullptr;
    DrawQuads(textureRenderData, quads, quadsCount);
}

void UiRenderContext::DrawTexture(Texture* texture, Color32 theColor, const Rect2D& theDest)
{
    GpuTexture2D* textureRenderData = texture ? texture->AcquireTextureRenderData() : nullptr;

    Quad2D quad;
    quad.BuildTextureQuad(texture ? texture->GetTextureRegion() : IdentTextureRegion, theDest, theColor);
    DrawQuads(textureRenderData, &quad, 1);
}

void UiRenderContext::DrawTexture(Texture* texture, Color32 theColor, const Point2D& theDest)
{
    const Point2D& imageDimensions = texture ? texture->GetTextureRegion().GetDimensions() : ZeroDimensions;
    const Rect2D dstRect { theDest.x, theDest.y, imageDimensions.x, imageDimensions.y };
    DrawTexture(texture, theColor, dstRect);
}

void UiRenderContext::DrawTexture(GpuTexture2D* texture, Color32 theColor, const Rect2D& theDest, const Rect2D& theSrc)
{
    const Point2D& dimensions = texture ? texture->GetDimensions() : ZeroDimensions;

    Quad2D quad;
    quad.BuildTextureQuad(dimensions, theSrc, theDest, theColor);
    DrawQuads(texture, &quad, 1);
}

void UiRenderContext::DrawTexture(GpuTexture2D* texture, Color32 theColor, const Point2D& theDest, const Rect2D& theSrc)
{
    const Rect2D dstRect { theDest.x, theDest.y, theSrc.w, theSrc.h };
    DrawTexture(texture, theColor, dstRect, theSrc);
}

void UiRenderContext::DrawTexture(GpuTexture2D* texture, Color32 theColor, const Rect2D& theDest)
{
    const Point2D& dimensions = texture ? texture->GetDimensions() : ZeroDimensions;
    const Rect2D srcRect { 0, 0, dimensions.x, dimensions.y };

    Quad2D quad;
    quad.BuildTextureQuad(dimensions, srcRect, theDest, theColor);
    DrawQuads(texture, &quad, 1);
}

void UiRenderContext::DrawTexture(GpuTexture2D* texture, Color32 theColor, const Point2D& theDest)
{
    const Point2D& dimensions = texture ? texture->GetDimensions() : ZeroDimensions;
    const Rect2D srcRect { 0, 0, dimensions.x, dimensions.y };
    const Rect2D dstRect { theDest.x, theDest.y, srcRect.w, srcRect.h };

    Quad2D quad;
    quad.BuildTextureQuad(dimensions, srcRect, dstRect, theColor);
    DrawQuads(texture, &quad, 1);
}

void UiRenderContext::DrawQuads(GpuTexture2D* texture, const Quad2D* quads, int quadsCount)
{
    if ((quads == nullptr) || (quadsCount < 1))
    {
        cxx_assert(false);
        return;
    }

    SwitchTexture(texture);

    Vertex2D* vertices = nullptr;
    ALLOCATE_VERTICES(6 * quadsCount, vertices);

    // push all quad vertices to vertex cache
    for (int iquad = 0; iquad < quadsCount; ++iquad)
    {
        const auto& points = quads[iquad].mPoints;
        const auto& qverts = vertices + (iquad * 6);
        qverts[0] = points[0];
        qverts[1] = points[1];
        qverts[2] = points[2];
        qverts[3] = points[0];
        qverts[4] = points[2];
        qverts[5] = points[3];
    }
    TransformVertices(vertices);
}

void UiRenderContext::DrawTextQuads(Font* font, const Quad2D* quads, int quadsCount)
{
    if (font && !font->IsRenderDataInited())
    {
        font->InitRenderData();
    }
    GpuTexture2D* gpuTexture = font ? font->GetGpuTexturePtr() : nullptr;
    DrawQuads(gpuTexture, quads, quadsCount);
}

eBlendingMode UiRenderContext::BeginBlendingMode(eBlendingMode newBlendingMode)
{
    const eBlendingMode prevBlendingMode = mCurrentBlendingMode;
    SwitchBlendingMode(newBlendingMode);
    return prevBlendingMode;
}

void UiRenderContext::ResetBlendingMode()
{
    SwitchBlendingMode(mRestoreRenderStates.mBlendingMode);
}

void UiRenderContext::FlushDeferred()
{
    if (mBatchVertexCount < 1)
        return;

    BufferAccessBits bufferAccess = BufferAccess_UnsynchronizedWrite;
    if (mBatchVertexCount > (MaxBatchVertices - mVertexBufferCursor))
    {
        mVertexBufferCursor = 0;
        bufferAccess = bufferAccess | BufferAccess_InvalidateBuffer; // orphan
    }

    const int vertexDataLength = mBatchVertexCount * Sizeof_Vertex2D;
    void* verticesPoiner = mVertexBuffer->Lock(bufferAccess, mVertexBufferCursor * Sizeof_Vertex2D, vertexDataLength);
    cxx_assert(verticesPoiner);

    bool isSuccess = false;
    if (verticesPoiner)
    {
        // upload vertices data
        ::memcpy(verticesPoiner, mBatchVertices, vertexDataLength);
        isSuccess = mVertexBuffer->Unlock();
        cxx_assert(isSuccess);
    }

    if (isSuccess)
    {
        if (mCurrentTexture)
        {
            gRenderDevice.BindTexture2D(eTextureUnit_0, mCurrentTexture);
        }
        // draw
        gRenderDevice.BindVertexBuffer(mVertexBuffer.get());
        gRenderDevice.RenderPrimitives(ePrimitiveType_Triangles, mVertexBufferCursor, mBatchVertexCount);
    }
    // advance cursor
    mVertexBufferCursor += mBatchVertexCount;
    mBatchVertexCount = 0;
}

void UiRenderContext::SwitchTexture(GpuTexture2D* texture)
{
    if (texture == nullptr)
    {
        // use fallback
        texture = mWhiteTexture ? mWhiteTexture->AcquireTextureRenderData() : nullptr;
        cxx_assert(texture);
    }

    if (mCurrentTexture == texture) 
        return;

    FlushDeferred();
    mCurrentTexture = texture;
    SwitchCurrentTextureIsA8(mCurrentTexture->GetPixelFormat() == ePixelFormat_R8);
}

void UiRenderContext::SwitchCurrentTextureIsA8(bool isAlphaTexture)
{
    if (mCurrentTextureIsA8 == isAlphaTexture) 
        return;

    FlushDeferred();
    mCurrentTextureIsA8 = isAlphaTexture;
    mShaderProgram->SetTextureAlpha(mCurrentTextureIsA8);
}

void UiRenderContext::SwitchBlendingMode(eBlendingMode newBlendingMode)
{
    if (mCurrentBlendingMode == newBlendingMode) 
        return;

    FlushDeferred();
    mCurrentBlendingMode = newBlendingMode;

    RenderStates renderStates = gRenderDevice.GetCurrentRenderStates();
    renderStates.mBlendingMode = newBlendingMode;
    gRenderDevice.SetRenderState(renderStates);
}

void UiRenderContext::TransformVertices(Vertex2D* vertices)
{
    if (mCurrentTransform == nullptr || vertices == nullptr)
        return;

    Vertex2D* vertices_end = mBatchVertices + mBatchVertexCount;
    for (; vertices != vertices_end; ++vertices)
    {
        vertices->mPosition = glm::vec2(*mCurrentTransform * glm::vec4(vertices->mPosition, 0.0f, 1.0f));
    }
}

void UiRenderContext::TransformClipRect(Rect2D& rectangle) const
{
    if (mCurrentTransform)
    {
        glm::vec2 pos = glm::vec2(*mCurrentTransform * glm::vec4(rectangle.x * 1.0, rectangle.y * 1.0f, 0.0f, 1.0f));
        glm::vec2 dim = glm::vec2(*mCurrentTransform * glm::vec4(rectangle.w * 1.0, rectangle.h * 1.0f, 0.0f, 0.0f));

        rectangle.w = (int) dim.x;
        rectangle.h = (int) dim.y;

        rectangle.x = (int) pos.x;
        rectangle.y = (int) pos.y;

        rectangle.y = mScreenRect.h - (rectangle.y + rectangle.h);
    }
}

bool UiRenderContext::EnterChildClipArea(const Rect2D& rcLocal)
{
    Rect2D newCliprect = rcLocal;
    TransformClipRect(newCliprect);

    const Rect2D& currentCliprect = gRenderDevice.GetScissorBox();

    newCliprect = newCliprect.GetIntersection(currentCliprect);
    if (newCliprect.h < 1 || newCliprect.w < 1)
    {
        return false;
    }

    mClipRectsStack.push_back(currentCliprect);
    if (newCliprect != currentCliprect)
    {
        FlushDeferred();

        gRenderDevice.SetScissorRect(newCliprect);
    }
    return true;
}

void UiRenderContext::LeaveChildClipArea()
{
    if (mClipRectsStack.empty())
    {
        cxx_assert(false);
        return;
    }

    Rect2D prevCliprect = mClipRectsStack.back();
    mClipRectsStack.pop_back();

    const Rect2D& currentCliprect = gRenderDevice.GetScissorBox();

    if (currentCliprect != prevCliprect)
    {
        FlushDeferred();
        gRenderDevice.SetScissorRect(prevCliprect);
    }
}