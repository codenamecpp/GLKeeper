#pragma once

#include "bounding_volumes.h"

namespace cxx
{
    // defines ray in 3d space
    struct ray3d_t
    {
    public:
        ray3d_t() = default;
        ray3d_t(const glm::vec3& origin, const glm::vec3& direction)
            : mOrigin(origin)
            , mDirection(direction)
        {}
    public:
        glm::vec3 mOrigin;
        glm::vec3 mDirection; // Must be normalized!
    };
    
    // test intersection with two aabb
    // @param bboxA, bboxB: Bounding boxes
    bool intersects(const aabbox& bboxA, const aabbox& bboxB);

    // test intersection aabox vs ray
    // @param bbox: Bouding box
    // @param ray3d: Ray
    // @param distanceNear, distanceFar: Intersection distance near/far
    bool intersects(const aabbox& bbox, const ray3d_t& ray3d, float& outDistanceNear, float& outDistanceFar);
    bool intersects(const bounding_sphere& sphere, const ray3d_t& ray3d, float& outDistance);

    // test intersection ray versus triangle
    // @param ray3d: Ray
    // @param tv0, tv1, tv2: Triangle vertices
    // @param outPoint: Intersetion point if success
    bool intersects(const ray3d_t& ray3d, const glm::vec3& tv0, const glm::vec3& tv1, const glm::vec3& tv2, glm::vec3& outPoint);

} // namespace cxx