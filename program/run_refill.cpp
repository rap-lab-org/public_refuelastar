/*******************************************
 * Author: Zhongqiang Richard Ren.
 * All Rights Reserved.
 *******************************************/

#include "erca_refill.hpp"
#include "graph.hpp"
#include "load_data.hpp"
#include <iostream>
#include <string>
#include <vector>

void solve(std::string fname, const long s, const long t, const long K, const long Q) {

  rzq::basic::Roadmap g; // declare a graph
  std::vector<StationData> stations;
  load(fname, stations);
  build_graph(stations, g);
  double time_limit = 60;
  // // std::vector<long> resour_limit({26});
  // std::vector<long> resour_limit({17,4});
  rzq::search::EMOAResult res;
  rzq::search::AstarRefill planner;
  g._cdim = 2;

  planner.SetRoadmap(&g);
  planner.SetQmax(Q);
  planner.SetKmax(K);

  planner.Search(s, t, time_limit);
  res = planner.GetResult();

  std::cout << "---- print solutions for clarity:" << std::endl;
  for (auto iter : res.paths) {
    long k = iter.first; // id of a Pareto-optipmal solution
    // path nodes
    std::cout << " path nodes = ";
    for (auto xx : res.paths[k]) {
      std::cout << xx << ", ";
    }
    std::cout << std::endl;
    // cost
    std::cout << " cost = " << res.costs[k] << std::endl;
  }
}

int main(int argc, char **argv) {
  // long vo1 = 2;
  // long vd1 = 3;
  // ./<bin> <file> <s> <t> <K> <Q>
  std::string file = std::string(argv[1]);
  long s = std::stoi(argv[2]);
  long t = std::stoi(argv[3]);
  long K = std::stoi(argv[4]);
  long Q = std::stoi(argv[5]);
  // long kMax = 5, qMax = 6;
  solve(file, s, t, K, Q);
  return 0;
};
