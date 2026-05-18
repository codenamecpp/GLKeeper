#pragma once

namespace cxx
{
    // defines plane in 3d space
    struct plane3d
    {
    public:
        plane3d() = default;

        // @param origin: Plane origin
        // @param argNormal: Plane normal
        plane3d(const glm::vec3& origin, const glm::vec3& planeNormal)
            : mNormal(planeNormal)
        {
            mDistance = -glm::dot(planeNormal, origin);
        }

        // @param argNormal: Plane normal
        // @param argDistance: Plane distance
        plane3d(const glm::vec3& argNormal, float argDistance)
            : mDistance(argDistance)
            , mNormal(argNormal)
        {
        }
        // create plane from triangle points
        // @param ptA, ptB, ptC: Points of triangle
        static plane3d from_points(const glm::vec3& ptA, const glm::vec3& ptB, const glm::vec3& ptC)
        {
            const glm::vec3 aNormal = glm::cross(ptB - ptA, ptC - ptA);
            plane3d resultPlane 
            { 
                aNormal, -glm::dot(aNormal, ptA) 
            };

            resultPlane.normalize();
            return resultPlane;
        }

        // set normal and distance
        // @param a, b, c: Plane normal
        // @param d: Plane distance
        inline void setup(float a, float b, float c, float d) 
        {
            mNormal.x = a;
            mNormal.y = b;
            mNormal.z = c;
            mDistance = d;
        }
        
        // normalize plane
        inline void normalize()
        {
            auto normalLength = mNormal.length();
            if (normalLength)
            {
	            auto length = 1.0f / normalLength;
                mNormal *= length;
                mDistance *= length;
            }
        }

        // get distance from point to plane
        // returns:
        //  > 0 if the point lies in front of the plane 'p'
        //  < 0 if the point lies behind the plane 'p'
        //    0 if the point lies on the plane 'p'
        //
        // the signed distance from the point to the plane is returned.
        inline float get_distance_from_point(const glm::vec3& argPoint) const 
        {
            return glm::dot(mNormal, argPoint) + mDistance;
        }

    public:
        glm::vec3 mNormal;
        float mDistance;
    };

} // namespace cxx