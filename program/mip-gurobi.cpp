#include <cmath>
#include <gurobi_c++.h>
#include <gurobi_c.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <vector>
#include "expr_utils.hpp"


namespace gurobi {

using namespace std;
double RT, TIMELIMIT;
std::string GFILE;
long SID, TID, BEST, QMAX, KMAX;

int idmaping(const vector<StationData>& stations, map<long, long>& ids) {
  set<int> vars;
  ids.clear();
  for (auto s: stations) {
    vars.insert(s.id_from);
    vars.insert(s.id_to);
  }
  int cnt = 0;
  for (auto v: vars) {
    ids[v] = cnt++;
  }
  return cnt;
}

long solve(const vector<StationData> &station, 
           long s, long t, const long K, const long Q) {

  map<long, long> ids;
  int n = idmaping(station, ids);
  std::vector<long> c(n, numeric_limits<long>::max());
  std::vector<std::vector<long>> d(n, std::vector<long>(n, 0));
  std::vector<std::vector<int>> E(n, std::vector<int>(n, 0));
  for (auto i : station) {
    long frID = ids[i.id_from];
    long toID = ids[i.id_to];
    c[frID] = i.id_from == t? 0: i.cost;
    d[frID][toID] = i.distance;
  }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      E[i][j] = d[i][j] > 0 ? 1 : 0;

  s = ids[s];
  t = ids[t];

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
    vector<GRBVar> g(n); // the amount of gas purchased to refuel the vehicle at v_i
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
      for (int j = 0; j < n; j++) {
        if (i != j) {
          // if (x[i][j] == 1) then (...) == 0
          model.addGenConstrIndicator(x[i][j], 1,
                                      (r[i] + g[i] - d[i][j] - r[j] == 0));
        }
      }
      // Tank Capacity
      model.addConstr(r[i] + g[i] <= Q);

      // Stops Constraint
      GRBLinExpr stops = 0;
      for (int j=0; j<n; j++) if (j != i) {
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
    RT = model.get(GRB_DoubleAttr_Runtime);

  } catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch (...) {
    cout << "Exception during optimization" << endl;
  }
  return BEST;
} 

};

int main(int argc, char** argv) {

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
  RT *= 1e6; // s to us
  cout << " cost = " << BEST << endl;

  string mapname = get_name(GFILE);
  ofstream fout;
  fout.open("output/" + mapname + ".log", ios_base::app);
  fout << mapname << "," << SID << "," << TID << "," << KMAX << "," << QMAX << ",gurobi," 
       << BEST << "," << 0 << ","
       << setprecision(4) << RT << endl;
  return 0;
}
