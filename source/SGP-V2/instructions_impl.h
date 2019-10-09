
#ifndef EMP_SIGNALGP_V2_INST_IMPL_H
#define EMP_SIGNALGP_V2_INST_IMPL_H

#include <iostream>
#include <utility>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/Random.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"

#include "../EventLibrary.h"
#include "../InstructionLibrary.h"

#include "SignalGP.h"

namespace emp { namespace sgp_v2 { namespace inst_impl {

  // These instructions work with:
  // - SimpleExecutionStepper
  //  - LinearProgram
  //  - SimpleMemoryModel

  // NUMERIC ARG INSTRUCTIONS

  // Instructions to implement:
  // - Inst_Inc
  // - Inst_Dec
  // - Inst_Not
  // - Inst_Add
  // - Inst_Sub
  // - Inst_Mult
  // - Inst_Div
  // - Inst_Mod
  // - Inst_TestEqu
  // - Inst_TestNEqu
  // - Inst_TestLess
  // - Inst_If
  // - Inst_While
  // - Inst_Countdown
  // - Inst_Break
  // - Inst_Close
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

  /// Increment
  /// - requires: 1 argument
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Inc(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    // Increment value in local memory @ [ARG0]
    ++call_state.GetMemory().AccessWorking(inst.GetArg(0));
  }

  /// Decrement
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Dec(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    // Decrement value in local memory @ [ARG0]
    --call_state.GetMemory().AccessWorking(inst.GetArg(0));
  }

  /// Not
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Not(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(0), mem_state.GetWorking(inst.GetArg(0)) == 0.0);
  }

  // - Inst_Add
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Add(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(2), mem_state.AccessWorking(inst.GetArg(0)) + mem_state.AccessWorking(inst.GetArg(1)));
  }

