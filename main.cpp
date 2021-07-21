// напишите решение с нуля
// код сохраните в свой git-репозиторий


#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

int main() {

    renderer::MapRenderer renderer;
    transport::Catalogue catalogue;
    transport::Reader reader(catalogue, renderer);
    reader.ParseJson(std::cin);
    reader.PrintJson(std::cout);
}
