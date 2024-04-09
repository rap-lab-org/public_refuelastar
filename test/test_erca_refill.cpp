/*******************************************
 * Author: Zhongqiang Richard Ren.
 * All Rights Reserved.
 *******************************************/

#include "erca_refill.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct GasData {
  double nodeFrom;
  double nodeTo;
  double distance;
  double cost;
  double id_from;
  double id_to;
};

struct StationData {
  long nodeFrom;
  long nodeTo;
  long distance;
  long cost;
  long id_from;
  long id_to;
};

void load(std::string fname, std::vector<StationData>& stations) {
  std::vector<GasData> gasData;

  std::ifstream file(fname);

  if (!file.is_open()) {
    std::cout << "Error opening the file." << std::endl;
    return;
  }

  std::string line;
  std::getline(file, line); // Skip the header line

  while (std::getline(file, line)) {
    std::stringstream lineStream(line);
    std::string cell;
    GasData data;

    std::getline(lineStream, cell, ',');
    data.nodeFrom = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.nodeTo = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.distance = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.cost = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.id_from = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.id_to = std::stod(cell);

    gasData.push_back(data);
  }

  file.close();
  /*
    CONVERTING ALL THE LONG DOUBLES TO LONG TYPE:
  */
  stations.clear();
  // for (auto i : gasData) {
  //   long nodeFrom = std::round(i.nodeFrom);
  //   long nodeTo = std::round(i.nodeTo);
  //   long distance = std::round(1000 * i.distance);
  //   long cost = std::round(1000 * i.cost);
  //   long id_from = std::round(i.id_from);
  //   long id_to = std::round(i.id_to);
  //   stations.push_back({nodeFrom, nodeTo, distance, cost, id_from, id_to});
  // }
  //
  for (auto i : gasData) {
    long nodeFrom = i.nodeFrom;
    long nodeTo = i.nodeTo;
    long distance = i.distance;
    long cost = i.cost;
    long id_from = i.id_from;
    long id_to = i.id_to;
    stations.push_back({nodeFrom, nodeTo, distance, cost, id_from, id_to});
  }
}

