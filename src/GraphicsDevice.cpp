#include "pch.h"
#include "GraphicsDevice.h"
#include "Console.h"
#include "OpenGL_Defs.h"
#include "InputsManager.h"
#include "System.h"
#include "GpuBuffer.h"
#include "GpuBufferTexture.h"
#include "GpuProgram.h"
#include "GpuTexture2D.h"
#include "GpuTextureArray2D.h"

GraphicsDevice gGraphicsDevice;

// globals
static GLFWwindow* gGLFW_WindowHandle = nullptr;
static GLFWmonitor* gGLFW_MonitorHandle = nullptr;

//////////////////////////////////////////////////////////////////////////

// glfw to native input mapping

static eMouseButton GLFW_MouseButtonToNative(int mbutton)
{
    switch (mbutton)
    {
        case GLFW_MOUSE_BUTTON_LEFT: return eMouseButton_Left;
        case GLFW_MOUSE_BUTTON_RIGHT: return eMouseButton_Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return eMouseButton_Middle;
    }
    return eMouseButton_null;
}

static eKeycode GLFW_KeycodeToNative(int keycode)
{
    switch (keycode)
    {
        case GLFW_KEY_ESCAPE: return eKeycode_ESCAPE;
        case GLFW_KEY_SPACE: return eKeycode_SPACE;
        case GLFW_KEY_PAGE_UP: return eKeycode_PAGE_UP;
        case GLFW_KEY_PAGE_DOWN: return eKeycode_PAGE_DOWN;
        case GLFW_KEY_HOME: return eKeycode_HOME;
        case GLFW_KEY_END: return eKeycode_END;
        case GLFW_KEY_INSERT: return eKeycode_INSERT;
        case GLFW_KEY_DELETE: return eKeycode_DELETE;
        case GLFW_KEY_RIGHT_CONTROL: return eKeycode_RIGHT_CTRL;
        case GLFW_KEY_LEFT_CONTROL: return eKeycode_LEFT_CTRL;
        case GLFW_KEY_BACKSPACE: return eKeycode_BACKSPACE;
        case GLFW_KEY_ENTER: return eKeycode_ENTER;
        case GLFW_KEY_TAB: return eKeycode_TAB;
        case GLFW_KEY_GRAVE_ACCENT: return eKeycode_TILDE;
        case GLFW_KEY_F1: return eKeycode_F1;
        case GLFW_KEY_F2: return eKeycode_F2;
        case GLFW_KEY_F3: return eKeycode_F3;
        case GLFW_KEY_F4: return eKeycode_F4;
        case GLFW_KEY_F5: return eKeycode_F5;
        case GLFW_KEY_F6: return eKeycode_F6;
        case GLFW_KEY_F7: return eKeycode_F7;
        case GLFW_KEY_F8: return eKeycode_F8;
        case GLFW_KEY_F9: return eKeycode_F9;
        case GLFW_KEY_F10: return eKeycode_F10;
        case GLFW_KEY_F11: return eKeycode_F11;
        case GLFW_KEY_F12: return eKeycode_F12;
        case GLFW_KEY_A: return eKeycode_A;
        case GLFW_KEY_C: return eKeycode_C;
        case GLFW_KEY_F: return eKeycode_F;
        case GLFW_KEY_V: return eKeycode_V;
        case GLFW_KEY_X: return eKeycode_X;
        case GLFW_KEY_W: return eKeycode_W;
        case GLFW_KEY_D: return eKeycode_D;
        case GLFW_KEY_S: return eKeycode_S;
        case GLFW_KEY_Y: return eKeycode_Y;
        case GLFW_KEY_Z: return eKeycode_Z;
        case GLFW_KEY_R: return eKeycode_R;
        case GLFW_KEY_0: return eKeycode_0;
        case GLFW_KEY_1: return eKeycode_1;
        case GLFW_KEY_2: return eKeycode_2;
        case GLFW_KEY_3: return eKeycode_3;
        case GLFW_KEY_4: return eKeycode_4;
        case GLFW_KEY_5: return eKeycode_5;
        case GLFW_KEY_6: return eKeycode_6;
        case GLFW_KEY_7: return eKeycode_7;
        case GLFW_KEY_8: return eKeycode_8;
        case GLFW_KEY_9: return eKeycode_9;
        case GLFW_KEY_LEFT: return eKeycode_LEFT;
        case GLFW_KEY_RIGHT: return eKeycode_RIGHT;
        case GLFW_KEY_UP: return eKeycode_UP;
        case GLFW_KEY_DOWN: return eKeycode_DOWN;
    }
    return eKeycode_null;
}

