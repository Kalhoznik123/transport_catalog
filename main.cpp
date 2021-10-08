
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"
#include "request_handler.h"
#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        transport::renderer::MapRenderer renderer;
        transport::Catalogue transport_catalogue;
        transport::router::TransportRouter transport_router;
        transport::serialization::Serializator serializator;
        transport::Reader reader(transport_catalogue,renderer,transport_router,serializator);
        reader.ParseRequests(std::cin);

        transport_router.BuildRouter(transport_catalogue); //build router hear

        serializator.Serizalize(transport_catalogue,renderer,transport_router);
        // make base here

    } else if (mode == "process_requests"sv) {
        transport::renderer::MapRenderer renderer;
        transport::Catalogue transport_catalogue;
        transport::router::TransportRouter transport_router;
        transport::serialization::Serializator serializator;
        transport::Reader reader(transport_catalogue,renderer,transport_router,serializator);

        reader.ParseRequests(std::cin);
        serializator.Deserialize(transport_catalogue,renderer,transport_router);
        reader.PrintReply(std::cout);
        // process requests here

    } else {
        PrintUsage();
        return 1;
    }
}
