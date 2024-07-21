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

int main(int argc, char** argv) {
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
  // ./<bin> <file> <s> <t>
  std::string file = std::string(argv[1]);
  long s = std::stoi(argv[2]);
  long t = std::stoi(argv[3]);
  // long kMax = 10;
  // long qMax = 6000;
  long kMax = 5, qMax = 6;
  expr(file, s, t, qMax, kMax);
  return 0;
};
