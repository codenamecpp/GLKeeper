#include "stdafx.h"
#include "WorldViewCameraController.h"
#include "Camera.h"
#include "GameWorld.h"

//////////////////////////////////////////////////////////////////////////

#define CAM_ANGLE -53.0f // -55
#define CAM_MOTION_SPEED 12.0f
#define CAM_ROTATION_SPEED 120.0f
#define CAM_DISTANCE 1.0f
#define CAM_ACCELERATION_SPEED 80.0f
#define CAM_DECELERATION_SPEED 70.0f

//////////////////////////////////////////////////////////////////////////

WorldViewCameraController::WorldViewCameraController() 
    : mRotationAngles(CAM_ANGLE, 0.0f, 0.0f)
    , mStartPosition()
    , mIncreasingFov()
    , mDecreasingFov()
{
}

void WorldViewCameraController::CaptureCamera(Camera* camera)
{
    mCamera = camera;
    cxx_assert(camera);
    ResetCamera();
}

void WorldViewCameraController::ReleaseCamera()
{
    mCamera = nullptr;
}

void WorldViewCameraController::UpdateFrame(float deltaTime)
{
    if (mCamera == nullptr) return;

    // move
    {
        glm::vec3 inputDirection = GetMoveDirectionFromInputs();

        glm::vec3 targetVelocity = inputDirection * CAM_MOTION_SPEED;

        // accelerate
        if (glm::length2(targetVelocity) > 0.0f)
        {
            mMoveVelocity = cxx::move_towards(mMoveVelocity, targetVelocity, deltaTime * CAM_ACCELERATION_SPEED);
        }
        // decelerate
        else if (glm::length2(mMoveVelocity) > 0.0f)
        {
            mMoveVelocity = cxx::move_towards(mMoveVelocity, targetVelocity, deltaTime * CAM_DECELERATION_SPEED);
        }

        // update camera
        if (glm::length2(mMoveVelocity) > 0.0f)
        {
            mCamera->Translate(mMoveVelocity * deltaTime);
        }
    }

    glm::vec3 elevation {};

    if (mIncreasingFov)
    {
        Camera::ProjectionParams params(
            mCamera->mProjectionParams.mNearDistance, 
            mCamera->mProjectionParams.mFarDistance, 
            mCamera->mProjectionParams.mFovy + 5.0f * deltaTime);
        mCamera->SetupProjection(params);
    }

    if (mDecreasingFov)
    {
        Camera::ProjectionParams params(
            mCamera->mProjectionParams.mNearDistance, 
            mCamera->mProjectionParams.mFarDistance, 
            mCamera->mProjectionParams.mFovy - 5.0f * deltaTime);
        mCamera->SetupProjection(params);
    }

    // zoom
    float zoomDirection = GetZoomDirectionFromInputs();
    if (!cxx::eps_equals_zero(zoomDirection))
    {
        elevation = zoomDirection * mCamera->mForward * CAM_MOTION_SPEED * deltaTime;
    }

    // rotation
    float rotateDirection = GetRotateDirectionFromInputs();
    if (!cxx::eps_equals_zero(rotateDirection))
    {
        const glm::vec3 prevCameraPosition = mCamera->mPosition;
        float distanceFromTargetPoint = prevCameraPosition.y / sinf(glm::radians(90.0f));
        const glm::vec3 targetPosition = prevCameraPosition + mCamera->mForward * distanceFromTargetPoint;

        float rotatey = cxx::wrap_angle_to_180(mRotationAngles.y + (CAM_ROTATION_SPEED * deltaTime) * rotateDirection);
        mRotationAngles = {CAM_ANGLE, rotatey, 0.0f};

        mCamera->SetRotation(mRotationAngles);
        mCamera->SetPosition(targetPosition - mCamera->mForward * distanceFromTargetPoint);
    }

    // apply vectors
    mCamera->Translate(elevation);
}

void WorldViewCameraController::InputEvent(MouseButtonInputEvent& inputEvent)
{
}

void WorldViewCameraController::InputEvent(KeyInputEvent& inputEvent)
{
    if (mCamera == nullptr) return;

    if (inputEvent.mPressed && inputEvent.mKeycode == KEYCODE_SPACE)
    {
        ResetCamera();
    }

    if (inputEvent.mKeycode == KEYCODE_0)
    {
        mIncreasingFov = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == KEYCODE_9)
    {
        mDecreasingFov = inputEvent.mPressed;
    }
}

void WorldViewCameraController::InputEvent(MouseMovedInputEvent& inputEvent)
{
}

