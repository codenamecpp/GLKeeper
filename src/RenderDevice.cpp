#include "stdafx.h"
#include "RenderDevice.h"
#include "OpenGLDefs.h"
#include "GameMain.h"
#include "Version.h"

//////////////////////////////////////////////////////////////////////////

#define MIN_SCREEN_WIDTH    1024
#define MIN_SCREEN_HEIGHT   768

//////////////////////////////////////////////////////////////////////////

RenderDevice gRenderDevice;

//////////////////////////////////////////////////////////////////////////

RenderDevice::RenderDevice()
    : mCurrentStates()
    , mViewport()
    , mGraphicsWindow()
    , mGraphicsMonitor()
{
}

RenderDevice::~RenderDevice()
{
}

bool RenderDevice::Initialize(const Point2D& screenResolution, bool fullscreen, bool vsync)
{
    cxx_assert(mGraphicsWindow == nullptr);
    cxx_assert(mGraphicsMonitor == nullptr);

    if (::glfwInit() == GL_FALSE)
    {
        gConsole.LogMessage(eLogLevel_Warning, "GLFW initialization failed");
        return false;
    }

    // dump some information
    gConsole.LogMessage(eLogLevel_Info, "GLFW Information: %s", ::glfwGetVersionString());
    gConsole.LogMessage(eLogLevel_Info, "Initialize OpenGL %d.%d (Core profile)",
        OPENGL_CONTEXT_MAJOR_VERSION, 
        OPENGL_CONTEXT_MINOR_VERSION);

    GLFWmonitor* primaryMonitor = ::glfwGetPrimaryMonitor();
    cxx_assert(primaryMonitor);

    // set screen size
    mScreenResolution = 
    {  
        std::max(screenResolution.x, MIN_SCREEN_WIDTH),
        std::max(screenResolution.y, MIN_SCREEN_HEIGHT)
    };

    gConsole.LogMessage(eLogLevel_Info, "Screen resolution (%dx%d)", mScreenResolution.x, mScreenResolution.y);

    // opengl params
    ::glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_CONTEXT_MAJOR_VERSION);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_CONTEXT_MINOR_VERSION);
    // setup window params
    if (fullscreen)
    {
        ::glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    }
    else
    {
        ::glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    }

    ::glfwWindowHint(GLFW_RED_BITS, 8);
    ::glfwWindowHint(GLFW_GREEN_BITS, 8);
    ::glfwWindowHint(GLFW_BLUE_BITS, 8);
    ::glfwWindowHint(GLFW_ALPHA_BITS, 8);
    ::glfwWindowHint(GLFW_DEPTH_BITS, 24);

    // create window and set current context
    GLFWwindow* graphicsWindow = ::glfwCreateWindow(mScreenResolution.x, mScreenResolution.y, GAME_TITLE_VER, (fullscreen ? primaryMonitor : nullptr), nullptr);
    cxx_assert(graphicsWindow);
    if (!graphicsWindow)
    {
        gConsole.LogMessage(eLogLevel_Warning, "glfwCreateWindow failed");
        ::glfwTerminate();
        return false;
    }

    // window size limits
    ::glfwSetWindowSizeLimits(graphicsWindow, MIN_SCREEN_WIDTH, MIN_SCREEN_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);

    // set window to screen center
    if (!fullscreen && primaryMonitor)
    {
        if (const GLFWvidmode* desktop = glfwGetVideoMode(primaryMonitor))
        {
            int windowSizeX = 0;
            int windowSizeY = 0;
            ::glfwGetWindowSize(graphicsWindow, &windowSizeX, &windowSizeY);

            cxx_assert((windowSizeX > 0) && (windowSizeY > 0));
            glfwSetWindowPos(graphicsWindow, (desktop->width - windowSizeX) >> 1, (desktop->height - windowSizeY) >> 1);
        }
    }

    // setup current opengl context and register callback handlers
    ::glfwMakeContextCurrent(graphicsWindow);
    ::glfwSetMouseButtonCallback(graphicsWindow, [](GLFWwindow*, int button, int action, int mods)
        {
            if (action == GLFW_REPEAT) return;
            if (button >= 0)
            {
                MouseButtonInputEvent ev { button, mods, action == GLFW_PRESS };
                gGame.InputEvent(ev);
            }
        });
    ::glfwSetKeyCallback(graphicsWindow, [](GLFWwindow*, int keycode, int scancode, int action, int mods)
        {
            if (action == GLFW_REPEAT) return;
            if (keycode >= 0)
            {
                KeyInputEvent ev { keycode, scancode, mods, action == GLFW_PRESS };
                gGame.InputEvent(ev);
            }
        });
    ::glfwSetCharCallback(graphicsWindow, [](GLFWwindow*, unsigned int unicodechar)
        {
            KeyCharEvent ev ( unicodechar );
            gGame.InputEvent(ev);
        });
    ::glfwSetScrollCallback(graphicsWindow, [](GLFWwindow*, double xscroll, double yscroll)
        {
            MouseScrollInputEvent ev 
            { 
                static_cast<int>(xscroll), 
                static_cast<int>(yscroll) 
            };
            gGame.InputEvent(ev);
        });
    ::glfwSetCursorPosCallback(graphicsWindow, [](GLFWwindow*, double xposition, double yposition)
        {
            MouseMovedInputEvent ev 
            { 
                static_cast<int>(xposition),
                static_cast<int>(yposition),
            };
            gGame.InputEvent(ev);
        });
    ::glfwSetWindowSizeCallback(graphicsWindow, [](GLFWwindow*, int sizex, int sizey)
        {
            gRenderDevice.WindowSizeChanged(sizex, sizey);
        });

    // setup opengl extensions
    if (!InitializeOGLExtensions())
    {
        ::glfwDestroyWindow(graphicsWindow);
        ::glfwTerminate();
        return false;
    }

    mGraphicsWindow = graphicsWindow;
    mGraphicsMonitor = primaryMonitor;

    // clear opengl errors
    ClearGLError();

    // create global vertex array object
    ::glGenVertexArrays(1, &mVaoHandle);
    glCheckErrors();

    ::glBindVertexArray(mVaoHandle);
    glCheckErrors();

    // setup viewport
    mViewport.Configure(mScreenResolution);
    ::glViewport(mViewport.mScreenArea.x, mViewport.mScreenArea.y, mViewport.mScreenArea.w, mViewport.mScreenArea.h);
    glCheckErrors();

    // default value for scissor is a whole viewport
    mScissorBox = mViewport.mScreenArea;

    ::glEnable(GL_SCISSOR_TEST);
    glCheckErrors();

    ::glScissor(mScissorBox.x, mScissorBox.y, mScissorBox.w, mScissorBox.h);
    glCheckErrors();

    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckErrors();

    // force clear screen at stratup
    ::glfwSwapBuffers(mGraphicsWindow);

    // setup default render state
    static const RenderStates defaultRenderState;
    InternalSetRenderState(defaultRenderState, true);

    EnableVSync(vsync);
    EnableFullscreen(fullscreen);
    mFrameStats.Clear();
    mCurrentFrameStats.Clear();

    mScreenSizeChanged.reset();
    return true;
}

