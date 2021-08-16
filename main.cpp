
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

int main() {
  using namespace std::string_literals;
  using namespace std::string_view_literals;


  transport::Catalogue tc;
  tc.AddStop({"s1"s, {0, 0}});
  tc.AddStop({"s2"s, {0, 1}});
  const auto s1 = tc.GetStop("s1"sv);
  const auto s2 = tc.GetStop("s2"sv);
  tc.AddBus({"b"s, {s1, s2}, s1, s2, true});
  transport::router::TransportRouter tr({5, 60});
  tr.BuildRouter(tc);



//  transport::renderer::MapRenderer renderer;
//  transport::Catalogue transport_catalogue;
//  transport::router::TransportRouter transport_router;
//  transport::Reader reader(transport_catalogue, renderer,transport_router);

//  reader.ParseRequests(std::cin);
//  graph::DirectedWeightedGraph<double> graf(transport_catalogue.GetStops().size() * 2);
//  transport_router.SetGraf(std::move(graf));
//  transport_router.BuildRouter(transport_catalogue);
//  //std::cout<<"------------------------------------------"<<std::endl;
//  reader.PrintReply(std::cout);
}