void WorldViewCameraController::InputEvent(MouseScrollInputEvent& inputEvent)
{
}

void WorldViewCameraController::SetStartPosition(const glm::vec3& position)
{
    mStartPosition = position;
}

glm::vec3 WorldViewCameraController::GetMoveDirectionFromInputs() const
{
    cxx_assert(mCamera);

    bool ctrlPressed = (gInputs.GetKeyState(KEYCODE_LEFT_CTRL) || gInputs.GetKeyState(KEYCODE_RIGHT_CTRL));
    if (ctrlPressed)
        return glm::vec3{0.0f};

    bool northPressed = gInputs.GetKeyState(KEYCODE_W) || gInputs.GetKeyState(KEYCODE_UP);
    bool southPressed = gInputs.GetKeyState(KEYCODE_S) || gInputs.GetKeyState(KEYCODE_DOWN);
    bool eastPressed  = gInputs.GetKeyState(KEYCODE_D) || gInputs.GetKeyState(KEYCODE_RIGHT);
    bool westPressed  = gInputs.GetKeyState(KEYCODE_A) || gInputs.GetKeyState(KEYCODE_LEFT);

    glm::vec3 moveDirection {0.0f};
    if (northPressed || southPressed || eastPressed || westPressed)
    {
        if (northPressed || southPressed)
        {
            const glm::vec3 vmove = glm::normalize(glm::cross(WorldAxes::Y, mCamera->mRight));
            if (southPressed)
            {
                moveDirection += vmove;
            }
            if (northPressed)
            {
                moveDirection -= vmove;
            }
        }

        if (eastPressed)
        {
            moveDirection -= mCamera->mRight;
        }
        if (westPressed)
        {
            moveDirection += mCamera->mRight;
        }
        // don't normalize : diagonal movement speedup is intent
        moveDirection = moveDirection;
    }
    return moveDirection;
}

float WorldViewCameraController::GetRotateDirectionFromInputs() const
{
    bool ctrlPressed = (gInputs.GetKeyState(KEYCODE_LEFT_CTRL) || gInputs.GetKeyState(KEYCODE_RIGHT_CTRL));
    if (ctrlPressed)
    {
        bool eastPressed = gInputs.GetKeyState(KEYCODE_D) || gInputs.GetKeyState(KEYCODE_RIGHT);
        bool westPressed = gInputs.GetKeyState(KEYCODE_A) || gInputs.GetKeyState(KEYCODE_LEFT);
        return (eastPressed ? -1.0f : 0.0f) + (westPressed ? 1.0f : 0.0f);
    }
    return 0.0f;
}

float WorldViewCameraController::GetZoomDirectionFromInputs()
{
    bool ctrlPressed = (gInputs.GetKeyState(KEYCODE_LEFT_CTRL) || gInputs.GetKeyState(KEYCODE_RIGHT_CTRL));
    if (ctrlPressed)
    {
        bool northPressed = gInputs.GetKeyState(KEYCODE_W) || gInputs.GetKeyState(KEYCODE_UP);
        bool southPressed = gInputs.GetKeyState(KEYCODE_S) || gInputs.GetKeyState(KEYCODE_DOWN);
        return (northPressed ? 1.0f : 0.0f) + (southPressed ? -1.0f : 0.0f);
    }
    else
    {
        bool zoomIn = gInputs.GetKeyState(KEYCODE_PAGE_UP);
        bool zoomOut = gInputs.GetKeyState(KEYCODE_PAGE_DOWN);
        return (zoomIn ? 1.0f : 0.0f) + (zoomOut ? -1.0f : 0.0f);
    }
    return 0.0f;
}

void WorldViewCameraController::ResetCamera()
{
    if (mCamera == nullptr) return;

    mRotationAngles = glm::vec3(CAM_ANGLE, -45.0f, 0.0f);

    Camera::ProjectionParams cameraParams (DEFAULT_CAMERA_NEAR_DISTANCE, DEFAULT_CAMERA_FAR_DISTANCE, KEEPER_CAMERA_FOVY);
    mCamera->SetupProjection(cameraParams);
    mCamera->SetRotation(mRotationAngles);

    float distanceFromTargetPoint = mStartPosition.y / tanf(glm::radians(CAM_ANGLE));
    const glm::vec3 worldForward = glm::normalize(glm::cross(WorldAxes::Y, mCamera->mRight));
    mCamera->SetPosition(mStartPosition - worldForward * distanceFromTargetPoint);

    StopCamera();
}

void WorldViewCameraController::StopCamera()
{
    mIncreasingFov = false;
    mDecreasingFov = false;

    mMoveVelocity = {};
}
