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

void sanity_checking(rzq::basic::Roadmap &g) {
  // 1. cost(u, *) are same
  FILE *fp = fopen("sanity_checking.log", "w");

  for (auto u : g.GetNodes()) {
    auto cost = std::set<long>();
    for (auto v : g.GetSuccs(u)) {
      auto c = g.GetCost(u, v)[0];
      if (cost.size() > 0 and cost.find(c) == cost.end()) {
        fprintf(fp, "Inconsistant cost(%ld, %ld) = %ld, existing: ", u, v, c);
        for (auto c : cost)
          fprintf(fp, " %ld ", c);
        fprintf(fp, "\n");
      }
      cost.insert(g.GetCost(u, v)[0]);
    }
  }
  // 2. dist(u, v) = dist(v, u)
  for (auto u : g.GetNodes()) {
    for (auto v : g.GetSuccs(u)) {
      auto c0 = g.GetCost(u, v)[1];
      auto c1 = g.GetCost(v, u)[1];
      if (c0 != c1) {
        fprintf(fp, "Inconsistant dist (%ld, %ld)=%ld, (%ld, %ld)=%ld\n", u, v,
                c0, v, u, c1);
      }
    }
  }
}

void expr(std::string fname, long vo, long vd, long qMax, long kMax) {

  rzq::basic::Roadmap g; // declare a graph
  std::vector<StationData> stations;
  load(fname, stations);
  for (auto i : stations) {
    if (!g.HasNode(i.id_from))
      g.AddNode(i.id_from);
    if (!g.HasNode(i.id_to))
      g.AddNode(i.id_to);
  }
  for (auto i : stations) {
    auto cv = rzq::basic::CostVector(0, 2);
    cv[0] = i.cost;
    cv[1] = i.distance;
    g.AddEdge(i.id_from, i.id_to, cv);
  }

  sanity_checking(g);

  double time_limit = 60;
  // // std::vector<long> resour_limit({26});
  // std::vector<long> resour_limit({17,4});
  rzq::search::EMOAResult res;
  rzq::search::AstarRefill planner;
  g._cdim = 2;

  planner.SetRoadmap(&g);
  planner.SetQmax(qMax);
  planner.SetKmax(kMax);

  planner.Search(vo, vd, time_limit);
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
  // TestRoadmapToyExample(vo1, vd1);
  // ./<bin> <file> <s> <t>
  std::string file = std::string(argv[1]);
  long s = std::stoi(argv[2]);
  long t = std::stoi(argv[3]);
  long kMax = std::stoi(argv[4]);
  long qMax = std::stoi(argv[5]);
  // long kMax = 5, qMax = 6;
  expr(file, s, t, qMax, kMax);
  return 0;
};
