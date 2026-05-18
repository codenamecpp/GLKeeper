#pragma once

namespace cxx
{
    // defines axis aligned bounding box
    struct aabbox
    {
    public:
        aabbox()
            : mMin( 9999.0f,  9999.0f,  9999.0f)
            , mMax(-9999.0f, -9999.0f, -9999.0f)
        {}

        // @param argMinPoint: Min
        // @param argMaxPoint: Max
        aabbox(const glm::vec3& argMinPoint, const glm::vec3& argMaxPoint)
            : mMin(argMinPoint)
            , mMax(argMaxPoint)
        {}

        inline bool is_valid() const
        {
            return (mMax.x >= mMin.x) && (mMax.y >= mMin.y) && (mMax.z >= mMin.z);
        }

        // expand box if points is beyond of boundaries
        // @param point: Point
        inline void extend(const glm::vec3& point) 
        {
            extend(point.x, point.y, point.z);
        }

        inline void extend(float theX, float theY, float theZ)
        {
            if (theX < mMin.x) mMin.x = theX; else if (theX > mMax.x) mMax.x = theX;
            if (theY < mMin.y) mMin.y = theY; else if (theY > mMax.y) mMax.y = theY;
            if (theZ < mMin.z) mMin.z = theZ; else if (theZ > mMax.z) mMax.z = theZ;
        }

        inline void reset()
        {
            mMin = {9999.0f,9999.0f,9999.0f};
            mMax = {-9999.0f,-9999.0f,-9999.0f};
        }

        inline void set_to_point(const glm::vec3& point)
        {
            mMin = point;
            mMax = point;
        }

        inline void set_to_zero()
        {
            mMin = {0.0f, 0.0f, 0.0f};
            mMax = {0.0f, 0.0f, 0.0f};
        }

        // expand box if points is beyond of boundaries
        // @param bbox: Bounding box
        inline void extend(const aabbox& bbox)
        {
            extend(bbox.mMin);
            extend(bbox.mMax);
        }

        // merge two bounding boxes
        // @param bbox: Bounding box
        inline aabbox union_with(const aabbox& bbox) const
        {
            aabbox boxcurrent (mMin, mMax);
            boxcurrent.extend(bbox.mMin);
            boxcurrent.extend(bbox.mMax);
            return boxcurrent;
        }

        // test whether point in within bounding box
        // @param point: Point
        inline bool contains(const glm::vec3& point) const 
        {
            return (point.x >= mMin.x) && (point.x <= mMax.x) && 
                (point.y >= mMin.y) && (point.y <= mMax.y) && 
                (point.z >= mMin.z) && (point.z <= mMax.z);
        }

        // test whether bounding box in within bounding box
        // @param point: Point
	    inline bool contains(const aabbox& other) const
	    {
		    return other.mMin.x >= mMin.x &&
			    other.mMax.x <= mMax.x &&
			    other.mMin.y >= mMin.y &&
			    other.mMax.y <= mMax.y &&
			    other.mMin.z >= mMin.z &&
			    other.mMax.z <= mMax.z;
	    }

	    //! stores all 8 edges of the box into a array
	    //! \param edges: Pointer to array of 8 edges
        inline void get_edges(glm::vec3* edges) const
	    {
		    const glm::vec3 middle = get_center();
		    const glm::vec3 diag = (middle - mMax);

		    /*
			    Edges are stored in this way:
                  /1--------/3
                 /  |      / |
                /   |     /  |
                5---------7  |
                |   0- - -| -2
                |  /      |  /
                |/        | /
                4---------6/ 
		    */

		    edges[0] = {middle.x + diag.x, middle.y + diag.y, middle.z + diag.z};
		    edges[1] = {middle.x + diag.x, middle.y - diag.y, middle.z + diag.z};
		    edges[2] = {middle.x + diag.x, middle.y + diag.y, middle.z - diag.z};
		    edges[3] = {middle.x + diag.x, middle.y - diag.y, middle.z - diag.z};
		    edges[4] = {middle.x - diag.x, middle.y + diag.y, middle.z + diag.z};
		    edges[5] = {middle.x - diag.x, middle.y - diag.y, middle.z + diag.z};
		    edges[6] = {middle.x - diag.x, middle.y + diag.y, middle.z - diag.z};
		    edges[7] = {middle.x - diag.x, middle.y - diag.y, middle.z - diag.z};
	    }

