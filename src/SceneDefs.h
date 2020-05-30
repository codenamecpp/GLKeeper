#pragma once

// Default water surface parameters
#define DEFAULT_WATER_LEVEL         0.92f
#define DEFAULT_WATER_TRANSLUCENCY  0.65f
#define DEFAULT_WATER_WAVE_WIDTH    0.0f
#define DEFAULT_WATER_WAVE_HEIGHT   0.0f
#define DEFAULT_WATER_WAVE_FREQ     2.4f

// Default lava surface parameters
#define DEFAULT_LAVA_LEVEL          0.82f
#define DEFAULT_LAVA_TRANSLUCENCY   1.0f
#define DEFAULT_LAVA_WAVE_WIDTH     0.76f
#define DEFAULT_LAVA_WAVE_HEIGHT    0.15f
#define DEFAULT_LAVA_WAVE_FREQ      2.5f

// forwards
class SceneCamera;
class RenderScene;
class CameraController;
class SceneRenderList;
class SceneRenderContext;

// camera mode
enum eSceneCameraMode
{
    eSceneCameraMode_Perspective,
    eSceneCameraMode_Orthographic,
};

decl_enum_strings(eSceneCameraMode);

// get scene global space right direction vector
const glm::vec3 SceneAxisX (1.0f, 0.0f, 0.0f);

// get scene global space up direction vector
const glm::vec3 SceneAxisY (0.0f, 1.0f, 0.0f);

// get scene global space forward direction vector
const glm::vec3 SceneAxisZ (0.0f, 0.0f, 1.0f);

const glm::mat4 SceneIdentyMatrix (1.0f);

// vertex animation state
struct BlendFramesAnimState
{
public:
    // current animation params and state
    int mStartFrame = 0;
    int mFinalFrame = 0;

    int mFrame0 = 0; // base blend frame
    int mFrame1 = 0; // next blend frame

    int mCyclesCount = 0; // number of full animation cycles

    float mMixFrames = 0.0f; // blend frames value in range [0..1]
    float mAnimationTime = 0.0f; // current time progress
    float mAnimationEndTime = 0.0f;
    float mFramesPerSecond = 0.0f;  // speed

    bool mIsAnimationActive = false;
    bool mIsAnimationLoop = false;
    bool mIsAnimationPaused = false;
};