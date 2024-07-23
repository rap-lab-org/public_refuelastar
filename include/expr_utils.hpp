#include <fstream>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "graph.hpp"

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

const long PREC = 100;

inline void load(std::string fname, std::vector<StationData> &stations) {
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
  for (auto i : gasData) {
    long nodeFrom = i.nodeFrom;
    long nodeTo = i.nodeTo;
    long distance = i.distance;
    long cost = i.cost * PREC;
    long id_from = i.id_from;
    long id_to = i.id_to;
    stations.push_back({nodeFrom, nodeTo, distance, cost, id_from, id_to});
  }
}

inline void sanity_checking(rzq::basic::Roadmap &g) {
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
      auto euv = g.GetCost(u, v);
      auto evu = g.GetCost(v, u);
      if (euv.size() == evu.size() && euv.size() > 0) {
        auto c0 = euv[1];
        auto c1 = evu[1];
        if (c0 != c1) {
          fprintf(fp, "Inconsistant dist (%ld, %ld)=%ld, (%ld, %ld)=%ld\n", u, v,
                  c0, v, u, c1);
        }
      }
      else {
        fprintf(fp, "Edge(%ld, %ld) exists, but Edge(%ld, %ld) does not\n", u, v, v, u);
      }
    }
  }
}

inline void build_graph(const std::vector<StationData>& stations, rzq::basic::Roadmap& g) {
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
}

inline std::string get_name(const std::string& fpath) {
  
  // remove suffix ".csv";
  std::string res = fpath.substr(0, fpath.size() - 4);
  // remove parent dir prefix
  res = res.substr(res.find_last_of('/')+1);
  return res;
}
