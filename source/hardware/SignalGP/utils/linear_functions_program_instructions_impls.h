
#ifndef EMP_SIGNALGP_LINEAR_FUNCTIONS_PROGRAM_INST_IMPL_H
#define EMP_SIGNALGP_LINEAR_FUNCTIONS_PROGRAM_INST_IMPL_H

#include <iostream>
#include <utility>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/Random.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"

#include "../EventLibrary.h"
#include "InstructionLibrary.h"

namespace emp { namespace signalgp { namespace lfp_inst_impl {

  // These instructions work with:
  // - LinearFunctionsProgram<Tags=BitSet, Args=int>
  // - SimpleMemoryModel

  // Instructions to implement:
  // - Inst_If
  // - Inst_While
  // - Inst_Countdown
  // - Inst_Routine

  // - Inst_If TODO - TEST EDGE CASES!
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_If(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
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
      hw.GetFlowHandler().OpenFlow(hw,
                                   {lsgp_utils::FlowType::BASIC,
                                    cur_mp,
                                    cur_ip,
                                    bob,
                                    eob
                                   },
                                   hw.GetCurThread().GetExecState());
    }
  }

  // - Inst_While
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_While(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
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
      hw.GetFlowHandler().OpenFlow(hw,{lsgp_utils::FlowType::WHILE_LOOP,
                                       cur_mp,
                                       cur_ip,
                                       bob,
                                       eob},
                                       hw.GetCurThread().GetExecState());
    }
  }

