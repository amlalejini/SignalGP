#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <limits>
#include <utility>
#include <unordered_set>

#include "emp/bits/BitSet.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/math/Range.hpp"

#include "sgp/utils/random_utils.hpp"

TEST_CASE("RandomBitSet<W>") {
  constexpr size_t RANDOM_SEED = 1;
  emp::Random random(RANDOM_SEED);
  std::unordered_set<uint32_t> uset; // Will be used to double-check uniqueness.

  // Generate a bunch of big random tags. No uniqueness guarantees.
  for (size_t i = 0; i < 100; ++i) auto tag = sgp::utils::RandomBitSet<1024>(random);

  // Enumerate all 2-bit tags
  emp::vector<emp::BitSet<2>> tags2;
  uset.clear();
  // std::cout << "All two-bit tags: " << std::endl;
  for (size_t i = 0; i < emp::Pow2(2); ++i) {
    tags2.emplace_back(sgp::utils::RandomBitSet<2>(random, tags2));
    uset.emplace(tags2.back().GetUInt(0));
    // std::cout << "  ";
    // tags2.back().Print();
    // std::cout << " : " << tags2.back().GetUInt(0) << std::endl;
  }
  REQUIRE(tags2.size() == emp::Pow2(2));
  REQUIRE(uset.size() == emp::Pow2(2));
  for (size_t i = 0; i < emp::Pow2(2); ++i) REQUIRE(emp::Has(uset, i));

  // Enumerate all 4-bit tags
  emp::vector<emp::BitSet<4>> tags4;
  uset.clear();
  // std::cout << "All four-bit tags: " << std::endl;
  for (size_t i = 0; i < emp::Pow2(4); ++i) {
    tags4.emplace_back(sgp::utils::RandomBitSet<4>(random, tags4));
    uset.emplace(tags4.back().GetUInt(0));
    // std::cout << "  ";
    // tags4.back().Print();
    // std::cout << " : " << tags4.back().GetUInt(0) << std::endl;
  }
  REQUIRE(tags4.size() == emp::Pow2(4));
  REQUIRE(uset.size() == emp::Pow2(4));
  for (size_t i = 0; i < emp::Pow2(4); ++i) REQUIRE(emp::Has(uset, i));

  // Generate a bunch of 16-bit tags.
  emp::vector<emp::BitSet<16>> tags16;
  for (size_t k = 0; k < 100; ++k) {
    uset.clear();
    tags16.clear();
    for (size_t i = 0; i < 1000; ++i) {
      tags16.emplace_back(sgp::utils::RandomBitSet<16>(random, tags16));
      uset.emplace(tags16.back().GetUInt(0));
    }
    REQUIRE(uset.size() == 1000);
  }
}

TEST_CASE("RandomBitSets<W>") {
  constexpr size_t RANDOM_SEED = 1;
  emp::Random random(RANDOM_SEED);

  std::unordered_set<uint32_t> uset; // Will be used to double-check uniqueness.

  // Generate lots of small tags with no guarantees on uniqueness.
  auto small_tags = sgp::utils::RandomBitSets<2>(random, 1000);
  REQUIRE(small_tags.size() == 1000);
  // Generate lots of large tags with no guarantees on uniqueness.
  auto big_tags = sgp::utils::RandomBitSets<1024>(random, 1000);
  REQUIRE(big_tags.size() == 1000);

  // Use generator to enumerate all 2-bit tags.
  auto tags2 = sgp::utils::RandomBitSets<2>(random, emp::Pow2(2), true);
  uset.clear();
  for (size_t i = 0; i < tags2.size(); ++i) {
    uset.emplace(tags2[i].GetUInt(0));
  }
  REQUIRE(tags2.size() == emp::Pow2(2));
  REQUIRE(uset.size() == emp::Pow2(2));
  for (size_t i = 0; i < emp::Pow2(2); ++i) REQUIRE(emp::Has(uset, i));

  // Use generator to enumerate all 4-bit tags.
  auto tags4 = sgp::utils::RandomBitSets<4>(random, emp::Pow2(4), true);
  uset.clear();
  for (size_t i = 0; i < tags4.size(); ++i) {
    uset.emplace(tags4[i].GetUInt(0));
  }
  REQUIRE(tags4.size() == emp::Pow2(4));
  REQUIRE(uset.size() == emp::Pow2(4));
  for (size_t i = 0; i < emp::Pow2(4); ++i) REQUIRE(emp::Has(uset, i));

  // Generate a bunch of 8-bit tags (50% of the tag-space).
  // Check for uniqueness.
  auto tags8 = sgp::utils::RandomBitSets<8>(random, 128, true);
  uset.clear();
  for (size_t i = 0; i < tags8.size(); ++i) uset.emplace(tags8[i].GetUInt(0));
  REQUIRE(tags8.size() == 128);
  REQUIRE(uset.size() == 128);

  // Generate a bunch of 8-bit tags using previously generated tag as
  // reserved tag-space. Check for proper uniqueness.
  for (size_t i = 0; i < 100; ++i) {
    std::unordered_set<uint32_t> temp_set(uset);
    auto tags = sgp::utils::RandomBitSets<8>(random, 64, true, tags8);
    REQUIRE(tags.size() == 64);
    for (size_t t = 0; t < tags.size(); ++t) temp_set.emplace(tags[t].GetUInt(0));
    REQUIRE(temp_set.size() == 128+64);
  }
}
