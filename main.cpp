
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

int main() {



  transport::renderer::MapRenderer renderer;
  transport::Catalogue transport_catalogue;
  transport::router::TransportRouter transport_router;
  transport::Reader reader(transport_catalogue, renderer,transport_router);

  reader.ParseRequests(std::cin);
  graph::DirectedWeightedGraph<double> graf(transport_catalogue.GetStops().size() * 2);
  transport_router.SetGraf(std::move(graf));
  transport_router.BuildRouter(transport_catalogue);
  //std::cout<<"------------------------------------------"<<std::endl;
  reader.PrintReply(std::cout);
}
