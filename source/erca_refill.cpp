
#include "erca_refill.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <limits>
#include <queue>

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
  auto s = std::chrono::steady_clock::now();

  _vo = vo;
  _vd = vd;

	// preprocessing for all queries, which can be amortized
	// so we don't count it in elapsed time
  _computeReachableSets();
  // ### init heu and compute reachable sets ###
  auto t1 = std::chrono::steady_clock::now();
	_res.rt_preproc = std::chrono::duration<double>(t1 - s).count();
	if (this->heurW > 0) {
		InitHeu(vd); // use the same one as in EMOAKd (EMOA).
	}
  auto t2 = std::chrono::steady_clock::now();
  _res.rt_initHeu = std::chrono::duration<double>(t2 - t1).count();

  // ### init ###
  auto tstart = std::chrono::steady_clock::now();
  basic::CostVector init_vec(0, _vec_len);
  Label lo(_GenLabelId(), _vo, init_vec, _Heuristic(_vo));
  _label[lo.id] = lo;
  _res.n_generated++;
  _open.insert(std::make_pair(lo.f, lo.id));

  // ### main search loop ###
  while (!_open.empty()) {

    // check timeout
    auto tnow = std::chrono::steady_clock::now();
    if (std::chrono::duration<double>(tnow - tstart).count() > time_limit) {
      _res.timeout = true;
      break;
    }

    // ## select label l ##
    Label l = _label[_open.begin()->second];
    _open.erase(_open.begin());

    // TODO read this notes:
    // l.g[0] = the cost value of this label. the g-value
    // l.g[1] = the remaining fuel of this label. the q-values
    // l.g[2] = the number of stops.
    // l.g[3] = the amount of time taken.
    // l.f[0] = the f-cost value of this label. the f-value
    // l.f[1] = meaningless, which should be and must be always zero.

    if (_checkForPrune(l)) {
      continue;
    }

    _filterAndAddFront(l); // make sure if l.v == _vd, l is added to _alpha[l.v]
                           // so that later _PostProcRes() will work correctly.

    if (l.v == _vd) {
			_res.rt_search = std::chrono::duration<double>(tnow - tstart).count();
      break; // [DIFF FROM EMOA]
    }

    if (l.g[2] == _k_max) {
      continue;
    }

    auto nghs = _reachableSets[l.v];

    alg_iter++;

    for (auto u: nghs) {
      auto distV2U = _reachableDist[l.v][u];
      if (u == l.v)
        continue;
      auto costV = _stationCost[l.v];
      auto costU = _stationCost[u];

      Label l_prime;
      long g_prime, q_prime, k_prime;

      // Lemma 1: fill up entirely at vertex u
      if (costU > costV) {
        g_prime = l.g[0] + (_q_max - l.g[1]) * costV;
        q_prime = _q_max - distV2U;
        assert(_q_max >= distV2U);
        k_prime = l.g[2] + 1;
      } else {                  // Lemma 1: fill up enough to reach vertex u
        if (distV2U >= l.g[1]) { // need to fill
          g_prime = l.g[0] + (distV2U - l.g[1]) * costV;
          q_prime = 0;
          k_prime = l.g[2] + 1;
        } else { // no need to fill
          // g_prime = l.g[0];
          // q_prime = l.g[1] - distV2U;
          // k_prime = l.g[2];

          // reaching a station with no-empty tank is suboptimal
          continue;
        }
      }

      l_prime.id = _GenLabelId(); //-->working
      l_prime.v = u;
      l_prime.f = _Heuristic(u);
      l_prime.g = basic::CostVector({g_prime, q_prime, k_prime});

      if (_checkForPrune(l_prime)) {
        continue;
      }

      l_prime.f[0] += l_prime.g[0];
      _label[l_prime.id] = l_prime;
      _parent[l_prime.id] = l.id;

      _open.insert(std::make_pair(l_prime.f, l_prime.id));

      _res.n_generated++;
    }
  }

  auto e = std::chrono::steady_clock::now();
  _PostProcRes();

  // std::cout
  //     << "Elapsed time in microseconds: "
  //     << std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
  //     << " us" << std::endl;

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
	if (this->heurW == 0) {
		return basic::CostVector(std::vector<long>(_vec_len, 0));
	}
  basic::CostVector out;
  out.resize(_vec_len, 0);
  out[0] = EMOA::_Heuristic(v)[1] * this->minPrice; // this is the distance field!
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

  return;
};

void AstarRefill::_PostProcRes() {
  if (_alpha.find(_vd) != _alpha.end()) {
		_res.dist = 0;
		_res.shortest_dist = 0;
		if (this->heurW > 0)
			_res.shortest_dist = _dijks[1].GetCost(_vo);
    for (auto lid : _alpha[_vd].label_ids) {
      _res.paths[lid] = _BuildPath(lid);
      _res.costs[lid] = _label[lid].g;
    }
  }
  return;
};

void AstarRefill::_computeReachableSetsV(long v) {

  auto V = _roadmap->GetNodes();
  std::unordered_map<long, long> dist;
  typedef std::pair<long, long> _Node;
  std::priority_queue<_Node, std::vector<_Node>, std::greater<_Node>> q;

  for (auto i : V)
    dist[i] = std::numeric_limits<long>::max();
  dist[v] = 0.0;
  q.push({0, v});

  while (!q.empty()) {
    auto cur = q.top();
    q.pop();
    long cdist = cur.first;
    long cid = cur.second;

    if (cdist > _q_max || cdist != dist[cid])
      continue;
    // now, we can guarantee cdist is the shortest distance from v to cid
    _reachableSets[v].insert(cid);
    _reachableDist[v][cid] = cdist;

    for (auto nxt : _roadmap->GetSuccs(cid)) {
      // cost[1] is distance
      auto ecost = _roadmap->GetCost(cid, nxt)[1];
      if (dist[nxt] > dist[cid] + ecost) {
        dist[nxt] = dist[cid] + ecost;
        q.push({dist[nxt], nxt});
      }
    }
  }
}

void AstarRefill::_computeReachableSets() {

  auto all_vertices = _roadmap->GetNodes();
  _stationCost = {};
  for (auto v : all_vertices) {
    for (auto u : _roadmap->GetSuccs(v)) {
      _stationCost[v] = _roadmap->GetCost(v, u)[0];
      break;
    }
  }

  // fix issue #5: set target cost to be 0
  _stationCost[_vd] = 0;

  // Initialize reachable sets
  for (auto v : _roadmap->GetNodes()) {
    _reachableSets[v] = {};
    _reachableDist[v] = {};
    _computeReachableSetsV(v);
  }
  return;
};

} // end namespace search
} // end namespace rzq
