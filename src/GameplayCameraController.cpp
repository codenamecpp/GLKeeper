#include "stdafx.h"
#include "GameplayCameraController.h"
#include "Camera.h"
#include "GameWorld.h"
#include "UiManager.h"
#include "MapUtils.h"

//////////////////////////////////////////////////////////////////////////

// default camera params
#define CAM_ANGLE               -53.0f // -55
#define CAM_MOTION_SPEED        12.0f
#define CAM_ROTATION_SPEED      120.0f
#define CAM_MIN_HEIGHT          3.0f
#define CAM_MAX_HEIGHT          16.0f
#define CAM_HEIGHT              7.0f
#define CAM_ACCELERATION_SPEED  100.0f
#define CAM_DECELERATION_SPEED  90.0f

//////////////////////////////////////////////////////////////////////////

GameplayCameraController::GameplayCameraController() 
    : mRotationAngles(CAM_ANGLE, 0.0f, 0.0f)
    , mStartPosition()
    , mIncreasingFov()
    , mDecreasingFov()
    , mCameraHeight(CAM_HEIGHT)
{
}

void GameplayCameraController::CaptureCamera(Camera* camera)
{
    mCamera = camera;
    cxx_assert(camera);
    ResetCamera();
}

void GameplayCameraController::ReleaseCamera()
{
    mCamera = nullptr;
}

void GameplayCameraController::UpdateFrame(float deltaTime)
{
    if (mCamera == nullptr) 
        return;

    bool cameraPositionChanged = false;
    bool cameraHeightChanged = false;
    bool cameraRotationChanged = false;

    // move
    {
        glm::vec2 inputDirection = GetCameraMoveVectorFromInputs();

        glm::vec2 targetVelocity = inputDirection * CAM_MOTION_SPEED;

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
            mCurrentPosition += mMoveVelocity * deltaTime;
            ClampWithinBounds(mCurrentPosition);
            cameraPositionChanged = true;
        }
    }

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
        mCameraHeight -= zoomDirection * CAM_MOTION_SPEED * deltaTime;
        ClampCameraHeight(mCameraHeight);
        cameraHeightChanged = true;
    }

    // rotation
    float rotateDirection = GetRotateDirectionFromInputs();
    if (!cxx::eps_equals_zero(rotateDirection))
    {
        mRotationAngles.y = cxx::wrap_angle_to_180(mRotationAngles.y + (CAM_ROTATION_SPEED * deltaTime) * rotateDirection);
        cameraRotationChanged = true;
    }

    if (cameraPositionChanged || cameraHeightChanged || cameraRotationChanged)
    {
        // stop auto scroll
        mAutoScroll = {};
        ApplyPositionAndRotation();
    }
    else
    {
        UpdateAutoScroll(deltaTime);
    }
}

void GameplayCameraController::InputEvent(MouseButtonInputEvent& inputEvent)
{
}

