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

template<typename HARDWARE_T, int MULTIPLIER=1>
struct Inst_SetRegulator : BaseInstructionSpec<Inst_SetRegulator<HARDWARE_T, MULTIPLIER>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Set the regulator of function specified by tag0 to working[arg0]";
  }

  static std::string name() {
    return "SetRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun(hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1));
    if (best_fun.size() == 0) { return; }
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const double regulator_val = MULTIPLIER * mem_state.AccessWorking(inst.GetArg(0));
    // (+) values down regulate
    // (-) values up regulate
    hw.GetMatchBin().SetRegulator(best_fun[0], regulator_val);
  }

};

template<typename HARDWARE_T, int MULTIPLIER=1>
struct Inst_SetOwnRegulator : BaseInstructionSpec<Inst_SetOwnRegulator<HARDWARE_T, MULTIPLIER>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Sets the regulator of the currently executing function to working[arg0]";
  }

  static std::string name() {
    return "SetOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& flow = call_state.GetTopFlow();
    const double regulator_val = MULTIPLIER * mem_state.AccessWorking(inst.GetArg(0));
    // (+) values down regulate
    // (-) values up regulate
    hw.GetMatchBin().SetRegulator(flow.GetMP(), regulator_val);
  }

};

template<typename HARDWARE_T>
struct Inst_ClearRegulator : BaseInstructionSpec<Inst_ClearRegulator<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Set regulator of function specified by tag0 to 0";
  }

  static std::string name() {
    return "ClearRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun(hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1));
    if (best_fun.size() == 0) { return; }
    hw.GetMatchBin().SetRegulator(best_fun[0], 0);
  }

};

template<typename HARDWARE_T>
struct Inst_ClearOwnRegulator : BaseInstructionSpec<Inst_ClearOwnRegulator<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Set regulator of currently executing function to 0";
  }

  static std::string name() {
    return "ClearOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& flow = call_state.GetTopFlow();
    hw.GetMatchBin().SetRegulator(flow.GetMP(), 0);
  }

};

template<typename HARDWARE_T, int MULTIPLIER=1>
struct Inst_AdjRegulator : BaseInstructionSpec<Inst_AdjRegulator<HARDWARE_T, MULTIPLIER>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Adjust regulator of function specified by tag0";
  }

  static std::string name() {
    return "AdjRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (!best_fun.size()) return;
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const double adj = MULTIPLIER * mem_state.AccessWorking(inst.GetArg(0));
    hw.GetMatchBin().AdjRegulator(best_fun[0], adj);
  }

};

template<typename HARDWARE_T, int MULTIPLIER=1>
struct Inst_AdjOwnRegulator : BaseInstructionSpec<Inst_AdjOwnRegulator<HARDWARE_T, MULTIPLIER>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Adjust the regulator of the currently executing function";
  }

  static std::string name() {
    return "AdjOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& flow = call_state.GetTopFlow();
    const double adj = MULTIPLIER * mem_state.AccessWorking(inst.GetArg(0));
    hw.GetMatchBin().AdjRegulator(flow.GetMP(), adj);
  }

};

template<typename HARDWARE_T>
struct Inst_IncRegulator : BaseInstructionSpec<Inst_IncRegulator<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Increment regulator of function specified by tag0 by 1";
  }

  static std::string name() {
    return "IncRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (!best_fun.size()) return;
    hw.GetMatchBin().AdjRegulator(best_fun[0], 1.0);
  }

};

template<typename HARDWARE_T>
struct Inst_IncOwnRegulator : BaseInstructionSpec<Inst_IncOwnRegulator<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Increment the regulator of the currently executing function by 1";
  }

  static std::string name() {
    return "IncOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& flow = call_state.GetTopFlow();
    hw.GetMatchBin().AdjRegulator(flow.GetMP(), 1.0);
  }

};

template<typename HARDWARE_T>
struct Inst_DecRegulator : BaseInstructionSpec<Inst_DecRegulator<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Decrement the regulator of the function specified by tag0 by 1";
  }

  static std::string name() {
    return "DecRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (!best_fun.size()) return;
    hw.GetMatchBin().AdjRegulator(best_fun[0], -1.0);
  }

};

template<typename HARDWARE_T>
struct Inst_DecOwnRegulator : BaseInstructionSpec<Inst_DecOwnRegulator<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Decrement the regulator of the currently executing function by 1";
  }

  static std::string name() {
    return "DecOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& flow = call_state.GetTopFlow();
    hw.GetMatchBin().AdjRegulator(flow.GetMP(), -1.0);
  }

};

template<typename HARDWARE_T>
struct Inst_SenseRegulator : BaseInstructionSpec<Inst_SenseRegulator<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "working[arg0] = regulator value of function specified by tag0";
  }

  static std::string name() {
    return "SenseRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (best_fun.size()) {
      auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
      auto& mem_state = call_state.GetMemory();
      mem_state.SetWorking(
        inst.GetArg(0),
        hw.GetMatchBin().ViewRegulator(best_fun[0])
      );
    }
  }

};

template<typename HARDWARE_T>
struct Inst_SenseOwnRegulator : BaseInstructionSpec<Inst_SenseOwnRegulator<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "working[arg0] = regulator value of currently executing function";
  }

  static std::string name() {
    return "SenseOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& flow = call_state.GetTopFlow();
    mem_state.SetWorking(
      inst.GetArg(0),
      hw.GetMatchBin().ViewRegulator(flow.GetMP())
    );
  }

};

}