void RenderDevice::Shutdown()
{
    // destroy vertex array object
    ::glBindVertexArray(0);
    glCheckErrors();

    ::glDeleteVertexArrays(1, &mVaoHandle);
    glCheckErrors();

    if (mGraphicsWindow) // shutdown glfw system
    {
        ::glfwDestroyWindow(mGraphicsWindow);
        ::glfwTerminate();

        mGraphicsMonitor = nullptr;
        mGraphicsWindow = nullptr;
    }
}

void RenderDevice::EnableVSync(bool vsyncEnabled)
{
    cxx_assert(mGraphicsWindow);
#if 0
    // this does work for Intel HD Graphics
    if (!!::wglSwapIntervalEXT)
    {
        ::wglSwapIntervalEXT(vsyncEnabled ? 1 : 0);
        return;
    }
#endif
    ::glfwSwapInterval(vsyncEnabled ? 1 : 0);
}

void RenderDevice::EnableFullscreen(bool fullscreenEnabled)
{
    cxx_assert(mGraphicsWindow);
    // todo
    (void) fullscreenEnabled;
}

bool RenderDevice::InitializeOGLExtensions()
{
    // initialize opengl extensions
    ::glewExperimental = GL_TRUE;

    // initialize glew
    GLenum resultCode = ::glewInit();
    if (resultCode != GLEW_OK)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Could not initialize OpenGL extensions (%s)", ::glewGetErrorString(resultCode));
        return false;
    }

    if (!GLEW_VERSION_3_2)
    {
        gConsole.LogMessage(eLogLevel_Warning, "OpenGL 3.2 API is not available");
        return false;
    }

    // dump opengl information
    gConsole.LogMessage(eLogLevel_Info, "OpenGL Vendor: %s", ::glGetString(GL_VENDOR));
    gConsole.LogMessage(eLogLevel_Info, "OpenGL Renderer: %s", ::glGetString(GL_RENDERER));
    gConsole.LogMessage(eLogLevel_Info, "OpenGL Version: %s", ::glGetString(GL_VERSION));
    gConsole.LogMessage(eLogLevel_Info, "GLSL Version: %s", ::glGetString(GL_SHADING_LANGUAGE_VERSION));
