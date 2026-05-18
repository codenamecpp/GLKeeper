#pragma once

namespace cxx
{
    // get next power of two value
    inline unsigned int get_next_pot(unsigned int value)
    {
        if (value > 0)
        {
            --value;
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            ++value;
        }
        return value;
    }

    // test whether value is power of two
    inline bool is_pot(unsigned int value)
    {
        unsigned int nextPowerOfTwo = get_next_pot(value);
        return nextPowerOfTwo == value;
    }

    //////////////////////////////////////////////////////////////////////////

    // test is number even or odd
    inline bool is_even(int value) { return (value % 2) == 0; }
    inline bool is_odd(int value)  { return (value % 2)  > 0; }

    //////////////////////////////////////////////////////////////////////////

    // wrap angle to -180..180
    // degrees
    inline float wrap_angle_to_180(float angleDegrees) 
    { 
        static const float inv = (1.0f / 360.0f);
        return angleDegrees - 360.0f * std::floorf((angleDegrees + 180.0f) * inv);
    }

    // wrap angle to 0..360
    inline float wrap_angle_to_360(float angleDegrees) 
    {  
        static const float inv = (1.0f / 360.0f);
        return angleDegrees - 360.0f * std::floor(angleDegrees * inv);
    }

    // wrap angle to [-pi..pi]
    inline float wrap_angle_to_pi(float angleRadians)
    {
        static const float inv = (1.0f / glm::two_pi<float>());
        return angleRadians - glm::two_pi<float>() * std::floorf((angleRadians + glm::pi<float>()) * inv);
    }

    // wrap angle to [0..2pi)
    inline float wrap_angle_to_2pi(float angleRadians)
    {
        static const float inv = (1.0f / glm::two_pi<float>());
        return angleRadians - glm::two_pi<float>() * std::floor(angleRadians * inv);
    }

    //////////////////////////////////////////////////////////////////////////

    inline unsigned int round_up_to(unsigned int value, unsigned int pad)
    {
        if (pad > 0 && (value % pad))
        {
            const unsigned int value_to_add = pad - (value % pad);
            return value + value_to_add;
        }
        return value;
    }

    inline unsigned int align_up(unsigned int value, unsigned int alignment)
    {
        if (alignment > 0)
        {
            value = ((value) + (alignment - 1)) & ~(alignment - 1);
        }
        return value;
    }

    inline unsigned int aligh_down(unsigned int value, unsigned int alignment)
    {
        if (alignment > 0)
        {
            value = value & ~(alignment - 1);
        }
        return value;
    }

    inline bool is_aligned(unsigned int value, unsigned alignment)
    {
        return ((value) & (alignment - 1)) == 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // convert Euler angles(x,y,z) to axes(left, up, forward)
    // Each column of the rotation matrix represents left, up and forward axis.
    // The order of rotation is Roll->Yaw->Pitch (Rx*Ry*Rz)
    // Rx: rotation about X-axis, pitch
    // Ry: rotation about Y-axis, yaw(heading)
    // Rz: rotation about Z-axis, roll
    //    Rx           Ry          Rz
    // |1  0   0| | Cy  0 Sy| |Cz -Sz 0|   | CyCz        -CySz         Sy  |
    // |0 Cx -Sx|*|  0  1  0|*|Sz  Cz 0| = | SxSyCz+CxSz -SxSySz+CxCz -SxCy|
    // |0 Sx  Cx| |-Sy  0 Cy| | 0   0 1|   |-CxSyCz+SxSz  CxSySz+SxCz  CxCy|
    //////////////////////////////////////////////////////////////////////////

    // @param angles specified in radians
    inline void angles_to_axes(const glm::vec3& angles, glm::vec3* right, glm::vec3* up, glm::vec3* forward)
    {
        float sx, sy, sz, cx, cy, cz;

        // rotation angle about X-axis (pitch)
        sx = std::sinf(angles.x);
        cx = std::cosf(angles.x);

        // rotation angle about Y-axis (yaw)
        sy = std::sinf(angles.y);
        cy = std::cosf(angles.y);

        // rotation angle about Z-axis (roll)
        sz = std::sinf(angles.z);
        cz = std::cosf(angles.z);

        // determine left axis
        right->x = cy * cz;
        right->y = sx * sy * cz + cx * sz;
        right->z = -cx * sy * cz + sx * sz;

        // determine up axis
        up->x = -cy * sz;
        up->y = -sx * sy * sz + cx * cz;
        up->z = cx * sy * sz + sx * cz;

        // determine forward axis
        forward->x = sy;
        forward->y = -sx * cy;
        forward->z = cx * cy;
    }

    //////////////////////////////////////////////////////////////////////////

    // loops the value within range [0, 1]
    inline float repeat(float value, float length)
    {
        return value - glm::floor(value / length) * length;
    }

    //////////////////////////////////////////////////////////////////////////

    // ping-pongs the value in range [0, 1]
    inline float ping_pong(float value, float length)
    {
        value = repeat(value, length * 2.0f);
        return length - glm::abs(value - length);
    }

    //////////////////////////////////////////////////////////////////////////

    constexpr float EpsFloatValue = 0.0001f;
    inline bool eps_equals(float lhs, float rhs) { return glm::epsilonEqual(lhs, rhs, EpsFloatValue); }
    inline bool eps_equals_zero(float value) { return glm::epsilonEqual(value, 0.0f, EpsFloatValue);  }

    //////////////////////////////////////////////////////////////////////////

    inline long long distance2i(const glm::ivec2& lhs, const glm::ivec2& rhs)
    {
        const long long dx = (rhs.x - lhs.x);
        const long long dy = (rhs.y - lhs.y);
        return (dx * dx) + (dy * dy);
    }

    //////////////////////////////////////////////////////////////////////////

    template<typename TVec>
    inline TVec move_towards(const TVec& current, const TVec& target, float maxDelta)
    {
        if (maxDelta <= 0.0f) return current;

        const TVec direction = target - current;

        float sqDistanceToTarget = glm::length2(direction);
        if (eps_equals_zero(sqDistanceToTarget) || (sqDistanceToTarget <= (maxDelta * maxDelta)))
        {
            return target;
        }
        float distanceToTarget = std::sqrt(sqDistanceToTarget);
        return current + (direction / distanceToTarget) * maxDelta;
    }

    //////////////////////////////////////////////////////////////////////////

} // namespace cxx