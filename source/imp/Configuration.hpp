#pragma once

#include <numbers>

#include "fcl/fcl.h"
#include "imp/json/JSON.hpp"

namespace imp
{

/**
 * @brief Struct representing a motion planning configuration in 3D-Space.
 *
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
struct Configuration : public json::JSONable
{
    /////////
    // json
    /////////
public:
    JSON_IMPL(JSOND(Position) JSON(Rotation))

    /////////
    // data
    /////////
public:
    fcl::Vector3f Position{0.f, 0.f, 0.f};
    fcl::Quaternionf Rotation{fcl::Quaternionf::Identity()};

    /////////
    // constructors
    /////////
public:
    Configuration() = default;
    Configuration(const fcl::Vector3f & position) : Position{position} {}
    Configuration(const fcl::Quaternionf & rotation) : Rotation{rotation} {}
    Configuration(const fcl::Vector3f & position, const fcl::Quaternionf & rotation)
        : Position{position}, Rotation{rotation}
    {}
    /////////
    // properties
    /////////
public:
    float & operator[](const size_t & i)
    {
        switch (i)
        {
            case 0: return Position.x();
            case 1: return Position.y();
            case 2: return Position.z();
            case 3: return Rotation.w();
            case 4: return Rotation.x();
            case 5: return Rotation.y();
            default: return Rotation.z();
        }
    }

    float operator[](const size_t & i) const { return (*const_cast<Configuration *>(this))[i]; }
};

using DistancePair = std::pair<float, float>;
float Distance(const imp::Configuration & a, const Configuration & b);
float PDistance(const Configuration & a, const Configuration & b);
float RDistance(const Configuration & a, const Configuration & b);
float Distance(const Configuration & a, const Configuration & b, float rotation_scale);
DistancePair PairDistance(const Configuration & a, const Configuration & b);

} // namespace imp