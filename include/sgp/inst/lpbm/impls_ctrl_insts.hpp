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

#include "../BaseInstructionSpec.hpp"
#include "../InstructionLibrary.hpp"

namespace sgp::inst::lpbm {

template<typename HARDWARE_T>
struct Inst_ModuleDef : BaseInstructionSpec<Inst_ModuleDef<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Mark the beginning of a module.";
  }

  static std::string name() {
    return "ModuleDef";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{ inst_prop_t::MODULE };
  }

  static void run(hw_t& hw, const inst_t& inst) { ; }

};

template<typename HARDWARE_T>
struct Inst_If : BaseInstructionSpec<Inst_If<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "If";
  }

  static std::string name() {
    return "If";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{inst_prop_t::BLOCK_DEF};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const size_t prog_len = hw.GetProgram().GetSize();
    size_t cur_ip = call_state.GetIP();
    const size_t cur_mp = call_state.GetMP();
    const auto& module = hw.GetModule(cur_mp);
    const size_t module_begin = module.GetBegin();
    const size_t module_end = module.GetEnd();
    // Beginning of block (if instruction).
    const size_t bob = (cur_ip == 0) ? prog_len - 1 : cur_ip - 1;
    // Find end of flow. ==> PROBLEM: what if 'If' is last instruction
    cur_ip = (
      (cur_ip == prog_len) &&
      (module_begin > module_end) &&
      module.InModule(0)
    ) ? 0 : cur_ip;
    // CurIP is next instruction (not the one currently executing)
    const size_t eob = hw.FindEndOfBlock(cur_mp, cur_ip);
    const bool skip = mem_state.AccessWorking(inst.GetArg(0)) == 0.0;
    if (skip) {
      // Skip to EOB
      call_state.SetIP(eob);
      // Advance past the block close if not at end of module.
      if (hw.IsValidProgramPosition(cur_mp, eob)) {
        ++call_state.IP();
      }
    } else {
      // Open flow
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

template<typename HARDWARE_T>
struct Inst_While : BaseInstructionSpec<Inst_While<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "While loop";
  }

  static std::string name() {
    return "While";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{inst_prop_t::BLOCK_DEF};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const size_t prog_len = hw.GetProgram().GetSize();
    size_t cur_ip = call_state.GetIP();
    const size_t cur_mp = call_state.GetMP();
    const auto& module = hw.GetModule(cur_mp);
    const size_t module_begin = module.GetBegin();
    const size_t module_end = module.GetEnd();
    // Beginning of block (if instruction).
    const size_t bob = (cur_ip == 0) ? prog_len - 1 : cur_ip - 1;
    // Find end of flow. ==> PROBLEM: what if 'If' is last instruction
    cur_ip = (
      cur_ip == prog_len
      && module_begin > module_end
      && module.InModule(0)
    ) ? 0 : cur_ip;
    const size_t eob = hw.FindEndOfBlock(cur_mp, cur_ip);
    const bool skip = mem_state.AccessWorking(inst.GetArg(0)) == 0.0;
    if (skip) {
      // Skip to EOB
      call_state.SetIP(eob);
      // Advance past the block close if not at end of module.
      if (hw.IsValidProgramPosition(cur_mp, eob)) {
        ++call_state.IP();
      }
    } else {
      // Open flow
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

template<typename HARDWARE_T>
struct Inst_Countdown : BaseInstructionSpec<Inst_Countdown<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Countdown loop. Loop until [arg0] memory value <= 0.";
  }

  static std::string name() {
    return "Countdown";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{inst_prop_t::BLOCK_DEF};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const size_t prog_len = hw.GetProgram().GetSize();
    size_t cur_ip = call_state.GetIP();
    const size_t cur_mp = call_state.GetMP();
    const auto& module = hw.GetModule(cur_mp);
    const size_t module_begin = module.GetBegin();
    const size_t module_end = module.GetEnd();
    // Beginning of block (if instruction).
    const size_t bob = (cur_ip == 0) ? prog_len - 1 : cur_ip - 1;
    // Find end of flow. ==> PROBLEM: what if 'If' is last instruction
    cur_ip = (
      cur_ip == prog_len
      && module_begin > module_end
      && module.InModule(0)
    ) ? 0 : cur_ip;
    const size_t eob = hw.FindEndOfBlock(cur_mp, cur_ip);
    const bool skip = mem_state.AccessWorking(inst.GetArg(0)) == 0.0;
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

template<typename HARDWARE_T>
struct Inst_Break : BaseInstructionSpec<Inst_Break<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Break out of nearest loop in the flow stack (that isn't preceded by a routine or call).";
  }

  static std::string name() {
    return "Break";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    using flow_type_t = cpu::linprg::FlowType;
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    // break out of the nearest loop:
    //     loop = false;
    // (1) While (true) {
    //       if (basic) continue; ++i
    //       elif (loop) loop = true; break;
    //       else break;
    //     }
    bool found_loop = false;
    int flow_pos = call_state.flow_stack.size() - 1;
    while (flow_pos >= 0) {
      auto& flow = call_state.flow_stack[flow_pos];
      if (flow.GetType() == flow_type_t::BASIC) {
        --flow_pos;
        continue;
      } else if (flow.GetType() == flow_type_t::WHILE_LOOP) {
        found_loop = true;
        break;
      } else {
        break;
      }
    }
    // (2) While (loop) {
    //       if (basic) pop_back()
    //       if (loop) {
    //         BreakFlow(loop, exec_state);
    //         break;
    //       }
    //     }
    while (found_loop) {
      if (call_state.GetTopFlow().GetType() == flow_type_t::BASIC) {
        call_state.flow_stack.pop_back(); // todo - CloseFlow?
      } else {
        emp_assert(call_state.GetTopFlow().GetType() == flow_type_t::WHILE_LOOP);
        hw.GetFlowHandler().BreakFlow(
          hw,
          flow_type_t::WHILE_LOOP,
          hw.GetCurThread().GetExecState()
        );
        break;
      }
    }
  }

};

template<typename HARDWARE_T>
struct Inst_Close : BaseInstructionSpec<Inst_Close<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Mark the end of basic and loop control flow.";
  }

  static std::string name() {
    return "Close";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{inst_prop_t::BLOCK_CLOSE};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    using flow_type_t = cpu::linprg::FlowType;
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    const flow_type_t cur_flow_type = call_state.GetTopFlow().GetType();
    // If current flow is BASIC or a WHILE loop, close the flow.
    if (
      cur_flow_type == flow_type_t::BASIC
      || cur_flow_type == flow_type_t::WHILE_LOOP
    ) {
      hw.GetFlowHandler().CloseFlow(
        hw,
        cur_flow_type,
        hw.GetCurThread().GetExecState()
      );
    }
  }

};

