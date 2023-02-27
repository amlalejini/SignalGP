#pragma once

#include <iostream>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <functional>
#include <ratio>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/datastructs/map_utils.hpp"

#include "../InstructionLibrary.hpp"
#include "../BaseInstructionSpec.hpp"
#include "../lpbm/impls_ctrl_insts.hpp"

namespace sgp::inst::lfpbm {

// Inst_If
template<typename HARDWARE_T>
struct Inst_If : BaseInstructionSpec<Inst_If<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "If statement. Defines the beginning of an if control flow block";
  }

  static std::string name() {
    return "If";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{ inst_prop_t::BLOCK_DEF };
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    size_t cur_ip = call_state.GetIP();
    const size_t cur_mp = call_state.GetMP();
    emp_assert(cur_ip > 0);
    // CurIP is the next instruction (not the one currently executing)
    // Because IP gets incremented before execution, cur_ip should never be 0.
    const size_t bob = cur_ip - 1; // beginning of block
    const size_t eob = hw.FindEndOfBlock(cur_mp, cur_ip); // end of block
    const bool skip = !((bool)mem_state.AccessWorking(inst.GetArg(0)));
    if (skip) {
      // Skip to EOB
      call_state.SetIP(eob);
      // Advance past the block close if not at end of module.
      if (hw.IsValidProgramPosition(cur_mp, eob)) {
        ++call_state.IP();
      }
    } else {
      // Open flow
      emp_assert(cur_mp < hw.GetProgram().GetSize());
      hw.GetFlowHandler().OpenFlow(
        hw,
        {
          cpu::linprg::FlowType::BASIC,
          cur_mp,
          cur_ip,
          bob,
          eob
        },
        hw.GetCurThread().GetExecState()
      );
    }
  }

};

// Inst_While
template<typename HARDWARE_T>
struct Inst_While : BaseInstructionSpec<Inst_While<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Marks the beginning of a while loop.";
  }

  static std::string name() {
    return "While";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{ inst_prop_t::BLOCK_DEF };
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    size_t cur_ip = call_state.GetIP();
    const size_t cur_mp = call_state.GetMP();
    emp_assert(cur_ip > 0);
    // CurIP is the next instruction (not the one currently executing)
    // Because IP gets incremented before execution, cur_ip should never be 0.
    const size_t bob = cur_ip - 1;
    const size_t eob = hw.FindEndOfBlock(cur_mp, cur_ip);
    const bool skip = !((bool)mem_state.AccessWorking(inst.GetArg(0)));
    if (skip) {
      // Skip to EOB
      call_state.SetIP(eob);
      // Advance past the block close if not at end of module.
      if (hw.IsValidProgramPosition(cur_mp, eob)) {
        ++call_state.IP();
      }
    } else {
      // Open flow
      emp_assert(cur_mp < hw.GetProgram().GetSize());
      hw.GetFlowHandler().OpenFlow(
        hw,
        {
          cpu::linprg::FlowType::WHILE_LOOP,
          cur_mp,
          cur_ip,
          bob,
          eob
        },
        hw.GetCurThread().GetExecState()
      );
    }
  }

};

// Inst_Countdown
template<typename HARDWARE_T>
struct Inst_Countdown : BaseInstructionSpec<Inst_Countdown<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Countdown loop. Loop until [arg0] memory value <= 0.";
  }

  static std::string name() {
    return "Countdown";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{ inst_prop_t::BLOCK_DEF };
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    size_t cur_ip = call_state.GetIP();
    const size_t cur_mp = call_state.GetMP();
    emp_assert(cur_ip > 0);
    // CurIP is the next instruction (not the one currently executing)
    // Because IP gets incremented before execution, cur_ip should never be 0.
    const size_t bob = cur_ip - 1;
    const size_t eob = hw.FindEndOfBlock(cur_mp, cur_ip);
    const bool skip = mem_state.AccessWorking(inst.GetArg(0)) <= 0;
    if (skip) {
      // Skip to EOB
      call_state.SetIP(eob);
      // Advance past the block close if not at end of module.
      if (hw.IsValidProgramPosition(cur_mp, eob)) {
        ++call_state.IP();
      }
    } else {
      --mem_state.AccessWorking(inst.args[0]);
      // Open flow
      emp_assert(cur_mp < hw.GetProgram().GetSize());
      hw.GetFlowHandler().OpenFlow(
        hw,
        {
          cpu::linprg::FlowType::WHILE_LOOP,
          cur_mp,
          cur_ip,
          bob,
          eob
        },
        hw.GetCurThread().GetExecState()
      );
    }
  }

};

// Inst_Routine
template<typename HARDWARE_T>
struct Inst_Routine : BaseInstructionSpec<Inst_Routine<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Call a module specified by tag0 as a routine (shares local memory with current call).";
  }

  static std::string name() {
    return "Routine";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    using flow_type_t = cpu::linprg::FlowType;
    emp::vector<size_t> matches(
      hw.FindModuleMatch(inst.GetTag(0))
    );
    if (matches.size()) {
      const size_t module_id = matches[0];
      emp_assert(module_id < hw.GetProgram().GetSize());
      const auto& target_module = hw.GetProgram()[module_id];
      // Flow: type mp ip begin end
      hw.GetFlowHandler().OpenFlow(
        hw,
        {
          flow_type_t::ROUTINE,
          module_id,
          0,
          0,
          target_module.GetSize()
        },
        hw.GetCurThread().GetExecState()
      );
    }
  }

};

// --- Implementations from lpbm work for this CPU ---

// Break
template<typename HARDWARE_T>
using Inst_Break = lpbm::Inst_Break<HARDWARE_T>;

// Close
template<typename HARDWARE_T>
using Inst_Close = lpbm::Inst_Close<HARDWARE_T>;

// Call
template<typename HARDWARE_T>
using Inst_Call = lpbm::Inst_Call<HARDWARE_T>;

// Return
template<typename HARDWARE_T>
using Inst_Return = lpbm::Inst_Return<HARDWARE_T>;

// Fork
template<typename HARDWARE_T>
using Inst_Fork = lpbm::Inst_Fork<HARDWARE_T>;

// Terminate
template<typename HARDWARE_T>
using Inst_Terminate = lpbm::Inst_Terminate<HARDWARE_T>;

}