#if 0
    // query extensions
    GLint glNumExtensions = 0;

    ::glGetIntegerv(GL_NUM_EXTENSIONS, &glNumExtensions);
    if (glNumExtensions > 0)
    {
        gConsole.LogMessage(LogLevel_Info, "Supported OpenGL Extensions:");
        // enum all extensions
        for (GLint iextension = 0; iextension < glNumExtensions; ++iextension)
        {
            gConsole.LogMessage(LogLevel_Info, "%s", ::glGetStringi(GL_EXTENSIONS, iextension));
        }
    } // if extensions
#endif

    return true;
}

void RenderDevice::BindVertexBuffer(GpuVertexBuffer* sourceBuffer, const VertexFormat& streamDefinition)
{
    cxx_assert(GpuProgram::sCurrentProgram);
    cxx_assert(sourceBuffer);

    if (sourceBuffer)
    {
        GpuBuffer* previousBuffer = sourceBuffer->BindBuffer();
        if (previousBuffer != sourceBuffer)
        {
            ++mCurrentFrameStats.mNumSwitchVertexBuffers;
        }
        SetupVertexAttributes(streamDefinition);
    }
}

void RenderDevice::BindVertexBuffer(GpuVertexBuffer* sourceBuffer)
{
    cxx_assert(GpuProgram::sCurrentProgram);
    cxx_assert(sourceBuffer);

    if (sourceBuffer)
    {
        GpuBuffer* previousBuffer = sourceBuffer->BindBuffer();
        if (previousBuffer != sourceBuffer)
        {
            ++mCurrentFrameStats.mNumSwitchVertexBuffers;
        }
        SetupVertexAttributes(sourceBuffer->GetVertexDefinition());
    }
}

void RenderDevice::BindIndexBuffer(GpuIndexBuffer* sourceBuffer)
{
    cxx_assert(sourceBuffer);

    if (sourceBuffer)
    {
        GpuBuffer* previousBuffer = sourceBuffer->BindBuffer();
        if (previousBuffer != sourceBuffer)
        {
            ++mCurrentFrameStats.mNumSwitchIndexBuffers;
        }
    }
}