//////////////////////////////////////////////////////////////////////////

bool GraphicsDevice::Initialize(const Size2D& screenDimensions, bool fullscreen, bool vsync)
{
    ::glfwSetErrorCallback([](int errorCode, const char * errorString)
    {
        gConsole.LogMessage(eLogMessage_Error, "GLFW error occurred: %s", errorString);
    });

    gConsole.LogMessage(eLogMessage_Debug, "GraphicsDevice Initialization (%dx%d, Vsync: %s, Fullscreen: %s)",
        screenDimensions.x, screenDimensions.y, vsync ? "enabled" : "disabled", fullscreen ? "yes" : "no");

    if (::glfwInit() == GL_FALSE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "GLFW initialization failed");
        return false;
    }

    // dump some information
    gConsole.LogMessage(eLogMessage_Info, "GLFW version string: %s", ::glfwGetVersionString());
    gConsole.LogMessage(eLogMessage_Info, "OpenGL %d.%d %s",
        OPENGL_CONTEXT_MAJOR_VERSION, 
        OPENGL_CONTEXT_MINOR_VERSION,
#ifdef OPENGL_CORE_PROFILE
        "(Core profile)"
#else 
        ""
#endif   
        );

    GLFWmonitor* graphicsMonitor = nullptr;
    if (fullscreen)
    {
        graphicsMonitor = ::glfwGetPrimaryMonitor();
        debug_assert(graphicsMonitor);
    }

    // opengl params
#ifdef OPENGL_CORE_PROFILE
    ::glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_CONTEXT_MAJOR_VERSION);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_CONTEXT_MINOR_VERSION);
    // setup window params
    ::glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    ::glfwWindowHint(GLFW_RED_BITS, 8);
    ::glfwWindowHint(GLFW_GREEN_BITS, 8);
    ::glfwWindowHint(GLFW_BLUE_BITS, 8);
    ::glfwWindowHint(GLFW_ALPHA_BITS, 8);
    ::glfwWindowHint(GLFW_DEPTH_BITS, 16);

    // create window and set current context
    GLFWwindow* graphicsWindow = ::glfwCreateWindow(screenDimensions.x, screenDimensions.y, GAME_TITLE, graphicsMonitor, nullptr);
    debug_assert(graphicsWindow);
    if (graphicsWindow == nullptr)
    {
        gConsole.LogMessage(eLogMessage_Warning, "glfwCreateWindow failed");
        ::glfwTerminate();
        return false;
    }

    // setup current opengl context and register callback handlers
    ::glfwMakeContextCurrent(graphicsWindow);
    ::glfwSetMouseButtonCallback(graphicsWindow, [](GLFWwindow*, int button, int action, int mods)
    {
        if (action == GLFW_REPEAT)
            return;

        eMouseButton mbuttonNative = GLFW_MouseButtonToNative(button);
        if (mbuttonNative != eMouseButton_null)
        {
            MouseButtonInputEvent ev { mbuttonNative, mods, action == GLFW_PRESS };
            gInputsManager.HandleInputEvent(ev);
        }
    });
    ::glfwSetKeyCallback(graphicsWindow, [](GLFWwindow*, int keycode, int scancode, int action, int mods)
    {
        if (action == GLFW_REPEAT)
            return;

        eKeycode keycodeNative = GLFW_KeycodeToNative(keycode);
        if (keycodeNative != eKeycode_null)
        {
            int nativeMods = 0;
            if (mods & GLFW_MOD_SHIFT)
            {
                nativeMods |= KeyModifier_Shift;
            }
            if (mods & GLFW_MOD_CONTROL)
            {
                nativeMods |= KeyModifier_Ctrl;
            }
            if (mods & GLFW_MOD_ALT)
            {
                nativeMods |= KeyModifier_Alt;
            }

            KeyInputEvent ev { keycodeNative, scancode, nativeMods, action == GLFW_PRESS };
            gInputsManager.HandleInputEvent(ev);
        }
    });
    ::glfwSetCharCallback(graphicsWindow, [](GLFWwindow*, unsigned int unicodechar)
    {
        KeyCharEvent ev ( unicodechar );
        gInputsManager.HandleInputEvent(ev);
    });
    ::glfwSetScrollCallback(graphicsWindow, [](GLFWwindow*, double xscroll, double yscroll)
    {
        MouseScrollInputEvent ev 
        { 
            static_cast<int>(xscroll), 
            static_cast<int>(yscroll) 
        };
        gInputsManager.HandleInputEvent(ev);
    });
    ::glfwSetCursorPosCallback(graphicsWindow, [](GLFWwindow*, double xposition, double yposition)
    {
        MouseMovedInputEvent ev 
        { 
            static_cast<int>(xposition),
            static_cast<int>(yposition),
        };
        gInputsManager.HandleInputEvent(ev);
    });

    gGLFW_WindowHandle = graphicsWindow;
    gGLFW_MonitorHandle = graphicsMonitor;

    // setup opengl extensions
    if (!InitOpenGLExtensions())
    {
        Deinit();
        return false;
    }

    // clear opengl errors
    glClearError();

    QueryGraphicsDeviceCaps();

    // create primary vertex array object
    ::glGenVertexArrays(1, &mDeviceContext.mMainVaoHandle);
    glCheckError();

    ::glBindVertexArray(mDeviceContext.mMainVaoHandle);
    glCheckError();

    // scissor test always enabled
    ::glEnable(GL_SCISSOR_TEST);
    glCheckError();

    // setup viewport
    mViewportRect.Set(0, 0, screenDimensions.x, screenDimensions.y);

    ::glViewport(mViewportRect.mX, mViewportRect.mY, mViewportRect.mSizeX, mViewportRect.mSizeY);
    glCheckError();

    // default value for scissor is a whole viewport
    mScissorBox = mViewportRect;

    ::glScissor(mScissorBox.mX, mScissorBox.mY, mScissorBox.mSizeX, mScissorBox.mSizeY);
    glCheckError();

    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();

    // force clear screen at stratup
    ::glfwSwapBuffers(gGLFW_WindowHandle);
    glCheckError();

    // setup default render state
    static const RenderStates defaultRenderStates;
    InternalSetRenderStates(defaultRenderStates, true);

    EnableFullscreen(fullscreen);
    EnableVSync(vsync);

    return true;
}

