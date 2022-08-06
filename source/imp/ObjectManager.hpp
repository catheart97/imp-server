#pragma once

#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "fcl/fcl.h"
#include "fcl/math/motion/interp_motion.h"

#include "imp/Configuration.hpp"
#include "imp/Settings.hpp"
#include "imp/math/Math.hpp"
#include "imp/random/Sampler.hpp"
#include "imp/json/JSON.hpp"

namespace imp
{

class EST;
class WorldTree;

/**
 * @brief Manager for scene objects.
 *
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class ObjectManager : public imp::json::JSONable
{
    /////////
    // data
    /////////
private:
    // movable data
    std::mutex _movable_mutex;
    std::vector<std::shared_ptr<fcl::BVHModel<fcl::OBBf>>> _movable_bvhs;
    std::vector<std::unique_ptr<EST>> _ests;
    std::vector<std::shared_ptr<WorldTree>> _wtrees;

    // static data
    std::mutex _static_mutex;
    std::vector<std::shared_ptr<fcl::BVHModel<fcl::OBBf>>> _static_bvhs;
    std::vector<Configuration> _static_transforms;
    std::vector<std::shared_ptr<fcl::CollisionObjectf>> _static_collision_objects;

    /////////
    // properties
    /////////
public:
    /**
     * @brief Checks if the given id is a valid movable id.
     */
    bool hasMovable(size_t id) { return _movable_bvhs.size() > id; }

    inline std::unique_ptr<EST> & est(size_t id) { return _ests[id]; }
    inline std::shared_ptr<WorldTree> & wtree(size_t id) { return _wtrees[id]; }

    /**
     * @brief Checks if the given id is a valid movable id.
     */
    bool hasStatic(size_t id) { return _static_bvhs.size() > id; }

    /**
     * @brief Resets this instance.
     */
    void clear();

    /**
     * @brief Removes the object by setting it to nullptr.
     *
     * @param movable
     * @param index
     */
    void remove(bool movable, size_t index);

    /**
     * @brief Adds the object to the collection.
     *
     * @param movable Whether its an environment (static) or non-environment (movable) object.
     * @param vertices The vertices of the object.
     * @param triangles The triangles of the object.
     * @return size_t The id of the created object.
     */
    size_t add(bool movable,                           //
               std::vector<fcl::Vector3f> & vertices,  //
               std::vector<fcl::Triangle> & triangles, //
               const Configuration & config);

    /**
     * @brief Runs a simple collision query for the movable object with the given id and
     * configuration against ALL environment objects.
     *
     * @param movable_id
     * @param configuration
     */
    bool collides(size_t movable_id, const Configuration & configuration);

    /**
     * @brief Checks if the linear interpolated path from start to end is collision free.
     *
     * @param movable_id
     * @param start
     * @param end
     * @return true
     * @return false
     */
    bool isCollisionFreePath(size_t movable_id, Configuration start, Configuration end);

    /**
     * @brief Generate a new local configuration.
     *
     * @param MOVABLE_ID
     * @param start
     * @param end
     * @return Configuration
     */
    std::pair<bool, Configuration> newLocalClosest(const size_t MOVABLE_ID,     //
                                                   const Configuration & start, //
                                                   const Configuration & end);

    std::string toJSON() const override;

    float bounding(const size_t MOVABLE_ID)
    {
        return _movable_bvhs[MOVABLE_ID]->aabb_radius;
    }

protected:
    /**
     * @brief worker for newLocalClosest
     *
     * @param MOVABLE_ID
     * @param NUM_LOCAL_SAMPLES
     * @param start
     * @param end
     * @return std::tuple<bool, float, imp::Configuration>
     */
    std::tuple<bool, float, imp::Configuration>
    newLocalClosestWorker(const size_t MOVABLE_ID, const size_t NUM_LOCAL_SAMPLES,
                          const Configuration & start, const Configuration & end);

    size_t movableNext();

    size_t staticNext();

    /////////
    // methods
    /////////
public:
    fcl::Transform3f toFCL(const Configuration & transform);
};

} // namespace imp