void RenderDevice::BindTexture2D(eTextureUnit textureUnit, GpuTexture2D* texture2D)
{
    cxx_assert(textureUnit < eTextureUnit_COUNT);
    if (GpuTexture2D::sCurrentTextures2D[textureUnit] == texture2D)
        return;

    // activate texture unit
    if (GpuTexture2D::sCurrentTextureUnit != textureUnit)
    {
        GpuTexture2D::sCurrentTextureUnit = textureUnit;
        ::glActiveTexture(GL_TEXTURE0 + textureUnit);
        glCheckErrors();
    }

    GpuTexture2D::sCurrentTextures2D[textureUnit] = texture2D;
    ++mCurrentFrameStats.mNumSwitchTextures;
    ::glBindTexture(GL_TEXTURE_2D, texture2D ? texture2D->mResourceHandle : 0);
    glCheckErrors();
}

void RenderDevice::BindShaderProgram(GpuProgram* program)
{
    if (GpuProgram::sCurrentProgram == program)
        return;

    ++mCurrentFrameStats.mNumSwitchPrograms;
    ::glUseProgram(program ? program->mResourceHandle : 0);
    glCheckErrors();
    if (program)
    {
        bool programAttributes[eVertexAttribute_MAX] = {};
        for (int streamIndex = 0; streamIndex < eVertexAttribute_MAX; ++streamIndex)
        {
            if (program->mAttributes[streamIndex] == -1)
                continue;

            programAttributes[program->mAttributes[streamIndex]] = true;
        }

        // setup attribute streams
        for (int ivattribute = 0; ivattribute < eVertexAttribute_COUNT; ++ivattribute)
        {
            if (programAttributes[ivattribute])
            {
                ::glEnableVertexAttribArray(ivattribute);
                glCheckErrors();
            }
            else
            {
                ::glDisableVertexAttribArray(ivattribute);
                glCheckErrors();
            }
        }
    }
    else
    {
        for (int ivattribute = 0; ivattribute < eVertexAttribute_MAX; ++ivattribute)
        {
            ::glDisableVertexAttribArray(ivattribute);
            glCheckErrors();
        }
    }
    GpuProgram::sCurrentProgram = program;
}

void RenderDevice::BeginFrame()
{
    cxx_assert(mGraphicsWindow);
    mCurrentFrameStats.Clear();

    // apply new screen size
    if (mScreenSizeChanged)
    {
        mScreenResolution = *mScreenSizeChanged;
        mScreenSizeChanged.reset();

        gConsole.LogMessage(eLogLevel_Debug, "Screen size changed (%dx%d)", mScreenResolution.x, mScreenResolution.y);

        ResetViewportRect();
        ResetScissorRect();

        // notify
        gGame.ScreenResolutionChanged();
    }
}

void RenderDevice::EndFrame()
{
    cxx_assert(mGraphicsWindow);

    ::glfwSwapBuffers(mGraphicsWindow);
    // process window messages
    ::glfwPollEvents();
    if (::glfwWindowShouldClose(mGraphicsWindow) == GL_TRUE)
    {
        gGame.RequestQuit();
        return;
    }
    mFrameStats = mCurrentFrameStats;
}

void RenderDevice::SetViewportRect(const Rect2D& theRectangle)
{
    if (mViewport.mScreenArea == theRectangle)
        return;

    mViewport.Configure(theRectangle);
    ::glViewport(theRectangle.x, theRectangle.y, theRectangle.w, theRectangle.h);
    glCheckErrors();
}

void RenderDevice::ResetViewportRect()
{
    Rect2D screenArea {0, 0, mScreenResolution.x, mScreenResolution.y};
    SetViewportRect(screenArea);
}

void RenderDevice::SetScissorRect(const Rect2D& sourceRectangle)
{
    if (mScissorBox == sourceRectangle)
        return;

    mScissorBox = sourceRectangle;
    ::glScissor(mScissorBox.x, mScissorBox.y, mScissorBox.w, mScissorBox.h);
    glCheckErrors();
}

