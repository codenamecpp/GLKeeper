#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GraphicsDefs.h"
#include "Camera.h"

//////////////////////////////////////////////////////////////////////////

class RenderView: public cxx::noncopyable
{
public:
    RenderView() = default;

    // enable or disable render view
    inline bool IsActive() const { return mIsActive; }
    void SetActive(bool isActive)
    {
        mIsActive = isActive;
    }

    inline const Camera& GetCamera() const { return mCamera; }

    inline Camera& GetCamera() { return mCamera; }

    // pool
    inline void OnRecycle()
    {
        mCamera = {};
        mIsActive = true;
    }
private:
    Camera mCamera;
    bool mIsActive = true;
};

//////////////////////////////////////////////////////////////////////////