  // - Inst_Sub
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Sub(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(2), mem_state.AccessWorking(inst.GetArg(0)) - mem_state.AccessWorking(inst.GetArg(1)));
  }

  // - Inst_Mult
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Mult(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(2), mem_state.AccessWorking(inst.GetArg(0)) * mem_state.AccessWorking(inst.GetArg(1)));
  }

  // - Inst_Div
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Div(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    const auto & denom = mem_state.AccessWorking(inst.GetArg(1));
    if (denom == 0.0) return; // Do nothing.
    const auto & num = mem_state.AccessWorking(inst.GetArg(0));
    mem_state.SetWorking(inst.GetArg(2), num / denom);
  }

  // - Inst_Mod
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Mod(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    const int denom = (int)mem_state.AccessWorking(inst.GetArg(1));
    if (denom == 0.0) return; // Do nothing.
    const int num = (int)mem_state.AccessWorking(inst.GetArg(0));
    mem_state.SetWorking(inst.GetArg(2), Mod(num, denom));
  }

  // - Inst_TestEqu
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_TestEqu(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(2), mem_state.AccessWorking(inst.GetArg(0)) == mem_state.AccessWorking(inst.GetArg(1)));
  }

  // - Inst_TestNEqu
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_TestNEqu(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(2), mem_state.AccessWorking(inst.GetArg(0)) != mem_state.AccessWorking(inst.GetArg(1)));
  }

  // - Inst_TestLess
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_TestLess(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(2), mem_state.AccessWorking(inst.GetArg(0)) < mem_state.AccessWorking(inst.GetArg(1)));
  }

  // - Inst_TestLessEqu
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_TestLessEqu(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(2), mem_state.AccessWorking(inst.GetArg(0)) <= mem_state.AccessWorking(inst.GetArg(1)));
  }

  // - Inst_TestGreater
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_TestGreater(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(2), mem_state.AccessWorking(inst.GetArg(0)) > mem_state.AccessWorking(inst.GetArg(1)));
  }

  // - Inst_TestGreater
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_TestGreaterEqu(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(2), mem_state.AccessWorking(inst.GetArg(0)) >= mem_state.AccessWorking(inst.GetArg(1)));
  }

  // - Inst_If TODO - TEST EDGE CASES!
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_If(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & exec_stepper = hw.GetExecStepper();
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    const size_t prog_len = exec_stepper.GetProgram().GetSize();
    const size_t cur_ip = call_state.GetIP();
    const size_t cur_mp = call_state.GetMP();
    auto & cur_module = exec_stepper.GetModules()[cur_mp];
    // Beginning of block (if instruction).
    const size_t bob = cur_ip;
    // First instruction of block (next instruction)
    size_t next_ip = (cur_ip + 1 >= prog_len
                      && exec_stepper.IsValidProgramPosition(cur_mp, 0)
                      && cur_module.GetBegin() != 0) ? 0 : cur_ip + 1;
    // Find end of flow.
    const size_t eob = exec_stepper.FindEndOfBlock(cur_mp, next_ip); // NOTE - if IP is current instruction, want to start looking past current BLOCK_DEF
    if (mem_state.AccessWorking(inst.GetArg(0)) == 0.0) {
      // Skip to EOB
      call_state.SetIP(eob);
      // Advance past the block close if not at end of module.
      if (exec_stepper.IsValidProgramPosition(cur_mp, eob)) {
        ++call_state.IP();
      }
    } else {
      // Open flow
      exec_stepper.GetFlowHandler().OpenFlow({HARDWARE_T::exec_stepper_t::FlowType::BASIC,
                                              cur_mp,
                                              next_ip,
                                              bob,
                                              eob},
                                              hw.GetCurExecState());
    }
  }

  // - Inst_While
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_While(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & exec_stepper = hw.GetExecStepper();
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    const size_t prog_len = exec_stepper.GetProgram().GetSize();
    const size_t cur_ip = call_state.GetIP();
    const size_t cur_mp = call_state.GetMP();
    auto & cur_module = exec_stepper.GetModules()[cur_mp];
    // Beginning of block (while instruction).
    const size_t bob = cur_ip;
    // First instruction of block (next instruction)
    size_t next_ip = (cur_ip + 1 >= prog_len
                      && exec_stepper.IsValidProgramPosition(cur_mp, 0)
                      && cur_module.GetBegin() != 0) ? 0 : cur_ip + 1;
    // Find end of flow.
    const size_t eob = exec_stepper.FindEndOfBlock(cur_mp, next_ip); // NOTE - if IP is current instruction, want to start looking past current BLOCK_DEF
    if (mem_state.AccessWorking(inst.GetArg(0)) == 0.0) {
      // Skip to EOB
      call_state.SetIP(eob);
      // Advance past the block close if not at end of module.
      if (exec_stepper.IsValidProgramPosition(cur_mp, eob)) {
        ++call_state.IP();
      }
    } else {
      // Open flow
      exec_stepper.GetFlowHandler().OpenFlow({HARDWARE_T::exec_stepper_t::FlowType::WHILE_LOOP,
                                              cur_mp,
                                              next_ip,
                                              bob,
                                              eob},
                                              hw.GetCurExecState());
    }
  }

  // - Inst_Countdown
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Countdown(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    auto & exec_stepper = hw.GetExecStepper();
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    auto & mem_state = call_state.GetMemory();
    const size_t prog_len = exec_stepper.GetProgram().GetSize();
    const size_t cur_ip = call_state.GetIP();
    const size_t cur_mp = call_state.GetMP();
    auto & cur_module = exec_stepper.GetModules()[cur_mp];
    // Beginning of block (while instruction).
    const size_t bob = cur_ip;
    // First instruction of block (next instruction)
    size_t next_ip = (cur_ip + 1 >= prog_len
                      && exec_stepper.IsValidProgramPosition(cur_mp, 0)
                      && cur_module.GetBegin() != 0) ? 0 : cur_ip + 1;
    // Find end of flow.
    const size_t eob = exec_stepper.FindEndOfBlock(cur_mp, next_ip); // NOTE - if IP is current instruction, want to start looking past current BLOCK_DEF
    if (mem_state.AccessWorking(inst.GetArg(0)) == 0.0) {
      // Skip to EOB
      call_state.SetIP(eob);
      // Advance past the block close if not at end of module.
      if (exec_stepper.IsValidProgramPosition(cur_mp, eob)) {
        ++call_state.IP();
      }
    } else {
      --mem_state.AccessWorking(inst.args[0]);
      // Open flow
      exec_stepper.GetFlowHandler().OpenFlow({HARDWARE_T::exec_stepper_t::FlowType::WHILE_LOOP,
                                              cur_mp,
                                              next_ip,
                                              bob,
                                              eob},
                                              hw.GetCurExecState());
    }
  }

  // - Inst_Break
  //   - break out of nearest loop in flow stack (that isn't preceded by a routine or call)
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Break(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    using flow_type_t = typename HARDWARE_T::exec_stepper_t::FlowType;
    auto & exec_stepper = hw.GetExecStepper();
    auto & call_state = hw.GetCurExecState().GetTopCallState();
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
      auto & flow = call_state.flow_stack[flow_pos];
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
        call_state.flow_stack.pop_back();
      } else {
        emp_assert(call_state.GetTopFlow().GetType() == flow_type_t::WHILE_LOOP);
        exec_stepper.GetFlowHandler().BreakFlow(flow_type_t::WHILE_LOOP, exec_stepper);
        break;
      }
    }
  }

  // - Inst_Close
  //   - close basic and while_loop flow
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Close(HARDWARE_T & hw, const INSTRUCTION_T & inst) {
    using flow_type_t = typename HARDWARE_T::exec_stepper_t::FlowType;
    auto & exec_stepper = hw.GetExecStepper();
    auto & call_state = hw.GetCurExecState().GetTopCallState();
    const flow_type_t cur_flow_type = call_state.GetTopFlow().GetType();
    if (cur_flow_type == flow_type_t::BASIC || cur_flow_type == flow_type_t::WHILE_LOOP) {
      exec_stepper.GetFlowHandler().CloseFlow(cur_flow_type, exec_state);
    }
  }

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
  ///  - do nothing
  template<typename HARDWARE_T, typename INSTRUCTION_T>
  void Inst_Nop(HARDWARE_T & hw, const INSTRUCTION_T & inst) { ; }

}}}

#endif