        // get box center point
        inline glm::vec3 get_center() const 
        {
            return (mMin + mMax) * 0.5f;
        }

        // simple geometry stuff
	    inline float get_height() const { return mMax.y - mMin.y; }
	    inline float get_width() const { return mMax.x - mMin.x; }
	    inline float get_depth() const { return mMax.z - mMin.z; }
        inline float get_surface_area() const 
        { 
            const float multiplication = get_width() * get_height() + get_width() * get_depth() + get_height() * get_depth();
            return 2.0f * multiplication; 
        }

    public:
        glm::vec3 mMin;
        glm::vec3 mMax;
    };

    inline bool operator == (const aabbox& lhs, const aabbox& rhs) { return (lhs.mMin == rhs.mMin) && (lhs.mMax == rhs.mMax); }
    inline bool operator != (const aabbox& lhs, const aabbox& rhs) { return !(lhs == rhs); }

    //////////////////////////////////////////////////////////////////////////

    // defines bounding sphere
    struct bounding_sphere
    {
    public:
        bounding_sphere() = default;

        // @param argSphereOrigin: Sphere center position
        // @param argSphereRadius: Sphere radius
        bounding_sphere(const glm::vec3& argSphereOrigin, float argSphereRadius)
            : mOrigin(argSphereOrigin)
            , mRadius(argSphereRadius)
        {}

        // Test whether point is within bounding sphere
        // @param point: Point
        inline bool contains(const glm::vec3& point) const 
        {
	        const glm::vec3 disp {point - mOrigin};
	        float lengthSq = (disp.x * disp.x) + (disp.y * disp.y) + (disp.z * disp.z);
	        return lengthSq < (mRadius * mRadius);
        }

    public:
        glm::vec3 mOrigin;
        float mRadius;
    };

    //////////////////////////////////////////////////////////////////////////

    // create a new interpolated bounding box
    // @param interpolation: value between 0.0f and 1.0f.
    inline aabbox interpolate_aabbox(const aabbox& aaboxA, const aabbox& aaboxB, float interpolation)
    {
	    float inv = 1.0f - interpolation;
	    return {
            (aaboxB.mMin * inv) + (aaboxA.mMin * interpolation),
		    (aaboxB.mMax * inv) + (aaboxA.mMax * interpolation)};
    }

    // transform aabox
    // @param aabox: Source aabox
    // @param transformation: Transformation matrix
    inline aabbox transform_aabbox(const aabbox& aabox, const glm::mat4& tmat)
    {
 	    // Efficient algorithm for transforming an AABB, taken from Graphics Gems
		
        aabbox transformedAabb;

	    for (int i = 0; i < 3; ++i)
	    {
		    transformedAabb.mMin[i] = tmat[3][i];
		    transformedAabb.mMax[i] = tmat[3][i];
			
		    for (int j = 0; j < 3; ++j)
		    {
			    float x = aabox.mMin[j] * tmat[j][i];
			    float y = aabox.mMax[j] * tmat[j][i];
			    transformedAabb.mMin[i] += std::fminf(x, y);
			    transformedAabb.mMax[i] += std::fmaxf(x, y);
		    }
	    }
        return transformedAabb;
    }

    // Create bounding sphere from aabbox
    // @param aabb: Bounding box
    inline bounding_sphere compute_bounding_sphere(const aabbox& aabb)
    {
        bounding_sphere sphere {
            // origin
            (aabb.mMin + aabb.mMax) * 0.5f,
            // radius
            (aabb.mMax - aabb.mMin).length() * 0.5f
        };
        return sphere;
    }

} // namespace cxx