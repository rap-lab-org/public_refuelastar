
#include "erca_refill.hpp"
#include <chrono>

//DEFINED QMAX:
long qmax = 10;


namespace rzq{
namespace search{

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

FrontierNaive::FrontierNaive() {
  return;
};

FrontierNaive::~FrontierNaive() {
  return;
};

bool FrontierNaive::Check(basic::CostVector g) {
  for (auto l_prime: labels) {
    // ********** TODO, implement this method ***********
    // check if the input g is worse than any existing l_prime.g
    if (g[0] >= l_prime.second.g[0] && g[1] <= l_prime.second.g[1]) 
    {
      return true;
    }
  }
  return false;
};

void FrontierNaive::Update(Label l) {

  for (auto l_prime: labels) 
    {
    // ********** TODO, implement this method ***********
    // check any existing l_prime.g is worse than the input l.g
    if (l.g[0] >= l_prime.second.g[0] && l.g[1] <= l_prime.second.g[1])
      {
      label_ids.erase(l_prime.first);
      }
    else
      {
      return; 
      }
    }

  // add l to the frontier.
  this->label_ids.insert(l.id);
  this->labels[l.id] = l;
  return ;
};

std::ostream& operator<<(std::ostream& os, FrontierNaive& f) {
  os << "{";
  for (auto v : f.label_ids) {
    std::cout << v << ",";
  }
  os << "}";
  return os;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


AstarRefill::AstarRefill() {};

AstarRefill::~AstarRefill() {};

int AstarRefill::Search(long vo, long vd, double time_limit) {

  std::cout << "[INFO] AstarRefill::Search starts !" << std::endl;
  
  // ### init heu and compute reachable sets ###
  auto t1 = std::chrono::steady_clock::now();
  InitHeu(vd); // use the same one as in EMOAKd (EMOA).
  _computeReachableSets();
  auto t2 = std::chrono::steady_clock::now();
  _res.rt_initHeu = std::chrono::duration<double>(t2-t1).count();

  // ### init ###
  auto tstart = std::chrono::steady_clock::now();
  _vo = vo;
  _vd = vd;
  basic::CostVector init_vec;
  init_vec.resize(_vec_len, 0);
  Label lo(_GenLabelId(), vo, init_vec, _Heuristic(_vo));
  _label[lo.id] = lo;
  _res.n_generated++;
  _open.insert( std::make_pair(lo.f, lo.id) );
  
  // ### main search loop ###
  while ( !_open.empty() ) {

    // check timeout
    auto tnow = std::chrono::steady_clock::now();
    if (std::chrono::duration<double>(tnow-tstart).count() > time_limit) {
      std::cout << "[INFO] AstarRefill::Search timeout !" << std::endl;
      _res.timeout = true;
      break;
    }

    // ## select label l ##
    Label l = _label[ _open.begin()->second ];
    _open.erase(_open.begin());
    std::cout<<"Label now is: "<< l.id<<" "<<l.g[0] <<std::endl;

    // TODO read this notes:
    // l.g[0] = the cost value of this label. the g-value
    // l.g[1] = the remaining fuel of this label. the q-value
    // l.f[0] = the f-cost value of this label. the f-value
    // l.f[1] = meaningless, which should be and must be always zero.

    if (DEBUG_REFILL > 0) {
      std::cout << "[DEBUG] ### Pop l = " << l << std::endl;
    }

    if (_checkForPrune(l)) {
      continue;
    }

    _filterAndAddFront(l); // make sure if l.v == _vd, l is added to _alpha[l.v] so that later _PostProcRes() will work correctly.

    if (l.v == _vd) {
      break; // [DIFF FROM EMOA]
    }

    auto nghs = _reachableSets[l.v];

    for (auto u : nghs) {
       // ********** TODO, implement the expansion of label l ***********
      basic::CostVector c = _roadmap->GetCost(l.v, u);
      long edge_cost = c[0];
      
      long g_prime, q_prime;

      if(c[1] > c[0])
      {
        g_prime = l.g[0] + (qmax - l.g[1])*c[0];
        q_prime = qmax + _roadmap->GetCost(u, l.v)[0];
      }

      else 
      {
        if(_roadmap->GetCost(u, l.v)[0] > l.g[1])
        {
          g_prime = l.g[0] + (_roadmap->GetCost(u, l.v)[0] - l.g[1]) * c[0];
          q_prime = 0;
        }
        else
        {
          g_prime = l.g[0];
          q_prime = l.g[1] - _roadmap->GetCost(u, l.v)[0];
        }
      }
      Label l_prime(_GenLabelId(), u, basic::CostVector{g_prime, q_prime}, _Heuristic(u));

      if (_checkForPrune(l_prime)) {
        continue;
      }

      _label[l_prime.id] = l_prime;
      _open.insert(std::make_pair(l_prime.f, l_prime.id));
      std::cout<<"Label now is: "<< l_prime <<std::endl;
      _res.n_generated++;
    } // end of the for-loop.
  } // end of the while-loop

  _PostProcRes();

  std::cout << "[INFO] AstarRefill::Search exit." << std::endl;
  return 1;
};

void AstarRefill::SetRefillCosts( std::unordered_map<long,long> refillCosts )
{
  _refill_costs = refillCosts;
  return ;
}

void AstarRefill::SetRoadmap(basic::Roadmap* g) {
  _roadmap = g;
  _graph = _roadmap;
};

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

  if (_alpha.find(l.v) == _alpha.end()) {
    _alpha[l.v] = FrontierNaive();
  }

  _alpha[l.v].Update(l);
  
  return ;
};


void AstarRefill::_computeReachableSets() {

  auto all_vertices = _roadmap->GetNodes();

  // Initialize reachable sets
  for (auto v : all_vertices)
  {  
    _reachableSets[v] = {};
  }

  for (auto v : all_vertices)
  
    // ********** TODO, implement this method ***********

    // Compute _reachableSets, which is an unordered map.
    // it maps an vertex ID to an unordered_set that stores the reachable vertex IDs.
    // E.g. _reachableSets[3] is an unordered_set of vertices that are reachable from vertex 3.

    // to get the successors of a vertex w in the graph:
    //   succs = _roadmap->GetSuccs(w); // here, succs is an unordered_set of vertex IDs.
    
    // to get the cost of an edge (v,u) c = _roadmap->GetCost(v,u), 
    //      which returns a CostVector of length one, and c[0] is the cost value of the edge.
  {
    std::unordered_map<int, long> d_star;
    std::unordered_set<int> OPENv;
    
    // Initialize distances
    for (auto u : all_vertices)
      {
        d_star[u] = std::numeric_limits<long>::infinity();
      }
    d_star[v] = 0.0;

    OPENv.insert(v);

    while (!OPENv.empty()) {
      int u = *OPENv.begin();
      OPENv.erase(OPENv.begin());

      if (d_star[u] > qmax) {
        continue;
      } 
      else {
        _reachableSets[v].insert(u);
      }
      // Get successors of u
      std::unordered_set<long> succs = _roadmap->GetSuccs(u);

      for (auto u_prime : succs) {
        if (_reachableSets[v].count(u_prime) > 0) {
          continue;
        }

        // Get cost of edge (u, u_prime)
        std::vector<long> c = _roadmap->GetCost(u, u_prime);
        double edge_cost = c[0];

        if (d_star[u_prime] > d_star[u] + edge_cost) {
          d_star[u_prime] = d_star[u] + edge_cost;
          OPENv.insert(u_prime);
        }
      }
      }
    }

  _reachableSets = std::move(_reachableSets);
    
  return;
};

} // end namespace search
} // end namespace rzq
