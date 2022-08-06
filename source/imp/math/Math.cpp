#include "imp/math/Math.hpp"

fcl::Quaternionf imp::math::fromToAsQuat(const fcl::Vector3f & from, const fcl::Vector3f & to)
{
    auto axis = from.cross(to).normalized();
    auto eta = std::acosf(from.dot(to));

    float sineta = std::sinf(eta / 2.0f);
    float coseta = std::cosf(eta / 2.0f);

    fcl::Quaternionf rotation;
    rotation.x() = sineta * axis.x();
    rotation.y() = sineta * axis.y();
    rotation.z() = sineta * axis.z();
    rotation.w() = coseta;

    return rotation;
}