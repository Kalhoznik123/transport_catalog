// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include <sstream>
#include <cassert>
#include "transport_catalogue.h"
#include "domain.h"
#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

using namespace std::string_literals;




int main() {

    renderer::MapRenderer renderer;
    transport::Catalogue catalogue;
    json::Reader reader(catalogue, renderer);
    reader.ParseJson(std::cin);
    reader.PrintJson(std::cout);
}