void GraphicsDevice::Deinit()
{
    if (gGLFW_WindowHandle == nullptr)
        return;

    if (mDeviceContext.mMainVaoHandle)
    {
        // destroy primary vertex array object
        ::glDeleteVertexArrays(1, &mDeviceContext.mMainVaoHandle);
        glCheckError();
    }

    ::glfwDestroyWindow(gGLFW_WindowHandle);
    ::glfwTerminate();
    gGLFW_WindowHandle = nullptr;
    gGLFW_MonitorHandle = nullptr;
}

void GraphicsDevice::EnableVSync(bool vsyncEnabled)
{
    debug_assert(gGLFW_WindowHandle);
    ::glfwSwapInterval(vsyncEnabled ? 1 : 0);
}

void GraphicsDevice::EnableFullscreen(bool fullscreenEnabled)
{
    debug_assert(gGLFW_WindowHandle);
    if (gGLFW_MonitorHandle == nullptr && fullscreenEnabled)
    {
        gGLFW_MonitorHandle = ::glfwGetPrimaryMonitor();
        ::glfwSetWindowMonitor(gGLFW_WindowHandle, gGLFW_MonitorHandle, 0, 0, mViewportRect.mSizeX, mViewportRect.mSizeY, 0);
    }
    else
    {
        gGLFW_MonitorHandle = nullptr;
        ::glfwSetWindowMonitor(gGLFW_WindowHandle, gGLFW_MonitorHandle, 60, 60, mViewportRect.mSizeX, mViewportRect.mSizeY, 0);
    }
}