std::unique_ptr<GpuTexture2D> RenderDevice::CreateTexture2D(const BitmapImage& theSourceImage,
    eTextureFiltering theFiltering, eTextureRepeating theRepeating)
{
    bool isGoodSource = theSourceImage.HasContent() && theSourceImage.IsPOT();
    cxx_assert(isGoodSource);
    if (!isGoodSource)
        return nullptr;

    // setup mipmaps info
    std::vector<Texture2DMip> mips;
    mips.reserve(theSourceImage.GetMipsCount());
    for (int imipmap = 0; imipmap < theSourceImage.GetMipsCount(); ++imipmap)
    {
        Point2D mipDims = theSourceImage.GetDimensions(imipmap);

        Texture2DMip& textureMip = mips.emplace_back();
        textureMip.mSizex = mipDims.x;
        textureMip.mSizey = mipDims.y;
        textureMip.mPixelsData = theSourceImage.GetMipPixels(imipmap);
    }

    std::unique_ptr<GpuTexture2D> texture2D = std::make_unique<GpuTexture2D>();
    if (texture2D->Create(theSourceImage.GetPixelFormat(), theSourceImage.GetMipsCount(), mips.data()))
    {
        texture2D->SetSamplerState(theFiltering, theRepeating);
    }
    else
    {
        // destroy texture on load error
        texture2D.reset();
    }

    return std::move(texture2D);
}

std::unique_ptr<GpuTexture2D> RenderDevice::CreateTexture2D(const Point2D& textureDims, ePixelFormat pixelFormat, const void* pixeldata, 
    eTextureFiltering filter, 
    eTextureRepeating repeat)
{
    cxx_assert(cxx::is_pot(textureDims.x) && cxx::is_pot(textureDims.y));
    if ((textureDims.x <= 0) || (textureDims.y <= 0))
    {
        cxx_assert(false);
        return nullptr;
    }
    cxx_assert(pixelFormat != ePixelFormat_Null);
    if (pixelFormat == ePixelFormat_Null)
    {
        return nullptr;
    }

    std::unique_ptr<GpuTexture2D> texture2D = std::make_unique<GpuTexture2D>();
    if (texture2D->Create(pixelFormat, textureDims.x, textureDims.y, pixeldata))
    {
        texture2D->SetSamplerState(filter, repeat);
    }
    else
    {
        // destroy texture on load error
        texture2D.reset();
    }

    return std::move(texture2D);
}

std::unique_ptr<GpuProgram> RenderDevice::CreateShaderProgram(const std::string& vertShaderSource, const std::string& fragShaderSource)
{
    std::unique_ptr<GpuProgram> gpuProgram = std::make_unique<GpuProgram>();
    if (!gpuProgram->CompileShader(vertShaderSource.c_str(), fragShaderSource.c_str()))
    {
        // destroy program object on error
        gpuProgram.reset();
    }
    return gpuProgram;
}

std::unique_ptr<GpuVertexBuffer> RenderDevice::CreateVertexBuffer(eBufferUsage bufferUsage, unsigned int bufferLength)
{
    std::unique_ptr<GpuVertexBuffer> vertexBuffer = std::make_unique<GpuVertexBuffer>();
    // try to create buffer of specified size
    if (!vertexBuffer->Create(bufferUsage, bufferLength, nullptr))
    {
        // failed so deallocate invalid buffer
        vertexBuffer.reset();
    }
    return vertexBuffer;
}

std::unique_ptr<GpuIndexBuffer> RenderDevice::CreateIndexBuffer(eBufferUsage bufferUsage, unsigned int bufferLength)
{
    std::unique_ptr<GpuIndexBuffer> indexBuffer = std::make_unique<GpuIndexBuffer>();
    // try to create buffer of specified size
    if (!indexBuffer->Create(bufferUsage, bufferLength, nullptr))
    {
        // failed so deallocate invalid buffer
        indexBuffer.reset();
    }
    return indexBuffer;
}

