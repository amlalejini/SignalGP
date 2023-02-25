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

template<typename HARDWARE_T, int MULTIPLIER=1>
struct Inst_SetRegulator : BaseInst<Inst_SetRegulator<HARDWARE_T, MULTIPLIER>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Set the regulator of function specified by tag0 to working[arg0]";
  }

  static std::string Name() {
    return "SetRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
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
struct Inst_SetOwnRegulator : BaseInst<Inst_SetOwnRegulator<HARDWARE_T, MULTIPLIER>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Sets the regulator of the currently executing function to working[arg0]";
  }

  static std::string Name() {
    return "SetOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
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
struct Inst_ClearRegulator : BaseInst<Inst_ClearRegulator<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Set regulator of function specified by tag0 to 0";
  }

  static std::string Name() {
    return "ClearRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun(hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1));
    if (best_fun.size() == 0) { return; }
    hw.GetMatchBin().SetRegulator(best_fun[0], 0);
  }

};

template<typename HARDWARE_T>
struct Inst_ClearOwnRegulator : BaseInst<Inst_ClearOwnRegulator<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Set regulator of currently executing function to 0";
  }

  static std::string Name() {
    return "ClearOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& flow = call_state.GetTopFlow();
    hw.GetMatchBin().SetRegulator(flow.GetMP(), 0);
  }

};

template<typename HARDWARE_T, int MULTIPLIER=1>
struct Inst_AdjRegulator : BaseInst<Inst_AdjRegulator<HARDWARE_T, MULTIPLIER>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Adjust regulator of function specified by tag0";
  }

  static std::string Name() {
    return "AdjRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (!best_fun.size()) return;
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const double adj = MULTIPLIER * mem_state.AccessWorking(inst.GetArg(0));
    hw.GetMatchBin().AdjRegulator(best_fun[0], adj);
  }

};

template<typename HARDWARE_T, int MULTIPLIER=1>
struct Inst_AdjOwnRegulator : BaseInst<Inst_AdjOwnRegulator<HARDWARE_T, MULTIPLIER>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Adjust the regulator of the currently executing function";
  }

  static std::string Name() {
    return "AdjOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    auto& flow = call_state.GetTopFlow();
    const double adj = MULTIPLIER * mem_state.AccessWorking(inst.GetArg(0));
    hw.GetMatchBin().AdjRegulator(flow.GetMP(), adj);
  }

};

template<typename HARDWARE_T>
struct Inst_IncRegulator : BaseInst<Inst_IncRegulator<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Increment regulator of function specified by tag0 by 1";
  }

  static std::string Name() {
    return "IncRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (!best_fun.size()) return;
    hw.GetMatchBin().AdjRegulator(best_fun[0], 1.0);
  }

};

template<typename HARDWARE_T>
struct Inst_IncOwnRegulator : BaseInst<Inst_IncOwnRegulator<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Increment the regulator of the currently executing function by 1";
  }

  static std::string Name() {
    return "IncOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& flow = call_state.GetTopFlow();
    hw.GetMatchBin().AdjRegulator(flow.GetMP(), 1.0);
  }

};

template<typename HARDWARE_T>
struct Inst_DecRegulator : BaseInst<Inst_DecRegulator<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Decrement the regulator of the function specified by tag0 by 1";
  }

  static std::string Name() {
    return "DecRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(inst.GetTag(0), 1);
    if (!best_fun.size()) return;
    hw.GetMatchBin().AdjRegulator(best_fun[0], -1.0);
  }

};

template<typename HARDWARE_T>
struct Inst_DecOwnRegulator : BaseInst<Inst_DecOwnRegulator<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Decrement the regulator of the currently executing function by 1";
  }

  static std::string Name() {
    return "DecOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& flow = call_state.GetTopFlow();
    hw.GetMatchBin().AdjRegulator(flow.GetMP(), -1.0);
  }

};

template<typename HARDWARE_T>
struct Inst_SenseRegulator : BaseInst<Inst_SenseRegulator<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "working[arg0] = regulator value of function specified by tag0";
  }

  static std::string Name() {
    return "SenseRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
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
struct Inst_SenseOwnRegulator : BaseInst<Inst_SenseOwnRegulator<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "working[arg0] = regulator value of currently executing function";
  }

  static std::string Name() {
    return "SenseOwnRegulator";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
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