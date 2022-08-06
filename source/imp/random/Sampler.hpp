#pragma once

#include <chrono>
#include <cmath>
#include <fcl/fcl.h>
#include <numbers>
#include <random>

#include "imp/math/Math.hpp"
#include "imp/Settings.hpp"
#include "imp/Configuration.hpp"

namespace imp::random
{

namespace _implementation
{

/**
 * @brief Class that capsules various sampling methods for motion planning, such as
 * limited random rotations. It is intended to be used in a multithreaded environment.
 * Use imp::random::Sampler() to get the thread local instance of the sampler. The seed is
 * ensured to be different for each instance.
 *
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class Sampler
{
    /////////
    // data
    /////////
private:
    std::mt19937 _random_engine;
    std::uniform_real_distribution<float> _distribution;
    std::uniform_int_distribution<unsigned int> _seed_distribution;
    std::uniform_int_distribution<size_t> _coin_distribution;

    /////////
    // constructors
    /////////
public:
    Sampler() : _coin_distribution(0, 1){};

    Sampler(unsigned int seed) : _random_engine{std::mt19937(seed)}, _coin_distribution(0, 1) {}

    /////////
    // methods
    /////////
public:
    inline unsigned int seed() { return _seed_distribution(_random_engine); }

    inline unsigned int coin() { return _coin_distribution(_random_engine); }

    inline float rand() { return _distribution(_random_engine); }

    fcl::Vector3f randPointOnUnitSphere();

    fcl::Vector3f randUniformPointInUnitSphere(float min_radius = 0.0f);

    fcl::Quaternionf randLimitUnitRotation(float angle, float min_angle = 0.0f);

    inline fcl::Quaternionf randUnitQuaternion() { return randLimitUnitRotation(1.0f); }

    fcl::Quaternionf randUniformUnitQuaternion();

    fcl::Vector3f randUniformInCircle(float radius);

    fcl::Vector3f randUniformInCone(float angle, float z_offset = 0.0f, float height_scale = 1.0f);

    imp::Configuration randConfigurationArround(const imp::Configuration & base, float radius,
                                                float angle);

    imp::Configuration randConfigurationArroundMinimumDistance(const imp::Configuration & base,
                                                               float radius, float angle,
                                                               float min_radius, float min_angle);

    imp::Configuration randTargetConfigurationChange(const imp::Configuration & start,
                                                     const imp::Configuration & end);

    static Sampler & get();
};

} // namespace _implementation

/**
 * @brief Get the sampler instance for the current thread. Do not use _implementation::Sampler
 * directly!
 *
 * @return _implementation::Sampler& The sampler instance.
 */
inline _implementation::Sampler & Sampler() { return _implementation::Sampler::get(); }

} // namespace imp::random