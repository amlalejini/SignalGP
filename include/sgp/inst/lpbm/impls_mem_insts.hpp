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

// NOTE - Not sure what the best way to organize instruction implementations would be.
namespace sgp::inst::lpbm {

template<typename HARDWARE_T>
struct Inst_SetMem : BaseInstructionSpec<Inst_SetMem<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "[arg0] = arg1";
  }

  static std::string name() {
    return "SetMem";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(0), (double)inst.GetArg(1));
  }

};

template<typename HARDWARE_T>
struct Inst_CopyMem : BaseInstructionSpec<Inst_CopyMem<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "[arg1] = [arg0]";
  }

  static std::string name() {
    return "CopyMem";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(1), mem_state.AccessWorking(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_SwapMem : BaseInstructionSpec<Inst_SwapMem<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Swap [arg0] and [arg1]";
  }

  static std::string name() {
    return "SwapMem";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const double val_0 = mem_state.AccessWorking(inst.GetArg(0));
    const double val_1 = mem_state.AccessWorking(inst.GetArg(1));
    mem_state.SetWorking(inst.GetArg(0), val_1);
    mem_state.SetWorking(inst.GetArg(1), val_0);
  }

};

template<typename HARDWARE_T>
struct Inst_InputToWorking : BaseInstructionSpec<Inst_InputToWorking<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "working[arg1] = input[arg0]";
  }

  static std::string name() {
    return "InputToWorking";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(1), mem_state.AccessInput(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_WorkingToOutput : BaseInstructionSpec<Inst_WorkingToOutput<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "output[arg1] = working[arg0]";
  }

  static std::string name() {
    return "WorkingToOutput";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetOutput(inst.GetArg(1), mem_state.AccessWorking(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_WorkingToGlobal : BaseInstructionSpec<Inst_WorkingToGlobal<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "global[arg1] = working[arg0]";
  }

  static std::string name() {
    return "WorkingToGlobal";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& mem_model = hw.GetMemoryModel();
    mem_model.SetGlobal(inst.GetArg(1), mem_state.AccessWorking(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_GlobalToWorking : BaseInstructionSpec<Inst_GlobalToWorking<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "working[arg1] = global[arg0]";
  }

  static std::string name() {
    return "GlobalToWorking";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& mem_model = hw.GetMemoryModel();
    mem_state.SetWorking(inst.GetArg(1), mem_model.AccessGlobal(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_FullWorkingToGlobal : BaseInstructionSpec<Inst_FullWorkingToGlobal<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Copy entire working memory buffer into global memory, overwriting conflicts.";
  }

  static std::string name() {
    return "FullWorkingToGlobal";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& mem_model = hw.GetMemoryModel();
    auto& working_mem_buffer = mem_state.GetWorkingMemory();
    for (auto& mem : working_mem_buffer) {
      mem_model.SetGlobal(mem.first, mem.second);
    }
  }

};

template<typename HARDWARE_T>
struct Inst_FullGlobalToWorking : BaseInstructionSpec<Inst_FullGlobalToWorking<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Copy entire global memory buffer into working memory, overwriting conflicts.";
  }

  static std::string name() {
    return "FullGlobalToWorking";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& mem_model = hw.GetMemoryModel();
    auto& global_mem_buffer = mem_model.GetGlobalBuffer();
    for (auto& mem : global_mem_buffer) {
      mem_state.SetWorking(mem.first, mem.second);
    }
  }

};

}