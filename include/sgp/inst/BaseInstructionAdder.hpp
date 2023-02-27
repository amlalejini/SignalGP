#pragma once

#include <map>
#include <string>

#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/set_utils.hpp"

#include "InstructionLibrary.hpp"

namespace sgp::inst {

/// @brief BaseInstructionAdder provides convient functions for adding instructions
///        from a set of default instructions. Derived adders (for a particular
///        instruction set) are responsible for populating 'default instructions'.
/// @tparam HARDWARE_T
template <typename HARDWARE_T>
class BaseInstructionAdder {
public:
  using hw_t = HARDWARE_T;
  using inst_t = typename hw_t::inst_t;
  using inst_def_t = InstructionDef<hw_t, inst_t>;
  using inst_lib_t = InstructionLibrary<hw_t, inst_t>;

protected:
  std::map<std::string, inst_def_t> default_instructions;

  void AddAllInstructions(
    const std::map<std::string, inst_def_t>& avail_instructions,
    inst_lib_t& inst_lib,
    const std::unordered_set<std::string>& except={}
  ) const {
    for (auto& entry : avail_instructions) {
      const std::string& name = entry.first;
      if (emp::Has(except, name)) continue;
      inst_lib.AddInst(
        entry.second
      );
    }
  }

  void AddInstructions(
    const std::map<std::string, inst_def_t>& avail_instructions,
    inst_lib_t& inst_lib,
    const emp::vector<std::string>& include
  ) const {
    for (const std::string& name : include) {
      if (emp::Has(avail_instructions, name)) {
        inst_lib.AddInst(
          avail_instructions[name]
        );
      }
    }
  }

public:
  void AddAllDefaultInstructions(
    inst_lib_t& inst_lib,
    const std::unordered_set<std::string>& except={}
  ) const {
    AddAllInstructions(
      default_instructions,
      inst_lib,
      except
    );
  }

  void AddDefaultInstructions(
    inst_lib_t& inst_lib,
    const emp::vector<std::string>& include
  ) const {
    AddInstructions(
      default_instructions,
      inst_lib,
      include
    );
  }

};


}