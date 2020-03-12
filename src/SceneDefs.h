#pragma once

// forwards
class Renderable;
class SceneCamera;
class RenderScene;
class SceneCameraControl;
class AnimatingModel;
class SceneRenderList;

enum eSceneCameraMode
{
    eSceneCameraMode_Perspective,
    eSceneCameraMode_Orthographic,
};

decl_enum_strings(eSceneCameraMode);

// get scene global space right direction vector
inline const glm::vec3& SceneAxis_X()
{
    static const glm::vec3 axis_X {1.0f, 0.0f, 0.0f};
    return axis_X;
}

// get scene global space up direction vector
inline const glm::vec3& SceneAxis_Y()
{
    static const glm::vec3 axis_Y {0.0f, 1.0f, 0.0f};
    return axis_Y;
}

// get scene global space forward direction vector
inline const glm::vec3& SceneAxis_Z()
{
    static const glm::vec3 axis_Z {0.0f, 0.0f, 1.0f};
    return axis_Z;
}