  // - Inst_Countdown
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Countdown(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
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
      --mem_state.AccessWorking(inst.args[0]);
      // Open flow
      emp_assert(cur_mp < hw.GetProgram().GetSize());
      hw.GetFlowHandler().OpenFlow(hw,{lsgp_utils::FlowType::WHILE_LOOP,
                                       cur_mp,
                                       cur_ip,
                                       bob,
                                       eob},
                                       hw.GetCurThread().GetExecState());
    }
  }

  // - Inst_Routine
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Routine(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    using flow_type_t = lsgp_utils::FlowType;
    emp::vector<size_t> matches(hw.FindModuleMatch(inst.GetTag(0)));
    if (matches.size()) {
      const size_t module_id = matches[0];
      emp_assert(module_id < hw.GetProgram().GetSize());
      const auto & target_module = hw.GetProgram()[module_id];
      // Flow: type mp ip begin end
      hw.GetFlowHandler().OpenFlow(hw, {flow_type_t::ROUTINE,
                                    module_id,
                                    0,
                                    0,
                                    target_module.GetSize()},
                                    hw.GetCurThread().GetExecState());
    }
  }


  /// Non-default instruction: SetRegulator
  /// Number of arguments: 2
  /// Description: Sets the regulator of a tag in the matchbin.
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_SetRegulator(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    emp::vector<size_t> best_fun(hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1));
    if (best_fun.size() == 0) { return; }

    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();

    double regulator = mem_state.AccessWorking(inst.GetArg(0));
    if (regulator < 0) {
      regulator = std::max(regulator, std::numeric_limits<double>::min());
      regulator /= std::numeric_limits<double>::min();
    } else {
      regulator += 1.0;
    }

    hw.GetMatchBin().SetRegulator(best_fun[0], regulator);
    // @discussion - ??
    const size_t dur = 2 + mem_state.AccessWorking(inst.GetArg(1));
    hw.GetMatchBin().GetVal(best_fun[0]) = dur;
  }


  /// Non-default instruction: SetOwnRegulator
  /// Number of arguments: 2
  /// Description: Sets the regulator of the currently executing function.
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  static void Inst_SetOwnRegulator(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    auto & flow = call_state.GetTopFlow();
    double regulator = mem_state.AccessWorking(inst.GetArg(0));

    if (regulator < 0) {
      regulator = std::max(regulator, std::numeric_limits<double>::min());
      regulator /= std::numeric_limits<double>::min();
    } else {
      regulator += 1.0;
    }

    hw.GetMatchBin().SetRegulator(flow.GetMP(), regulator);
    const size_t dur = 2 + mem_state.AccessWorking(inst.GetArg(1));
    hw.GetMatchBin().GetVal(flow.GetMP()) = dur;
  }

  /// Non-default instruction: AdjRegulator
  /// Number of arguments: 3
  /// Description: adjusts the regulator of a tag in the matchbin
  /// towards a target.
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  static void Inst_AdjRegulator(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    // const State & state = hw.GetCurState();
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (!best_fun.size()) return;

    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    auto & flow = call_state.GetTopFlow();

    double target = mem_state.AccessWorking(inst.GetArg(0));
    if (target < 0) {
      target = std::max(target, std::numeric_limits<double>::min());
      target /= std::numeric_limits<double>::min();
    } else {
      target += 1.0;
    }

    const double budge = emp::Mod(mem_state.AccessWorking(inst.GetArg(1)) + 0.2, 1.0);
    const double cur = hw.GetMatchBin().ViewRegulator(best_fun[0]);

    hw.GetMatchBin().SetRegulator(best_fun[0], target * budge + cur * (1 - budge));

    const size_t dur = 2 + mem_state.AccessWorking(inst.GetArg(2));
    hw.GetMatchBin().GetVal(best_fun[0]) = dur;
  }

  /// Non-default instruction: AdjOwnRegulator
  /// Number of arguments: 3
  /// Description: adjusts the regulator of a tag in the matchbin
  /// towards a target.
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  static void Inst_AdjOwnRegulator(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    auto & flow = call_state.GetTopFlow();

    double target = mem_state.AccessWorking(inst.GetArg(0));
    if (target < 0) {
      target = std::max(target, std::numeric_limits<double>::min());
      target /= std::numeric_limits<double>::min();
    } else {
      target += 1.0;
    }

    const double budge = emp::Mod(mem_state.AccessWorking(inst.GetArg(1)) + 0.2, 1.0);
    const double cur = hw.GetMatchBin().ViewRegulator(flow.GetMP());

    hw.GetMatchBin().SetRegulator(flow.GetMP(), target * budge + cur * (1 - budge));

    const size_t dur = 2 + mem_state.AccessWorking(inst.GetArg(2));
    hw.GetMatchBin().GetVal(flow.GetMP()) = dur;

  }

  /// Non-default instruction: ExtRegulator
  /// Number of arguments: 1
  /// Description: extends the decay counter of a
  /// regulator of a tag in the matchbin.
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  static void Inst_ExtRegulator(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    // const State & state = hw.GetCurState();
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (!best_fun.size()) return;

    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();

    const size_t dur = 1 + mem_state.AccessWorking(inst.GetArg(0));
    hw.GetMatchBin().GetVal(best_fun[0]) += dur;
  }


  /// Non-default instruction: SenseRegulator
  /// Number of arguments: 1
  /// Description: senses the value of the regulator of another function.
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  static void Inst_SenseRegulator(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (best_fun.size()) {
      auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
      auto & mem_state = call_state.GetMemory();
      mem_state.SetWorking(inst.GetArg(0), hw.GetMatchBin().ViewRegulator(best_fun[0]));
    }
  }


  /// Non-default instruction: SenseOwnRegulator
  /// Number of arguments: 1
  /// Description: senses the value of the regulator the current function.
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  static void Inst_SenseOwnRegulator(HARDWARE_T & hw, const INSTRUCTION_T & inst){
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    auto & flow = call_state.GetTopFlow();
    mem_state.SetWorking(inst.GetArg(0), hw.GetMatchBin().ViewRegulator(flow.GetMP()));
  }

  /// Non-default instruction: Terminal
  /// Number of arguments: 1
  /// Description: writes a genetically-encoded value into a register.
  // template<typename MaxRatio=std::ratio<1>, typename MinRatio=std::ratio<0>>
  // static void Inst_Terminal(EventDrivenGP_t & hw, const inst_t & inst) {

  //   constexpr double max = static_cast<double>(MaxRatio::num) / MaxRatio::den;
  //   constexpr double min = static_cast<double>(MaxRatio::num) / MaxRatio::den;

  //   State & state = hw.GetCurState();
  //   const auto & tag = inst.affinity;

  //   const double val = (
  //     tag.GetDouble() / tag.MaxDouble()
  //   ) * (max - min) - min;

  //   state.SetLocal(inst.args[0], val);

  // }

}}}

#endif