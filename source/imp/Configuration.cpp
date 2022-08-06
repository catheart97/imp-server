#include "imp/Configuration.hpp"

float imp::Distance(const Configuration & a, const Configuration & b)
{
    float pos_distance = PDistance(a, b);
    float rot_distance = RDistance(a, b);
    return std::sqrt(pos_distance * pos_distance + rot_distance * rot_distance);
}

float imp::Distance(const Configuration & a, const Configuration & b, float rotation_scale)
{
    float pos_distance = PDistance(a, b);
    float rot_distance = RDistance(a, b) * rotation_scale;
    return std::sqrt(pos_distance * pos_distance + rot_distance * rot_distance);
}

float imp::PDistance(const Configuration & a, const Configuration & b)
{
    return (a.Position - b.Position).norm();
}

float imp::RDistance(const Configuration & a, const Configuration & b)
{
    return (a.Rotation.angularDistance(b.Rotation)) / 360.f * 2.0f * std::numbers::pi_v<float>;
}

imp::DistancePair imp::PairDistance(const Configuration & a, const Configuration & b)
{
    float pos_distance = PDistance(a, b);
    float rot_distance = RDistance(a, b);
    return std::make_pair(pos_distance, rot_distance);
}

// std::string imp::Configuration::toJSON() const
// {
//     std::stringstream ss;
//     ss << "{\"Position\":{";
//     ss << "\"x\":" << Position.x() << ",";
//     ss << "\"y\":" << Position.y() << ",";
//     ss << "\"z\":" << Position.z();
//     ss << "}"
//        << ","; // position end

//     ss << "\"Rotation\":{";
//     ss << "\"x\":" << Rotation.x() << ",";
//     ss << "\"y\":" << Rotation.y() << ",";
//     ss << "\"z\":" << Rotation.z() << ",";
//     ss << "\"w\":" << Rotation.w();
//     ss << "}}"; // rotation end

//     return ss.str();
// }