#include <iomanip>
#include <limits>
#include <queue>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include "expr_utils.hpp"
#include "graph.hpp"


namespace dp {


using Graph = rzq::basic::Roadmap;
using Sets = std::unordered_map<long, std::unordered_set<long>>;
using Map = std::unordered_map<long, long>; // m[i] = v
using Maps = std::unordered_map<long, Map>; // m[i][j] = v
using Maps3D = std::unordered_map<long, Maps>; // m[i][j][k] = v


struct State {
  long k, q, v;
};

double RT, TIMELIMIT;
std::string GFILE;
long SID, TID, BEST, QMAX, KMAX;

void init_v(long v, long Q, Graph& g, Sets& reachVert, Maps& reachDist) {
  Map dist;
  typedef std::pair<long, long> Node;
  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> q;
  for (auto i: g.GetNodes())
    dist[i] = std::numeric_limits<long>::max();
  dist[v] = 0;
  q.push({0, v});

  while (!q.empty()) {
    auto cur = q.top();
    q.pop();
    long cdist = cur.first;
    long cid = cur.second;

    if (cdist > Q || cdist != dist[cid])
      continue;
    // now, we can guarantee cdist is the shortest distance from v to cid
    reachVert[v].insert(cid);
    reachDist[v][cid] = cdist;

    for (auto nxt : g.GetSuccs(cid)) {
      // cost[1] is distance
      auto ecost = g.GetCost(cid, nxt)[1];
      if (dist[nxt] > dist[cid] + ecost) {
        dist[nxt] = dist[cid] + ecost;
        q.push({dist[nxt], nxt});
      }
    }
  }
}

void init(long t, long Q, Graph& g, Map& Cost, Sets& reachVert, Maps& reachDist) {
  Cost.clear();
  for (auto v: g.GetNodes()) {
    for (auto u: g.GetSuccs(v)) {
      Cost[v] = g.GetCost(v, u)[0];
      break;
    }
  }
  Cost[t] = 0;

  for (auto v: g.GetNodes()) {
    reachVert[v] = {};
    reachDist[v] = {};
    init_v(v, Q, g, reachVert, reachDist);
  }
}


long getG(Maps3D& dp, int k, int q, int v) {
  if (dp.find(k) == dp.end() ||
      dp.at(k).find(q) == dp.at(k).end() ||
      dp.at(k).at(q).find(v) == dp.at(k).at(q).end()) {
    return std::numeric_limits<long>::max();
  }
  return dp.at(k).at(q).at(v);
}


long solve(const std::vector<StationData>& stations, const long s, const long t, const long K, const long Q) {
  Graph g;
  Sets reachVs;
  Maps reachDist;
  Map stationCost;
  build_graph(stations, g);
  init(t, Q, g, stationCost, reachVs, reachDist);
  // state: dp[k][q][v]
  Maps3D dp;
  dp[0][0][s] = 0;
  std::queue<State> q;
  q.push({0, 0, s});

  BEST = std::numeric_limits<long>::max();

  auto tstart = std::chrono::steady_clock::now();

  while (!q.empty()) {
    auto c = q.front(); q.pop();
    long costCur = stationCost[c.v];
    long gCur = dp[c.k][c.q][c.v];
    if (c.k > KMAX)
      continue;;
    if (c.v == t)
      BEST = std::min(BEST, gCur);

    auto tnow = std::chrono::steady_clock::now();
    if (std::chrono::duration<double>(tnow - tstart).count() > TIMELIMIT) {
      break;
    }

    for (auto nxt: reachVs[c.v]) {
      long costNxt = stationCost[nxt];
      long dist = reachDist[c.v][nxt];
      long qNxt, gNxt, kNxt;
      if (costNxt > costCur) {
        qNxt = Q - dist;
        kNxt = c.k + 1;
        gNxt = gCur + (Q - c.q) * costCur;
      } else {
        if (dist > c.q) {
          qNxt = 0;
          kNxt = c.k + 1;
          gNxt = gCur + (dist - c.q) * costCur;
        }
        else {
          // reaching a station with non-empty tank is not optimal
          continue;
        }
      }
      if (getG(dp, kNxt, qNxt, nxt) > gNxt) {
        dp[kNxt][qNxt][nxt] = gNxt;
        q.push({kNxt, qNxt, nxt});
      }
    }
  }

  auto tnow = std::chrono::steady_clock::now();
  RT = std::chrono::duration_cast<std::chrono::microseconds>(tnow - tstart).count();
  return BEST;
}

}

int main(int argc, char **argv) {
  using namespace dp;
  using namespace std;
  GFILE = std::string(argv[1]);
  SID = std::stoi(argv[2]);
  TID = std::stoi(argv[3]);
  KMAX = std::stoi(argv[4]);
  QMAX = std::stoi(argv[5]);
  TIMELIMIT = 60;

  std::vector<StationData> stations;
  load(GFILE, stations);
  solve(stations, SID, TID, KMAX, QMAX);

  cout << " cost = " << BEST << endl;
  string mapname = get_name(GFILE);
  ofstream fout;
  fout.open("output/" + mapname + ".log", ios_base::app);
  fout << mapname << "," << SID << "," << TID << "," << KMAX << "," << QMAX << ",dp," << BEST << "," 
       << setprecision(4) << RT << endl;
}
