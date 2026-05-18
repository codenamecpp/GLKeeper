#pragma once

//////////////////////////////////////////////////////////////////////////

#define DEFEULT_CAMERA_FOVY             45.0f
#define KEEPER_CAMERA_FOVY              60.0f
#define DEFAULT_CAMERA_NEAR_DISTANCE    0.5f
#define DEFAULT_CAMERA_FAR_DISTANCE     50.0f
#define DEFAULT_ANIMATION_FPS           30

//////////////////////////////////////////////////////////////////////////

class Camera;
class GameWorld;

//////////////////////////////////////////////////////////////////////////

namespace WorldAxes
{
    static const glm::vec3 X {1.0f, 0.0f, 0.0f};
    static const glm::vec3 Y {0.0f, 1.0f, 0.0f};
    static const glm::vec3 Z {0.0f, 0.0f, 1.0f};
};

//////////////////////////////////////////////////////////////////////////

struct WorldStatistics
{
public:
    WorldStatistics() = default;
    inline void Clear()
    {
        mNumSceneObjectsActive = 0;
    }
public:
    int mNumSceneObjectsActive = 0;
};