void GameplayCameraController::InputEvent(KeyInputEvent& inputEvent)
{
    if (mCamera == nullptr) 
        return;

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

void GameplayCameraController::InputEvent(MouseMovedInputEvent& inputEvent)
{
}

void GameplayCameraController::InputEvent(MouseScrollInputEvent& inputEvent)
{
    mCameraHeight -= inputEvent.mScroll.y * 0.25f;
    ClampCameraHeight(mCameraHeight);
    ApplyPositionAndRotation();
}

void GameplayCameraController::SetStartPosition(const glm::vec2& position)
{
    mStartPosition = position;
}

void GameplayCameraController::SetPositionBounds(const glm::vec2& boundsMin, const glm::vec2& boundsMax)
{
    mBoundsMin = glm::min(boundsMin, boundsMax);
    mBoundsMax = glm::max(boundsMin, boundsMax);
}

glm::vec2 GameplayCameraController::GetCameraMoveVectorFromInputs() const
{
    cxx_assert(mCamera);

    glm::vec3 moveDirection {0.0f};

    EnumSet<eDirection> cardinalDirs;
    if (GetCameraMoveDirections(cardinalDirs))
    {
        if (cardinalDirs.Contains(eDirection_N) || cardinalDirs.Contains(eDirection_S))
        {
            const glm::vec3 vmove = glm::normalize(glm::cross(WorldAxes::Y, mCamera->mRight));
            if (cardinalDirs.Contains(eDirection_S))
            {
                moveDirection += vmove;
            }
            if (cardinalDirs.Contains(eDirection_N))
            {
                moveDirection -= vmove;
            }
        }

        if (cardinalDirs.Contains(eDirection_E))
        {
            moveDirection -= mCamera->mRight;
        }
        if (cardinalDirs.Contains(eDirection_W))
        {
            moveDirection += mCamera->mRight;
        }
        // don't normalize : diagonal movement speedup is intent
    }
    return glm::vec2{moveDirection.x, moveDirection.z};
}

bool GameplayCameraController::GetCameraMoveDirections(EnumSet<eDirection>& cardinalDirs) const
{
    cardinalDirs.Clear();

    bool ctrlPressed = (gInputs.GetKeyState(KEYCODE_LEFT_CTRL) || gInputs.GetKeyState(KEYCODE_RIGHT_CTRL));
    if (!ctrlPressed)
    {
        // north
        if (gInputs.GetKeyState(KEYCODE_W) || gInputs.GetKeyState(KEYCODE_UP))
        {
            cardinalDirs.Include(eDirection_N);
        }
        // south
        if (gInputs.GetKeyState(KEYCODE_S) || gInputs.GetKeyState(KEYCODE_DOWN))
        {
            cardinalDirs.Include(eDirection_S);
        }
        // east
        if (gInputs.GetKeyState(KEYCODE_D) || gInputs.GetKeyState(KEYCODE_RIGHT))
        {
            cardinalDirs.Include(eDirection_E);
        }
        // west
        if (gInputs.GetKeyState(KEYCODE_A) || gInputs.GetKeyState(KEYCODE_LEFT))
        {
            cardinalDirs.Include(eDirection_W);
        }
    }

    // edge scrolling
    if (mEnableEdgeScrolling)
    {
        const Point2D& mousePos = gInputs.GetMousePosition();
        const Point2D edgeTolerance {2, 2};
        Rect2D screenRect = gUiManager.GetScreenRect();
        screenRect.Inflate(-edgeTolerance);
        // north
        if (mousePos.y < screenRect.y)
        {
            cardinalDirs.Include(eDirection_N);
        }
        // south
        if (mousePos.y >= (screenRect.y + screenRect.h))
        {
            cardinalDirs.Include(eDirection_S);
        }
        // east
        if (mousePos.x >= (screenRect.x + screenRect.w))
        {
            cardinalDirs.Include(eDirection_E);
        }
        // west
        if (mousePos.x < screenRect.x)
        {
            cardinalDirs.Include(eDirection_W);
        }
    }
    return !cardinalDirs.Empty();
}

float GameplayCameraController::GetRotateDirectionFromInputs() const
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

float GameplayCameraController::GetZoomDirectionFromInputs()
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

void GameplayCameraController::ClampWithinBounds(glm::vec2& position) const
{
    position = glm::clamp(position, mBoundsMin, mBoundsMax);
}

void GameplayCameraController::ClampCameraHeight(float& height) const
{
    height = std::clamp(height, CAM_MIN_HEIGHT, CAM_MAX_HEIGHT);
}

void GameplayCameraController::ApplyPositionAndRotation()
{
    mCamera->SetRotation(mRotationAngles);

    float distanceFromTargetPoint = mCameraHeight / tanf(glm::radians(mRotationAngles.x));
    const glm::vec3 worldForward = glm::normalize(glm::cross(WorldAxes::Y, mCamera->mRight));
    const glm::vec3 cameraPosition 
    {
        mCurrentPosition.x,
        mCameraHeight,
        mCurrentPosition.y
    };
    mCamera->SetPosition(cameraPosition - worldForward * distanceFromTargetPoint);
}

void GameplayCameraController::UpdateAutoScroll(float deltaTime)
{
    if (!mAutoScroll)
        return;

    AutoScrollState& autoScrollState = *mAutoScroll;

    mCurrentPosition = cxx::smooth_damp(mCurrentPosition, autoScrollState.mTargetPosition, autoScrollState.mCurrentVelocity, 0.35f, deltaTime);
    if (glm::length(autoScrollState.mTargetPosition - mCurrentPosition) < 0.01f)
    {
        mAutoScroll = {};
    }
    ApplyPositionAndRotation();
}

void GameplayCameraController::ResetCamera()
{
    if (mCamera == nullptr) 
        return;

    StopCamera();

    mCameraHeight = CAM_HEIGHT;
    mCurrentPosition = mStartPosition;
    mRotationAngles = glm::vec3(CAM_ANGLE, -45.0f, 0.0f);

    Camera::ProjectionParams cameraParams (DEFAULT_CAMERA_NEAR_DISTANCE, DEFAULT_CAMERA_FAR_DISTANCE, KEEPER_CAMERA_FOVY);
    mCamera->SetPosition({});
    mCamera->SetupProjection(cameraParams);

    ApplyPositionAndRotation();
}

void GameplayCameraController::StopCamera()
{
    mIncreasingFov = false;
    mDecreasingFov = false;

    mMoveVelocity = {};
    mAutoScroll = {};
}

void GameplayCameraController::FocusOnMapLocation(const Point2D& tileLocation)
{
    if (mCamera == nullptr)
        return;

    const glm::vec2 cameraPosition = MapUtils::ComputeTileCenter2d(tileLocation);
    if (cameraPosition == mCurrentPosition)
        return;

    if (mAutoScroll && (mAutoScroll->mTargetPosition == cameraPosition))
        return;

    StopCamera();

    AutoScrollState& autoScrollState = mAutoScroll.emplace();
    autoScrollState.mTargetPosition = cameraPosition;
    autoScrollState.mCurrentVelocity = {};
}
