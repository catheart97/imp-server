#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "fcl/fcl.h"

namespace imp::json
{

class JSONable
{
public:
    virtual ~JSONable() = default;
    virtual std::string toJSON() const = 0;
};

template <typename value_t>
inline std::string __makeJSON(const value_t * v) requires(!std::derived_from<value_t, JSONable>)
{
    std::stringstream ss;
    ss << *v;
    return ss.str();
}

template <> inline std::string __makeJSON(const fcl::Vector3f * v)
{
    std::stringstream ss;
    ss << "[";
    ss << v->x() << ",";
    ss << v->y() << ",";
    ss << v->z();
    ss << "]";
    return ss.str();
}

template <> inline std::string __makeJSON(const fcl::Quaternionf * v)
{
    std::stringstream ss;
    ss << "[";
    ss << v->w() << ",";
    ss << v->x() << ",";
    ss << v->y() << ",";
    ss << v->z();
    ss << "]";
    return ss.str();
}

template <> inline std::string __makeJSON(const fcl::Triangle * v)
{
    std::stringstream ss;
    ss << "[";
    ss << v->operator[](0) << "," << v->operator[](1) << "," << v->operator[](2);
    ss << "]";
    return ss.str();
}

inline std::string __makeJSON(const imp::json::JSONable * v)
{
    std::stringstream ss;
    ss << v->toJSON();
    return ss.str();
}

template <typename data_t> inline std::string __makeJSON(const std::vector<data_t> * v)
{
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < v->size(); ++i)
    {
        auto & val = v->operator[](i);
        ss << (std::derived_from<decltype(val), imp::json::JSONable>
                   ? __makeJSON((imp::json::JSONable *)(&val))
                   : __makeJSON(&val));
        if (i != v->size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

template <typename data_t> inline std::string __makeJSON(const std::shared_ptr<data_t> * v)
{
    std::stringstream ss;

    if (*v)
    {
        auto & val = *(*v);
        ss << (std::derived_from<decltype(val), imp::json::JSONable>
                   ? __makeJSON((imp::json::JSONable *)(&val))
                   : __makeJSON(&val));
    }
    else
    {
        ss << "null";
    }

    return ss.str();
}

template <typename data_t> inline std::string __makeJSON(const std::optional<data_t> * v)
{
    std::stringstream ss;

    if (v->has_value())
    {
        auto & val = v->value();
        ss << (std::derived_from<decltype(val), imp::json::JSONable>
                   ? __makeJSON((imp::json::JSONable *)(&val))
                   : __makeJSON(&val));
    }
    else
    {
        ss << "null";
    }

    return ss.str();
}

#define JSONP(obj)                                                                                 \
    ss << (std::derived_from<decltype(obj), imp::json::JSONable>                                   \
               ? imp::json::__makeJSON((imp::json::JSONable *)(&obj))                              \
               : imp::json::__makeJSON(&obj));

#define JSON(obj)                                                                                  \
    ss << ("\"" #obj "\":");                                                                       \
    JSONP(obj)

#define JSONN(name, obj)                                                                           \
    ss << ("\"" #name "\":");                                                                      \
    JSONP(obj)

#define JSONND(name, obj)                                                                          \
    ss << ("\"" #name "\":");                                                                      \
    JSONP(obj) ss << ",";

#define JSOND(obj) JSON(obj) ss << ",";

#define __START_JSON()                                                                             \
    std::stringstream ss;                                                                          \
    ss << "{";
#define __END_JSON()                                                                               \
    ss << "}";                                                                                     \
    return ss.str();

#define JSON_IMPL(CONTENT)                                                                         \
public:                                                                                            \
    inline std::string toJSON() const override { __START_JSON() CONTENT __END_JSON() }

} // namespace imp::json