void RenderDevice::ClearScreen(eDeviceClearMode clearMode)
{
    GLbitfield bitfield = 0;
    switch (clearMode)
    {
        case eDeviceClear_ColorDepthBuffers: bitfield = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
            break;
        case eDeviceClear_ColorBuffer: bitfield = GL_COLOR_BUFFER_BIT;
            break;
        case eDeviceClear_DepthBuffer: bitfield = GL_DEPTH_BUFFER_BIT;
            break;
    }
    ::glClear(bitfield);
    glCheckErrors();
}

void RenderDevice::RenderIndexedPrimitives(ePrimitiveType primitiveType, eIndicesType indicesType, unsigned int dataOffset, unsigned int numIndices)
{    
    GpuBuffer* indexBuffer = GpuBuffer::sCurrentBuffers[eBufferTarget_Indices];
    GpuBuffer* vertexBuffer = GpuBuffer::sCurrentBuffers[eBufferTarget_Attributes];
    cxx_assert(indexBuffer && vertexBuffer && GpuProgram::sCurrentProgram);

    GLenum primitives = ToGLEnum(primitiveType);
    switch (primitiveType)
    {
        case ePrimitiveType_Triangles: 
            mCurrentFrameStats.mNumTrianglesDrawn += (numIndices / 3);
        break;
        case ePrimitiveType_TriangleFan:
        case ePrimitiveType_TriangleStrip: 
            mCurrentFrameStats.mNumTrianglesDrawn += (numIndices - 2);
        break;
    }
    GLenum indicesTypeGL = EnumToGL(indicesType);
    ++mCurrentFrameStats.mNumDIPs;
    ::glDrawElements(primitives, numIndices, indicesTypeGL, BUFFER_OFFSET(dataOffset));
    glCheckErrors();
}

void RenderDevice::RenderIndexedPrimitives(ePrimitiveType primitiveType, eIndicesType indicesType, unsigned int dataOffset, unsigned int numIndices, unsigned int baseVertex)
{
    GpuBuffer* indexBuffer = GpuBuffer::sCurrentBuffers[eBufferTarget_Indices];
    GpuBuffer* vertexBuffer = GpuBuffer::sCurrentBuffers[eBufferTarget_Attributes];
    cxx_assert(indexBuffer && vertexBuffer && GpuProgram::sCurrentProgram);

    GLenum primitives = ToGLEnum(primitiveType);
    switch (primitiveType)
    {
        case ePrimitiveType_Triangles: 
            mCurrentFrameStats.mNumTrianglesDrawn += (numIndices / 3);
        break;
        case ePrimitiveType_TriangleFan:
        case ePrimitiveType_TriangleStrip: 
            mCurrentFrameStats.mNumTrianglesDrawn += (numIndices - 2);
        break;
    }
    GLenum indicesTypeGL = EnumToGL(indicesType);
    ++mCurrentFrameStats.mNumDIPs;
    ::glDrawElementsBaseVertex(primitives, numIndices, indicesType, BUFFER_OFFSET(dataOffset), baseVertex);
    glCheckErrors();
}

void RenderDevice::RenderPrimitives(ePrimitiveType primitiveType, unsigned int offset, unsigned int numElements)
{
    GpuBuffer* vertexBuffer = GpuBuffer::sCurrentBuffers[eBufferTarget_Attributes];
    cxx_assert(vertexBuffer && GpuProgram::sCurrentProgram);

    GLenum primitives = ToGLEnum(primitiveType);
    switch (primitiveType)
    {
    case ePrimitiveType_Triangles: 
        mCurrentFrameStats.mNumTrianglesDrawn += (numElements / 3);
    break;
    case ePrimitiveType_TriangleFan:
    case ePrimitiveType_TriangleStrip: 
        mCurrentFrameStats.mNumTrianglesDrawn += (numElements - 2);
    break;
    }
    ++mCurrentFrameStats.mNumDIPs;
    ::glDrawArrays(primitives, offset, numElements);
    glCheckErrors();
}

void RenderDevice::SetClearColor(Color32 clearColor)
{
    static const float inv = 1.0f / 255.0f;
    ::glClearColor(clearColor.mR * inv, clearColor.mG * inv, clearColor.mB * inv, clearColor.mA * inv);
    glCheckErrors();
}