void expr(std::string fname, long vo, long vd, long qMax, long kMax) {

  rzq::basic::Roadmap g; // declare a graph
  std::vector<StationData> stations;
  load(fname, stations);
  for (auto i: stations) {
    if (!g.HasNode(i.id_from)) g.AddNode(i.id_from);
    if (!g.HasNode(i.id_to)) g.AddNode(i.id_to);
  }
  for (auto i : stations) {
    auto cv = rzq::basic::CostVector(0, 2);
    cv[0] = i.cost;
    cv[1] = i.distance;
    g.AddEdge(i.id_from, i.id_to, cv);
  }

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

int TestRoadmapToyExample(long vo, long vd) {
  rzq::basic::Roadmap g; // declare a graph
  g.Init(4, 1); // The graph has 6 vertices. Each edge in the graph has a scalar
                // cost (of length 1).
                // graph vertex ID start from 1 and increase by one by one.
                // g.Init(423,1);
  /*
    manual graph generation:
  */
  rzq::basic::CostVector cv =
      rzq::basic::CostVector(0, 3); // create a vector of length 1. All values
                                    // in the vector is initialized to 0.
  // cv = rzq::basic::CostVector(0, 2); // create a vector of length 1. All
  // values in the vector is initialized to 0.
  cv[0] = 2; // set the cost vector to be [1].
  cv[1] = 2;
  cv[2] = 1;
  g.AddEdge(1, 2, cv);
  cv = rzq::basic::CostVector(0, 3); // create a vector of length 1. All values
                                     // in the vector is initialized to 0.
  cv[0] = 2;                         // set the cost vector to be [1].
  cv[1] = 5;                         // energy spent.
  cv[2] = 1;
  g.AddEdge(1, 3, cv);
  cv = rzq::basic::CostVector(0, 3); // create a vector of length 1. All values
                                     // in the vector is initialized to 0.
  cv[0] = 2;                         // set the cost vector to be [1].
  cv[1] = 7;                         // energy spent.
  cv[2] = 1;
  g.AddEdge(1, 4, cv);

  cv = rzq::basic::CostVector(0, 3); // create a vector of length 1. All values
                                     // in the vector is initialized to 0.
  cv[0] = 3;                         // set the cost vector to be [1].
  cv[1] = 2;
  cv[2] = 2;
  g.AddEdge(2, 1, cv);
  cv = rzq::basic::CostVector(0, 3); // create a vector of length 1. All values
                                     // in the vector is initialized to 0.
  cv[0] = 3;                         // set the cost vector to be [1].
  cv[1] = 3;
  cv[2] = 2;
  g.AddEdge(2, 3, cv);
  cv = rzq::basic::CostVector(0, 3); // create a vector of length 1. All values
                                     // in the vector is initialized to 0.
  cv[0] = 3;                         // set the cost vector to be [1].
  cv[1] = 5;                         // energy spent.
  cv[2] = 2;
  g.AddEdge(2, 4, cv);

  cv = rzq::basic::CostVector(0, 3); // create a vector of length 1. All values
                                     // in the vector is initialized to 0.
  cv[0] = 1;                         // set the cost vector to be [1].
  cv[1] = 5;                         // energy spent.
  cv[2] = 2;
  g.AddEdge(3, 1, cv);
  cv = rzq::basic::CostVector(
      0, 3); // create a vector of length 1. All values in the vector is
             // initialized to 0. cv[0] = 1; // set the cost vector to be [1].
             // cv[1] = 3; //energy spent.
  cv[0] = 1;
  cv[1] = 3;
  cv[2] = 2;
  g.AddEdge(3, 2, cv);
  cv = rzq::basic::CostVector(0, 3); // create a vector of length 1. All values
                                     // in the vector is initialized to 0.
  cv[0] = 1;                         // set the cost vector to be [1].
  cv[1] = 6;                         // energy spent.
  cv[2] = 2;
  g.AddEdge(3, 4, cv);

  cv = rzq::basic::CostVector(0, 3); // create a vector of length 1. All values
                                     // in the vector is initialized to 0.
  cv[0] = 0;                         // set the cost vector to be [1].  //2
  cv[1] = 7;                         // energy spent.
  cv[2] = 1;
  g.AddEdge(4, 1, cv);
  cv = rzq::basic::CostVector(0, 3);
  cv[0] = 0; // set the cost vector to be [1].
  cv[1] = 5; // energy spent.
  cv[2] = 1;
  g.AddEdge(4, 2, cv);
  cv = rzq::basic::CostVector(0, 2); // create a vector of length 1. All values
                                     // in the vector is initialized to 0.
  cv[0] = 0;                         // set the cost vector to be [1].
  cv[1] = 6;                         // energy spent.
  cv[2] = 1;
  g.AddEdge(4, 3, cv);

  //    cv = rzq::basic::CostVector(0, 2);
  //    cv[0] = 0; // set the cost vector to be [1].
  //    cv[1] = 9; //energy spent.
  //    g.AddEdge(5, 1, cv);
  //    cv = rzq::basic::CostVector(0, 2); // create a vector of length 1. All
  //    values in the vector is initialized to 0. cv[0] = 0; // set the cost
  //    vector to be [1]. cv[1] = 3; //energy spent. g.AddEdge(5, 2, cv); cv =
  //    rzq::basic::CostVector(0, 2); // create a vector of length 1. All values
  //    in the vector is initialized to 0. cv[0] = 0; // set the cost vector to
  //    be [1]. cv[1] = 4; //energy spent. g.AddEdge(5, 3, cv); cv =
  //    rzq::basic::CostVector(0, 2); // create a vector of length 1. All values
  //    in the vector is initialized to 0. cv[0] = 0; // set the cost vector to
  //    be [1]. cv[1] = 2; //energy spent. g.AddEdge(5, 4, cv);
  //
  //    cv = rzq::basic::CostVector(0, 2); // create a vector of length 1. All
  //    values in the vector is initialized to 0. cv[0] = 2; // set the cost
  //    vector to be [1]. cv[1] = 9; //energy spent.
  //      g.AddEdge(1, 5, cv);
  //    cv = rzq::basic::CostVector(0, 2); // create a vector of length 1. All
  //    values in the vector is initialized to 0. cv[0] = 3; // set the cost
  //    vector to be [1]. cv[1] = 3; //energy spent. g.AddEdge(2, 5, cv); cv =
  //    rzq::basic::CostVector(0, 2); // create a vector of length 1. All values
  //    in the vector is initialized to 0. cv[0] = 1; // set the cost vector to
  //    be [1]. cv[1] = 4; //energy spent. g.AddEdge(3, 5, cv); cv =
  //    rzq::basic::CostVector(0, 2); // create a vector of length 1. All values
  //    in the vector is initialized to 0. cv[0] = 4; // set the cost vector to
  //    be [1]. cv[1] = 2; //energy spent. g.AddEdge(4, 5, cv);

  // do some print and verification to make sure the generated graph is correct.

  // std::cout << "num_nodes: " << g.GetNumberOfNodes() << std::endl;
  // std::cout << "num_edges: " << g.GetNumberOfEdges() << std::endl;
  // std::cout << "cdims: " << g.GetCostDim() << std::endl;
  // std::cout << "g.GetCost(1, 2): " << g.GetCost(1, 2) << std::endl;
  // std::cout << "g.GetCost(1, 3): " << g.GetCost(1, 3) << std::endl;
  // std::cout << "g.GetCost(4, 5): " << g.GetCost(4, 5) << std::endl;
  // std::cout << "g.GetCost(60, 61): " << g.GetCost(60, 61) << std::endl;
  // std::cout << "g.GetCost(61, 60): " << g.GetCost(61, 60) << std::endl;

  // long vo = 1; // starting vertex in the graph.
  // long vd = 4;
  double time_limit = 60;
  // // std::vector<long> resour_limit({26});
  // std::vector<long> resour_limit({17,4});
  rzq::search::EMOAResult res;
  rzq::search::AstarRefill planner;

  planner.SetRoadmap(&g);

  // std::unordered_map<long,long> refill_costs;
  // refill_costs[1]  = 2;
  // refill_costs[2] = 3;
  // refill_costs[3] = 1;
  // refill_costs[4] = 0;
  // planner.SetRefillCost(refill_costs);
  planner.SetQmax(6); // 6000000   //13900000 //6
  // planner.SetKmax(2);

  planner.Search(vo, vd,
                 time_limit); // TODO, this is the entry point for details.

  res = planner.GetResult();

  // print paths, times and costs
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

  return 1;
};

int main() {
  // AGENT 1;
  // long vo = 1;
  // long vd = 4;
  // TestRoadmapToyExample(vo, vd);
  //
  // std::cout << "----------------" << std::endl;
  //
  // long vo1 = 2;
  // long vd1 = 3;
  // TestRoadmapToyExample(vo1, vd1);
  std::string file = "./City Data/Phil_gas.csv";
  long kMax = 10;
  long qMax = 6000;
  long s = 2;
  long t = 30;
  expr(file, s, t, qMax, kMax);
  return 0;
};
