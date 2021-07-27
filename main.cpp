
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

int main() {



  transport::renderer::MapRenderer renderer;
  transport::Catalogue catalogue;
  transport::Reader reader(catalogue, renderer);

  reader.ParseRequests(std::cin);
  reader.PrintReply(std::cout);
}
