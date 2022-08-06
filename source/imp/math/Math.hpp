#pragma once

#include <cmath>
#include <numbers>

#include <fcl/fcl.h>

namespace imp::math
{

/**
 * @brief Returns the unit quaternion rotation that rotates vector from to vector to.
 *
 * @param from Normalized vector.
 * @param to Normalized vector.
 * @return fcl::Quaternionf The unit quaternion rotation that rotates vector from to vector to.
 */
fcl::Quaternionf fromToAsQuat(const fcl::Vector3f & from, const fcl::Vector3f & to);

inline fcl::Quaternionf lerp(const fcl::Quaternionf & a, const fcl::Quaternionf & b, float alpha)
{
    return a.slerp(alpha, b);
}

inline fcl::Vector3f lerp(const fcl::Vector3f & a, const fcl::Vector3f & b, float alpha)
{
    return alpha * b + (1.0f - alpha) * a;
}

template <typename value_t> value_t sdiv(const value_t a, const value_t b)
{
    return (a + b - 1) / b;
}

} // namespace imp::math