template<typename HARDWARE_T>
struct Inst_Call : BaseInstructionSpec<Inst_Call<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Call a module specified by tag0.";
  }

  static std::string name() {
    return "Call";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    hw.CallModule(inst.GetTag(0), hw.GetCurThread().GetExecState());
  }

};

template<typename HARDWARE_T>
struct Inst_Routine : BaseInstructionSpec<Inst_Routine<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
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
      const auto& target_module = hw.GetModule(matches[0]);
      // Flow: type mp ip begin end
      hw.GetFlowHandler().OpenFlow(
        hw,
        {
          flow_type_t::ROUTINE,
          target_module.id,
          target_module.begin,
          target_module.begin,
          target_module.end
        },
        hw.GetCurThread().GetExecState()
      );
    }
  }

};

template<typename HARDWARE_T>
struct Inst_Return : BaseInstructionSpec<Inst_Return<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Return from nearest routine in the flow stack. Or, if no routines, the current call.";
  }

  static std::string name() {
    return "Return";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    using flow_type_t = cpu::linprg::FlowType;
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    // Return from CALL or ROUTINE
    while (call_state.IsFlow()) {
      auto& top = call_state.GetTopFlow();
      // Return from nearest CALL or ROUTINE.
      // Will close all flows until reaching nearest call or routine.
      if (top.GetType() == flow_type_t::CALL) {
        hw.GetFlowHandler().CloseFlow(
          hw,
          top.GetType(),
          hw.GetCurThread().GetExecState()
        );
        // Actually return from call!
        hw.ReturnCall(
          hw.GetCurThread().GetExecState()
        );
      } else if (top.GetType() == flow_type_t::ROUTINE) {
        hw.GetFlowHandler().CloseFlow(
          hw,
          top.GetType(),
          hw.GetCurThread().GetExecState()
        );
      } else {
        hw.GetFlowHandler().CloseFlow(
          hw,
          top.GetType(),
          hw.GetCurThread().GetExecState()
        );
        // Continue closing flows
        continue;
      }
      break;
    }
  }

};

template<typename HARDWARE_T>
struct Inst_Fork : BaseInstructionSpec<Inst_Fork<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Fork a new thread with a function call specified by tag0";
  }

  static std::string name() {
    return "Fork";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    const emp::vector<size_t> matches(hw.FindModuleMatch(inst.GetTag(0)));
    if (matches.size()) {
      auto spawned = hw.SpawnThreadWithID(matches[0]);
      if (spawned) {
        const size_t thread_id = spawned.value();
        // Hold up there cowboy! If the module was empty, the hardware will ignore the CallModule request.
        if (hw.GetThread(thread_id).GetExecState().GetCallStack().size()) {
          // Spawned valid thread.
          // Do whatever it is that the memory model says we should do on a function call.
          emp_assert(hw.GetCurThread().GetExecState().GetCallStack().size());
          auto& forker = hw.GetCurThread().GetExecState().GetTopCallState();
          emp_assert(hw.GetThread(thread_id).GetExecState().GetCallStack().size());
          auto& forkee = hw.GetThread(thread_id).GetExecState().GetTopCallState();
          hw.GetMemoryModel().OnModuleCall(forker.GetMemory(), forkee.GetMemory());
        }
      }
    }
  }

};

template<typename HARDWARE_T>
struct Inst_Terminate : BaseInstructionSpec<Inst_Terminate<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Mark the current thread as dead.";
  }

  static std::string name() {
    return "Terminate";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    hw.GetCurThread().SetDead();
  }

};

}