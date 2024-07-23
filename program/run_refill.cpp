/*******************************************
 * Author: Zhongqiang Richard Ren.
 * All Rights Reserved.
 *******************************************/

#include "erca_refill.hpp"
#include "graph.hpp"
#include "expr_utils.hpp"
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace refill {

double RT, TIMELIMIT;
std::string GFILE;
long SID, TID, BEST, QMAX, KMAX;
rzq::search::EMOAResult res;

long solve(std::string fname, const long s, const long t, const long K, const long Q) {

  rzq::basic::Roadmap g; // declare a graph
  std::vector<StationData> stations;
  load(fname, stations);
  build_graph(stations, g);
  // // std::vector<long> resour_limit({26});
  // std::vector<long> resour_limit({17,4});
  rzq::search::AstarRefill planner;
  g._cdim = 2;
  BEST = std::numeric_limits<long>::max();
  planner.SetRoadmap(&g);
  planner.SetQmax(Q);
  planner.SetKmax(K);

  planner.Search(s, t, TIMELIMIT);
  res = planner.GetResult();
  RT = res.rt_search;

  for (auto iter : res.paths) {
    long k = iter.first; // id of a Pareto-optipmal solution
    std::cout << " cost = " << res.costs[k] << std::endl;
    BEST = res.costs[k][0];
  }
  return BEST;
}

}

int main(int argc, char **argv) {
  // long vo1 = 2;
  // long vd1 = 3;
  // ./<bin> <file> <s> <t> <K> <Q>
  using namespace refill;
  using namespace std;
  GFILE = std::string(argv[1]);
  SID = std::stoi(argv[2]);
  TID = std::stoi(argv[3]);
  KMAX = std::stoi(argv[4]);
  QMAX = std::stoi(argv[5]);
  TIMELIMIT = 60;
  // long kMax = 5, qMax = 6;
  solve(GFILE, SID, TID, KMAX, QMAX);

  string mapname = get_name(GFILE);
  ofstream fout;
  fout.open("output/" + mapname + ".log", ios_base::app);
  fout << mapname << "," << SID << "," << TID << "," << KMAX << "," << QMAX << ",erca," << BEST << "," 
       << setprecision(4) << RT << endl;
  return 0;
};
