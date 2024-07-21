
#include "erca_refill.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <limits>

namespace rzq {
namespace search {

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// long prned=0; //--> to count the number of pruned labels
// std::unordered_map<long, std::vector<long>> Frontier_size;
// std::vector<long> median;
// long total_frontier_size=0;

FrontierNaive::FrontierNaive() { return; };

FrontierNaive::~FrontierNaive() { return; };

bool FrontierNaive::Check(basic::CostVector g) {
  for (auto l_prime : labels) {
    // ********** TODO, implement this method ***********
    // check if the input g is worse than any existing l_prime.g
    if (g[0] >= l_prime.second.g[0] && g[1] <= l_prime.second.g[1] &&
        g[2] <= l_prime.second.g[2]) {
      // prned++;
      return true;
    }
  }
  return false;
};

void FrontierNaive::Update(Label l) {

  for (auto l_prime : labels) {
    // ********** TODO, implement this method ***********
    // check any existing l_prime.g is worse than the input l.g
    if (l.g[0] >= l_prime.second.g[0] && l.g[1] <= l_prime.second.g[1] &&
        l.g[2] <= l_prime.second.g[2]) {
      label_ids.erase(l_prime.first);
    }
  }

  // add l to the frontier.
  this->label_ids.insert(l.id);
  this->labels[l.id] = l;

  // Frontier_size[l.v].push_back(labels.size());
  return;
};

// std::ostream& operator<<(std::ostream& os, FrontierNaive& f) {
//   os << "{";
//   for (auto v : f.label_ids) {
//     std::cout << v << ",";
//   }
//   os << "}";
//   return os;
// };

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

AstarRefill::AstarRefill(){};

AstarRefill::~AstarRefill(){};

int AstarRefill::Search(long vo, long vd, double time_limit) {

  // std::cout << "[INFO] AstarRefill::Search starts !" << std::endl;
  auto s = std::chrono::steady_clock::now();

  // ### init heu and compute reachable sets ###
  auto t1 = std::chrono::steady_clock::now();
  InitHeu(vd); // use the same one as in EMOAKd (EMOA).
  _computeReachableSets();
  auto t2 = std::chrono::steady_clock::now();
  _res.rt_initHeu = std::chrono::duration<double>(t2 - t1).count();

  // std::cout<< "Check before entering OPEN while loop--1"<< std::endl; //-->
  // This is working

  // ### init ###
  auto tstart = std::chrono::steady_clock::now();
  _vo = vo;
  _vd = vd;
  basic::CostVector init_vec;
  init_vec.resize(_vec_len, 0);
  std::cout << _Heuristic(_vo) << std::endl; // TODO: this is not working!!
  Label lo(_GenLabelId(), vo, init_vec, _Heuristic(_vo));
  _label[lo.id] = lo;
  _res.n_generated++;
  _open.insert(std::make_pair(lo.f, lo.id));

  // std::cout<< "Check before entering OPEN while loop--2"<< std::endl;

  // ### main search loop ###
  while (!_open.empty()) {

    // check timeout
    auto tnow = std::chrono::steady_clock::now();
    if (std::chrono::duration<double>(tnow - tstart).count() > time_limit) {
      // std::cout << "[INFO] AstarRefill::Search timeout !" << std::endl;
      _res.timeout = true;
      break;
    }

    // ## select label l ##
    Label l = _label[_open.begin()->second];
    _open.erase(_open.begin());
    std::cout << "g , q, k , t, cap: " << l.g[0] << " " << l.g[1] << " "
              << l.g[2] << " " << l.g[3] << std::endl;

    // TODO read this notes:
    // l.g[0] = the cost value of this label. the g-value
    // l.g[1] = the remaining fuel of this label. the q-values
    // l.g[2] = the number of stops.
    // l.g[3] = the amount of time taken.
    // l.f[0] = the f-cost value of this label. the f-value
    // l.f[1] = meaningless, which should be and must be always zero.

    if (DEBUG_REFILL > 0) {
      std::cout << "[DEBUG] ### Pop l = " << l << std::endl;
    }

    // std::cout<<"Label now is: id ="<< l.id<<" v ="<<l.v<<" g ="<<l.g[0]<<" q
    // ="<<l.g[1]<<" f ="<<l.f <<std::endl; std::cout<<"Label vertex : "<<
    // l.v<<std::endl; std::cout<<"Heuristic now is :"<< _Heuristic(l.v)[0]
    // <<std::endl;
    if (_checkForPrune(l)) {
      continue;
    }

    _filterAndAddFront(l); // make sure if l.v == _vd, l is added to _alpha[l.v]
                           // so that later _PostProcRes() will work correctly.

    if (l.v == _vd) {
      break; // [DIFF FROM EMOA]
    }

    if (l.g[2] == _k_max) {
      continue;
    }

    auto nghs = _reachableSets[l.v];

    // for (auto u: nghs)
    // {
    //   std::cout<< "--the neighbours are-- : "<< u<<std::endl;
    //   std::cout << _roadmap->GetCost(u, l.v) << std::endl;
    //   std::cout<< "the neighbours are : "<< u<<" with edge costs
    //   "<<_roadmap->GetCost(u, l.v)[0]<<" "; std::cout <<" and
    //   "<<_roadmap->GetCost(u, l.v)[1] << " "; std::cout <<" and heuristic as
    //   : "<<_Heuristic(u)[0]<<std::endl;
    // }

    alg_iter++;

    for (auto u : nghs) {
      if (u == l.v) {
        continue;
      }
      Label l_prime;
      // ********** TODO, implement the expansion of label l ***********
      basic::CostVector c = _roadmap->GetCost(l.v, u);

      long g_prime, q_prime, k_prime; //, t_prime;
                                      // long g_prime, q_prime;

      if (_roadmap->GetCost(u, l.v)[0] > c[0]) {
        g_prime = l.g[0] + (_q_max - l.g[1]) * c[0];
        q_prime = _q_max - _roadmap->GetCost(l.v, u)[1];
        k_prime = l.g[2] + 1;
        // t_prime = l.g[3] + _t_fuel*(_q_max - l.g[1]) +
        // _t_fuel*(_roadmap->GetCost(l.v, u)[1]);
      }

      else {
        if (_roadmap->GetCost(u, l.v)[1] > l.g[1]) {
          g_prime = l.g[0] + (_roadmap->GetCost(u, l.v)[1] - l.g[1]) * c[0];
          q_prime = 0;
          k_prime = l.g[2] + 1;
          // t_prime = l.g[3] + _t_fuel*(_roadmap->GetCost(u, l.v)[1] - l.g[1])
          // + _t_fuel*(_roadmap->GetCost(u, l.v)[1]);
        }
      }

      l_prime.id = _GenLabelId(); //-->working
      // std::cout<<"l_prime id : "<<l_prime.id<<std::endl;
      l_prime.v = u;
      // std::cout<<"l_prime v : "<<l_prime.v<<std::endl;
      l_prime.f = _Heuristic(u);
      // std::cout<<"f_value is : "<<l_prime.f<<std::endl;

      basic::CostVector g_prime_vec;
      g_prime_vec.resize(_vec_len, 0);
      g_prime_vec[0] = g_prime;
      g_prime_vec[1] = q_prime;
      g_prime_vec[2] = k_prime;
      // g_prime_vec[3] = t_prime;
      l_prime.g = g_prime_vec;

      if (_checkForPrune(l_prime)) {
        continue;
      }

      l_prime.f[0] += l_prime.g[0];
      _label[l_prime.id] = l_prime;
      _parent[l_prime.id] = l.id;

      _open.insert(std::make_pair(l_prime.f, l_prime.id));

      _res.n_generated++;
    } // end of the for-loop.
  } // end of the while-loop

  _PostProcRes();
  auto e = std::chrono::steady_clock::now();
  std::cout << "[INFO] AstarRefill::Search exit." << std::endl;

  std::cout
      << "Elapsed time in microseconds: "
      << std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
      << " us" << std::endl;

  // std::cout<< "MET-1==================>"<< std::endl;
  // std::cout<< "Number of iterations or the number of expanded labels
  // :"<<alg_iter<< std::endl; std::cout<< "Number of labels pruned : "<< prned
  // << std::endl; long _m = -1; long max_val =
  // std::numeric_limits<long>::min();

  // for (auto& i: Frontier_size)
  // {
  //     //  std::cout<< "size of frontier set at vertex "<< i.first <<": { ";
  //     // for (auto& j: i.second)
  //     // {
  //     //      std::cout<<j <<", ";
  //     // }
  //     long vert = i.first;
  //     const std::vector<long>& frontier = i.second;
  //     total_frontier_size += frontier.size();
  //     long max = *std::max_element(frontier.begin(), frontier.end());
  //     if(max > max_val)
  //     {
  //         _m = vert;
  //         max_val = max;
  //     }
  //     std::size_t n = i.second.size() / 2;
  //     std::nth_element(i.second.begin(), i.second.begin()+n, i.second.end());
  //     median.push_back(i.second[n]);

  //     // std::cout<<" }"<<" (Min, max)
  //     :"<<*std::min_element(i.second.begin(), i.second.end()) <<
  //     //     " "<< *std::max_element(i.second.begin(),
  //     i.second.end())<<std::endl;
  //
  // }

  // // Median of the median set:
  // std::size_t n_m = median.size() / 2;
  // std::nth_element(median.begin(), median.begin()+n_m, median.end());
  // std::cout<<"Median : "<<median[n_m] << std::endl;
  // std::cout<< "max vertex and val : "<< _m <<" " << max_val << std::endl;
  // std::cout<< "the size of the entire frontier set for this case is :"<<
  // total_frontier_size<< std::endl;

  return 1;
};

void AstarRefill::SetRefillCosts(std::unordered_map<long, long> refillCosts) {
  _refill_costs = refillCosts;
  return;
}

void AstarRefill::SetRoadmap(basic::Roadmap *g) {
  _roadmap = g;
  _graph = _roadmap;
};

void AstarRefill::SetQmax(long qm) { _q_max = qm; };

void AstarRefill::SetKmax(long km) { _k_max = km; };

basic::CostVector AstarRefill::_Heuristic(long v) {
  basic::CostVector out;
  out.resize(_vec_len, 0);
  out[0] = EMOA::_Heuristic(v)[0];
  return out;
};

bool AstarRefill::_checkForPrune(Label l) {

  if (_alpha.find(l.v) == _alpha.end()) {
    return false;
  }
  return _alpha[l.v].Check(l.g);
};

void AstarRefill::_filterAndAddFront(Label l) {
  std::cout << " _filterAndAddFront input l = " << l << std::endl;
  if (_alpha.find(l.v) == _alpha.end()) {
    _alpha[l.v] = FrontierNaive();
  }
  _alpha[l.v].Update(l);

  return;
};

void AstarRefill::_PostProcRes() {
  std::cout << "_PostProcRes enter _vd = " << _vd << std::endl;
  if (_alpha.find(_vd) != _alpha.end()) {
    std::cout << "_PostProcRes" << _alpha[_vd].label_ids.size() << std::endl;
    for (auto lid : _alpha[_vd].label_ids) {
      _res.paths[lid] = _BuildPath(lid);
      _res.costs[lid] = _label[lid].g;
    }
  }
  return;
};

void AstarRefill::_computeReachableSets() {

  auto all_vertices = _roadmap->GetNodes();

  /*  TO CHECK IF ALL VERTICES IS WORKING OR NOT  */
  // for (int element : all_vertices) {
  //       std::cout << element << " ";
  //   }
  // std::cout << std::endl;

  // Initialize reachable sets
  for (auto v : all_vertices) {
    _reachableSets[v] = {};
  }

  for (auto v : all_vertices)

  // ********** TODO, implement this method ***********

  // Compute _reachableSets, which is an unordered map.
  // it maps an vertex ID to an unordered_set that stores the reachable vertex
  // IDs. E.g. _reachableSets[3] is an unordered_set of vertices that are
  // reachable from vertex 3.

  // to get the successors of a vertex w in the graph:
  //   succs = _roadmap->GetSuccs(w); // here, succs is an unordered_set of
  //   vertex IDs.

  // to get the cost of an edge (v,u) c = _roadmap->GetCost(v,u),
  //      which returns a CostVector of length one, and c[0] is the cost value
  //      of the edge.
  {
    std::unordered_map<int, long> d_star;
    std::unordered_set<int> OPENv;

    // std::cout<<"V IS :"<<v<<std::endl; //-->working

    // Initialize distances
    for (auto u : all_vertices) {
      d_star[u] = std::numeric_limits<long>::max();
      // std::cout<<"D IS :"<<d_star[u]<<std::endl; //-->working
    }
    d_star[v] = 0.0;
    // std::cout<<"------------------------"<<std::endl; //-->working

    OPENv.insert(v);

    while (!OPENv.empty()) {
      int u = *OPENv.begin();
      OPENv.erase(OPENv.begin());

      // std::cout<<" U is (CRS) :"<<u<<std::endl; //-->working

      // std::cout << " _q_max = " << _q_max << std::endl;

      if (d_star[u] > _q_max) {
        // std::cout<<d_star[u] << " and "<< _q_max<< std::endl;
        continue;
      } else {
        _reachableSets[v].insert(u);
      }

      // Get successors of u
      std::unordered_set<long> succs = _roadmap->GetSuccs(u);

      for (auto u_prime : succs) {
        // std::cout<<"SUCCS is : "<<u_prime<<std::endl; //-->working
        if (_reachableSets[v].count(u_prime) > 0) {
          continue;
        }

        // Get cost of edge (u, u_prime)
        std::vector<long> c = _roadmap->GetCost(u, u_prime);
        long edge_cost = c[1]; // distance
        // std::cout<<"EDGE COST IS : "<<edge_cost<<std::endl; //-->working

        // std::cout<<"u_prime d: "<< d_star[u_prime]<< " & u d:"<<d_star[u]<<"
        // + edge cost : "<<edge_cost<<std::endl; //--> working as needed.

        if (d_star[u_prime] > (d_star[u] + edge_cost)) {
          d_star[u_prime] = d_star[u] + edge_cost;
          // std::cout<<"FINAL d_star_cost :"<<d_star[u_prime]<<std::endl;
          // //-->working
          OPENv.insert(u_prime);
        }
      }
    }
  }

  // for (const auto& entry : _reachableSets) {
  //       long key = entry.first;
  //       const std::unordered_set<long>& setValues = entry.second;
  //       std::cout << "Key: " << key << ", Set Values: ";
  //       for (const auto& value : setValues) {
  //           std::cout << value << " ";
  //       }
  //       std::cout << std::endl;
  //   }
  /*
    OUTPUT IS:
      For 6 node graph:
        Key: 1, Set Values: 2 3 1
        Key: 2, Set Values: 1 3 4 2
        Key: 3, Set Values: 1 2 5 4 6 3
        Key: 4, Set Values: 1 2 3 5 6 4
        Key: 5, Set Values: 3 4 6 5
        Key: 6, Set Values: 3 4 5 6

      for second graph: (4 nodes only)
        Key: 1, Set Values: 2 3 4 1
        Key: 2, Set Values: 1 3 4 2
        Key: 3, Set Values: 1 2 4 3
        Key: 4, Set Values: 1 2 3 4
  */

  return;
};

} // end namespace search
} // end namespace rzq
