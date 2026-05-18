#pragma once

#include "GraphicsDefs.h"
#include "GpuBuffer.h"
#include "ShaderProgram.h"
#include "Font.h"
#include "UiDefs.h"
#include "Texture.h"

//////////////////////////////////////////////////////////////////////////
// Gui Render Context
//////////////////////////////////////////////////////////////////////////

class UiRenderContext: public cxx::noncopyable
{
public:
    bool Initialize();
    void Deinit();
    
    // setup view matrix and initialize resources
    // it should be called before using any gui drawing methods
    void BeginFrame();

    // will flush pending drawing operations and restore previous render state
    // it should be called at end of gui rendering
    void EndFrame();

    // set transformation matrix for drawing gui elements
    void SetTransform(glm::mat4* matrix);

    // @returns false if area is being cut off entirely entirely 
    bool EnterChildClipArea(const Rect2D& rcLocal);
    void LeaveChildClipArea();

    // draw without textures
    void FillRect(const Rect2D& rect, Color32 fillColor);
    void DrawRect(const Rect2D& rect, Color32 lineColor, int lineWidth = 1);

    void DrawTexture(Texture* texture, Color32 theColor, const Rect2D& theDest, const Rect2D& theSrc);
    void DrawTexture(Texture* texture, Color32 theColor, const Point2D& theDest, const Rect2D& theSrc);
    void DrawTexture(Texture* texture, Color32 theColor, const Rect2D& theDest);
    void DrawTexture(Texture* texture, Color32 theColor, const Point2D& theDest);
    
    // draw quads
    void DrawQuads(Texture* texture, const Quad2D* quads, int quadsCount);
    void DrawQuad(Texture* texture, const Quad2D& quad)
    {
        DrawQuads(texture, &quad, 1);
    }

    // Render 2D text on the screen, mesh should be initialized properly
    void DrawTextQuads(Font* font, const std::vector<Quad2D>& quads);

    eBlendingMode BeginBlendingMode(eBlendingMode newBlendingMode);
    void ResetBlendingMode();

    const Rect2D& GetScreenRect() const { return mScreenRect; }

private:
    void FlushDeferred();

    void SwitchTexture(Texture* texture);
    void SwitchFont(Font* font);
    void SwitchCurrentTextureIsA8(bool isAlphaTexture);
    void SwitchBlendingMode(eBlendingMode newBlendingMode);

    void TransformVertices(Vertex2D* vertices);
    void TransformClipRect(Rect2D& rectangle) const;

private:
    RenderStates mRestoreRenderStates;

    eBlendingMode mCurrentBlendingMode = RENDER_STATES_BLENDMODE_ALPHA;

    ShaderProgram_UI* mShaderProgram = nullptr;

    glm::mat4 mProjectionMatrix2D;
    glm::mat4* mCurrentTransform = nullptr;

    std::vector<Rect2D> mClipRectsStack;
    Rect2D mScreenRect;

    Texture* mWhiteTexture = nullptr;
    Texture* mCurrentTexture = nullptr;
    Font* mCurrentFont = nullptr;

    bool mCurrentTextureIsA8 = false;

    static const int MaxBatchVertices = 16384;
    int mBatchVertexCount = 0;
    int mVertexBufferCursor = 0; // base vertex within vbo
    Vertex2D mBatchVertices[MaxBatchVertices];

    std::unique_ptr<GpuVertexBuffer> mVertexBuffer;
};