bool GraphicsDevice::InitOpenGLExtensions()
{
    // initialize opengl extensions
    ::glewExperimental = GL_TRUE;

    // initialize glew
    GLenum resultCode = ::glewInit();
    if (resultCode != GLEW_OK)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Could not initialize OpenGL extensions (%s)", ::glewGetErrorString(resultCode));
        return false;
    }

    if (!GLEW_VERSION_3_2)
    {
        gConsole.LogMessage(eLogMessage_Warning, "OpenGL 3.2 API is not available");
        return false;
    }

    // dump opengl information
    gConsole.LogMessage(eLogMessage_Info, "OpenGL Vendor: %s", ::glGetString(GL_VENDOR));
    gConsole.LogMessage(eLogMessage_Info, "OpenGL Renderer: %s", ::glGetString(GL_RENDERER));
    gConsole.LogMessage(eLogMessage_Info, "OpenGL Version: %s", ::glGetString(GL_VERSION));
    gConsole.LogMessage(eLogMessage_Info, "GLSL Version: %s", ::glGetString(GL_SHADING_LANGUAGE_VERSION));

#if 0
    // query extensions
    GLint glNumExtensions = 0;

    ::glGetIntegerv(GL_NUM_EXTENSIONS, &glNumExtensions);
    if (glNumExtensions > 0)
    {
        gConsole.LogMessage(eLogMessage_Info, "Supported OpenGL Extensions:");
        // enum all extensions
        for (GLint iextension = 0; iextension < glNumExtensions; ++iextension)
        {
            gConsole.LogMessage(eLogMessage_Info, "%s", ::glGetStringi(GL_EXTENSIONS, iextension));
        }
    } // if extensions
#endif

    return true;
}

void GraphicsDevice::QueryGraphicsDeviceCaps()
{
    mCaps.mFeatures[eGraphicsDeviceFeature_NPOT_Textures] = (GLEW_ARB_texture_non_power_of_two == GL_TRUE);
    mCaps.mFeatures[eGraphicsDeviceFeature_ABGR] = (GLEW_EXT_abgr == GL_TRUE);

    ::glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &mCaps.mMaxTextureBufferSize);
    glCheckError();

    ::glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &mCaps.mMaxArrayTextureLayers);
    glCheckError();

    gConsole.LogMessage(eLogMessage_Info, "Graphics Device caps:");
    gConsole.LogMessage(eLogMessage_Info, " - max array texture layers: %d", mCaps.mMaxArrayTextureLayers);
    gConsole.LogMessage(eLogMessage_Info, " - max texture buffer size: %d bytes", mCaps.mMaxTextureBufferSize);
}

