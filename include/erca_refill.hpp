
/*******************************************
 * Author: Zhongqiang Richard Ren.
 * All Rights Reserved.
 *******************************************/

#ifndef SEARCH_ERCA_REFILL_H_
#define SEARCH_ERCA_REFILL_H_

#include "erca.hpp"

namespace rzq {
namespace search {

#define DEBUG_REFILL 0

//////////////////////////////////////////////////////////////////

/**
 * @brief store all non-dominated vectors. Do linear scan for checking and
 * updating.
 */
class FrontierNaive : public Frontier {
public:
  /**
   * @brief
   */
  FrontierNaive();
  virtual ~FrontierNaive();
  virtual bool Check(basic::CostVector g) override;
  virtual void Update(Label l) override;

  // std::unordered_set<long> label_ids; // inherited from Frontier
  // std::unordered_map<long, Label> labels; // inherited from Frontier
};

/**
 * @brief
 */
std::ostream &operator<<(std::ostream &os, FrontierNaive &f);

// extern long prned;
// extern std::unordered_map<long, std::vector<long>> Frontier_size();
/**
 *
 */
class AstarRefill : public EMOAKd {
public:
  AstarRefill();

  virtual ~AstarRefill();

  // virtual void InitHeu(long vd); // use the same one as in EMOAKd (EMOA).

  virtual int Search(long vo, long vd, double time_limit) override;

  virtual void SetRefillCosts(std::unordered_map<long, long>); // new

  virtual void SetRoadmap(basic::Roadmap *);

  virtual void SetQmax(long qm);
  virtual void SetKmax(long km);

  // virtual void SetKmax(long km);

  long alg_iter = 0;
	long heurW = 1;

protected:
  virtual basic::CostVector _Heuristic(long v) override;

  virtual bool _checkForPrune(Label l);

  virtual void _filterAndAddFront(Label l);

  void _computeReachableSets();
  void _computeReachableSetsV(long v); 

  virtual void _PostProcRes() override;

  std::unordered_map<long, long>
      _refill_costs; // map a vertex to the refill cost at that vertex.
  // int _vec_len = 2; // first component is the accumulated cost, the second
  // component is the amount of remaining fuel.
  int _vec_len =
      3; // first component is the accumulated cost, the second component is the
         // amount of remaining fuel, third component is the stops. int _vec_len
         // = 4; // first component is the accumulated cost, second is the fuel
         // left, thirst is the number of stops made and last one is the time
         // taken at each node, fourth is the capacity constraint.

  std::unordered_map<long, FrontierNaive>
      _alpha; // map a vertex id (v) to alpha(v). override

  std::unordered_map<long, std::unordered_set<long>> _reachableSets;
  std::unordered_map<long, std::unordered_map<long, long>> _reachableDist;
  std::unordered_map<long, long> _stationCost;

  basic::Roadmap *_roadmap; // graph

  long _q_max = -1;
  long _k_max = -1;
  long _t_fuel = 1;
};

} // end namespace search
} // end namespace rzq

#endif // SEARCH_ERCA_REFILL_H_
