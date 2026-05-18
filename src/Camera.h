#pragma once

#include "frustum.h"
#include "GameWorldDefs.h"

//////////////////////////////////////////////////////////////////////////
// 3D Perspective camera object
//////////////////////////////////////////////////////////////////////////

class Camera
{
public:

    //////////////////////////////////////////////////////////////////////////
    // Perspective projection params for camera
    struct ProjectionParams
    {
    public:
        ProjectionParams() = default;
        ProjectionParams(float nearDistance, float farDistance, float fovy)
            : mNearDistance(nearDistance)
            , mFarDistance(farDistance)
            , mFovy(fovy)
        {
        }
    public:
        float mNearDistance = 1.0f;
        float mFarDistance = 1.0f;
        float mFovy = 90.0f;
    };

    //////////////////////////////////////////////////////////////////////////

    // public for convenience, should not be modified directly
    ProjectionParams mProjectionParams;
    glm::vec3 mPosition;
    glm::vec3 mRight, mUp, mForward;

    // before reading those matrices make sure to ComputeMatricesAndFrustum
    // should not be modified directly
    glm::mat4 mProjectionMatrix;
    glm::mat4 mViewMatrix, mViewProjectionMatrix;
    cxx::frustum_t mFrustum;

    RenderLayerMask mRenderLayersMask = (RenderLayer_WorldObjects | RenderLayer_WorldTerrain);

public:
    Camera();

    // Initialize camera projection matrix
    void SetupProjection(const ProjectionParams& desc);

    // Set camera position
    void SetPosition(const glm::vec3& position);

    // Set camera rotation
    void SetRotation(const glm::vec3& rotationAngles);

    // Set orientation vectors
    void LookAt(const glm::vec3& targetPosition, const glm::vec3& up);

    // Look straight forward
    void ResetOrientation();

    // Move camera position
    void Translate(const glm::vec3& direction);
       
    // Refresh view and projection matrices along with camera frustum
    // Will not do any unnecessary calculations if nothing changed
    void ComputeMatricesAndFrustum(const Viewport& viewport);

private:
    bool mProjMatrixDirty; // projection matrix need recomputation
    bool mViewMatrixDirty; // view matrix need recomputation
    Point2D mViewportSizeCache {}; // previous viewport used for matrices and fructum computation
};