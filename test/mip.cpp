#include "api.hpp"
#include "erca_refill.hpp"
#include "ilconcert/iloexpression.h"
#include "ilconcert/ilosys.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <ilcplex/cplex.h>
#include <ilcplex/ilocplex.h>
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

// Define input data structures
struct Edge {
  double distance;
  double cost;
  int index_from;
  int index_to;
};

ILOSTLBEGIN

std::vector<StationData> station(long vo, long vd, long qmax);

int main() {

  long K_max = 10;
  long n = 61; // change based on input, number of vertices
  long U = 6000;
  long s = 1;
  long t = 5;

  std::vector<StationData> Station = station(s, t, U);
  std::vector<long> c(n, 0);
  std::vector<std::vector<long>> d(n, std::vector<long>(n, 0));
  for (auto i : Station) {
    if (i.id_from == t) {
      c[i.id_from - 1] = 0;
      d[i.id_from - 1][i.id_to - 1] = i.distance;
    } else {
      c[i.id_from - 1] = i.cost;
      d[i.id_from - 1][i.id_to - 1] = i.distance;
    }
  }

  auto st = std::chrono::steady_clock::now();
  IloEnv env;
  try {
    IloModel model(env);
    IloArray<IloNumVarArray> x(env, n);
    for (int i = 0; i < n; i++) {
      x[i] = IloNumVarArray(env, n, 0, 1, ILOBOOL);
    }
    IloNumVarArray r(env, n, 0, U, ILOINT); // amount of gas left in the tank
    IloNumVarArray g(
        env, n, 0, U,
        ILOINT); // the amount of gas purchased to refuel the vehicle at v_i
    IloNumVarArray y(env, n, 0, 1, ILOBOOL); // stop made or not!
    IloNumVarArray cost(env, n, 0, IloInfinity, ILOFLOAT);

    for (int i = 0; i < n; i++) {
      model.add(cost[i] == c[i]);
    }
    model.add(r[0] == 0);
    // Objective Function: Minimize sum(c(v)*g_i)
    IloExpr obj(env);
    for (int i = 0; i < n; i++) {
      // std::cout<<"hehehe" <<c[i]<<std::endl; //WORKING
      obj += cost[i] * g[i];
    }
    model.add(IloMinimize(env, obj));
    obj.end();

    std::vector<std::vector<int>> E(n, std::vector<int>(n, 0));
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        if (d[i][j] > 0) {
          E[i][j] = 1;
        } else {
          E[i][j] = 0;
        }
        model.add(x[i][j] <= E[i][j]);
      }
    }

    // Constraints
    for (int i = 0; i < n; i++) {

      // Flow Constraint;
      IloExpr flow_in(env);
      IloExpr flow_out(env);
      for (int j = 0; j < n; j++) {
        std::cout << "i >> " << i << std::endl;
        if (i != j) {
          flow_in += x[j][i];
          flow_out += x[i][j];
        }
      }
      if (i == s - 1) {
        std::cout << "s-1 " << s - 1 << std::endl;
        model.add(flow_out - flow_in == 1);
      } else if (i == t - 1) {
        model.add(flow_in - flow_out == 1);
      } else {
        model.add(flow_in - flow_out == 0);
      }
      flow_in.end();
      flow_out.end();

      // Gas conservation
      for (int j = 0; j < n; j++) {
        if (i != j) {
          model.add(
              IloIfThen(env, x[i][j] == 1, r[i] + g[i] - d[i][j] - r[j] == 0));
        }
      }

      // Tank capacity
      model.add(r[i] + g[i] <= U);

      // Stops constraint
      IloExpr stops(env);
      for (int j = 0; j < n; j++) {
        if (i != j) {
          stops += y[i];
        }
      }
      model.add(stops <= K_max);
      stops.end();
    }

    // IloCplex cplex(model);
    IloCplex cplex(env);
    cplex.extract(model);
    // cplex.solve();
    // solve the model
    if (!cplex.solve()) {
      env.error() << "Failed to optimize LP." << endl;
      throw(-1);
    }

    IloNumArray vals(env);
    env.out() << "Solution status = " << cplex.getStatus() << endl;
    env.out() << "Solution value  = " << cplex.getObjValue() << endl;

    // Get the values of the decision variables
    // for (int i = 0; i < n; i++) {
    //   env.out() << "value of g["<<i<<"] = "<<
    //   cplex.getValue(g[i])<<std::endl; env.out() << "value of r["<<i<<"] =
    //   "<<  cplex.getValue(r[i])<<std::endl;
    // for (int j = 0; j < n; j++) {
    // env.out() << "Value of x[" << i << "][" << j << "] = " <<
    // cplex.getValue(x[i][j]) << endl;
    // }
    // }
  } catch (IloException &e) {
    cerr << "Concert exception caught: " << e << endl;
  } catch (...) {
    cerr << "Unknown exception caught" << endl;
  }
  env.end();
  auto et = std::chrono::steady_clock::now();
  std::cout
      << "Elapsed time in microseconds: "
      << std::chrono::duration_cast<std::chrono::microseconds>(et - st).count()
      << " us" << std::endl;

  return 0;
}

std::vector<StationData> station(long vo, long vd, long qmax) {

  std::vector<GasData> gasData;
  std::vector<StationData> Station;

  std::ifstream file("./dev_erca_refill/City Data/Phil_gas.csv"); // FILE PATH

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
