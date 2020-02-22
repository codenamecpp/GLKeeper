#include "pch.h"
#include "GraphicsDevice.h"
#include "Console.h"
#include "OpenGL_Defs.h"
#include "InputsManager.h"
#include "System.h"

GraphicsDevice gGraphicsDevice;

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

bool GraphicsDevice::Initialize(int screensizex, int screensizey, bool fullscreen, bool vsync)
{
    ::glfwSetErrorCallback([](int errorCode, const char * errorString)
    {
        gConsole.LogMessage(eLogMessage_Error, "GLFW error occurred: %s", errorString);
    });

    gConsole.LogMessage(eLogMessage_Debug, "GraphicsDevice Initialization (%dx%d, Vsync: %s, Fullscreen: %s)",
        screensizex, screensizey, vsync ? "enabled" : "disabled", fullscreen ? "yes" : "no");

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
    GLFWwindow* graphicsWindow = ::glfwCreateWindow(screensizex, screensizey, GAME_TITLE, graphicsMonitor, nullptr);
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
            KeyInputEvent ev { keycodeNative, scancode, mods, action == GLFW_PRESS };
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

    mWindowHandle = graphicsWindow;
    mMonitorHandle = graphicsMonitor;

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
    mViewportRect.Set(0, 0, screensizex, screensizey);

    ::glViewport(mViewportRect.mX, mViewportRect.mY, mViewportRect.mSizeX, mViewportRect.mSizeY);
    glCheckError();

    // default value for scissor is a whole viewport
    mScissorBox = mViewportRect;

    ::glScissor(mScissorBox.mX, mScissorBox.mY, mScissorBox.mSizeX, mScissorBox.mSizeY);
    glCheckError();

    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();

    // force clear screen at stratup
    ::glfwSwapBuffers(mWindowHandle);
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
    if (mWindowHandle == nullptr)
        return;

    if (mDeviceContext.mMainVaoHandle)
    {
        // destroy primary vertex array object
        ::glDeleteVertexArrays(1, &mDeviceContext.mMainVaoHandle);
        glCheckError();
    }

    ::glfwDestroyWindow(mWindowHandle);
    ::glfwTerminate();
    mWindowHandle = nullptr;
    mMonitorHandle = nullptr;
}

void GraphicsDevice::EnableVSync(bool vsyncEnabled)
{
    debug_assert(mWindowHandle);
    ::glfwSwapInterval(vsyncEnabled ? 1 : 0);
}

void GraphicsDevice::EnableFullscreen(bool fullscreenEnabled)
{
    debug_assert(mWindowHandle);
    if (mMonitorHandle == nullptr && fullscreenEnabled)
    {
        mMonitorHandle = ::glfwGetPrimaryMonitor();
        ::glfwSetWindowMonitor(mWindowHandle, mMonitorHandle, 0, 0, mViewportRect.mSizeX, mViewportRect.mSizeY, 0);
    }
    else
    {
        mMonitorHandle = nullptr;
        ::glfwSetWindowMonitor(mWindowHandle, mMonitorHandle, 60, 60, mViewportRect.mSizeX, mViewportRect.mSizeY, 0);
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
    if (forceState || (mCurrentStates.mFillMode != renderStates.mFillMode))
    {
        GLenum mode = GL_FILL;
        switch (renderStates.mFillMode)
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
    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_DepthTest))
    {
        if (renderStates.IsEnabled(RenderStateFlags_DepthTest))
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

    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_DepthWrite))
    {
        ::glDepthMask(renderStates.IsEnabled(RenderStateFlags_DepthWrite) ? GL_TRUE : GL_FALSE);
        glCheckError();
    }

    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_ColorWrite))
    {
        const GLboolean isEnabled = renderStates.IsEnabled(RenderStateFlags_ColorWrite) ? GL_TRUE : GL_FALSE;
        ::glColorMask(isEnabled, isEnabled, isEnabled, isEnabled);
        glCheckError();
    }

    // blending
    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_AlphaBlend))
    {
        if (renderStates.IsEnabled(RenderStateFlags_AlphaBlend))
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
    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_FaceCulling))
    {
        if (renderStates.IsEnabled(RenderStateFlags_FaceCulling))
        {
            ::glEnable(GL_CULL_FACE);
        }
        else
        {
            ::glDisable(GL_CULL_FACE);
        }
        glCheckError();
    }

    if (forceState || (mCurrentStates.mCullMode != renderStates.mCullMode))
    {
        GLenum mode = GL_BACK;
        switch (renderStates.mCullMode)
        {
            case eCullMode_Back: mode = GL_BACK; break;
            case eCullMode_Front: mode = GL_FRONT; break;
            case eCullMode_FrontAndBack: mode = GL_FRONT_AND_BACK; break;
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
    debug_assert(mWindowHandle);
    // process window messages
    ::glfwPollEvents();
    if (::glfwWindowShouldClose(mWindowHandle) == GL_TRUE)
    {
        gSystem.QuitRequest();
        return;
    }
}

void GraphicsDevice::Present()
{
    debug_assert(mWindowHandle);
    ::glfwSwapBuffers(mWindowHandle);
}

void GraphicsDevice::SetViewportRect(const Rect2D& sourceRectangle)
{
    debug_assert(mWindowHandle);
    if (mViewportRect == sourceRectangle)
        return;

    mViewportRect = sourceRectangle;
    ::glViewport(mViewportRect.mX, mViewportRect.mY, mViewportRect.mSizeX, mViewportRect.mSizeY);
    glCheckError();
}

void GraphicsDevice::SetScissorRect(const Rect2D& sourceRectangle)
{
    debug_assert(mWindowHandle);
    if (mScissorBox == sourceRectangle)
        return;

    mScissorBox = sourceRectangle;
    ::glScissor(mScissorBox.mX, mScissorBox.mY, mScissorBox.mSizeX, mScissorBox.mSizeY);
    glCheckError();
}

void GraphicsDevice::SetClearColor(Color32 clearColor)
{
    debug_assert(mWindowHandle);

    const float inv = 1.0f / 255.0f;
    ::glClearColor(clearColor.mR * inv, clearColor.mG * inv, clearColor.mB * inv, clearColor.mA * inv);
    glCheckError();
}

void GraphicsDevice::ClearScreen()
{
    debug_assert(mWindowHandle);

    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();
}