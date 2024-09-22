#include "expr_utils.hpp"
#include "graph.hpp"
#include <cassert>
#include <chrono>
#include <iomanip>
#include <limits>
#include <queue>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace dp {

using namespace std;
using Graph = rzq::basic::Roadmap;
using Sets = std::unordered_map<long, std::unordered_set<long>>;
using Map = std::unordered_map<long, long>;    // m[i] = v
using Maps = std::unordered_map<long, Map>;    // m[i][j] = v
using Maps3D = std::unordered_map<long, Maps>; // m[i][j][k] = v

static const long INF = numeric_limits<long>::max();

struct State {
  long k, q, v;
};

double RT, RTINIT, RTPREC, TIMELIMIT;
std::string GFILE;
long SID, TID, BEST, QMAX, KMAX, NUMSTATE, PATHLENGTH, SHORTESTLENGTH;
long BEST_STOP;

void init_v(long v, long Q, Graph &g, Sets &reachVert, Maps &reachDist) {
  Map dist;
  typedef std::pair<long, long> Node;
  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> q;
  for (auto i : g.GetNodes())
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

void init(long t, long Q, Graph &g, Map &Cost, Sets &reachVert,
          Maps &reachDist) {
  Cost.clear();
  for (auto v : g.GetNodes()) {
    for (auto u : g.GetSuccs(v)) {
      Cost[v] = g.GetCost(v, u)[0];
      break;
    }
  }
  Cost[t] = 0;

  for (auto v : g.GetNodes()) {
    reachVert[v] = {};
    reachDist[v] = {};
    init_v(v, Q, g, reachVert, reachDist);
  }
}

long getVal(const Maps3D &dp, long a, long b, long c) {
  // get value of state (a, b, c) from 3D dp
  if (dp.find(a) == dp.end() || dp.at(a).find(b) == dp.at(a).end() ||
      dp.at(a).at(b).find(c) == dp.at(a).at(b).end())
    return INF;
  return dp.at(a).at(b).at(c);
}

long add(long a, long b) {
  if (a == INF || b == INF)
    return INF;
  return a + b;
}

void fill_table_Qn3(const Graph &gr, const Sets &reachVs, const Sets &predVs,
                    const Maps &reachDist, const Map &c, const long s,
                    const long t, const long Q, const long U,
                    const unordered_map<long, set<long>> &GV, Maps3D &dp) {

  // fill rows
  // O(Q * n^3) version
  for (int q = 1; q <= Q; q++) {
    for (auto u : gr.GetNodes())
      if (u != t) {
        // ensure dp.at(u).at(q) is not null
        dp[u][q] = {};
        for (auto v : reachVs.at(u))
          if (v != u) {
            for (auto g : GV.at(u)) {
              long distuv = reachDist.at(u).at(v);
              long g_at_v = c.at(u) < c.at(v) ? U - distuv : 0;
              long cost_vt = getVal(dp, v, q - 1, g_at_v);
              if (cost_vt == INF)
                continue;
              long cost_uv;
              if (c.at(u) < c.at(v)) {
                cost_uv = (U - g) * c.at(u);
                assert(g_at_v == U - distuv);
              } else if (distuv >= g) {
                cost_uv = (distuv - g) * c.at(u);
                assert(g_at_v == 0);
              } else
                continue;

              NUMSTATE++;
              long exist = getVal(dp, u, q, g);
              if (exist > cost_uv + cost_vt) {
                dp[u][q][g] = cost_uv + cost_vt;

                if (u == s && g == 0 && BEST > dp[u][q][g]) {
                  // printf(
                  //     "dp[%ld][%d][%ld]=%ld, getting from dp[%ld][%d][%ld] (%
                  //     " "ld)\n ", u, q, g, cost_uv + cost_vt, v, q - 1,
                  //     g_at_v, dp[v][q - 1][g_at_v]);
                  BEST = dp[u][q][g];
                  BEST_STOP = q;
                }
              }
            }
          }
      }
  }

  // print g
  // for (auto u : gr.GetNodes()) {
  //   for (auto v : reachVs[u])
  //     printf(" %ld %ld %ld\n", u, v, reachDist[u][v]);
  // }
}

void fill_table_Qnlogn(const Graph &gr, const Sets &reachVs, const Sets &predVs,
                       const Maps &reachDist, const Map &c, const long s,
                       const long t, const long Q, const long U,
                       const unordered_map<long, set<long>> &GV, Maps3D &dp) {

  // O(Q*n^2*logn) version

  for (int q = 1; q <= Q; q++) {
    for (auto u : gr.GetNodes())
      if (u != t) {
        // printf("GV(%ld): ", u);
        // for (auto g : GV[u])
        //   printf("%ld ", g);
        // printf("\n");
        // ensure dp.at(u).at(q) is not null
        dp[u][q] = {};
        vector<pair<long, long>> indVs;
        long indVar;
        for (auto v : reachVs.at(u))
          if (v != u) {
            if (c.at(u) < c.at(v)) {
              indVar = add(getVal(dp, v, q - 1, U - reachDist.at(u).at(v)),
                           U * c.at(u));
            } else
              indVar =
                  add(getVal(dp, v, q - 1, 0), reachDist.at(u).at(v) * c.at(u));
            if (indVar != INF)
              indVs.push_back({indVar, v});
          }
        sort(indVs.begin(), indVs.end());
        int i = 0, v;
        for (auto g : GV.at(u)) {
          while (i < indVs.size()) {
            v = indVs[i].second;
            if (g > reachDist.at(u).at(v))
              i++;
            else
              break;
          }
          if (i < indVs.size()) {
            long gp = c.at(u) < c.at(v) ? U - reachDist.at(u).at(v) : 0;
            // printf("dp[%ld][%d][%ld]=%ld, getting from dp[%d][%d][%ld]
            // (%ld)\n",
            //        u, q, g, indVs[i].first - g * c[u], v, q - 1, gp,
            //        dp[v][q - 1][gp]);
            dp[u][q][g] = indVs[i].first - g * c.at(u);
            NUMSTATE++;
            assert(indVs[i].first - g * c.at(u) >= 0);
            if (u == s and g == 0 && BEST > dp[u][q][g]) {
              BEST = dp[u][q][g];
              BEST_STOP = q;
            }
          }
        }
      }
  }
}

long solve_table(const std::vector<StationData> &stations, const long s,
                 const long t, const long Q, const long U) {
  // variable names are consist with the paper 'to fill or not to fill'
  // s, t: start and target
  // Q: max stop
  // U: max capacity
  Graph gr;
  Sets reachVs, predVs;
  Maps reachDist;
  Map c; // cost on each station
  build_graph(stations, gr);

	// preprocessing for all queries, which can be amortized
	// so we don't count it in elapsed time
	auto tpre = std::chrono::steady_clock::now();
  init(t, U, gr, c, reachVs, reachDist);
  for (auto u : gr.GetNodes()) {
    for (auto v : reachVs[u]) { // edge: u -> v
      predVs[v].insert(u);
    }
  }
  unordered_map<long, set<long>> GV;
  for (auto u : gr.GetNodes()) {
    GV[u] = {0};
    for (auto v : predVs[u]) {
      if (c[v] < c[u]) {
        GV[u].insert(U - reachDist[v][u]);
      }
    }
  }
  auto tstart = std::chrono::steady_clock::now();
	RTPREC = std::chrono::duration<double>(tstart - tpre).count();

  RTINIT = 0;
  // state: dp[v][q][g] min cost from v to t, starting with g gas, using q stops
  // including v dp(v, q, g) =
  // 1. dp(v', q-1, 0) + c(v) * (d(v, v')-g) <-- if c(v) > c(v');
  // 2. dp(v', q-1, U-d(v, v')) + c(v) * (U - g) <--- if c(v) < c(v')
  Maps3D dp;
  NUMSTATE = 1;
  BEST = std::numeric_limits<long>::max();
  // init
  dp[t][0][0] = 0;
  c[t] = 0;
  // fill_table_Qn3(gr, reachVs, predVs, reachDist, c, s, t, Q, U, GV, dp);
  fill_table_Qnlogn(gr, reachVs, predVs, reachDist, c, s, t, Q, U, GV, dp);

  auto tnow = std::chrono::steady_clock::now();
  RT = std::chrono::duration<double>(tnow - tstart).count();
  return BEST;
}

long solve_bfs(const std::vector<StationData> &stations, const long s,
               const long t, const long K, const long Q) {
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
  NUMSTATE = 1;

  BEST = std::numeric_limits<long>::max();

  auto tstart = std::chrono::steady_clock::now();

  while (!q.empty()) {
    auto c = q.front();
    q.pop();
    long costCur = stationCost[c.v];
    long gCur = dp[c.k][c.q][c.v];
    if (c.k > KMAX)
      continue;
    ;
    if (c.v == t && BEST > gCur) {
      BEST = gCur;
      BEST_STOP = c.k;
    }

    auto tnow = std::chrono::steady_clock::now();
    if (std::chrono::duration<double>(tnow - tstart).count() > TIMELIMIT) {
      break;
    }

    for (auto nxt : reachVs[c.v]) {
      long costNxt = stationCost[nxt];
      long dist = reachDist[c.v][nxt];
      long qNxt, gNxt, kNxt;
      if (costNxt > costCur) {
        qNxt = Q - dist;
        kNxt = c.k + 1;
        gNxt = gCur + (Q - c.q) * costCur;
      } else {
        if (dist >= c.q) {
          qNxt = 0;
          kNxt = c.k + 1;
          gNxt = gCur + (dist - c.q) * costCur;
        } else {
          // reaching a station with non-empty tank is not optimal
          continue;
        }
      }
      ++NUMSTATE;
      if (getVal(dp, kNxt, qNxt, nxt) > gNxt) {
        dp[kNxt][qNxt][nxt] = gNxt;
        q.push({kNxt, qNxt, nxt});
      }
    }
  }

	PATHLENGTH = SHORTESTLENGTH = 0;

  auto tnow = std::chrono::steady_clock::now();
  RT = std::chrono::duration<double>(tnow - tstart).count();
  return BEST;
}

} // namespace dp

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
  // solve_bfs(stations, SID, TID, KMAX, QMAX);
  solve_table(stations, SID, TID, KMAX, QMAX);

  string mapname = get_name(GFILE);
  stringstream row;
  ofstream fout;
  fout.open("output/" + mapname + ".log", ios_base::app);
	// s -> us
	RT *= 1e6;
	RTINIT *= 1e6;
	RTPREC *= 1e6;
  row << mapname << "," << SID << "," << TID << "," << KMAX << "," << QMAX
      << ",dp," << BEST << "," << NUMSTATE << "," << setprecision(4) << RT
      << "," << RTINIT << "," << RTPREC << "," << PATHLENGTH << "," << SHORTESTLENGTH;
  fout << row.str() << endl;
  cout << row.str() << endl;
}
