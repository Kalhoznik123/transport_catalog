
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

int main() {

  transport::renderer::MapRenderer renderer;
  transport::Catalogue catalogue;
  transport::Reader reader(catalogue, renderer);
  reader.FillCatalog(std::cin);
  reader.PrintRequests(std::cout);
}
