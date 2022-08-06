#pragma once

/**
 * @file DTO.hpp
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 * @brief This file contains all data transfer objects. These should mirror the DTO file in imp.
 */

#include <atomic>

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>

namespace imp::server
{

#include OATPP_CODEGEN_BEGIN(DTO)

class ObjectCreationResult : public oatpp::DTO
{
    DTO_INIT(ObjectCreationResult, DTO)
    DTO_FIELD(Boolean, movable);
    DTO_FIELD(Int32, id);
};

class ObjectsTrashRequest : public oatpp::DTO
{
    DTO_INIT(ObjectsTrashRequest, DTO)
    DTO_FIELD(List<Boolean>, movables);
    DTO_FIELD(List<Int32>, ids);
};

class MovedRequest : public oatpp::DTO
{
    DTO_INIT(MovedRequest, DTO)
    
    DTO_FIELD(Int32, movable_id);

    DTO_FIELD(Float32, start_position_x);
    DTO_FIELD(Float32, start_position_y);
    DTO_FIELD(Float32, start_position_z);
    DTO_FIELD(Float32, start_rotation_w);
    DTO_FIELD(Float32, start_rotation_x);
    DTO_FIELD(Float32, start_rotation_y);
    DTO_FIELD(Float32, start_rotation_z);

    DTO_FIELD(Float32, end_position_x);
    DTO_FIELD(Float32, end_position_y);
    DTO_FIELD(Float32, end_position_z);
    DTO_FIELD(Float32, end_rotation_w);
    DTO_FIELD(Float32, end_rotation_x);
    DTO_FIELD(Float32, end_rotation_y);
    DTO_FIELD(Float32, end_rotation_z);
};

class ObjectCreationRequest : public oatpp::DTO
{
    DTO_INIT(ObjectCreationRequest, DTO)
    DTO_FIELD(Boolean, movable);
    DTO_FIELD(List<Float32>, vertices);
    DTO_FIELD(List<Int32>, triangles);
    DTO_FIELD(Float32, position_x);
    DTO_FIELD(Float32, position_y);
    DTO_FIELD(Float32, position_z);
    DTO_FIELD(Float32, rotation_w);
    DTO_FIELD(Float32, rotation_x);
    DTO_FIELD(Float32, rotation_y);
    DTO_FIELD(Float32, rotation_z);
};

class CollisionRequest : public oatpp::DTO
{
    DTO_INIT(CollisionRequest, DTO)
    DTO_FIELD(Int32, movable_id);
    DTO_FIELD(Float32, position_x);
    DTO_FIELD(Float32, position_y);
    DTO_FIELD(Float32, position_z);
    DTO_FIELD(Float32, rotation_w);
    DTO_FIELD(Float32, rotation_x);
    DTO_FIELD(Float32, rotation_y);
    DTO_FIELD(Float32, rotation_z);
};

class MultipleCollisionRequest : public oatpp::DTO
{
    DTO_INIT(MultipleCollisionRequest, DTO)
    DTO_FIELD(Int32, movable_id);
    DTO_FIELD(List<Float32>, position_x);
    DTO_FIELD(List<Float32>, position_y);
    DTO_FIELD(List<Float32>, position_z);
    DTO_FIELD(List<Float32>, rotation_w);
    DTO_FIELD(List<Float32>, rotation_x);
    DTO_FIELD(List<Float32>, rotation_y);
    DTO_FIELD(List<Float32>, rotation_z);
};

class NewLocalClosestRequest : public oatpp::DTO
{
    DTO_INIT(NewLocalClosestRequest, DTO)
    DTO_FIELD(Int32, movable_id);

    DTO_FIELD(Float32, start_position_x);
    DTO_FIELD(Float32, start_position_y);
    DTO_FIELD(Float32, start_position_z);
    DTO_FIELD(Float32, start_rotation_w);
    DTO_FIELD(Float32, start_rotation_x);
    DTO_FIELD(Float32, start_rotation_y);
    DTO_FIELD(Float32, start_rotation_z);