void GraphicsDevice::InternalSetRenderStates(const RenderStates& renderStates, bool forceState)
{
    if (mCurrentStates == renderStates && !forceState)
        return;

    // polygon mode
    if (forceState || (mCurrentStates.mPolygonFillMode != renderStates.mPolygonFillMode))
    {
        GLenum mode = GL_FILL;
        switch (renderStates.mPolygonFillMode)
        {
            case ePolygonFillMode_WireFrame: mode = GL_LINE; break;
            case ePolygonFillMode_Solid: mode = GL_FILL; break;
            default:
                debug_assert(false);
            break;
        }
        ::glPolygonMode(GL_FRONT_AND_BACK, mode);
        glCheckError();
    }

    // depth testing
    if (forceState || mCurrentStates.mIsDepthTestEnabled != renderStates.mIsDepthTestEnabled)
    {
        if (renderStates.mIsDepthTestEnabled)
        {
            ::glEnable(GL_DEPTH_TEST);
        }
        else
        {
            ::glDisable(GL_DEPTH_TEST);
        }
        glCheckError();
    }

    // depth function
    if (forceState || (mCurrentStates.mDepthFunc != renderStates.mDepthFunc))
    {
        GLenum mode = GL_LEQUAL;
        switch (renderStates.mDepthFunc)
        {
            case eDepthTestFunc_NotEqual: mode = GL_NOTEQUAL; break;
            case eDepthTestFunc_Always: mode = GL_ALWAYS; break;
            case eDepthTestFunc_Equal: mode = GL_EQUAL; break;
            case eDepthTestFunc_Less: mode = GL_LESS; break;
            case eDepthTestFunc_Greater: mode = GL_GREATER; break;
            case eDepthTestFunc_LessEqual: mode = GL_LEQUAL; break;
            case eDepthTestFunc_GreaterEqual: mode = GL_GEQUAL; break;
            default:
                debug_assert(false);
            break;
        }
        ::glDepthFunc(mode);
        glCheckError();
    }

    if (forceState || mCurrentStates.mIsDepthWriteEnabled != renderStates.mIsDepthWriteEnabled)
    {
        ::glDepthMask(renderStates.mIsDepthWriteEnabled ? GL_TRUE : GL_FALSE);
        glCheckError();
    }

    if (forceState || mCurrentStates.mIsColorWriteEnabled != renderStates.mIsColorWriteEnabled)
    {
        const GLboolean isEnabled = renderStates.mIsColorWriteEnabled ? GL_TRUE : GL_FALSE;
        ::glColorMask(isEnabled, isEnabled, isEnabled, isEnabled);
        glCheckError();
    }

    // blending
    if (forceState || mCurrentStates.mIsAlphaBlendEnabled != renderStates.mIsAlphaBlendEnabled)
    {
        if (renderStates.mIsAlphaBlendEnabled)
        {
            ::glEnable(GL_BLEND);
        }
        else
        {
            ::glDisable(GL_BLEND);
        }
        glCheckError();
    }

    if (forceState || (mCurrentStates.mBlendingMode != renderStates.mBlendingMode))
    {
        GLenum srcFactor = GL_ZERO;
        GLenum dstFactor = GL_ZERO;

        switch (renderStates.mBlendingMode)
        {
            case eBlendingMode_Alpha:
                srcFactor = GL_SRC_ALPHA;
                dstFactor = GL_ONE_MINUS_SRC_ALPHA;
            break;
            case eBlendingMode_Additive:
                srcFactor = GL_SRC_ALPHA;
                dstFactor = GL_ONE;
            break;
            case eBlendingMode_Multiply:
                srcFactor = GL_DST_COLOR;
                dstFactor = GL_ZERO;
            break;
            case eBlendingMode_Premultiplied:
                srcFactor = GL_ONE;
                dstFactor = GL_ONE_MINUS_SRC_ALPHA;
            break;
            case eBlendingMode_Screen:
                srcFactor = GL_ONE_MINUS_DST_COLOR;
                dstFactor = GL_ONE;
            break;
            default:
                debug_assert(false);
            break;
        }

        ::glBlendFunc(srcFactor, dstFactor);
        glCheckError();
    }

    // culling
    if (forceState || mCurrentStates.mIsFaceCullingEnabled != renderStates.mIsFaceCullingEnabled)
    {
        if (renderStates.mIsFaceCullingEnabled)
        {
            ::glEnable(GL_CULL_FACE);
        }
        else
        {
            ::glDisable(GL_CULL_FACE);
        }
        glCheckError();
    }

    if (forceState || (mCurrentStates.mCullingMode != renderStates.mCullingMode))
    {
        GLenum mode = GL_BACK;
        switch (renderStates.mCullingMode)
        {
            case eCullingMode_Back: mode = GL_BACK; break;
            case eCullingMode_Front: mode = GL_FRONT; break;
            case eCullingMode_FrontAndBack: mode = GL_FRONT_AND_BACK; break;
            default:
                debug_assert(false);
            break;
        }
        ::glCullFace(mode);
        glCheckError();
    }

    mCurrentStates = renderStates;
}

void GraphicsDevice::ProcessInputEvents()
{
    debug_assert(gGLFW_WindowHandle);
    // process window messages
    ::glfwPollEvents();
    if (::glfwWindowShouldClose(gGLFW_WindowHandle) == GL_TRUE)
    {
        gSystem.QuitRequest();
        return;
    }
}

void GraphicsDevice::Present()
{
    debug_assert(gGLFW_WindowHandle);
    ::glfwSwapBuffers(gGLFW_WindowHandle);
}

void GraphicsDevice::SetViewportRect(const Rect2D& sourceRectangle)
{
    debug_assert(gGLFW_WindowHandle);
    if (mViewportRect == sourceRectangle)
        return;

    mViewportRect = sourceRectangle;
    ::glViewport(mViewportRect.mX, mViewportRect.mY, mViewportRect.mSizeX, mViewportRect.mSizeY);
    glCheckError();
}