void RenderDevice::InternalSetRenderState(const RenderStates& rstate, bool forceState)
{
    if (mCurrentStates == rstate && !forceState)
        return;

    // polygon mode
    if (forceState || (mCurrentStates.mPolygonFill != rstate.mPolygonFill))
    {
        GLenum mode = GL_FILL;
        switch (rstate.mPolygonFill)
        {
            case RENDER_STATES_POLYGON_WIREFRAME: mode = GL_LINE; break;
            case RENDER_STATES_POLYGON_SOLID: mode = GL_FILL; break;
            default:
                cxx_assert(false);
            break;
        }
        ::glPolygonMode(GL_FRONT_AND_BACK, mode);
        glCheckErrors();
    }

    // depth testing
    if (forceState || (rstate.mIsDepthTestEnabled != mCurrentStates.mIsDepthTestEnabled))
    {
        if (rstate.mIsDepthTestEnabled)
        {
            ::glEnable(GL_DEPTH_TEST);
            glCheckErrors();
        }
        else
        {
            ::glDisable(GL_DEPTH_TEST);
            glCheckErrors();
        }
    }

    // depth function
    if (forceState || (rstate.mDepthFunc != mCurrentStates.mDepthFunc))
    {
        GLenum mode = GL_LEQUAL;
        switch (rstate.mDepthFunc)
        {
            case RENDER_STATES_DEPTHTESTFUNC_NOTEQUAL: mode = GL_NOTEQUAL; break;
            case RENDER_STATES_DEPTHTESTFUNC_ALWAYS: mode = GL_ALWAYS; break;
            case RENDER_STATES_DEPTHTESTFUNC_EQUAL: mode = GL_EQUAL; break;
            case RENDER_STATES_DEPTHTESTFUNC_LESS: mode = GL_LESS; break;
            case RENDER_STATES_DEPTHTESTFUNC_GREATER: mode = GL_GREATER; break;
            case RENDER_STATES_DEPTHTESTFUNC_LEQUAL: mode = GL_LEQUAL; break;
            case RENDER_STATES_DEPTHTESTFUNC_GEQUAL: mode = GL_GEQUAL; break;
            default:
                cxx_assert(false);
            break;
        }
        ::glDepthFunc(mode);
        glCheckErrors();
    }

    if (forceState || (rstate.mIsDepthWriteEnabled != mCurrentStates.mIsDepthWriteEnabled))
    {
        ::glDepthMask(rstate.mIsDepthWriteEnabled ? GL_TRUE : GL_FALSE);
        glCheckErrors();
    }

    if (forceState || (rstate.mIsColorWriteEnabled != mCurrentStates.mIsColorWriteEnabled))
    {
        const GLboolean isEnabled = rstate.mIsColorWriteEnabled ? GL_TRUE : GL_FALSE;
        ::glColorMask(
            isEnabled, 
            isEnabled, 
            isEnabled, 
            isEnabled);
        glCheckErrors();
    }

    // blending
    if (forceState || (rstate.mIsAlphaBlendEnabled != mCurrentStates.mIsAlphaBlendEnabled))
    {
        if (rstate.mIsAlphaBlendEnabled)
        {
            ::glEnable(GL_BLEND);
            glCheckErrors();
        }
        else
        {
            ::glDisable(GL_BLEND);
            glCheckErrors();
        }
    }

    if (forceState || (rstate.mBlendingMode != mCurrentStates.mBlendingMode))
    {
        GLenum srcFactor = GL_ZERO;
        GLenum dstFactor = GL_ZERO;

        switch (rstate.mBlendingMode)
        {
            case RENDER_STATES_BLENDMODE_ALPHA:
                srcFactor = GL_SRC_ALPHA;
                dstFactor = GL_ONE_MINUS_SRC_ALPHA;
            break;
            case RENDER_STATES_BLENDMODE_ADDITIVE:
                srcFactor = GL_ONE;
                dstFactor = GL_ONE;
            break;
            case RENDER_STATES_BLENDMODE_ALPHA_ADDITIVE:
                srcFactor = GL_SRC_ALPHA;
                dstFactor = GL_ONE;
            break;
            case RENDER_STATES_BLENDMODE_MULTIPLY:
                srcFactor = GL_DST_COLOR;
                dstFactor = GL_ZERO;
            break;
            case RENDER_STATES_BLENDMODE_PREMULTIPLIED:
                srcFactor = GL_ONE;
                dstFactor = GL_ONE_MINUS_SRC_ALPHA;
            break;
            case RENDER_STATES_BLENDMODE_SCREEN:
                srcFactor = GL_ONE_MINUS_DST_COLOR;
                dstFactor = GL_ONE;
            break;
            default:
                cxx_assert(false);
            break;
        }

        ::glBlendFunc(srcFactor, dstFactor);
        glCheckErrors();
    }

    // culling
    if (forceState || (rstate.mIsFaceCullingEnabled != mCurrentStates.mIsFaceCullingEnabled))
    {
        if (rstate.mIsFaceCullingEnabled)
        {
            ::glEnable(GL_CULL_FACE);
            glCheckErrors();
        }
        else
        {
            ::glDisable(GL_CULL_FACE);
            glCheckErrors();
        }
    }

    if (forceState || (rstate.mCullMode != mCurrentStates.mCullMode))
    {
        GLenum mode = GL_BACK;
        switch (rstate.mCullMode)
        {
            case RENDER_STATES_CULLFACEMODE_BACK: mode = GL_BACK; break;
            case RENDER_STATES_CULLFACEMODE_FRONT: mode = GL_FRONT; break;
            default:
                cxx_assert(false);
            break;
        }
        ::glCullFace(mode);
        glCheckErrors();
    }

    mCurrentStates = rstate;
}

