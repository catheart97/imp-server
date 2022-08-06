#pragma once

#include <exception>
#include <functional>
#include <future>
#include <numeric>
#include <thread>
#include <unordered_map>

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "imp/EST.hpp"
#include "imp/ObjectManager.hpp"
#include "imp/server/DTO.hpp"

namespace imp::server
{

#include OATPP_CODEGEN_BEGIN(ApiController) ///< Begin Codegen

/**
 * @brief Server controller for imp-server. This class manages routing.
 *
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class ServerController : public oatpp::web::server::api::ApiController
{
    /////////
    // nested
    /////////
private:
    struct PathToTask
    {
        size_t ID;
        std::future<std::pair<int64_t, std::vector<imp::Configuration>>> Future;
    };

    /////////
    // data
    /////////
private:
    imp::ObjectManager _manager;

    std::atomic<int32_t> _path_to_counter{0};
    std::unordered_map<int32_t, PathToTask> _path_to_tasks;

    long long _dump_counter{0};
    std::mutex _dump_mutex;

public:
    ServerController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper)
    {}

    /////////
    // endpoints
    /////////
public:
    ENDPOINT("GET", "/", root) { return createResponse(Status::CODE_200, "imp-server running."); }

    ENDPOINT("GET", "/clear", clear)
    {
        OATPP_LOGI("REQUEST ", " /clear")
        _manager.clear();
        return createResponse(Status::CODE_200, "OK");
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    createIMPL(const imp::server::ObjectCreationRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/create", create, BODY_DTO(Object<ObjectCreationRequest>, req_dto))
    {
        return createIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    trashesIMPL(const imp::server::ObjectsTrashRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/trashes", trashes, BODY_DTO(Object<ObjectsTrashRequest>, req_dto))
    {
        return trashesIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    movedIMPL(const imp::server::MovedRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/moved", moved, BODY_DTO(Object<MovedRequest>, req_dto))
    {
        return movedIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    collidesIMPL(const imp::server::CollisionRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/collides", collides, BODY_DTO(Object<CollisionRequest>, req_dto))
    {
        return collidesIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    collides_anyIMPL(const imp::server::MultipleCollisionRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/collides-any", collides_any,
             BODY_DTO(Object<MultipleCollisionRequest>, req_dto))
    {
        return collides_anyIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    new_local_closestIMPL(const imp::server::NewLocalClosestRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/new-local-closest", new_local_closest,
             BODY_DTO(Object<NewLocalClosestRequest>, req_dto))
    {
        return new_local_closestIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    is_collision_free_pathIMPL(const imp::server::NewLocalClosestRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/is-collision-free-path", is_collision_free_path,
             BODY_DTO(Object<IsCollisionFreePathRequest>, req_dto))
    {
        return is_collision_free_pathIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    path_toIMPL(const imp::server::PathToRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/path-to", path_to, BODY_DTO(Object<PathToRequest>, req_dto))
    {
        return path_toIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    path_to_statusIMPL(const imp::server::PathToStatusRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/path-to-status", path_to_status,
             BODY_DTO(Object<PathToStatusRequest>, req_dto))
    {
        return path_to_statusIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    path_to_abortIMPL(const imp::server::PathToStatusRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/path-to-abort", path_to_abort, BODY_DTO(Object<PathToStatusRequest>, req_dto))
    {
        return path_to_abortIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    path_to_getIMPL(const imp::server::PathToGetRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/path-to-get", path_to_get, BODY_DTO(Object<PathToGetRequest>, req_dto))
    {
        return path_to_getIMPL(req_dto);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    dumpwtIMPL(const imp::server::DumpRequest::Wrapper & req_dto);
    ENDPOINT("PUT", "/dumpwt", dumpwt, BODY_DTO(Object<DumpRequest>, req_dto))
    {
        return dumpwtIMPL(req_dto);
    }
};

#include OATPP_CODEGEN_END(ApiController) ///< End Codegen

} // namespace imp::server
