#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>

#include <omp.h>

#include "imp/server/ServerController.hpp"
#include "AppComponent.hpp"

std::string HostConfiguration::Host = HOST_IP;
v_uint16 HostConfiguration::Port = HOST_PORT;

void run()
{
    // Register Components in scope of run() method
    AppComponent components;

    // Get router component
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    // Create ServerController and add all of its endpoints to router
    auto controller = std::make_shared<imp::server::ServerController>();
    controller->addEndpointsToRouter(router);

    // Get connection handler component
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

    // Get connection provider component
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    // Create server which takes provided TCP connections and passes them to HTTP connection handler
    // */
    oatpp::network::Server server(connectionProvider, connectionHandler);

    // print info
    std::cout << "\n<<< imp-server >>>" << std::endl;
    auto host = connectionProvider->getProperty("host").getData();
    auto port = connectionProvider->getProperty("port").getData();

    std::stringstream ss;
    ss << " SERVING ON " << host << ":" << port << " USING " << MAX_OMP_THREADS << " OMP THREADS";
    OATPP_LOGI("SYSTEM ", ss.str().c_str())

    // Run server
    server.run();
}

int main(int argc, char ** argv)
{
    // create logger
    auto logger =
        std::make_shared<oatpp::base::DefaultLogger>(oatpp::base::DefaultLogger::Config( //
            "%Y-%m-%d %H:%M:%S",                                                         //
            false,                                                                       //
            (0 << oatpp::base::Logger::PRIORITY_V) |                                     // Verbose
                (0 << oatpp::base::Logger::PRIORITY_D) |                                 // Debug
                (1 << oatpp::base::Logger::PRIORITY_I) |                                 // Info
                (1 << oatpp::base::Logger::PRIORITY_W) |                                 // Warning
                (1 << oatpp::base::Logger::PRIORITY_E)                                   // Error
            ));

    // Init oatpp Environment
    oatpp::base::Environment::init(logger);

    // Setup OMP
    omp_set_num_threads(MAX_OMP_THREADS);

    // Run App
    run();

    // Destroy oatpp Environment
    oatpp::base::Environment::destroy();

    return 0;
}