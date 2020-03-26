#pragma once

#include "SceneDefs.h"
#include "RenderDefs.h"

class SceneObject: public cxx::noncopyable
{
    friend class RenderScene;

public:
    // readonly
    glm::vec3 mDirectionRight; // direction vector along x axis, should be normalized
    glm::vec3 mDirectionUpward; // direction vector along y axis, should be normalized
    glm::vec3 mDirectionForward; // direction vector along z axis, should be normalized
    glm::vec3 mPosition; // position is scene, global
    float mScaling; // uniform scaling
    glm::mat4 mTransformation; // should be manually updated so make sure to do ComputeTransformation

    // transformed bounds should be manually updated so make sure to ComputeTransformation
    cxx::aabbox mBoundsTransformed; // world space
    cxx::aabbox mBounds; // untransformed, used for culling and mouse tests

    Color32 mDebugColor; // color used for debug draw

    float mDistanceToCameraSquared; // this value gets updated during scene rendition

public:
    SceneObject();
    virtual ~SceneObject();

    // process scene update frame
    // @param deltaTime: Time since last update
    virtual void UpdateFrame(float deltaTime) {}

    // request entity to register itself in render lists
    // @param renderList: Render lists
    virtual void RegisterForRendering(SceneRenderList& renderList) {}

    // set entity orienation vectors, expecting all normalized
    // @param directionRight: Vector X
    // @param directionForward: Vector Z
    // @param directionUpward: Vector Y
    void SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward, const glm::vec3& directionUpward);
    void SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward);

    // setup entity orientation, look at point
    // @param point: Point world
    // @param upward: Up vector
    void OrientTowards(const glm::vec3& point);
    void OrientTowards(const glm::vec3& point, const glm::vec3& upward);

    // reset entity orientation on scene to defaults
    void ResetOrientation();

    // Reset Position, Scale and Rotation to identity
    void ResetTransformation();

    // set entity position on the scene
    // @param position: Position
    void SetPosition(const glm::vec3& position);

    // set entity uniform scale
    // @param scaling: Scaling factor
    void SetScaling(float scaling);

    // rotate entity around specified axis, note that it is local rotation
    // @param rotationAxis: Reference axis
    // @param rotationAngle: Angle specified in radians
    void Rotate(const glm::vec3& rotationAxis, float rotationAngle);

    // move entity on scene
    // @param translation: Translation
    void Translate(const glm::vec3& translation);

    // compute transformation matrix and world space bounding box
    void ComputeTransformation();

    // set local space bounding volume of entity
    // @param aabox: Bounds
    void SetLocalBoundingBox(const cxx::aabbox& aabox);

    // test whether scene entity is currently attached to scene and therefore may be rendered
    bool IsAttachedToScene() const;

private:
    // process entity transform or bounds changed
    void InvalidateTransform(); 
    void InvalidateBounds();    

private:
    // scene data
    cxx::intrusive_node<SceneObject> mListNodeTransformed;
    cxx::intrusive_node<SceneObject> mListNodeOnScene;

    // dirty flags
    bool mTransformDirty;
    bool mBoundingBoxDirty;
};