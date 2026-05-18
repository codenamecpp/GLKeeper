#pragma once

#include "BitmapImage.h"
#include "SurfaceMaterial.h"
#include "GpuProgram.h"
#include "GpuBuffer.h"
#include "GpuTexture2D.h"

//////////////////////////////////////////////////////////////////////////
// Render Device
// Provides a low-level rendering operations and gpu resources
//////////////////////////////////////////////////////////////////////////

class RenderDevice final
{
public:
    RenderFrameStatistics mFrameStats;

public:
    RenderDevice();
    ~RenderDevice();

    // Initialize video system, switch fullscreen mode and enable vsync
    bool Initialize(const Point2D& screenResolution, bool fullscreen, bool vsync);
    void Shutdown();

    // Prepare to render next frame, poll window system events
    void BeginFrame();
    void EndFrame();

    // Set clear color for render revice
    void SetClearColor(Color32 clearColor);

    // Clear color and depth of current framebuffer
    void ClearScreen(eDeviceClearMode clearMode = eDeviceClear_ColorDepthBuffers);

    // Turn vsync mode on or off
    void EnableVSync(bool vsyncEnabled);

    // Turn fullscreen mode on or off
    void EnableFullscreen(bool fullscreenEnabled);

    // Create 2D texture from source image
    std::unique_ptr<GpuTexture2D> CreateTexture2D(const BitmapImage& picture, 
        eTextureFiltering filter = eTextureFiltering_None, 
        eTextureRepeating repeat = eTextureRepeating_ClampToEdge);

    std::unique_ptr<GpuTexture2D> CreateTexture2D(const Point2D& textureDims, 
        ePixelFormat pixelFormat,
        const void* pixeldata,
        eTextureFiltering filter = eTextureFiltering_None, 
        eTextureRepeating repeat = eTextureRepeating_ClampToEdge);

    // Create render program from shader source code
    std::unique_ptr<GpuProgram> CreateShaderProgram(const std::string& vertShaderSource, const std::string& fragShaderSource);

    // Create hardware vertex / index buffer of specified length
    std::unique_ptr<GpuVertexBuffer> CreateVertexBuffer(eBufferUsage bufferUsage, unsigned int bufferLength);
    std::unique_ptr<GpuIndexBuffer> CreateIndexBuffer(eBufferUsage bufferUsage, unsigned int bufferLength);

    // Set source buffer for geometries vertex data and setup layout for bound shader
    void BindVertexBuffer(GpuVertexBuffer* sourceBuffer, const VertexFormat& streamDefinition);
    void BindVertexBuffer(GpuVertexBuffer* sourceBuffer);
    void BindIndexBuffer(GpuIndexBuffer* sourceBuffer);
    void BindTexture2D(eTextureUnit textureUnit, GpuTexture2D* texture2D);
    void BindShaderProgram(GpuProgram* program);
        
    // Set current render state
    inline void SetRenderState(const RenderStates& rstate)
    {
        InternalSetRenderState(rstate, false);
    }

    const RenderStates& GetCurrentRenderStates() const { return mCurrentStates; }

    // Render geometry
    void RenderIndexedPrimitives(ePrimitiveType primitiveType, eIndicesType indicesType, unsigned int dataOffset, unsigned int numIndices);
    void RenderIndexedPrimitives(ePrimitiveType primitiveType, eIndicesType indicesType, unsigned int dataOffset, unsigned int numIndices, unsigned int baseVertex);
    void RenderPrimitives(ePrimitiveType primitiveType, unsigned int firstIndex, unsigned int numElements);

    const Point2D& GetScreenResolution() const { return mScreenResolution; }

    // Setup dimensions of graphic device viewport
    void SetViewportRect(const Rect2D& theRectangle);
    void ResetViewportRect();

    // Get dimensions of graphic device viewport
    const Viewport& GetViewport() const { return mViewport; }

    // setup clip rect, default is whole viewport area
    void SetScissorRect(const Rect2D& sourceRectangle);
    void ResetScissorRect();

    const Rect2D& GetScissorBox() const { return mScissorBox; }

private:
    // Force render state
    void InternalSetRenderState(const RenderStates& rstate, bool forceState);
    bool InitializeOGLExtensions();

    void SetupVertexAttributes(const VertexFormat& streamDefinition);

    void WindowSizeChanged(int sizex, int sizey);

private:
    GpuResourceHandle mVaoHandle;

    RenderStates mCurrentStates;
    Viewport mViewport;
    Point2D mScreenResolution;
    Rect2D mScissorBox;
    GLFWwindow* mGraphicsWindow;
    GLFWmonitor* mGraphicsMonitor;

    RenderFrameStatistics mCurrentFrameStats;

    std::optional<Point2D> mScreenSizeChanged;
};

extern RenderDevice gRenderDevice;