
/*******************************************
 * Author: Zhongqiang Richard Ren.
 * All Rights Reserved.
 *******************************************/

#include "api.hpp"
#include "erca_refill.hpp"

int TestRoadmapToyExample(); 

int main(){
  TestRoadmapToyExample();
  return 0;
};

int TestRoadmapToyExample() {
  rzq::basic::Roadmap g; // declare a graph
  g.Init(6, 1); // The graph has 6 vertices. Each edge in the graph has a scalar cost (of length 1).
  // graph vertex ID start from 1 and increase by one by one.

  rzq::basic::CostVector cv = rzq::basic::CostVector(0, 1); // create a vector of length 1. All values in the vector is initialized to 0.
  cv[0] = 1; // set the cost vector to be [1].
  g.AddEdge(1, 2, cv); // add an arc from vertex 1 to vertex 2 with edge cost being the cost vector we just set.
  g.AddEdge(2, 1, cv); // add an arc from vertex 2 to vertex 1 with edge cost being the cost vector we just set.

  cv = rzq::basic::CostVector(0, 1);
  cv[0] = 9;
  g.AddEdge(1, 3, cv);
  g.AddEdge(3, 1, cv);

  cv = rzq::basic::CostVector(0, 1);
  cv[0] = 5;
  g.AddEdge(2, 3, cv);
  g.AddEdge(3, 2, cv);

  cv = rzq::basic::CostVector(0, 1);
  cv[0] = 10;
  g.AddEdge(2, 4, cv);
  g.AddEdge(4, 2, cv);

  cv = rzq::basic::CostVector(0, 1);
  cv[0] = 4;
  g.AddEdge(3, 4, cv);
  g.AddEdge(4, 3, cv);

  cv = rzq::basic::CostVector(0, 1);
  cv[0] = 3;
  g.AddEdge(4, 5, cv);
  g.AddEdge(5, 4, cv);

  cv = rzq::basic::CostVector(0, 1);
  cv[0] = 10;
  g.AddEdge(1, 3, cv);
  g.AddEdge(3, 1, cv);

  cv = rzq::basic::CostVector(0, 1);
  cv[0] = 3;
  g.AddEdge(5, 6, cv);
  g.AddEdge(6, 5, cv);

  // do some print and verification to make sure the generated graph is correct.

  std::cout << "num_nodes: " << g.GetNumberOfNodes() << std::endl;
  std::cout << "num_edges: " << g.GetNumberOfEdges() << std::endl;
  std::cout << "cdims: " << g.GetCostDim() << std::endl;
  std::cout << "g.GetCost(1, 2): " << g.GetCost(1, 2) << std::endl;
  std::cout << "g.GetCost(1, 3): " << g.GetCost(1, 3) << std::endl;
  std::cout << "g.GetCost(4, 5): " << g.GetCost(4, 5) << std::endl;

  long vo = 1; // starting vertex in the graph.
  long vd = 6; // destination vertex id in the graph.
  double time_limit = 60; // e.g. one minute search time limit.

  rzq::search::EMOAResult res;

  // // std::vector<long> resour_limit({26});
  // std::vector<long> resour_limit({17,4});

  rzq::search::AstarRefill planner;

  planner.SetRoadmap(&g);

  planner.Search(vo, vd, time_limit); // TODO, this is the entry point for details.

  res = planner.GetResult();

  // print paths, times and costs
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

  return 1;

};