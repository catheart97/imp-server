#include "imp/ObjectManager.hpp"
#include "imp/EST.hpp" 
#include "imp/WorldTree.hpp"

void imp::ObjectManager::clear()
{
    _movable_bvhs.clear();
    _ests.clear();
    _static_bvhs.clear();
    _static_transforms.clear();
    _static_collision_objects.clear();
}

size_t imp::ObjectManager::add(bool movable,                           //
                               std::vector<fcl::Vector3f> & vertices,  //
                               std::vector<fcl::Triangle> & triangles, //
                               const Configuration & config)
{
    auto model = std::make_shared<fcl::BVHModel<fcl::OBBf>>();
    model->beginModel();
    model->addSubModel(vertices, triangles);
    model->endModel();

    if (movable)
    {
        std::lock_guard<std::mutex> guard(_movable_mutex);
        size_t movable_next = movableNext();
        if (movable_next == _movable_bvhs.size())
        {
            _movable_bvhs.emplace_back(model);
            _ests.emplace_back(std::make_unique<imp::EST>(*this, movable_next));
            _wtrees.emplace_back(std::make_shared<imp::WorldTree>(movable_next));
            return _movable_bvhs.size() - 1;
        }
        else
        {
            _movable_bvhs[movable_next] = model;
            _ests[movable_next] = std::make_unique<imp::EST>(*this, movable_next);
            _wtrees[movable_next] = std::make_shared<imp::WorldTree>(movable_next);
            return movable_next;
        }
    }
    else
    {
        std::lock_guard<std::mutex> guard(_static_mutex);
        size_t static_next = staticNext();
        if (static_next == _static_bvhs.size())
        {
            _static_bvhs.emplace_back(model);

            _static_transforms.emplace_back(config);
            fcl::Transform3f fcl_transform{toFCL(config)};

            auto static_collision_object =
                std::make_shared<fcl::CollisionObjectf>(model, fcl_transform);
            _static_collision_objects.emplace_back(static_collision_object);
            return _static_bvhs.size() - 1;
        }
        else
        {
            _static_bvhs[static_next] = model;
            _static_transforms[static_next] = config;
            fcl::Transform3f fcl_transform{toFCL(config)};
            _static_collision_objects[static_next] =
                std::make_shared<fcl::CollisionObjectf>(model, fcl_transform);
            return static_next;
        }
    }
}

bool imp::ObjectManager::collides(size_t movable_id, //
                                  const Configuration & configuration)
{
    auto obj =
        std::make_shared<fcl::CollisionObjectf>(_movable_bvhs[movable_id], toFCL(configuration));

    // collision test // todo ! use fcl::manager
    int collision_count = 0;
#pragma omp parallel for reduction(+ : collision_count)
    for (int i = 0; i < _static_bvhs.size(); ++i)
    {
        auto collision_object = _static_collision_objects[i].get();
        if (collision_object)
        {
            fcl::CollisionRequestf request;
            fcl::CollisionResultf result;
            fcl::collide(obj.get(), collision_object, request, result);
            collision_count += result.isCollision();
        }
    }

    return bool(collision_count);
}

bool imp::ObjectManager::isCollisionFreePath(size_t movable_id, Configuration start,
                                             Configuration end)
{
    int collision_count = 0;
    const size_t POSITIONAL_STEPS{
        size_t((end.Position - start.Position).norm() / PATH_VERIFICATION_POSITIONAL_STEP)};
    const size_t ROTATIONAL_STEPS{
        size_t(end.Rotation.angularDistance(start.Rotation) / PATH_VERIFICATION_ROTATIONAL_STEP)};

    const size_t STEPS{std::max(POSITIONAL_STEPS, ROTATIONAL_STEPS) + 1};

#pragma omp parallel for reduction(+ : collision_count)
    for (int i = 0; i < STEPS + 1; ++i)
    {
        float delta{(1.0f / STEPS) * i};

        auto position = imp::math::lerp(start.Position, end.Position, delta);
        auto rotation = imp::math::lerp(start.Rotation, end.Rotation, delta);

        collision_count += collides(movable_id, Configuration{position, rotation});
    }

    return !bool(collision_count);
}

