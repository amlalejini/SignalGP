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

#include "BaseInst.hpp"

// NOTE - Not sure what the best way to organize instruction implementations would be.
namespace sgp::inst::lpbm {

template<typename HARDWARE_T>
struct Inst_SetMem : BaseInst<Inst_SetMem<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "[arg0] = arg1";
  }

  static std::string Name() {
    return "SetMem";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(0), (double)inst.GetArg(1));
  }

};

template<typename HARDWARE_T>
struct Inst_CopyMem : BaseInst<Inst_CopyMem<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "[arg1] = [arg0]";
  }

  static std::string Name() {
    return "CopyMem";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(1), mem_state.AccessWorking(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_SwapMem : BaseInst<Inst_SwapMem<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Swap [arg0] and [arg1]";
  }

  static std::string Name() {
    return "SwapMem";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const double val_0 = mem_state.AccessWorking(inst.GetArg(0));
    const double val_1 = mem_state.AccessWorking(inst.GetArg(1));
    mem_state.SetWorking(inst.GetArg(0), val_1);
    mem_state.SetWorking(inst.GetArg(1), val_0);
  }

};

template<typename HARDWARE_T>
struct Inst_InputToWorking : BaseInst<Inst_InputToWorking<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "working[arg1] = input[arg0]";
  }

  static std::string Name() {
    return "InputToWorking";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(inst.GetArg(1), mem_state.AccessInput(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_WorkingToOutput : BaseInst<Inst_WorkingToOutput<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "output[arg1] = working[arg0]";
  }

  static std::string Name() {
    return "WorkingToOutput";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetOutput(inst.GetArg(1), mem_state.AccessWorking(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_WorkingToGlobal : BaseInst<Inst_WorkingToGlobal<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "global[arg1] = working[arg0]";
  }

  static std::string Name() {
    return "WorkingToGlobal";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& mem_model = hw.GetMemoryModel();
    mem_model.SetGlobal(inst.GetArg(1), mem_state.AccessWorking(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_GlobalToWorking : BaseInst<Inst_GlobalToWorking<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "working[arg1] = global[arg0]";
  }

  static std::string Name() {
    return "GlobalToWorking";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& mem_model = hw.GetMemoryModel();
    mem_state.SetWorking(inst.GetArg(1), mem_model.AccessGlobal(inst.GetArg(0)));
  }

};

template<typename HARDWARE_T>
struct Inst_FullWorkingToGlobal : BaseInst<Inst_FullWorkingToGlobal<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Copy entire working memory buffer into global memory, overwriting conflicts.";
  }

  static std::string Name() {
    return "FullWorkingToGlobal";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
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
struct Inst_FullGlobalToWorking : BaseInst<Inst_FullGlobalToWorking<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Copy entire global memory buffer into working memory, overwriting conflicts.";
  }

  static std::string Name() {
    return "FullGlobalToWorking";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
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