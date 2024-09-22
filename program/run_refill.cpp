/*******************************************
 * Author: Zhongqiang Richard Ren.
 * All Rights Reserved.
 *******************************************/

#include "erca_refill.hpp"
#include "expr_utils.hpp"
#include "graph.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace refill {

double RT, RTINIT, RTPREC, TIMELIMIT;
std::string GFILE;
long SID, TID, BEST, QMAX, KMAX, NUMSTATE, HW, PATHLENGTH, SHORTESTLENGTH;
rzq::search::EMOAResult res;

long solve(std::string fname, const long s, const long t, const long K,
           const long Q, const long heurW = 1) {

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
  planner.heurW = heurW; // whether use heuristic
	// set minPrice to compute heuristic
	for (const auto& station: stations)
		planner.minPrice = std::min(planner.minPrice, station.cost);

  auto tstart = std::chrono::steady_clock::now();
  planner.Search(s, t, TIMELIMIT);
  auto tend = std::chrono::steady_clock::now();
  res = planner.GetResult();
  NUMSTATE = res.n_generated;
  RT = res.rt_search + res.rt_initHeu;
  RTINIT = res.rt_initHeu;
	RTPREC = res.rt_preproc;
	PATHLENGTH = res.dist;
	SHORTESTLENGTH = res.shortest_dist;

  for (auto iter : res.paths) {
    long k = iter.first; // id of a Pareto-optipmal solution
    BEST = res.costs[k][0];
  }
  return BEST;
}

} // namespace refill

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
	HW = argc > 6? std::stoi(argv[6]): 1;
  TIMELIMIT = 60;
  // long kMax = 5, qMax = 6;
  solve(GFILE, SID, TID, KMAX, QMAX, HW);

  string mapname = get_name(GFILE);
  string ALGO = HW==0? "erca-noh": "erca";
  stringstream row;
  ofstream fout;
  fout.open("output/" + mapname + ".log", ios_base::app);
  // s -> us
  RT *= 1e6;
  RTINIT *= 1e6;
	RTPREC *= 1e6;
  row << mapname << "," << SID << "," << TID << "," << KMAX << "," 
		  << QMAX << "," << ALGO << "," << BEST << "," << NUMSTATE << "," 
			<< setprecision(4) << RT << "," << RTINIT << "," << RTPREC << ","
			<< PATHLENGTH << "," << SHORTESTLENGTH;
  fout << row.str() << endl;
  cout << row.str() << endl;
  return 0;
};
