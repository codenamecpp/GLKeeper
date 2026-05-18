#pragma once

//////////////////////////////////////////////////////////////////////////

#include "auxmath.h"

//////////////////////////////////////////////////////////////////////////

namespace cxx
{

    // thin float angle wrapper
    struct angle_t final
    {
    public:
        enum class units { degrees, radians };

        constexpr angle_t() = default;
        explicit angle_t(float angleValue, units angleUnits)
        {
            set_angle(angleValue, angleUnits);
        }
        // convert angle value to angle_t
        static angle_t from_radians(float angleValue) { return angle_t { angleValue, units::radians }; }
        static angle_t from_degrees(float angleValue) { return angle_t { angleValue, units::degrees }; }

        // convert angle_t to angle value
        inline constexpr float to_radians() const { return mAngleRadians; }
        inline float to_degrees() const 
        { 
            return glm::degrees(mAngleRadians); 
        }

        inline float to_degrees_normalize_180() const { return wrap_angle_to_180(to_degrees()); }
        inline float to_degrees_normalize_360() const { return wrap_angle_to_180(to_degrees()); }
        inline float to_radians_normalize_pi() const { return wrap_angle_to_pi(to_radians()); }
        inline float to_radians_normalize_2pi() const { return wrap_angle_to_2pi(to_radians()); }

        // init angle
        inline void set_angle(float angleValue, units angleUnits)
        {
            if (angleUnits == units::radians)
            {
                mAngleRadians = angleValue;   
            }
            else
            {
                mAngleRadians = glm::radians(angleValue);
            }
        }

        // get sin and cos for angle
        // @param outSin: Output sin value
        // @param outCos: Output cos value
        inline void get_sin_cos(float& outSin, float& outCos) const
        {
            outSin = sinf(mAngleRadians);
            outCos = cosf(mAngleRadians);
        }

        // clear angle
        inline void set_to_zero()
        {
            mAngleRadians = 0.0f;
        }

        // test whether angle is nearly zero
        inline bool is_zero() const { return cxx::eps_equals(mAngleRadians, 0.0f); }

        // operators
        inline angle_t operator + (angle_t rhs) const { return from_radians(mAngleRadians + rhs.mAngleRadians); }
        inline angle_t operator - (angle_t rhs) const { return from_radians(mAngleRadians - rhs.mAngleRadians); }
        inline angle_t operator - () const { return from_radians(-mAngleRadians); }
        inline angle_t operator * (float scalar) const { return from_radians(mAngleRadians * scalar); }
        inline angle_t operator / (float scalar) const { return from_radians(mAngleRadians / scalar); }

        inline angle_t& operator += (angle_t rhs) { mAngleRadians += rhs.mAngleRadians; return *this; }
        inline angle_t& operator -= (angle_t rhs) { mAngleRadians -= rhs.mAngleRadians; return *this; }
        inline angle_t& operator *= (float scalar) { mAngleRadians *= scalar; return *this; }
        inline angle_t& operator /= (float scalar) { mAngleRadians /= scalar; return *this; }

        inline bool operator == (angle_t rhs) const { return cxx::eps_equals(mAngleRadians, rhs.mAngleRadians); }
        inline bool operator != (angle_t rhs) const { return !(*this == rhs); }

    public:
        float mAngleRadians = 0.0f;
    };

} // namespace cxx