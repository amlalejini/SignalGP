
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
  // - LinearFunctionsProgram
  // - SimpleMemoryModel

  // NUMERIC ARG INSTRUCTIONS

  // Instructions to implement:
  // - Inst_If
  // - Inst_While
  // - Inst_Countdown
  // - Inst_Break
  // - Inst_Call
  // - Inst_Return
  // - Inst_SetMem
  // - Inst_CopyMem
  // - Inst_SwapMem
  // - Inst_Input
  // - Inst_Output
  // - Inst_Commit
  // - Inst_Pull
  // - Inst_Fork
  // - Inst_Terminate
  // - Inst_Nop

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
      hw.GetFlowHandler().OpenFlow(hw,{lsgp_utils::FlowType::WHILE_LOOP,
                                       cur_mp,
                                       cur_ip,
                                       bob,
                                       eob},
                                       hw.GetCurThread().GetExecState());
    }
  }

  /*
  // - Inst_Call
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Call(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    hw.CallModule(inst.GetTag(0), hw.GetCurThread().GetExecState());
  }

  // - Inst_Routine
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Routine(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    using flow_type_t = lsgp_utils::FlowType;
    emp::vector<size_t> matches(hw.FindModuleMatch(inst.GetTag(0)));
    if (matches.size()) {
      const auto & target_module = hw.GetModule(matches[0]);
      // Flow: type mp ip begin end
      hw.GetFlowHandler().OpenFlow(hw, {flow_type_t::ROUTINE,
                                    target_module.id,
                                    target_module.begin,
                                    target_module.begin,
                                    target_module.end},
                                    hw.GetCurThread().GetExecState());
    }
  }

  // - Inst_Return
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Return(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    using flow_type_t = lsgp_utils::FlowType;
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    // Return from CALL or ROUTINE
    while (call_state.IsFlow()) {
      auto & top = call_state.GetTopFlow();
      if (top.GetType() == flow_type_t::CALL || top.GetType() == flow_type_t::ROUTINE) {
        hw.GetFlowHandler().CloseFlow(hw, top.GetType(), hw.GetCurThread().GetExecState());
        break;
      } else {
        hw.GetFlowHandler().CloseFlow(hw, top.GetType(), hw.GetCurThread().GetExecState());
      }
    }
  }

  // - Inst_CopyMem
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_CopyMem(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(1), mem_state.AccessWorking(inst.GetArg(0)));
  }

  // - Inst_SwapMem
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_SwapMem(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    const double val_0 = mem_state.AccessWorking(inst.GetArg(0));
    const double val_1 = mem_state.AccessWorking(inst.GetArg(1));
    mem_state.SetWorking(inst.GetArg(0), val_1);
    mem_state.SetWorking(inst.GetArg(1), val_0);
  }

  // - Inst_Input
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_InputToWorking(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(1), mem_state.AccessInput(inst.GetArg(0)));
  }

  // - Inst_Output
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_WorkingToOutput(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetOutput(inst.GetArg(1), mem_state.AccessWorking(inst.GetArg(0)));
  }

  // - Inst_Commit (push value from working to global memory)
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_WorkingToGlobal(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    auto & mem_model = hw.GetMemoryModel();
    mem_model.SetGlobal(inst.GetArg(1), mem_state.AccessWorking(inst.GetArg(0)));
  }

  // - Inst_Pull (pull value from global to working memory)
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_GlobalToWorking(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    auto & mem_model = hw.GetMemoryModel();
    mem_state.SetWorking(inst.GetArg(1), mem_model.AccessGlobal(inst.GetArg(0)));
  }

  // - Inst_Fork
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Fork(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    const emp::vector<size_t> matches(hw.FindModuleMatch(inst.GetTag(0)));
    if (matches.size()) {
      auto spawned = hw.SpawnThreadWithID(matches[0]);
      if (spawned) {
        const size_t thread_id = spawned.value();
        // Spawned valid thread.
        // Do whatever it is that the memory model says we should do on a function call.
        auto & forker = hw.GetCurThread().GetExecState().GetTopCallState();
        auto & forkee = hw.GetThread(thread_id).GetExecState().GetTopCallState();
        hw.GetMemoryModel().OnModuleCall(forker.GetMemory(), forkee.GetMemory());
      }
    }
  }

  // - Inst_Terminate
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Terminate(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    hw.GetCurThread().SetDead();
  }

  // - Inst_Nop
  ///  - do nothing
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Nop(HARDWARE_T & hw, const INSTRUCTION_T & inst) { ; }
  */
}}}

#endif