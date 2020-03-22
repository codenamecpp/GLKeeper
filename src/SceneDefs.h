#pragma once

// forwards
class SceneObject;
class SceneCamera;
class RenderScene;
class SceneCameraControl;
class AnimatingModel;
class TerrainMesh;
class WaterLavaMesh;
class SceneRenderList;

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