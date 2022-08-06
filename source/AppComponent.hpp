#pragma once

#include <string>

#include "oatpp/core/macro/component.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"

#include "imp/Settings.hpp"

struct HostConfiguration
{
    static std::string Host;
    static v_uint16 Port;
};

/**
 * @brief Class which creates and holds Application components and registers components in
 * oatpp::base::Environment Order of components initialization is from top to bottom
 *
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class AppComponent
{
public:
    // Create ConnectionProvider component which listens on the port
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>,
                           serverConnectionProvider)
    ([] {
        return oatpp::network::tcp::server::ConnectionProvider::createShared(
            {HostConfiguration::Host.c_str(), HostConfiguration::Port,
             oatpp::network::Address::IP_4});
    }());

    // Create Router component
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)
    ([] { return oatpp::web::server::HttpRouter::createShared(); }());

    // Create ConnectionHandler component which uses Router component to route requests
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                           serverConnectionHandler)
    ([] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>,
                        router); // get Router component
        return oatpp::web::server::HttpConnectionHandler::createShared(router);
    }());

    // Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)
    ([] { return oatpp::parser::json::mapping::ObjectMapper::createShared(); }());
};