#include <cmath>
#include <fstream>
#include <gurobi_c++.h>
#include <gurobi_c.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

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

// Define input data structures
struct Edge {
  double distance;
  double cost;
  int index_from;
  int index_to;
};

std::vector<StationData> station(const std::string& fname, long vo, long vd) {

  std::vector<GasData> gasData;
  std::vector<StationData> Station;

  std::ifstream file(fname); // FILE PATH

  if (!file.is_open()) {
    std::cout << "Error opening the file !!" << std::endl;
  }
  //
  std::string line;
  std::getline(file, line); // skip the header line
  //
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

  // TODO
  // OPENSTREETMAP Dataset: //--> for the openstreetmap dataset since most data
  // is in float format, and we need to bring it to integral ones for (auto i:
  // gasData)
  // {
  //   long nodeFrom = std::round(i.nodeFrom);
  //   long nodeTo = std::round(i.nodeTo);
  //   long distance = std::round(1000*i.distance);
  //   long cost = std::round(1000*i.cost);
  //   long id_from = std::round(i.id_from);
  //   long id_to = std::round(i.id_to);
  //   Station.push_back({nodeFrom,nodeTo ,distance ,cost, id_from, id_to});
  // }

  /*
    CONVERTING ALL THE LONG DOUBLES TO LONG TYPE:
  */
  for (auto i : gasData) {
    long nodeFrom = i.nodeFrom;
    long nodeTo = i.nodeTo;
    long distance = i.distance;
    long cost = i.cost;
    long id_from = i.id_from;
    long id_to = i.id_to;
    Station.push_back({nodeFrom, nodeTo, distance, cost, id_from, id_to});
  }
  return Station;
};

void solve(const vector<StationData> &station, long Kmax, long U,
           long s, long t) {

  int n = station.size();
  std::vector<long> c(n, 0);
  std::vector<std::vector<long>> d(n, std::vector<long>(n, 0));
  std::vector<std::vector<int>> E(n, std::vector<int>(n, 0));
  for (auto i : station) {
    if (i.id_from == t) {
      c[i.id_from - 1] = 0;
      d[i.id_from - 1][i.id_to - 1] = i.distance;
    } else {
      c[i.id_from - 1] = i.cost;
      d[i.id_from - 1][i.id_to - 1] = i.distance;
    }
  }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      E[i][j] = d[i][j] > 0 ? 1 : 0;

  try {

    // Create an environment
    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "mip1.log");
    env.start();

    // Create an empty model
    GRBModel model = GRBModel(env);

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
      r[i] = model.addVar(0, U, 0, GRB_INTEGER);
      g[i] = model.addVar(0, U, 0, GRB_INTEGER);
      y[i] = model.addVar(0, 1, 0, GRB_BINARY);
    }

    // Constraints
    model.addConstr(r[0] == 0);
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
      if (i == s - 1) {
        model.addConstr(flow_out - flow_in == 1);
      } else if (i == t - 1) {
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
      model.addConstr(r[i] + g[i] <= U);

      // Stops Constraint
      GRBLinExpr stops = 0;
      for (int j=0; j<n; j++) if (j != i) {
        stops += y[j];
      }
      model.addConstr(stops <= Kmax);
    }

    // Objective: min sum(ci * gi)
    GRBLinExpr obj = 0;
    for (int i = 0; i < n; i++) {
      obj += c[i] * g[i];
    }
    model.setObjective(obj, GRB_MINIMIZE);
    model.optimize();

  } catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch (...) {
    cout << "Exception during optimization" << endl;
  }
}

int main(int argc, char** argv) {

  std::string file = std::string(argv[1]);
  long s = std::stoi(argv[2]);
  long t = std::stoi(argv[3]);
  // long K_max = 10;
  // long U = 6000;
  // long s = 2;
  // long t = 30;
  long kMax = 5, qMax = 6;

  std::vector<StationData> Station = station(file, s, t);
  solve(Station, kMax, qMax, s, t);
  return 0;
}
