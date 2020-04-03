#pragma once

#include "GameObjectComponent.h"

// transformation component of game object
class TransformComponent: public GameObjectComponent
{
public:
    // readonly
    glm::vec3 mDirectionRight; // direction vector along x axis, should be normalized
    glm::vec3 mDirectionUpward; // direction vector along y axis, should be normalized
    glm::vec3 mDirectionForward; // direction vector along z axis, should be normalized
    glm::vec3 mPosition; // position within world
    float mScaling; // uniform scaling
    glm::mat4 mTransformation; // should be manually updated so make sure to do ComputeTransformation

                               // transformed bounds should be manually updated so make sure to ComputeTransformation
    cxx::aabbox mBoundsTransformed; // world space
    cxx::aabbox mBounds; // untransformed, used for culling and mouse tests

public:
    TransformComponent(GameObject* gameObject);

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

    // reset orientation to defaults
    void ResetOrientation();

    // Reset Position, Scale and Rotation to identity
    void ResetTransformation();

    // set position
    // @param position: Position
    void SetPosition(const glm::vec3& position);

    // set uniform scale
    // @param scaling: Scaling factor
    void SetScaling(float scaling);

    // rotate entity around specified axis, note that it is local rotation
    // @param rotationAxis: Reference axis
    // @param rotationAngle: Angle specified in radians
    void Rotate(const glm::vec3& rotationAxis, float rotationAngle);

    // translate position
    // @param translation: Translation
    void Translate(const glm::vec3& translation);

    // compute transformation matrix and world space bounding box
    void ComputeTransformation();

    // set local space bounding volume of entity
    // @param aabox: Bounds
    void SetLocalBoundingBox(const cxx::aabbox& aabox);

    // process entity transform or bounds changed
    void InvalidateTransform(); 
    void InvalidateBounds();

private:
    // dirty flags
    bool mTransformDirty;
    bool mBoundingBoxDirty;
};