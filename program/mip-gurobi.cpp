#include "expr_utils.hpp"
#include "graph.hpp"
#include <chrono>
#include <cmath>
#include <gurobi_c++.h>
#include <gurobi_c.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gurobi {

using namespace std;
using Graph = rzq::basic::Roadmap;
using Graph = rzq::basic::Roadmap;
using Sets = unordered_map<long, std::unordered_set<long>>;
using Map = unordered_map<long, long>;      // m[i] = v
using Maps = std::unordered_map<long, Map>; // m[i][j] = v

double RT, TIMELIMIT;
std::string GFILE;
long SID, TID, BEST, QMAX, KMAX;

int idmaping(const vector<StationData> &stations, map<long, long> &ids) {
  set<int> vars;
  ids.clear();
  for (auto s : stations) {
    vars.insert(s.id_from);
    vars.insert(s.id_to);
  }
  int cnt = 0;
  for (auto v : vars) {
    ids[v] = cnt++;
  }
  return cnt;
}

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

void init(long t, long Q, Graph &g, Sets &reachVert, Maps &reachDist) {
  for (auto v : g.GetNodes()) {
    reachVert[v] = {};
    reachDist[v] = {};
    init_v(v, Q, g, reachVert, reachDist);
  }
}

long solve(const vector<StationData> &station, long s, long t, const long K,
           const long Q) {

  Graph gr;
  Sets reachVs, predVs;
  Maps reachDist;
  build_graph(station, gr);
  init(t, Q, gr, reachVs, reachDist);

  map<long, long> ids;
  int n = idmaping(station, ids);
  auto inf = numeric_limits<long>::max();
  std::vector<long> c(n, inf);
  std::vector<std::vector<long>> d(n, std::vector<long>(n, 0));
  std::vector<std::vector<long>> a(n, std::vector<long>(n, 0));
  std::vector<std::vector<int>> E(n, std::vector<int>(n, 0));
  for (auto i : station) {
    long frID = ids[i.id_from];
    long toID = ids[i.id_to];
    c[frID] = i.id_from == t ? 0 : i.cost;
  }
  for (auto u : gr.GetNodes()) {
    for (auto v : reachVs[u]) {
      auto dist = reachDist[u][v];
      auto uid = ids[u];
      auto vid = ids[v];
      d[uid][vid] = dist;
    }
  }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      E[i][j] = d[i][j] > 0 ? 1 : 0;

  s = ids[s];
  t = ids[t];

  auto tstart = std::chrono::steady_clock::now();
  BEST = std::numeric_limits<long>::max();
  try {

    // Create an environment
    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "mip1.log");
    env.set("LogToConsole", "0");
    env.start();

    // Create an empty model
    GRBModel model = GRBModel(env);
    model.set(GRB_DoubleParam_TimeLimit, TIMELIMIT);

    // decision varaibles
    vector<vector<GRBVar>> x(n, vector<GRBVar>(n));
    vector<GRBVar> r(n); // amount of gas left in the tank
    vector<GRBVar> g(
        n); // the amount of gas purchased to refuel the vehicle at v_i
    vector<GRBVar> y(n); // stop mad or not

    // init varaibles
    for (int i = 0; i < n; i++)
      for (int j = 0; j < n; j++) {
        x[i][j] = model.addVar(0, 1, 0, GRB_BINARY);
      }

    for (int i = 0; i < n; i++) {
      r[i] = model.addVar(0, Q, 0, GRB_INTEGER);
      g[i] = model.addVar(0, Q, 0, GRB_INTEGER);
      y[i] = model.addVar(0, 1, 0, GRB_BINARY);
    }

    // Constraints
    model.addConstr(r[s] == 0);
    for (int i = 0; i < n; i++)
      for (int j = 0; j < n; j++) {
        model.addConstr(x[i][j] <= E[i][j]);
      }
    for (int i = 0; i < n; i++) {
      // Flow Constraint
      GRBLinExpr flow_in, flow_out;
      for (int j = 0; j < n; j++) {
        if (i != j) {
          flow_in += x[j][i];
          flow_out += x[i][j];
        }
      }
      if (i == s) {
        model.addConstr(flow_out - flow_in == 1);
      } else if (i == t) {
        model.addConstr(flow_in - flow_out == 1);
      } else {
        model.addConstr(flow_in - flow_out == 0);
      }
      // Gas Conservation
      for (int j = 0; j < n; j++)
        if (j != i) {
          // if (x[i][j] == 1) then (...) == 0
          model.addGenConstrIndicator(x[i][j], 1,
                                      (r[i] + g[i] - d[i][j] - r[j] == 0));
					model.addGenConstrIndicator(x[i][j], 1, y[i] == 1);
          if (c[i] > c[j]) {
            model.addGenConstrIndicator(x[i][j], 1, g[i] == d[i][j]);
          } else {
            model.addGenConstrIndicator(x[i][j], 1, g[i] + r[i] == Q);
          }
        }
      // Tank Capacity
      model.addConstr(r[i] + g[i] <= Q);

      // Stops Constraint
      GRBLinExpr stops = 0;
      for (int j = 0; j < n; j++)
        if (j != i) {
          stops += y[j];
        }
      model.addConstr(stops <= K);
    }

    // Objective: min sum(ci * gi)
    GRBLinExpr obj = 0;
    for (int i = 0; i < n; i++) {
      obj += c[i] * g[i];
    }
    model.setObjective(obj, GRB_MINIMIZE);
    model.optimize();
    BEST = model.get(GRB_DoubleAttr_ObjVal);
  } catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch (...) {
    cout << "Exception during optimization" << endl;
  }
  auto tnow = std::chrono::steady_clock::now();
  RT = std::chrono::duration<double>(tnow - tstart).count();
  return BEST;
}

}; // namespace gurobi

int main(int argc, char **argv) {

  using namespace gurobi;

  GFILE = std::string(argv[1]);
  SID = std::stoi(argv[2]);
  TID = std::stoi(argv[3]);
  KMAX = std::stoi(argv[4]);
  QMAX = std::stoi(argv[5]);
  TIMELIMIT = 60;

  // long K_max = 10;
  // long Q = 6000;
  // long s = 2;
  // long t = 30;
  // long kMax = 5, qMax = 6;

  std::vector<StationData> stations;
  load(GFILE, stations);
  solve(stations, SID, TID, KMAX, QMAX);

  string mapname = get_name(GFILE);
  stringstream row;
  ofstream fout;
  fout.open("output/" + mapname + ".log", ios_base::app);
  // s -> us
  RT *= 1e6;
  row << mapname << "," << SID << "," << TID << "," << KMAX << "," << QMAX
      << ",mip," << BEST << "," << 0 << "," << setprecision(4) << RT << "," << 0
      << "," << 0;
  fout << row.str() << endl;
  cout << row.str() << endl;
}