void GraphicsDevice::SetScissorRect(const Rect2D& sourceRectangle)
{
    debug_assert(gGLFW_WindowHandle);
    if (mScissorBox == sourceRectangle)
        return;

    mScissorBox = sourceRectangle;
    ::glScissor(mScissorBox.mX, mScissorBox.mY, mScissorBox.mSizeX, mScissorBox.mSizeY);
    glCheckError();
}

void GraphicsDevice::SetClearColor(Color32 clearColor)
{
    debug_assert(gGLFW_WindowHandle);

    const float inv = 1.0f / 255.0f;
    ::glClearColor(clearColor.mR * inv, clearColor.mG * inv, clearColor.mB * inv, clearColor.mA * inv);
    glCheckError();
}

void GraphicsDevice::ClearScreen()
{
    debug_assert(gGLFW_WindowHandle);

    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();
}

GpuBufferTexture* GraphicsDevice::CreateBufferTexture()
{
    debug_assert(gGLFW_WindowHandle);

    GpuBufferTexture* texture = new GpuBufferTexture(mDeviceContext);
    return texture;
}

GpuBufferTexture* GraphicsDevice::CreateBufferTexture(eTextureFormat textureFormat, int dataLength, const void* sourceData)
{
    debug_assert(gGLFW_WindowHandle);

    GpuBufferTexture* texture = new GpuBufferTexture(mDeviceContext);
    if (!texture->Setup(textureFormat, dataLength, sourceData))
    {
        DestroyTexture(texture);
        return nullptr;
    }
    return texture;
}

GpuTexture2D* GraphicsDevice::CreateTexture2D()
{
    debug_assert(gGLFW_WindowHandle);

    GpuTexture2D* texture = new GpuTexture2D(mDeviceContext);
    return texture;
}

GpuTextureArray2D* GraphicsDevice::CreateTextureArray2D()
{
    debug_assert(gGLFW_WindowHandle);

    GpuTextureArray2D* texture = new GpuTextureArray2D(mDeviceContext);
    return texture;   
}

GpuTextureArray2D* GraphicsDevice::CreateTextureArray2D(eTextureFormat textureFormat, const Size2D& dimensions, int layersCount, const void* sourceData)
{
    debug_assert(gGLFW_WindowHandle);

    GpuTextureArray2D* texture = new GpuTextureArray2D(mDeviceContext);
    if (!texture->Setup(textureFormat, dimensions, layersCount, sourceData))
    {
        DestroyTexture(texture);
        return nullptr;
    }
    return texture;
}

GpuProgram* GraphicsDevice::CreateRenderProgram()
{
    debug_assert(gGLFW_WindowHandle);

    GpuProgram* program = new GpuProgram(mDeviceContext);
    return program;
}

GpuProgram* GraphicsDevice::CreateRenderProgram(const char* shaderSource)
{
    debug_assert(gGLFW_WindowHandle);

    GpuProgram* program = new GpuProgram(mDeviceContext);
    if (!program->CompileSourceCode(shaderSource))
    {
        DestroyProgram(program);
        return nullptr;
    }
    return program;
}

GpuBuffer* GraphicsDevice::CreateBuffer(eBufferContent bufferContent)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(bufferContent < eBufferContent_COUNT);
    GpuBuffer* bufferObject = new GpuBuffer(mDeviceContext, bufferContent);
    return bufferObject;
}

GpuBuffer* GraphicsDevice::CreateBuffer(eBufferContent bufferContent, eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(bufferContent < eBufferContent_COUNT);
    GpuBuffer* bufferObject = new GpuBuffer(mDeviceContext, bufferContent);
    if (!bufferObject->Setup(bufferUsage, bufferLength, dataBuffer))
    {
        DestroyBuffer(bufferObject);
        return nullptr;
    }
    return bufferObject;
}

void GraphicsDevice::BindVertexBuffer(GpuBuffer* sourceBuffer, const VertexFormat& streamDefinition)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(mDeviceContext.mCurrentProgram);
    if (sourceBuffer)
    {
        debug_assert(sourceBuffer->mContent == eBufferContent_Vertices);
    }

    if (mDeviceContext.mCurrentBuffers[eBufferContent_Vertices] != sourceBuffer)
    {
        GLenum bufferTargetGL = EnumToGL(eBufferContent_Vertices);
        mDeviceContext.mCurrentBuffers[eBufferContent_Vertices] = sourceBuffer;
        ::glBindBuffer(bufferTargetGL, sourceBuffer ? sourceBuffer->mResourceHandle : 0);
        glCheckError();
    }

    if (sourceBuffer)
    {
        SetupVertexAttributes(streamDefinition);
    }
}

