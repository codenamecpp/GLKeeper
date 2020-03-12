#include "pch.h"
#include "Renderable.h"
#include "RenderScene.h"

Renderable::Renderable()
    : mListNodeTransformed(this)
    , mListNodeOnScene(this)
    , mTransformDirty()
    , mBoundingBoxDirty()
    , mTransformation(1.0f)
    , mScaling(1.0f)
    , mPosition()
    , mDirectionRight(SceneAxis_X())
    , mDirectionUpward(SceneAxis_Y())
    , mDirectionForward(SceneAxis_Z())
{
}

Renderable::~Renderable()
{
    // entity must be detached from scene before destroy
    debug_assert(IsAttachedToScene() == false);
}

void Renderable::ComputeTransformation()
{
    // refresh transformations matrix
    if (mTransformDirty)
    {
        glm::mat4 orientation {1.0f};
        orientation[0] = glm::vec4(mDirectionRight, 0);
        orientation[1] = glm::vec4(mDirectionUpward, 0);
        orientation[2] = glm::vec4(mDirectionForward, 0);

        glm::vec3 scaling {mScaling, mScaling, mScaling};
        mTransformation = glm::translate(mPosition) * orientation * glm::scale(scaling);
        mTransformDirty = false;
        // force refresh world space bounds
        mBoundsTransformed = cxx::transform_aabbox(mBounds, mTransformation);
        mBoundingBoxDirty = false;
        return;
    }

    // refresh world space bounding box
    if (mBoundingBoxDirty)
    {
        mBoundsTransformed = cxx::transform_aabbox(mBounds, mTransformation);
        mBoundingBoxDirty = false;
    }
}

void Renderable::SetLocalBoundingBox(const cxx::aabbox& aabox)
{
    mBounds = aabox;

    InvalidateBounds();
}

void Renderable::SetPosition(const glm::vec3& position)
{
    mPosition = position;

    InvalidateTransform();
}

void Renderable::SetScaling(float scaling)
{
    mScaling = scaling;

    InvalidateTransform();
}

void Renderable::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward, const glm::vec3& directionUpward)
{
    mDirectionForward = directionForward;
    mDirectionRight = directionRight;
    mDirectionUpward = directionUpward;

    InvalidateTransform();
}

void Renderable::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward)
{
    debug_assert(!"not yet implemented"); // todo
}

void Renderable::OrientTowards(const glm::vec3& point)
{
    OrientTowards(point, SceneAxis_Y());
}

void Renderable::Rotate(const glm::vec3& rotationAxis, float rotationAngle)
{
    glm::mat3 rotationMatrix = glm::mat3(glm::rotate(rotationAngle, rotationAxis)); 

    mDirectionForward = glm::normalize(rotationMatrix * mDirectionForward);
    mDirectionRight = glm::normalize(rotationMatrix * mDirectionRight);
    mDirectionUpward = glm::normalize(rotationMatrix * mDirectionUpward);

    InvalidateTransform();
}

void Renderable::Translate(const glm::vec3& translation)
{
    mPosition += translation;

    InvalidateTransform();
}

void Renderable::OrientTowards(const glm::vec3& point, const glm::vec3& upward)
{
    glm::vec3 zaxis = glm::normalize(point - mPosition);
    glm::vec3 xaxis = glm::normalize(glm::cross(upward, zaxis));
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // now assign orientation vectors
    SetOrientation(xaxis, zaxis, yaxis);
}

void Renderable::ResetTransformation()
{
    mTransformation = glm::mat4{1.0f};
    mScaling = 1.0f;
    mPosition = glm::vec3{0.0f};
    mDirectionRight = SceneAxis_X();
    mDirectionUpward = SceneAxis_Y();
    mDirectionForward = SceneAxis_Z();

    InvalidateTransform();
}

void Renderable::InvalidateTransform()
{
    if (mTransformDirty)
        return;

    mTransformDirty = true;
    mBoundingBoxDirty = true; // force refresh world space bounds 

    if (IsAttachedToScene())
    {
        gRenderScene.HandleTransformChange(this);
    }
}

void Renderable::InvalidateBounds()
{
    if (mBoundingBoxDirty)
        return;

    mBoundingBoxDirty = true;
    if (IsAttachedToScene())
    {
        gRenderScene.HandleTransformChange(this);
    }
}

void Renderable::ResetOrientation()
{
    SetOrientation(SceneAxis_X(), SceneAxis_Z(), SceneAxis_Y());
}

bool Renderable::IsAttachedToScene() const
{
    return mListNodeOnScene.is_linked();
}