    DTO_FIELD(Float32, end_position_x);
    DTO_FIELD(Float32, end_position_y);
    DTO_FIELD(Float32, end_position_z);
    DTO_FIELD(Float32, end_rotation_w);
    DTO_FIELD(Float32, end_rotation_x);
    DTO_FIELD(Float32, end_rotation_y);
    DTO_FIELD(Float32, end_rotation_z);
};

using IsCollisionFreePathRequest = NewLocalClosestRequest;

class NewLocalClosestResult : public oatpp::DTO
{
    DTO_INIT(NewLocalClosestResult, DTO)
    DTO_FIELD(Boolean, successful);
    DTO_FIELD(Float32, position_x);
    DTO_FIELD(Float32, position_y);
    DTO_FIELD(Float32, position_z);
    DTO_FIELD(Float32, rotation_w);
    DTO_FIELD(Float32, rotation_x);
    DTO_FIELD(Float32, rotation_y);
    DTO_FIELD(Float32, rotation_z);
};

class PathToRequest : public oatpp::DTO
{
    DTO_INIT(PathToRequest, DTO)

    DTO_FIELD(Int32, movable_id);

    DTO_FIELD(Float32, start_position_x);
    DTO_FIELD(Float32, start_position_y);
    DTO_FIELD(Float32, start_position_z);
    DTO_FIELD(Float32, start_rotation_w);
    DTO_FIELD(Float32, start_rotation_x);
    DTO_FIELD(Float32, start_rotation_y);
    DTO_FIELD(Float32, start_rotation_z);
    
    // u path
    DTO_FIELD(List<Float32>, u_positions_x);
    DTO_FIELD(List<Float32>, u_positions_y);
    DTO_FIELD(List<Float32>, u_positions_z);
    DTO_FIELD(List<Float32>, u_rotations_w);
    DTO_FIELD(List<Float32>, u_rotations_x);
    DTO_FIELD(List<Float32>, u_rotations_y);
    DTO_FIELD(List<Float32>, u_rotations_z);
};

class PathToResult : public oatpp::DTO
{
    DTO_INIT(PathToResult, DTO)
    DTO_FIELD(Boolean, successful);
    DTO_FIELD(Int32, path_to_request_id);
};

class PathToStatusRequest : public oatpp::DTO
{
    DTO_INIT(PathToStatusRequest, DTO)
    DTO_FIELD(Int32, path_to_request_id);
};

class PathToStatusResult : public oatpp::DTO
{
    DTO_INIT(PathToStatusResult, DTO)
    DTO_FIELD(Int32, path_to_request_id);
    DTO_FIELD(Boolean, finished);
};

class PathToGetRequest : public oatpp::DTO
{
    DTO_INIT(PathToGetRequest, DTO)
    DTO_FIELD(Int32, path_to_request_id);
    DTO_FIELD(Int32, movable_id);
};

class DumpRequest : public oatpp::DTO
{
    DTO_INIT(DumpRequest, DTO)
    DTO_FIELD(Int32, movable_id);
};

class PathToGetResult : public oatpp::DTO
{
    DTO_INIT(PathToGetResult, DTO)

    DTO_FIELD(Int32, matchee_index);

    // p path
    DTO_FIELD(List<Float32>, p_positions_x);
    DTO_FIELD(List<Float32>, p_positions_y);
    DTO_FIELD(List<Float32>, p_positions_z);
    DTO_FIELD(List<Float32>, p_rotations_w);
    DTO_FIELD(List<Float32>, p_rotations_x);
    DTO_FIELD(List<Float32>, p_rotations_y);
    DTO_FIELD(List<Float32>, p_rotations_z);
};

class CollisionResult : public oatpp::DTO
{
    DTO_INIT(CollisionResult, DTO)
    DTO_FIELD(Boolean, is_colliding);
};

#include OATPP_CODEGEN_END(DTO)

} // namespace imp::server