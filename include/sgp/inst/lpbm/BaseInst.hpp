#pragma once

#include <iostream>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <functional>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/datastructs/map_utils.hpp"

// NOTE - Not sure what the best way to organize instruction implementation would be.
// TODO - Could refactor to consolidate InstructionLibrary::InstructionDef as the base instruction type.
namespace sgp::inst::lpbm {

// TODO - This struct might be better in the InstructionLibrary file
/// @brief BaseInst defines the structure followed by all derived instruction implementations
/// @tparam DERIVED_T
/// @tparam HARDWARE_T
template<typename DERIVED_T, typename HARDWARE_T>
struct BaseInst {
  using hw_t = HARDWARE_T;
  using inst_t = typename hw_t::inst_t;
  using inst_prop_t = typename hw_t::inst_prop_t;

  // Force base class to define static functions.
  static std::string Desc() {
    return DERIVED_T::Desc();
  }

  static std::string Name() {
    return DERIVED_T::Name();
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return DERIVED_T::Properties();
  }
  // Force derived class to implement their own run function
  static void Run(hw_t& hw, const inst_t& inst) {
    DERIVED_T::Run(hw, inst);
  }

};

}