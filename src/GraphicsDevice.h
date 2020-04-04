#pragma once

#include "GraphicsDeviceContext.h"
#include "VertexFormat.h"

class GraphicsDevice: public cxx::noncopyable
{
public:
    // readonly
    RenderStates mCurrentStates;
    Rect2D mViewportRect;
    Rect2D mScissorBox;
    GraphicsDeviceCaps mCaps;

    // current screen params
    Size2D mScreenResolution;

public:

    // initialize graphics system, setup resolution and vsync mode
    bool Initialize();

    // shutdown graphics system, any render operations will be ignored after this
    void Deinit();

    // turn vsync mode on or off
    // @param vsyncEnabled: True to enable or false to disable
    void EnableVSync(bool vsyncEnabled);

    // turn fullscreen mode on or off
    // @param fullscreenEnabled: True to fullscreen or false to windowed
    void EnableFullscreen(bool fullscreenEnabled);

    // create buffer texture, client is responsible for destroying resource
    // @param textureFormat: Format
    // @param dataLength: Data length in bytes
    // @param sourceData: Source data buffer
    GpuBufferTexture* CreateBufferTexture();
    GpuBufferTexture* CreateBufferTexture(eTextureFormat textureFormat, int dataLength, const void* sourceData);

    // create 2D texture, client is responsible for destroying resource
    GpuTexture2D* CreateTexture2D();

    // create texture array 2D, client is responsible for destroying resource
    // @param textureFormat: Format
    // @param dimensions: Texture dimensions, must be POT!
    // @param layersCount: Number of textures in array
    // @param sourceData: Source data buffer, all layers must be specified if not null
    GpuTextureArray2D* CreateTextureArray2D();
    GpuTextureArray2D* CreateTextureArray2D(eTextureFormat textureFormat, const Size2D& dimensions, int layersCount, const void* sourceData);

    // create render program, client is responsible for destroying resource
    // @param shaderSource: Source code
    GpuProgram* CreateRenderProgram();
    GpuProgram* CreateRenderProgram(const char* shaderSource);

    // create hardware buffer, client is responsible for destroying resource
    // @param bufferContent: Content type stored in buffer
    // @param bufferUsage: Usage hint of buffer
    // @param theLength: Data length
    // @param dataBuffer: Initial data, optional
    // @returns false if out of memory
    GpuBuffer* CreateBuffer(eBufferContent bufferContent);
    GpuBuffer* CreateBuffer(eBufferContent bufferContent, eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer);

    // set source buffer for geometries vertex data and setup layout for bound shader
    // @param sourceBuffer: Buffer reference or nullptr to unbind current
    // @param streamDefinition: Layout
    void BindVertexBuffer(GpuBuffer* sourceBuffer, const VertexFormat& streamDefinition);

    // set source buffer for geometries index data
    // @param sourceBuffer: Buffer reference or nullptr to unbind current
    void BindIndexBuffer(GpuBuffer* sourceBuffer);

    // set source texture on specified texture unit
    // @param textureUnit: Target unit
    // @param texture: Texture
    void BindTexture(eTextureUnit textureUnit, GpuBufferTexture* texture);
    void BindTexture(eTextureUnit textureUnit, GpuTexture2D* texture);
    void BindTexture(eTextureUnit textureUnit, GpuTextureArray2D* texture);

    // set source render program to render with
    // @param program: Target program
    void BindRenderProgram(GpuProgram* program);

    // free hardware resource
    // @param textureResource: Target texture, pointer becomes invalid
    void DestroyTexture(GpuBufferTexture* textureResource);
    void DestroyTexture(GpuTexture2D* textureResource);
    void DestroyTexture(GpuTextureArray2D* textureResource);

    // free hardware resource
    // @param programResource: Target render program, pointer becomes invalid
    void DestroyProgram(GpuProgram* programResource);

    // free hardware resource
    // @param bufferResource: Target buffer, pointer becomes invalid
    void DestroyBuffer(GpuBuffer* bufferResource);

    // set current render states
    // @param renderStates: Render states
    void SetRenderStates(const RenderStates& renderStates)
    {
        InternalSetRenderStates(renderStates, false);
    }

    // render indexed geometry
    // @param primitive: Type of primitives to render
    // @param indicesType: Type of indices data
    // @param offset: Offset within index buffer in bytes
    // @param numIndices: Number of elements
    // @param baseVertex: Specifies a constant that should be added to each element of indices when chosing elements from the vertex arrays
    void RenderIndexedPrimitives(ePrimitiveType primitive, eIndicesType indicesType, unsigned int offset, unsigned int numIndices);
    void RenderIndexedPrimitives(ePrimitiveType primitive, eIndicesType indicesType, unsigned int offset, unsigned int numIndices, unsigned int baseVertex);

    // render geometry
    // @param primitiveType: Type of primitives to render
    // @param firstIndex: Start position in attribute buffers, index
    // @param numElements: Number of elements to render
    void RenderPrimitives(ePrimitiveType primitiveType, unsigned int firstIndex, unsigned int numElements);

    // dispatch input events queue
    void ProcessInputEvents();

    // finish render frame, prenent on screen
    void Present();

    // setup dimensions of graphic device viewport
    // @param sourceRectangle: Viewport rectangle
    void SetViewportRect(const Rect2D& sourceRectangle);

    // setup clip rect, default is whole viewport area
    // @param sourceRectangle: Clipping rectangle
    void SetScissorRect(const Rect2D& sourceRectangle);

    // set clear color for render revice
    // @param clearColor: Color components
    void SetClearColor(Color32 clearColor);

    // clear color and depth of current framebuffer
    void ClearScreen();

    // get screen resolution aspect ratio
    float GetScreenResolutionAspect() const
    {
        return mScreenResolution.y > 0 ? (1.0f * mScreenResolution.x / mScreenResolution.y) : 1.0f;
    }

private:
    void InternalSetRenderStates(const RenderStates& renderStates, bool forceState);
    bool InitOpenGLExtensions();
    void QueryGraphicsDeviceCaps();
    void ActivateTextureUnit(eTextureUnit textureUnit);

    void HandleScreenResolutionChanged(int screenSizex, int screenSizey);

    void SetupVertexAttributes(const VertexFormat& streamDefinition);

private:
    GraphicsDeviceContext mDeviceContext;
};

extern GraphicsDevice gGraphicsDevice;