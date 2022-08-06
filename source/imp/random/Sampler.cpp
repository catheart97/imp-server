#include "imp/random/Sampler.hpp"

namespace imp
{

thread_local std::unique_ptr<imp::random::_implementation::Sampler> __imp_sampler{nullptr};
std::mutex __imp_sampler_lock;

} // namespace imp

/// thread local singleton
imp::random::_implementation::Sampler & imp::random::_implementation::Sampler::get()
{
    if (__imp_sampler) return *__imp_sampler.get();
    std::lock_guard<std::mutex> guard(__imp_sampler_lock);
    __imp_sampler = std::make_unique<imp::random::_implementation::Sampler>(
        static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count()));
    return *__imp_sampler.get();
}

fcl::Vector3f imp::random::_implementation::Sampler::randPointOnUnitSphere()
{
    float theta{2.0f * std::numbers::pi_v<float> * rand()};
    float phi{std::acosf(2.0f * rand() - 1.0f)};

    fcl::Vector3f point;
    point.x() = std::sinf(phi) * std::cosf(theta);
    point.y() = std::sinf(phi) * std::sinf(theta);
    point.z() = std::cosf(phi);

    return point;
}

fcl::Vector3f imp::random::_implementation::Sampler::randUniformPointInUnitSphere(float min_radius)
{
    auto position{randPointOnUnitSphere()};
    float radius = std::cbrtf(rand()) * (1.0f - min_radius) + min_radius;
    return radius * position;
}

fcl::Quaternionf imp::random::_implementation::Sampler::randLimitUnitRotation(float angle,
                                                                              float min_angle)
{
    fcl::Vector3f axis{randPointOnUnitSphere()};
    float eta{std::sqrtf(rand()) * (angle - min_angle) + min_angle};
    eta /= 2.0f;

    fcl::Quaternionf quaternion;
    quaternion.w() = std::cosf(eta);
    quaternion.x() = std::sinf(eta) * axis.x();
    quaternion.y() = std::sinf(eta) * axis.y();
    quaternion.z() = std::sinf(eta) * axis.z();

    return quaternion;
}

fcl::Quaternionf imp::random::_implementation::Sampler::randUniformUnitQuaternion()
{
    // from graphics gems
    float s = rand();
    float sigma1 = std::sqrtf(1.0f - s);
    float sigma2 = std::sqrtf(s);
    float theta1 = 2 * std::numbers::pi_v<float> * rand();
    float theta2 = 2 * std::numbers::pi_v<float> * rand();

    fcl::Quaternionf quaternion;
    quaternion.w() = std::cosf(theta2) * sigma2;
    quaternion.x() = std::sinf(theta1) * sigma1;
    quaternion.y() = std::cosf(theta1) * sigma1;
    quaternion.z() = std::sinf(theta2) * sigma2;

    return quaternion;
}

fcl::Vector3f imp::random::_implementation::Sampler::randUniformInCircle(float radius)
{
    float r = std::sqrtf(rand()) * radius;
    float theta = rand() * 2.0f * std::numbers::pi_v<float>;

    fcl::Vector3f position;
    position.x() = r * std::cos(theta);
    position.y() = r * std::sin(theta);
    position.z() = 0.0f;

    return position;
}

fcl::Vector3f imp::random::_implementation::Sampler::randUniformInCone(float angle, float z_offset,
                                                                       float height_scale)
{
    float m = std::tan(180.0f * angle / std::numbers::pi_v<float>);

    float height = std::sqrtf(rand()) * height_scale;
    float radius = m * height;

    auto position{randUniformInCircle(radius)};
    position.z() = height + z_offset;

    return position;
}

imp::Configuration
imp::random::_implementation::Sampler::randConfigurationArround(const imp::Configuration & base,
                                                                float radius, float angle)
{
    return imp::Configuration{base.Position + randUniformPointInUnitSphere() * radius,
                              base.Rotation * randLimitUnitRotation(angle)};
}

imp::Configuration imp::random::_implementation::Sampler::randConfigurationArroundMinimumDistance(
    const imp::Configuration & base, float radius, float angle, float min_radius, float min_angle)
{
    return imp::Configuration{base.Position +
                                  randUniformPointInUnitSphere(min_radius / radius) * radius,
                              base.Rotation * randLimitUnitRotation(angle, min_angle)};
}

imp::Configuration imp::random::_implementation::Sampler::randTargetConfigurationChange(
    const imp::Configuration & start, const imp::Configuration & end)
{
    fcl::Vector3f connection = (end.Position - start.Position).normalized();

    auto cone_rotation = imp::math::fromToAsQuat( //
        fcl::Vector3f{0.0f, 0.0f, 1.0f},          //
        connection                                //
    );
    auto cone = Sampler().randUniformInCone(  //
        50.0f,                                //
        -REPAIR_MAX_POSITIONAL_DISTANCE,      //
        2.0f * REPAIR_MAX_POSITIONAL_DISTANCE //
    );

    fcl::Vector3f positional_change = cone_rotation.toRotationMatrix() * cone;
    auto rotational_change = Sampler().randLimitUnitRotation(REPAIR_MAX_ROTATIONAL_DISTANCE);

    imp::Configuration config;
    config.Position = positional_change;
    config.Rotation = rotational_change;
    return config;
}