void RenderDevice::SetupVertexAttributes(const VertexFormat& streamDefinition)
{
    GpuProgram* currentProgram = GpuProgram::sCurrentProgram;
    for (int iattribute = 0; iattribute < eVertexAttribute_COUNT; ++iattribute)
    {
        if (currentProgram->mAttributes[iattribute] == -1)
        {
            // current vertex attribute is unused in shader
            continue;
        }

        const auto& attribute = streamDefinition.mAttributes[iattribute];
        if (attribute.mFormat == eVertexAttributeFormat_Null)
        {
            cxx_assert(false);
            continue;
        }

        unsigned int numComponents = GetAttributeComponentCount(attribute.mFormat);
        if (numComponents == 0)
        {
            cxx_assert(numComponents > 0);
            continue;
        }

        // set attribute location
        GLenum dataType = GetAttributeDataTypeGL(attribute.mFormat);
        if ((dataType == GL_FLOAT) || attribute.mNormalized)
        {
            ::glVertexAttribPointer(currentProgram->mAttributes[iattribute], numComponents, dataType, 
                attribute.mNormalized ? GL_TRUE : GL_FALSE, 
                streamDefinition.mDataStride, BUFFER_OFFSET(attribute.mDataOffset + streamDefinition.mBaseOffset));
            glCheckErrors();
        }
        else
        {
            ::glVertexAttribIPointer(currentProgram->mAttributes[iattribute], numComponents, dataType, 
                streamDefinition.mDataStride, BUFFER_OFFSET(attribute.mDataOffset + streamDefinition.mBaseOffset));
            glCheckErrors();
        }
    }
}

void RenderDevice::ResetScissorRect()
{
    SetScissorRect(mViewport.mScreenArea);
}

void RenderDevice::WindowSizeChanged(int sizex, int sizey)
{
    Point2D newScreenSize {sizex, sizey};
    if (mScreenResolution != newScreenSize)
    {
        mScreenSizeChanged.emplace(newScreenSize);
    }
}

