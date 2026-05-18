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
    // get render view camera
    inline Camera& GetCamera() { return mCamera; }
    inline const Camera& GetCamera() const { return mCamera; }

private:
    Camera mCamera;

    // render view state
    bool mIsActive = true;
};