void GraphicsDevice::BindIndexBuffer(GpuBuffer* sourceBuffer)
{
    debug_assert(gGLFW_WindowHandle);
    if (sourceBuffer)
    {
        debug_assert(sourceBuffer->mContent == eBufferContent_Indices);
    }

    if (mDeviceContext.mCurrentBuffers[eBufferContent_Indices] == sourceBuffer)
        return;

    mDeviceContext.mCurrentBuffers[eBufferContent_Indices] = sourceBuffer;
    GLenum bufferTargetGL = EnumToGL(eBufferContent_Indices);
    ::glBindBuffer(bufferTargetGL, sourceBuffer ? sourceBuffer->mResourceHandle : 0);
    glCheckError();
}

void GraphicsDevice::BindTexture(eTextureUnit textureUnit, GpuBufferTexture* texture)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(textureUnit < eTextureUnit_COUNT);
    if (mDeviceContext.mCurrentTextures[textureUnit].mBufferTexture == texture)
        return;

    ActivateTextureUnit(textureUnit);

    mDeviceContext.mCurrentTextures[textureUnit].mBufferTexture = texture;
    ::glBindTexture(GL_TEXTURE_BUFFER, texture ? texture->mResourceHandle : 0);
    glCheckError();
}

void GraphicsDevice::BindTexture(eTextureUnit textureUnit, GpuTexture2D* texture)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(textureUnit < eTextureUnit_COUNT);
    if (mDeviceContext.mCurrentTextures[textureUnit].mTexture2D == texture)
        return;

    ActivateTextureUnit(textureUnit);

    mDeviceContext.mCurrentTextures[textureUnit].mTexture2D = texture;
    ::glBindTexture(GL_TEXTURE_2D, texture ? texture->mResourceHandle : 0);
    glCheckError();
}

void GraphicsDevice::BindTexture(eTextureUnit textureUnit, GpuTextureArray2D* texture)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(textureUnit < eTextureUnit_COUNT);
    if (mDeviceContext.mCurrentTextures[textureUnit].mTextureArray2D == texture)
        return;

    ActivateTextureUnit(textureUnit);

    mDeviceContext.mCurrentTextures[textureUnit].mTextureArray2D = texture;
    ::glBindTexture(GL_TEXTURE_2D_ARRAY, texture ? texture->mResourceHandle : 0);
    glCheckError();
}

void GraphicsDevice::BindRenderProgram(GpuProgram* program)
{
    debug_assert(gGLFW_WindowHandle);
    if (mDeviceContext.mCurrentProgram == program)
        return;

    ::glUseProgram(program ? program->mResourceHandle : 0);
    glCheckError();
    if (program)
    {
        bool programAttributes[eVertexAttribute_MAX] = {};
        for (int streamIndex = 0; streamIndex < eVertexAttribute_MAX; ++streamIndex)
        {
            if (program->mAttributes[streamIndex] == GpuVariable_NULL)
                continue;

            programAttributes[program->mAttributes[streamIndex]] = true;
        }

        // setup attribute streams
        for (int ivattribute = 0; ivattribute < eVertexAttribute_COUNT; ++ivattribute)
        {
            if (programAttributes[ivattribute])
            {
                ::glEnableVertexAttribArray(ivattribute);
                glCheckError();
            }
            else
            {
                ::glDisableVertexAttribArray(ivattribute);
                glCheckError();
            }
        }
    }
    else
    {
        for (int ivattribute = 0; ivattribute < eVertexAttribute_MAX; ++ivattribute)
        {
            ::glDisableVertexAttribArray(ivattribute);
            glCheckError();
        }
    }
    mDeviceContext.mCurrentProgram = program;
}

void GraphicsDevice::DestroyTexture(GpuBufferTexture* textureResource)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(textureResource);
    delete textureResource;
}

void GraphicsDevice::DestroyTexture(GpuTexture2D* textureResource)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(textureResource);
    delete textureResource;
}

