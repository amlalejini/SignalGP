#pragma once

#include <unordered_set>

#include "InstructionLibrary.hpp"

namespace sgp::inst {

/// BaseInstSpec enforces requirements for instruction specification structs.
template<typename DERIVED_T>
struct BaseInstructionSpec {

  static std::string name() {
    return DERIVED_T::name();
  };

  static std::string desc() {
    return DERIVED_T::desc();
  };

  static std::unordered_set<InstProperty> properties() {
    return DERIVED_T::properties();
  };

  template<typename HARDWARE_T, typename INST_T>
  static void run(HARDWARE_T& hw, const INST_T& inst) {
    DERIVED_T::run(hw, inst);
  }
};

}