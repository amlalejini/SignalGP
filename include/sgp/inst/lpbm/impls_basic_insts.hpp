#pragma once

#include <iostream>
#include <string>
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

// NOTE - The best way to organize/define instruction specifications is still up
//        up for discussion. Still not sure what the best way forward is in terms
//        in terms of balancing flexibility, speed, ease of use, etc

namespace sgp::inst::lpbm {

template<typename HARDWARE_T>
struct Inst_Nop : BaseInstructionSpec<Inst_Nop<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "No operation.";
  }

  static std::string name() {
    return "Nop";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) { ; }

};

template<typename HARDWARE_T>
struct Inst_Inc : BaseInstructionSpec<Inst_Inc<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Increment. Requires 1 argument.";
  }

  static std::string name() {
    return "Inc";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    // Increment value in local memory @ [ARG0]
    ++call_state.GetMemory().AccessWorking(inst.GetArg(0));
  }

};

template<typename HARDWARE_T>
struct Inst_Dec : BaseInstructionSpec<Inst_Dec<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Decrement. Requires 1 argument.";
  }

  static std::string name() {
    return "Dec";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    // Decrement value in local memory @ [ARG0]
    --call_state.GetMemory().AccessWorking(inst.GetArg(0));
  }

};

template<typename HARDWARE_T>
struct Inst_Not : BaseInstructionSpec<Inst_Not<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Logical not. [arg0] = 1 if ([arg0] == 0) else 0";
  }

  static std::string name() {
    return "Not";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.GetWorking(inst.GetArg(0)) == 0.0
    );
  }

};

template<typename HARDWARE_T>
struct Inst_Add : BaseInstructionSpec<Inst_Add<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Add";
  }

  static std::string name() {
    return "Add";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.AccessWorking(inst.GetArg(1)) + mem_state.AccessWorking(inst.GetArg(2))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_Sub : BaseInstructionSpec<Inst_Sub<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Subtract";
  }

  static std::string name() {
    return "Sub";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.AccessWorking(inst.GetArg(1)) - mem_state.AccessWorking(inst.GetArg(2))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_Mult : BaseInstructionSpec<Inst_Mult<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Multiply";
  }

  static std::string name() {
    return "Mult";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.AccessWorking(inst.GetArg(1)) * mem_state.AccessWorking(inst.GetArg(2))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_Div : BaseInstructionSpec<Inst_Div<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Division";
  }

  static std::string name() {
    return "Div";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const auto& denom = mem_state.AccessWorking(inst.GetArg(2));
    if (denom == 0.0) return; // Do nothing.
    const auto& num = mem_state.AccessWorking(inst.GetArg(1));
    mem_state.SetWorking(inst.GetArg(0), num / denom);
  }

};

template<typename HARDWARE_T>
struct Inst_Mod : BaseInstructionSpec<Inst_Mod<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Modulo";
  }

  static std::string name() {
    return "Mod";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const int denom = (int)mem_state.AccessWorking(inst.GetArg(2));
    if (denom == 0.0) return; // Do nothing.
    const int num = (int)mem_state.AccessWorking(inst.GetArg(1));
    mem_state.SetWorking(
      inst.GetArg(0),
      static_cast<int64_t>(num) % static_cast<int64_t>(denom)
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestEqu : BaseInstructionSpec<Inst_TestEqu<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Test if equals";
  }

  static std::string name() {
    return "TestEqu";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.AccessWorking(inst.GetArg(1)) == mem_state.AccessWorking(inst.GetArg(2))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestNEqu : BaseInstructionSpec<Inst_TestNEqu<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Test if not equals";
  }

  static std::string name() {
    return "TestNEqu";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.AccessWorking(inst.GetArg(1)) != mem_state.AccessWorking(inst.GetArg(2))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestLess : BaseInstructionSpec<Inst_TestLess<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Test if less than";
  }

  static std::string name() {
    return "TestLess";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.AccessWorking(inst.GetArg(1)) < mem_state.AccessWorking(inst.GetArg(2))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestLessEqu : BaseInstructionSpec<Inst_TestLessEqu<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Test if less than or equal to";
  }

  static std::string name() {
    return "TestLessEqu";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.AccessWorking(inst.GetArg(1)) <= mem_state.AccessWorking(inst.GetArg(2))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestGreater : BaseInstructionSpec<Inst_TestGreater<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Test if greater than";
  }

  static std::string name() {
    return "TestGreater";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.AccessWorking(inst.GetArg(1)) > mem_state.AccessWorking(inst.GetArg(2))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestGreaterEqu : BaseInstructionSpec<Inst_TestGreaterEqu<HARDWARE_T>> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "Test if greater than or equal to";
  }

  static std::string name() {
    return "TestGreaterEqu";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.AccessWorking(inst.GetArg(1)) >= mem_state.AccessWorking(inst.GetArg(2))
    );
  }

};

template<
  typename HARDWARE_T,
  typename MaxRatio=std::ratio<1>,
  typename MinRatio=std::ratio<0>
>
struct Inst_Terminal : BaseInstructionSpec<
  Inst_Terminal<
    HARDWARE_T,
    MaxRatio,
    MinRatio
  >
> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = inst::InstProperty;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string desc() {
    return "working[arg0] = tag0 interpreted as a double.";
  }

  static std::string name() {
    return "Terminal";
  }

  static std::unordered_set<inst_prop_t> properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void run(hw_t& hw, const inst_t& inst) {
    constexpr double max = static_cast<double>(MaxRatio::num) / MaxRatio::den;
    constexpr double min = static_cast<double>(MinRatio::num) / MinRatio::den;

    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();

    const auto& tag = inst.GetTag(0);

    const double val = (
      // tag.GetDouble() / tag.MaxDouble()
      tag.GetValue() / (emp::Pow2(tag.GetSize()) - 1)
    ) * (max - min) - min;

    mem_state.SetWorking(inst.GetArg(0), val);
  }

};


}