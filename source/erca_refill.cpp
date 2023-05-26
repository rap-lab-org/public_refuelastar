
#include "erca_refill.hpp"

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
  }
  return false;
};

void FrontierNaive::Update(Label l) {

  for (auto l_prime: labels) {
    // ********** TODO, implement this method ***********
    // check any existing l_prime.g is worse than the input l.g
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

  for (auto v : all_vertices)
    
    // ********** TODO, implement this method ***********

    // Compute _reachableSets, which is an unordered map.
    // it maps an vertex ID to an unordered_set that stores the reachable vertex IDs.
    // E.g. _reachableSets[3] is an unordered_set of vertices that are reachable from vertex 3.

    // to get the successors of a vertex w in the graph:
    //   succs = _roadmap->GetSuccs(w); // here, succs is an unordered_set of vertex IDs.
    
    // to get the cost of an edge (v,u) c = _roadmap->GetCost(v,u), 
    //      which returns a CostVector of length one, and c[0] is the cost value of the edge.

  return ;
};



} // end namespace search
} // end namespace rzq
