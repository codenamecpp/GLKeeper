#pragma once

#include "SceneDefs.h"
#include "CommonTypes.h"

// defines camera in 3d space
class SceneCamera
{
public:
    // readonly
    glm::vec3 mPosition;
    glm::vec3 mFrontDirection;
    glm::vec3 mUpDirection;
    glm::vec3 mRightDirection;

    // make sure to ComputeMatricesAndFrustum before reading those
    cxx::frustum_t mFrustum;
    glm::mat4 mViewMatrix; 
    glm::mat4 mViewProjectionMatrix;
    glm::mat4 mProjectionMatrix;

    eSceneCameraMode mCurrentMode;

public:
    SceneCamera();

    // setup perspective projection camera mode
    // @param aspect: Screen aspect ratio
    // @param fovy: Field of view in degrees
    // @param nearPlane: Near clipping plane distance
    // @param farPlane: Far clipping plane distance
    void SetPerspective(float aspect, float fovy, float nearPlane, float farPlane);

    // setup orthographic projection camera mode
    // @param leftp, rightp, bottomp, topp
    void SetOrthographic(float leftp, float rightp, float bottomp, float topp);

    // set matrices and frustum dirty bits
    void InvalidateTransformation();

    // refresh view and projection matrices along with camera frustum
    // Will not do any unnecessary calculations if nothing changed
    void ComputeMatrices();

    // reset camera to initial state
    void SetIdentity();

    // reset camera orientation to initial state
    void SetIdentityOrientation();

    // set camera orientation vectors
    // @param dirForward, dirRight, dirUp: Axes, should be normalized 
    void SetOrientation(const glm::vec3& dirForward, const glm::vec3& dirRight, const glm::vec3& dirUp);

    // setup camera orientation, look at specified point
    // @param point: Point world
    // @param upward: Up vector, should be normalized
    void FocusAt(const glm::vec3& point, const glm::vec3& upward);

    // set camera position
    // @param position: Camera new position
    void SetPosition(const glm::vec3& position);

    // set camera rotation
    // @param rotationAngles: Angles in degrees
    void SetRotationAngles(const glm::vec3& rotationAngles);

    // move camera position
    // @param direction: Move direction
    void Translate(const glm::vec3& direction);

    // cast ray in specific viewport coordinate, make sure to ComputeMatricesAndFrustum
    // @param screenCoordinate: 2d coordinates on screen
    // @param screenViewport: viewport
    // @param resultRay: Output ray info
    bool CastRayFromScreenPoint(const glm::ivec2& screenCoordinate, const Rect2D& screenViewport, cxx::ray3d& resultRay);

private:
    bool mProjMatrixDirty; // projection matrix need recomputation
    bool mViewMatrixDirty; // view matrix need recomputation

    // projection parameters
    union CameraProjectionParams
    {
        struct // perspective
        {
            float mAspect;
            float mFovy;
            float mNearPlane;
            float mFarPlane;
        } mPerspective;

        struct // orthographic
        {
            float mLeftP;
            float mRightP;
            float mBottomP;
            float mTopP;
        } mOrthographic;
    };
    CameraProjectionParams mProjectionParams;
};