void GraphicsDevice::DestroyTexture(GpuTextureArray2D* textureResource)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(textureResource);
    delete textureResource;
}

void GraphicsDevice::DestroyProgram(GpuProgram* programResource)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(programResource);
    delete programResource;
}

void GraphicsDevice::DestroyBuffer(GpuBuffer* bufferResource)
{
    debug_assert(gGLFW_WindowHandle);
    debug_assert(bufferResource);
    delete bufferResource;
}

void GraphicsDevice::RenderIndexedPrimitives(ePrimitiveType primitive, eIndicesType indices, unsigned int offset, unsigned int numIndices)
{
    debug_assert(gGLFW_WindowHandle);

    GpuBuffer* indexBuffer = mDeviceContext.mCurrentBuffers[eBufferContent_Indices];
    GpuBuffer* vertexBuffer = mDeviceContext.mCurrentBuffers[eBufferContent_Vertices];
    debug_assert(indexBuffer && vertexBuffer && mDeviceContext.mCurrentProgram);

    GLenum primitives = EnumToGL(primitive);
    GLenum indicesTypeGL = EnumToGL(indices);
    ::glDrawElements(primitives, numIndices, indicesTypeGL, BUFFER_OFFSET(offset));
    glCheckError();
}

void GraphicsDevice::RenderIndexedPrimitives(ePrimitiveType primitive, eIndicesType indices, unsigned int offset, unsigned int numIndices, unsigned int baseVertex)
{
    debug_assert(gGLFW_WindowHandle);

    GpuBuffer* indexBuffer = mDeviceContext.mCurrentBuffers[eBufferContent_Indices];
    GpuBuffer* vertexBuffer = mDeviceContext.mCurrentBuffers[eBufferContent_Vertices];
    debug_assert(indexBuffer && vertexBuffer && mDeviceContext.mCurrentProgram);

    GLenum primitives = EnumToGL(primitive);
    GLenum indicesTypeGL = EnumToGL(indices);
    ::glDrawElementsBaseVertex(primitives, numIndices, indicesTypeGL, BUFFER_OFFSET(offset), baseVertex);
    glCheckError();
}

void GraphicsDevice::RenderPrimitives(ePrimitiveType primitiveType, unsigned int firstIndex, unsigned int numElements)
{
    debug_assert(gGLFW_WindowHandle);

    GpuBuffer* vertexBuffer = mDeviceContext.mCurrentBuffers[eBufferContent_Vertices];
    debug_assert(vertexBuffer && mDeviceContext.mCurrentProgram);

    GLenum primitives = EnumToGL(primitiveType);
    ::glDrawArrays(primitives, firstIndex, numElements);
    glCheckError();
}

void GraphicsDevice::ActivateTextureUnit(eTextureUnit textureUnit)
{
    if (mDeviceContext.mCurrentTextureUnit == textureUnit)
        return;

    mDeviceContext.mCurrentTextureUnit = textureUnit;

    ::glActiveTexture(GL_TEXTURE0 + textureUnit);
    glCheckError();
}

void GraphicsDevice::SetupVertexAttributes(const VertexFormat& streamDefinition)
{
    GpuProgram* currentProgram = mDeviceContext.mCurrentProgram;
    for (int iattribute = 0; iattribute < eVertexAttribute_COUNT; ++iattribute)
    {
        if (currentProgram->mAttributes[iattribute] == GpuVariable_NULL)
        {
            // current vertex attribute is unused in shader
            continue;
        }

        const auto& attribute = streamDefinition.mAttributes[iattribute];
        if (attribute.mFormat == eVertexAttributeFormat_Unknown)
        {
            debug_assert(false);
            continue;
        }

        unsigned int numComponents = GetAttributeComponentCount(attribute.mFormat);
        if (numComponents == 0)
        {
            debug_assert(numComponents > 0);
            continue;
        }

        GLenum dataType = GetAttributeDataTypeGL(attribute.mFormat);
        // set attribute location
        ::glVertexAttribPointer(currentProgram->mAttributes[iattribute], numComponents, dataType, 
            attribute.mNormalized ? GL_TRUE : GL_FALSE, 
            streamDefinition.mDataStride, BUFFER_OFFSET(attribute.mDataOffset + streamDefinition.mBaseOffset));
        glCheckError();
    }
}