#include "imp/server/ServerController.hpp"
#include "imp/WorldTree.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::createIMPL(
    const imp::server::ObjectCreationRequest::Wrapper & req_dto)
{
    OATPP_LOGI("REQUEST ", " /create")

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_create.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    fcl::Quaternionf rotation;
    rotation.w() = req_dto->rotation_w;
    rotation.x() = req_dto->rotation_x;
    rotation.y() = req_dto->rotation_y;
    rotation.z() = req_dto->rotation_z;

    fcl::Vector3f position;
    position.x() = req_dto->position_x;
    position.y() = req_dto->position_y;
    position.z() = req_dto->position_z;

    auto vertices = req_dto->vertices.get();
    auto triangles = req_dto->triangles.get();

    if (vertices->size() % 3 != 0)
        return createResponse(Status::CODE_400, "Invalid number of vertices points (x, y, z).");

    auto vertices_begin = vertices->begin();
    std::vector<fcl::Vector3f> vertices_Vector3f(vertices->size() / 3);
    for (size_t i = 0; i < vertices_Vector3f.size(); ++i)
    {
        vertices_Vector3f[i].x() = *(vertices_begin++);
        vertices_Vector3f[i].y() = *(vertices_begin++);
        vertices_Vector3f[i].z() = *(vertices_begin++);
    }

    if (triangles->size() % 3 != 0)
        return createResponse(Status::CODE_400, "Invalid number of triangle indices (a, b, c).");

    std::vector<fcl::Triangle> triangles_Triangle(triangles->size() / 3);
    auto triangles_begin = triangles->begin();
    for (size_t i = 0; i < triangles_Triangle.size(); ++i)
    {
        triangles_Triangle[i][0] = static_cast<size_t>(*(triangles_begin++));
        triangles_Triangle[i][1] = static_cast<size_t>(*(triangles_begin++));
        triangles_Triangle[i][2] = static_cast<size_t>(*(triangles_begin++));
    }

    auto id = _manager.add(req_dto->movable, vertices_Vector3f, triangles_Triangle,
                           Configuration{position, rotation});
    auto res_dto = ObjectCreationResult::createShared();
    res_dto->movable = req_dto->movable;
    res_dto->id = id;

    std::stringstream ss;
    ss << " /create | Created object with ID = " << id << " MOVABLE = " << res_dto->movable;
    OATPP_LOGI("RESULT ", ss.str().c_str())

    return createDtoResponse(Status::CODE_202, res_dto);
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::collidesIMPL(const imp::server::CollisionRequest::Wrapper & req_dto)
{
    OATPP_LOGV("REQUEST ", " /collides")

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_collides.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    const auto MOVABLE_ID{req_dto->movable_id};

    if (!_manager.hasMovable(MOVABLE_ID))
    {
        OATPP_LOGE("REQUEST ", " /collides | Collision request with invalid id.")
        return createResponse(Status::CODE_400, "Invalid JSON argument! ID does not exist!");
    }

    fcl::Quaternionf rotation;
    rotation.w() = req_dto->rotation_w;
    rotation.x() = req_dto->rotation_x;
    rotation.y() = req_dto->rotation_y;
    rotation.z() = req_dto->rotation_z;

    fcl::Vector3f position;
    position.x() = req_dto->position_x;
    position.y() = req_dto->position_y;
    position.z() = req_dto->position_z;

    auto res_dto = CollisionResult::createShared();
    res_dto->is_colliding = _manager.collides(MOVABLE_ID, Configuration{position, rotation});

    return createDtoResponse(Status::CODE_200, res_dto);
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::collides_anyIMPL(
    const imp::server::MultipleCollisionRequest::Wrapper & req_dto)
{
    const auto MOVABLE_ID = req_dto->movable_id;

    OATPP_LOGV("REQUEST ", " /collides-any")

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_collides-any.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    if (!_manager.hasMovable(MOVABLE_ID))
    {
        OATPP_LOGE("REQUEST ", " /collides-any | Collision request with invalid id.")
        return createResponse(Status::CODE_400, "Invalid JSON argument! ID does not exist!");
    }

    auto rotation_w = req_dto->rotation_w.get();
    auto rotation_x = req_dto->rotation_x.get();
    auto rotation_y = req_dto->rotation_y.get();
    auto rotation_z = req_dto->rotation_z.get();

    auto position_x = req_dto->position_x.get();
    auto position_y = req_dto->position_y.get();
    auto position_z = req_dto->position_z.get();

    auto size{rotation_y->size()};
    if (size != position_x->size() || size != position_y->size() || size != position_z->size() ||
        size != rotation_w->size() || size != rotation_x->size() || size != rotation_y->size() ||
        size != rotation_z->size())
        return createResponse(Status::CODE_400, "Invalid JSON argument! Size mismatch!");

    auto rotation_w_begin{rotation_w->begin()};
    auto rotation_x_begin{rotation_x->begin()};
    auto rotation_y_begin{rotation_y->begin()};
    auto rotation_z_begin{rotation_z->begin()};

    auto position_x_begin{position_x->begin()};
    auto position_y_begin{position_y->begin()};
    auto position_z_begin{position_z->begin()};

    std::vector<Configuration> transforms;
    for (size_t i = 0; i < size; ++i)
    {
        fcl::Quaternionf rotation;
        rotation.w() = *(rotation_w_begin++);
        rotation.x() = *(rotation_x_begin++);
        rotation.y() = *(rotation_y_begin++);
        rotation.z() = *(rotation_z_begin++);

        fcl::Vector3f position;
        position.x() = *(position_x_begin++);
        position.y() = *(position_y_begin++);
        position.z() = *(position_z_begin++);

        transforms.emplace_back(Configuration{position, rotation});
    }

    int collision_count = 0;
#pragma omp parallel for reduction(+ : collision_count)
    for (int i = 0; i < transforms.size(); ++i)
        collision_count += int(_manager.collides(MOVABLE_ID, transforms[i]));

    auto res_dto = CollisionResult::createShared();
    res_dto->is_colliding = bool(collision_count);
    return createDtoResponse(Status::CODE_200, res_dto);
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::new_local_closestIMPL(
    const imp::server::NewLocalClosestRequest::Wrapper & req_dto)
{
    const auto MOVABLE_ID = req_dto->movable_id;

    OATPP_LOGV("REQUEST ", " /new-local-closest")

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_new-local-closest.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    if (!_manager.hasMovable(MOVABLE_ID))
    {
        OATPP_LOGE("REQUEST ", " /new-local-closest | Collision request with invalid id.")
        return createResponse(Status::CODE_400, "Invalid JSON argument! ID does not exist!");
    }

    Configuration start;
    start.Position.x() = req_dto->start_position_x;
    start.Position.y() = req_dto->start_position_y;
    start.Position.z() = req_dto->start_position_z;

    start.Rotation.w() = req_dto->start_rotation_w;
    start.Rotation.x() = req_dto->start_rotation_x;
    start.Rotation.y() = req_dto->start_rotation_y;
    start.Rotation.z() = req_dto->start_rotation_z;

    Configuration end;
    end.Position.x() = req_dto->end_position_x;
    end.Position.y() = req_dto->end_position_y;
    end.Position.z() = req_dto->end_position_z;

    end.Rotation.w() = req_dto->end_rotation_w;
    end.Rotation.x() = req_dto->end_rotation_x;
    end.Rotation.y() = req_dto->end_rotation_y;
    end.Rotation.z() = req_dto->end_rotation_z;

    auto res_pair = _manager.newLocalClosest(MOVABLE_ID, start, end);
    auto res_dto = NewLocalClosestResult::createShared();
    if (!res_pair.first) // first determines if a sample could be found.
    {
        OATPP_LOGW("REQUEST ", " /new-local-closest | COULD NOT FIND A NEW SAMPLE!");

        res_dto->successful = false;

        res_dto->position_x = start.Position.x();
        res_dto->position_y = start.Position.y();
        res_dto->position_z = start.Position.z();

        res_dto->rotation_w = start.Rotation.w();
        res_dto->rotation_x = start.Rotation.x();
        res_dto->rotation_y = start.Rotation.y();
        res_dto->rotation_z = start.Rotation.z();
    }
    else
    {
        auto res = res_pair.second;

        res_dto->successful = true;

        res_dto->position_x = res.Position.x();
        res_dto->position_y = res.Position.y();
        res_dto->position_z = res.Position.z();

        res_dto->rotation_w = res.Rotation.w();
        res_dto->rotation_x = res.Rotation.x();
        res_dto->rotation_y = res.Rotation.y();
        res_dto->rotation_z = res.Rotation.z();
    }
    return createDtoResponse(Status::CODE_200, res_dto);
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::is_collision_free_pathIMPL(
    const imp::server::NewLocalClosestRequest::Wrapper & req_dto)
{
    const auto MOVABLE_ID = req_dto->movable_id;

    OATPP_LOGV("REQUEST ", " /is-collision-free-path")

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_is-collision-free-path.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    if (!_manager.hasMovable(MOVABLE_ID))
    {
        OATPP_LOGE("REQUEST ", " /is-collision-free-path | Collision request with invalid id.")
        return createResponse(Status::CODE_400, "Invalid JSON argument! ID does not exist!");
    }

    Configuration start;
    start.Position.x() = req_dto->start_position_x;
    start.Position.y() = req_dto->start_position_y;
    start.Position.z() = req_dto->start_position_z;

    start.Rotation.w() = req_dto->start_rotation_w;
    start.Rotation.x() = req_dto->start_rotation_x;
    start.Rotation.y() = req_dto->start_rotation_y;
    start.Rotation.z() = req_dto->start_rotation_z;

    Configuration end;
    end.Position.x() = req_dto->end_position_x;
    end.Position.y() = req_dto->end_position_y;
    end.Position.z() = req_dto->end_position_z;

    end.Rotation.w() = req_dto->end_rotation_w;
    end.Rotation.x() = req_dto->end_rotation_x;
    end.Rotation.y() = req_dto->end_rotation_y;
    end.Rotation.z() = req_dto->end_rotation_z;

    auto res = _manager.isCollisionFreePath(MOVABLE_ID, start, end);

    auto res_dto = CollisionResult::createShared();
    res_dto->is_colliding = !res;

    return createDtoResponse(Status::CODE_200, res_dto);
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::path_toIMPL(const imp::server::PathToRequest::Wrapper & req_dto)
{
    OATPP_LOGI("REQUEST ", " /path-to")

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_path-to.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    const auto MOVABLE_ID = req_dto->movable_id;

    fcl::Quaternionf rrotation;
    rrotation.w() = req_dto->start_rotation_w;
    rrotation.x() = req_dto->start_rotation_x;
    rrotation.y() = req_dto->start_rotation_y;
    rrotation.z() = req_dto->start_rotation_z;

    fcl::Vector3f rposition;
    rposition.x() = req_dto->start_position_x;
    rposition.y() = req_dto->start_position_y;
    rposition.z() = req_dto->start_position_z;

    Configuration root_configuration{rposition, rrotation};

    if (_manager.collides(MOVABLE_ID, root_configuration))
        OATPP_LOGE("ERROR", "Initial configurations collide!");

    // preparation phase
    auto rotation_w = req_dto->u_rotations_w.get();
    auto rotation_x = req_dto->u_rotations_x.get();
    auto rotation_y = req_dto->u_rotations_y.get();
    auto rotation_z = req_dto->u_rotations_z.get();

    auto position_x = req_dto->u_positions_x.get();
    auto position_y = req_dto->u_positions_y.get();
    auto position_z = req_dto->u_positions_z.get();

    auto size{rotation_y->size()};
    if (size != position_x->size() || size != position_y->size() || size != position_z->size() ||
        size != rotation_w->size() || size != rotation_x->size() || size != rotation_y->size() ||
        size != rotation_z->size())
    {
        OATPP_LOGE("REQUEST ", " /path-to | Invalid JSON argument!");
        return createResponse(Status::CODE_400, "Invalid JSON argument! Size mismatch!");
    }

    auto rotation_w_begin{rotation_w->begin()};
    auto rotation_x_begin{rotation_x->begin()};
    auto rotation_y_begin{rotation_y->begin()};
    auto rotation_z_begin{rotation_z->begin()};

    auto position_x_begin{position_x->begin()};
    auto position_y_begin{position_y->begin()};
    auto position_z_begin{position_z->begin()};

    std::optional<std::pair<size_t, Configuration>> matchee;
    fcl::Quaternionf rotation;
    fcl::Vector3f position;
    size_t counters = 0;
    for (size_t i = 0; i < size; ++i)
    {
        rotation.w() = *(rotation_w_begin++);
        rotation.x() = *(rotation_x_begin++);
        rotation.y() = *(rotation_y_begin++);
        rotation.z() = *(rotation_z_begin++);

        position.x() = *(position_x_begin++);
        position.y() = *(position_y_begin++);
        position.z() = *(position_z_begin++);

        Configuration configuration{position, rotation};
        if (!_manager.collides(MOVABLE_ID, configuration))
        {
            if (++counters == EST_MIN_MATCHEE_SECTION)
            {
                matchee = std::make_pair(i, configuration);
                break;
            }
        }
        else
        {
            counters = 0;
        }
    }

    auto res_dto = PathToResult::createShared();
    if (!matchee.has_value())
    {
        // res_dto->successful = false;
        // res_dto->path_to_request_id = 0;
        // return createDtoResponse(Status::CODE_200, res_dto);
        // solving phase => start task

        matchee = std::make_pair(size_t(0), Configuration{position, rotation});

        int32_t task_id = _path_to_counter++;
        _path_to_tasks.insert({task_id, PathToTask{size_t(MOVABLE_ID), //
                                                   std::async(std::launch::async, [=, this]() {
                                                       this->_manager.est(MOVABLE_ID)->stop();
                                                       return this->_manager.est(MOVABLE_ID)
                                                           ->explore(root_configuration,
                                                                     matchee.value(), false);
                                                   })}});

        res_dto->successful = true;
        res_dto->path_to_request_id = task_id;

        return createDtoResponse(Status::CODE_200, res_dto);
    }
    else
    {
        // solving phase => start task
        int32_t task_id = _path_to_counter++;
        _path_to_tasks.insert({task_id, PathToTask{size_t(MOVABLE_ID), //
                                                   std::async(std::launch::async, [=, this]() {
                                                       this->_manager.est(MOVABLE_ID)->stop();
                                                       return this->_manager.est(MOVABLE_ID)
                                                           ->explore(root_configuration,
                                                                     matchee.value());
                                                   })}});

        res_dto->successful = true;
        res_dto->path_to_request_id = task_id;

        return createDtoResponse(Status::CODE_200, res_dto);
    }
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::path_to_statusIMPL(
    const imp::server::PathToStatusRequest::Wrapper & req_dto)
{
    OATPP_LOGV("REQUEST ", " /path-to-status")

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_path-to-status.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    bool finished = false;

    if (_path_to_tasks.contains(req_dto->path_to_request_id))
    {
        auto res_dto = PathToStatusResult::createShared();
        res_dto->path_to_request_id = req_dto->path_to_request_id;
        res_dto->finished = (_path_to_tasks[req_dto->path_to_request_id].Future.wait_for(0s) ==
                             std::future_status::ready);
        return createDtoResponse(Status::CODE_200, res_dto);
    }
    else
    {
        return createResponse(Status::CODE_404, "ID not found!");
    }
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::path_to_abortIMPL(
    const imp::server::PathToStatusRequest::Wrapper & req_dto)
{
    OATPP_LOGV("REQUEST ", " /path-to-abort")

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_path-to-abort.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    if (_path_to_tasks.contains(req_dto->path_to_request_id))
    {
        _manager.est(_path_to_tasks[req_dto->path_to_request_id].ID)->stop();
        std::this_thread::sleep_for(100ms);
        _path_to_tasks.erase(req_dto->path_to_request_id);
    }

    return createResponse(Status::CODE_200, "OK");
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::path_to_getIMPL(
    const imp::server::PathToGetRequest::Wrapper & req_dto)
{
    OATPP_LOGI("REQUEST ", " /path-to-get")

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_path-to-get.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    if (_path_to_tasks.contains(req_dto->path_to_request_id))
    {
        auto result_pair = _path_to_tasks[static_cast<uint32_t>(req_dto->path_to_request_id)].Future.get();
        auto res_dto = PathToGetResult::createShared();
        auto & result = result_pair.second;

        oatpp::List<oatpp::Float32> p_positions_x{oatpp::List<oatpp::Float32>::createShared()},
            p_positions_y{oatpp::List<oatpp::Float32>::createShared()},
            p_positions_z{oatpp::List<oatpp::Float32>::createShared()},
            p_rotations_w{oatpp::List<oatpp::Float32>::createShared()},
            p_rotations_x{oatpp::List<oatpp::Float32>::createShared()},
            p_rotations_y{oatpp::List<oatpp::Float32>::createShared()},
            p_rotations_z{oatpp::List<oatpp::Float32>::createShared()};

        for (Configuration & config : result)
        {
            p_positions_x->emplace_back(config.Position.x());
            p_positions_y->emplace_back(config.Position.y());
            p_positions_z->emplace_back(config.Position.z());
            p_rotations_w->emplace_back(config.Rotation.w());
            p_rotations_x->emplace_back(config.Rotation.x());
            p_rotations_y->emplace_back(config.Rotation.y());
            p_rotations_z->emplace_back(config.Rotation.z());
        }

        res_dto->p_positions_x = p_positions_x;
        res_dto->p_positions_y = p_positions_y;
        res_dto->p_positions_z = p_positions_z;
        res_dto->p_rotations_w = p_rotations_w;
        res_dto->p_rotations_x = p_rotations_x;
        res_dto->p_rotations_y = p_rotations_y;
        res_dto->p_rotations_z = p_rotations_z;

        res_dto->matchee_index = result_pair.first;

        _path_to_tasks.erase(req_dto->path_to_request_id);

        auto id = req_dto->movable_id;
        if (!_manager.wtree(id)->join(*_manager.est(id)))
        {
            OATPP_LOGE("WorldTree ", " Unable to join EST!");
        }
        else 
        {
            OATPP_LOGI("WorldTree ", " Joined EST!");
        }

        return createDtoResponse(Status::CODE_200, res_dto);
    }

    return createResponse(Status::CODE_404, "Couldn't be found!");
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::trashesIMPL(
    const imp::server::ObjectsTrashRequest::Wrapper & req_dto)
{
    auto movables_begin = req_dto->movables->begin();
    const auto movables_end = req_dto->movables->end();
    auto ids_begin = req_dto->ids->begin();
    const auto ids_end = req_dto->ids->end();

#ifdef DUMP_REQUESTS

    {
        std::lock_guard<std::mutex> guard(_dump_mutex);

        std::stringstream filename;
        filename << "request_" << _dump_counter << "_trashes.json";

        std::ofstream fout(filename.str(), std::ofstream::out);

        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String json = jsonObjectMapper->writeToString(req_dto);
        fout << json.get()->c_str();

        _dump_counter++;
    }

#endif

    while (movables_begin != movables_end && ids_begin != ids_end)
    {
        bool movable = *(movables_begin++);
        size_t id = *(ids_begin++);

        _manager.remove(movable, id);
    }

    return createResponse(Status::CODE_200, "OK!");
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::movedIMPL(const imp::server::MovedRequest::Wrapper & req_dto)
{
    Configuration start;
    start.Position.x() = req_dto->start_position_x;
    start.Position.y() = req_dto->start_position_y;
    start.Position.z() = req_dto->start_position_z;
    start.Rotation.x() = req_dto->start_rotation_x;
    start.Rotation.y() = req_dto->start_rotation_y;
    start.Rotation.z() = req_dto->start_rotation_z;
    start.Rotation.w() = req_dto->start_rotation_w;

    Configuration end;
    end.Position.x() = req_dto->end_position_x;
    end.Position.y() = req_dto->end_position_y;
    end.Position.z() = req_dto->end_position_z;
    end.Rotation.x() = req_dto->end_rotation_x;
    end.Rotation.y() = req_dto->end_rotation_y;
    end.Rotation.z() = req_dto->end_rotation_z;
    end.Rotation.w() = req_dto->end_rotation_w;

    auto & tree = _manager.wtree(req_dto->movable_id);
    if (!tree->moveFromToInsert(start, end))
        OATPP_LOGE("WorldTree ", " Unable to apply movement!");
    return createResponse(Status::CODE_200, "OK!");
}

std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
imp::server::ServerController::dumpwtIMPL(const imp::server::DumpRequest::Wrapper & req_dto)
{
    auto id = req_dto->movable_id;
    auto & WTree = _manager.wtree(id);

    // construct filename
    std::ofstream ss;
    ss.open("world_tree.json", std::ios::out);

    ss << "{";
    JSON(WTree)
    ss << "}";

    ss.close();

    return createResponse(Status::CODE_200, "OK!");
}