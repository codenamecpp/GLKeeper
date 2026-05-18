#pragma once

//////////////////////////////////////////////////////////////////////////

#include "SceneDefs.h"
#include "GraphicsDefs.h"

//////////////////////////////////////////////////////////////////////////

// defines base of visual representation of 3d object within game world
// it has location and orientation in space along with bounding volume
class SceneObject: public cxx::noncopyable
{
public:
    Color32 mDebugColor; // debug draw tint

public:
    SceneObject();

    virtual ~SceneObject();

    void SetScene(Scene* scene);

    // process update
    virtual void UpdateFrame(float deltaTime)
    {
        // do nothing
    }

    // register object for rendering on this frame
    virtual void RegisterForRendering(SceneRenderLists& renderLists, float distanceToCamera2)
    {
        // do nothing
    }

    // enable or disable visual object
    // disabled objects will not be rendered / updated
    void SetObjectActive(bool isActive);

    // test whether object is enabled
    inline bool IsObjectActive() const { return mIsObjectActive; }

    // set position
    void SetPosition(const glm::vec3& position);

    // set uniform scale
    void SetScale(float factor);

    // set object orienation vectors, expecting all normalized
    void SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward, const glm::vec3& directionUpward);
    void SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward);

    // setup object orientation, look at point
    void OrientTowards(const glm::vec3& point);
    void OrientTowards(const glm::vec3& point, const glm::vec3& upward);

    // rotate object around specified axis, note that it is rotation relative to current orientation
    void RotateAroundAxis(const glm::vec3& rotationAxis, cxx::angle_t rotationAngle);

    // translate position relative to current location
    void Translate(const glm::vec3& translation);

    // set local space bounding volume of object
    void SetLocalBoundingBox(const cxx::aabbox& bounds);

    // reset orientation or transformations to defaults
    void ResetOrientation();
    void ResetTransformation();

    // compute transformation matrix and world space bounding box
    void ComputeTransformation();

    // get current bounds
    inline const cxx::aabbox& GetLocalBounds() const { return mBounds; }
    inline const cxx::aabbox& GetWorldBounds() const { return mBoundsTransformed; }

    // get position
    inline const glm::vec3& GetPosition() const { return mPosition; }
    
    // get orientation
    inline const glm::vec3& GetForward() const { return mForward; }
    inline const glm::vec3& GetRight() const { return mRight; }
    inline const glm::vec3& GetUpward() const { return mUpward; }

    // get visibility layers
    inline RenderLayerMask GetRenderLayers() const { return mRenderLayerMask; }
    inline void SetRenderLayers(RenderLayerMask layers) { mRenderLayerMask = layers; }
    inline void ChangeRenderLayers(RenderLayerMask enableLayers, RenderLayerMask disableLayers)
    {
        mRenderLayerMask = (mRenderLayerMask & ~disableLayers) | enableLayers;
    }

public:

    // pool
    virtual void OnRecycle();

protected:
    // set transform or bounds dirty flags
    void InvalidateTransform(); 
    void InvalidateBounds();

protected:
    Scene* mScene = nullptr;

    // current transformations
    glm::vec3 mRight; // direction vector along x axis, should be normalized
    glm::vec3 mUpward; // direction vector along y axis, should be normalized
    glm::vec3 mForward; // direction vector along z axis, should be normalized
    glm::vec3 mPosition; // position within world
    glm::vec3 mScale;
    glm::mat4 mTransformation; // should be manually updated so make sure to do ComputeTransformation

    // transformed bounds should be manually updated so make sure to ComputeTransformation
    cxx::aabbox mBoundsTransformed; // world space
    cxx::aabbox mBounds; // untransformed, used for culling and mouse tests
    
    RenderLayerMask mRenderLayerMask = RenderLayer_WorldObjects;

    // flags
    bool mIsObjectActive : 1;
    bool mTransformDirty: 1;
    bool mBoundingBoxDirty: 1;
};