#pragma once

#include "ConsoleDefs.h"

// forwards
class SceneObject;
class SceneCamera;
class GameScene;
class SceneCameraControl;

enum eSceneCameraMode
{
    eSceneCameraMode_Perspective,
    eSceneCameraMode_Orthographic,
};

decl_enum_strings(eSceneCameraMode);


inline const glm::vec3& GetSceneAxis_X()
{
    static const glm::vec3 axis_X {1.0f, 0.0f, 0.0f};
    return axis_X;
}

inline const glm::vec3& GetSceneAxis_Y()
{
    static const glm::vec3 axis_Y {0.0f, 1.0f, 0.0f};
    return axis_Y;
}

inline const glm::vec3& GetSceneAxis_Z()
{
    static const glm::vec3 axis_Z {0.0f, 0.0f, 1.0f};
    return axis_Z;
}

// scene cvars

extern CvarBoolean gCvarScene_DebugDrawAabb;
