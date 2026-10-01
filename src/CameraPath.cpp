#include "stdafx.h"
#include "CameraPath.h"

//////////////////////////////////////////////////////////////////////////

static const CameraPath::Frame sDummyCameraPathFrame = CameraPath::Frame {};

//////////////////////////////////////////////////////////////////////////

int CameraPath::GetFramesCount() const
{
    return static_cast<int>(mFrames.size());
}

const CameraPath::Frame& CameraPath::GetFrame(int frameIndex) const
{
    if ((frameIndex < 0) || (frameIndex >= GetFramesCount()))
    {
        cxx_assert(false);
        return sDummyCameraPathFrame;
    }

    return mFrames[frameIndex];
}

const CameraPath::Frame& CameraPath::GetFirstFrame() const
{
    if (mFrames.empty())
    {
        cxx_assert(false);
        return sDummyCameraPathFrame;
    }
    return mFrames.front();
}

const CameraPath::Frame& CameraPath::GetLastFrame() const
{
    if (mFrames.empty())
    {
        cxx_assert(false);
        return sDummyCameraPathFrame;
    }
    return mFrames.back();
}

CameraPath::Frame CameraPath::SampleFrame(float frameIndexf) const
{
    const int LastFrameIndex = GetFramesCount() - 1;

    if (LastFrameIndex < 0)
    {
        cxx_assert(false);
        return sDummyCameraPathFrame;
    }

    const float clampedFrameIndexf = std::clamp(frameIndexf, 0.0f, LastFrameIndex * 1.0f);

    const int frameIndexA = static_cast<int>(clampedFrameIndexf);
    const int frameIndexB = frameIndexA + 1;

    if (frameIndexA == LastFrameIndex)
    {
        return GetLastFrame();
    }

    const float t = clampedFrameIndexf - (frameIndexA * 1.0f);

    const Frame& frameA = mFrames[frameIndexA];
    const Frame& frameB = mFrames[frameIndexB];

    if (cxx::eps_equals_zero(t))
    {
        return frameA;
    }

    Frame resultFrame;

    resultFrame.mPosition = glm::mix(frameA.mPosition, frameB.mPosition, t);
    resultFrame.mFov.set_angle(
        glm::mix(
            frameA.mFov.mAngleRadians, 
            frameB.mFov.mAngleRadians, t), cxx::angle_t::units::radians);

    // mix basis vectors, do NLERP
    //const glm::vec3 rawForward = glm::mix(frameA.mForward, frameB.mForward, t);
    //const glm::vec3 rawUp = glm::mix(frameA.mUp, frameB.mUp, t);

    //resultFrame.mForward = glm::normalize(rawForward);
    //resultFrame.mRight = glm::normalize(glm::cross(resultFrame.mForward, rawUp));
    //resultFrame.mUp = glm::cross(resultFrame.mRight, resultFrame.mForward);

    resultFrame.mForward = glm::normalize(glm::mix(frameA.mForward, frameB.mForward, t));
    resultFrame.mRight = glm::normalize(glm::mix(frameA.mRight, frameB.mRight, t));
    resultFrame.mUp = glm::normalize(glm::mix(frameA.mUp, frameB.mUp, t));

    // do something
    return resultFrame;
}

CameraPath::Frame CameraPath::SampleFrameT(float t) const
{
    return SampleFrame(GetFramesCount() * std::clamp(t, 0.0f, 1.0f));
}
