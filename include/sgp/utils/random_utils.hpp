#pragma once

#include <unordered_set>
#include <string>
#include <functional>
#include <algorithm>

#include "emp/base/errors.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/math/math.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/datastructs/map_utils.hpp"

namespace sgp {

namespace utils {

/// Generate one random BitSet<W>.
/// Given a vector of other BitSets (unique_from), this function will guarantee
/// the generated BitSet is unique with respect to those BitSets.
/// @param rnd - Random number generator to use.
/// @param unique_from - Other BitSets that the generated BitSet should be unique from.
template<size_t W>
emp::BitSet<W> RandomBitSet(
  emp::Random& rnd,
  const emp::vector<emp::BitSet<W>>& unique_from=emp::vector<emp::BitSet<W>>()
) {
  // Create a set of bitsets we want the new bitset to be different than.
  std::unordered_set<emp::BitSet<W>> unique_from_set(
    unique_from.begin(),
    unique_from.end()
  );
  emp_assert(unique_from_set.size() < emp::Pow2(W), "emp::BitSet<W> is not large enough to be able to guarantee requested number of unique tags");
  // Continuously create new random bitsets until one of them is unique.
  emp::BitSet<W> new_bitset(rnd, 0.5);
  while (unique_from_set.size()) {
    if (emp::Has(unique_from_set, new_bitset)) {
      new_bitset.Randomize(rnd);
      continue;
    }
    break;
  }
  return new_bitset;
}

// BOOKMARK

/// Generate 'count' number of random emp::BitSet<W>.
/// Given a vector of other bitsets (unique_from), this function will guarantee the bitsets generated
/// and returned are unique with respect to unique_from.
/// @param rnd - Random number generator to use when generating a random bitset.
/// @param count - How many bitsets should be generated?
/// @param guarantee_unique - Should generated bitsets be guaranteed to be unique from each other?
/// @param unique_from - Other bitsets that the bitsets being generated should be unique with respect to. Only used if 'guarantee_unique' is true.
template<size_t W>
emp::vector<emp::BitSet<W>> RandomBitSets(
  emp::Random& rnd,
  size_t count,
  bool guarantee_unique=false,
  const emp::vector<emp::BitSet<W>> & unique_from=emp::vector<emp::BitSet<W>>()
) {
  // If we don't have to make any promises, run quickly!
  emp::vector<emp::BitSet<W>> new_bitsets(count);
  if (!guarantee_unique && unique_from.size() == 0) {
    for (auto& bs : new_bitsets) {
      bs.Randomize(rnd);
    }
    return new_bitsets;
  }
  // Otherwise, we need to go "slow" (checking against sets we want to be unique from).
  std::unordered_set<emp::BitSet<W>> unique_from_set(unique_from.begin(), unique_from.end());
  emp_assert(unique_from_set.size()+count <= emp::Pow2(W), "Not possible to generate requested number of BitSets");
  for (auto& bs : new_bitsets) {
    bs.Randomize(rnd);
    while (unique_from_set.size()) {
      if (emp::Has(unique_from_set, bs)) {
        bs.Randomize(rnd);
        continue;
      }
      break;
    }
    unique_from_set.emplace(bs);
  }
  return new_bitsets;
}

}

}