std::tuple<bool, float, imp::Configuration>
imp::ObjectManager::newLocalClosestWorker(const size_t MOVABLE_ID, const size_t NUM_LOCAL_SAMPLES,
                                          const Configuration & start, const Configuration & end)
{
    using namespace imp::random;

    auto positional_center{(end.Position - start.Position) / 2 + start.Position};
    // auto positional_center{start.position};
    auto rotational_center{imp::math::lerp(start.Rotation, end.Rotation, .5f)};

    std::tuple<bool, float, Configuration> closest{false, std::numeric_limits<float>::max(),
                                                   Configuration()};
    for (size_t i = 0; i < NUM_LOCAL_SAMPLES; ++i)
    {
        size_t sample_iteration = 0;
        Configuration config;
        do
        {
            Configuration configuration_change{Sampler().randTargetConfigurationChange(start, end)};
            config.Position = positional_center + configuration_change.Position;
            config.Rotation = rotational_center * configuration_change.Rotation; // todo : ?
        } while (!isCollisionFreePath(MOVABLE_ID, start, config) &&
                 sample_iteration++ < REPAIR_MAX_SAMPLE_TRIES);

        if (sample_iteration > REPAIR_MAX_SAMPLE_TRIES) continue;

        float distance = Distance(config, end);
        if (distance < std::get<1>(closest)) closest = std::make_tuple(true, distance, config);
    }

    return closest;
}

std::pair<bool, imp::Configuration>
imp::ObjectManager::newLocalClosest(const size_t MOVABLE_ID,     //
                                    const Configuration & start, //
                                    const Configuration & end)
{
    const auto SAMPLES_PER_TASK = math::sdiv<size_t>(REPAIR_NUM_SAMPLES, MAX_OMP_THREADS);

    std::array<std::future<std::tuple<bool, float, Configuration>>, MAX_OMP_THREADS> tasks;
    for (size_t i = 0; i < MAX_OMP_THREADS; ++i)
    {
        tasks[i] = std::async(std::launch::async, &imp::ObjectManager::newLocalClosestWorker, this,
                              MOVABLE_ID, SAMPLES_PER_TASK, start, end);
    }

    std::tuple<bool, float, Configuration> closest{false, std::numeric_limits<float>::max(),
                                                   Configuration()};
    for (auto & t : tasks)
    {
        auto res{t.get()};
        if (std::get<0>(res) && std::get<1>(res) < std::get<1>(closest)) closest = res;
    }

    return std::make_pair(std::get<0>(closest), std::get<2>(closest));
}

fcl::Transform3f imp::ObjectManager::toFCL(const Configuration & transform)
{
    fcl::Transform3f result;

    result.linear() = transform.Rotation.matrix();
    result.translation() = transform.Position;

    return result;
}

size_t imp::ObjectManager::movableNext()
{
    size_t result = 0;
    for (; result < _movable_bvhs.size(); ++result)
    {
        if (!_movable_bvhs[result]) break;
    }
    return result;
}

size_t imp::ObjectManager::staticNext()
{
    size_t result = 0;
    for (; result < _static_bvhs.size(); ++result)
    {
        if (!_static_bvhs[result]) break;
    }
    return result;
}

void imp::ObjectManager::remove(bool movable, size_t index)
{
    if (movable)
    {
        if (index >= _movable_bvhs.size()) return;
        std::lock_guard<std::mutex> guard(_movable_mutex);
        _movable_bvhs[index] = nullptr;
        _ests[index] = nullptr;
    }
    else
    {
        if (index >= _static_bvhs.size()) return;
        std::lock_guard<std::mutex> guard(_static_mutex);
        _static_bvhs[index] = nullptr;
        _static_collision_objects[index] = nullptr;
    }
}

std::string imp::ObjectManager::toJSON() const
{
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < _static_collision_objects.size(); ++i)
    {
        ss << "{";
        auto & so{_static_collision_objects[i]};
        auto & sb{_static_bvhs[i]};

        fcl::Vector3f Position = so->getTranslation();
        fcl::Matrix3f rot = so->getRotation();
        fcl::AngleAxisf angle_axis(rot);
        float Angle = angle_axis.angle();
        fcl::Vector3f Axis = angle_axis.axis();

        JSOND(Angle)
        JSOND(Axis)
        JSOND(Position)

        ss << "\"Vertices\":[";
        for (size_t v = 0; v < sb->num_vertices; ++v)
        {
            auto & vert = sb->vertices[v];
            JSONP(vert);
            if (v != sb->num_vertices - 1) ss << ",";
        }
        ss << "],\"Triangles\":[";
        for (size_t t = 0; t < sb->num_tris; ++t)
        {
            auto & tr = sb->tri_indices[t];
            JSONP(tr)
            if (t != sb->num_tris - 1) ss << ",";
        }
        ss << "]}";
        if (i != _static_collision_objects.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}