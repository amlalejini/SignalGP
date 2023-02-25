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
struct Inst_Nop : BaseInst<Inst_Nop<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "No operation.";
  }

  static std::string Name() {
    return "Nop";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(HARDWARE_T& hw, const inst_t& inst) { ; }

};

template<typename HARDWARE_T>
struct Inst_Inc : BaseInst<Inst_Inc<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Increment. Requires 1 argument.";
  }

  static std::string Name() {
    return "Inc";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    // Increment value in local memory @ [ARG0]
    ++call_state.GetMemory().AccessWorking(inst.GetArg(0));
  }

};

template<typename HARDWARE_T>
struct Inst_Dec : BaseInst<Inst_Dec<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Decrement. Requires 1 argument.";
  }

  static std::string Name() {
    return "Dec";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    // Decrement value in local memory @ [ARG0]
    --call_state.GetMemory().AccessWorking(inst.GetArg(0));
  }

};

template<typename HARDWARE_T>
struct Inst_Not : BaseInst<Inst_Not<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Logical not. [arg0] = 1 if ([arg0] == 0) else 0";
  }

  static std::string Name() {
    return "Not";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(0),
      mem_state.GetWorking(inst.GetArg(0)) == 0.0
    );
  }

};

template<typename HARDWARE_T>
struct Inst_Add : BaseInst<Inst_Add<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Add";
  }

  static std::string Name() {
    return "Add";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(2),
      mem_state.AccessWorking(inst.GetArg(0)) + mem_state.AccessWorking(inst.GetArg(1))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_Sub : BaseInst<Inst_Sub<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Subtract";
  }

  static std::string Name() {
    return "Sub";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(2),
      mem_state.AccessWorking(inst.GetArg(0)) - mem_state.AccessWorking(inst.GetArg(1))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_Mult : BaseInst<Inst_Mult<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Multiply";
  }

  static std::string Name() {
    return "Mult";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(2),
      mem_state.AccessWorking(inst.GetArg(0)) * mem_state.AccessWorking(inst.GetArg(1))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_Div : BaseInst<Inst_Div<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Division";
  }

  static std::string Name() {
    return "Div";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const auto& denom = mem_state.AccessWorking(inst.GetArg(1));
    if (denom == 0.0) return; // Do nothing.
    const auto& num = mem_state.AccessWorking(inst.GetArg(0));
    mem_state.SetWorking(inst.GetArg(2), num / denom);
  }

};

template<typename HARDWARE_T>
struct Inst_Mod : BaseInst<Inst_Mod<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Modulo";
  }

  static std::string Name() {
    return "Mod";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    const int denom = (int)mem_state.AccessWorking(inst.GetArg(1));
    if (denom == 0.0) return; // Do nothing.
    const int num = (int)mem_state.AccessWorking(inst.GetArg(0));
    mem_state.SetWorking(
      inst.GetArg(2),
      static_cast<int64_t>(num) % static_cast<int64_t>(denom)
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestEqu : BaseInst<Inst_TestEqu<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Test if equals";
  }

  static std::string Name() {
    return "TestEqu";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(2),
      mem_state.AccessWorking(inst.GetArg(0)) == mem_state.AccessWorking(inst.GetArg(1))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestNEqu : BaseInst<Inst_TestNEqu<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Test if not equals";
  }

  static std::string Name() {
    return "TestNEqu";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(2),
      mem_state.AccessWorking(inst.GetArg(0)) != mem_state.AccessWorking(inst.GetArg(1))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestLess : BaseInst<Inst_TestLess<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Test if less than";
  }

  static std::string Name() {
    return "TestLess";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(2),
      mem_state.AccessWorking(inst.GetArg(0)) < mem_state.AccessWorking(inst.GetArg(1))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestLessEqu : BaseInst<Inst_TestLessEqu<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Test if less than or equal to";
  }

  static std::string Name() {
    return "TestLessEqu";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(2),
      mem_state.AccessWorking(inst.GetArg(0)) <= mem_state.AccessWorking(inst.GetArg(1))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestGreater : BaseInst<Inst_TestGreater<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Test if greater than";
  }

  static std::string Name() {
    return "TestGreater";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(2),
      mem_state.AccessWorking(inst.GetArg(0)) > mem_state.AccessWorking(inst.GetArg(1))
    );
  }

};

template<typename HARDWARE_T>
struct Inst_TestGreaterEqu : BaseInst<Inst_TestGreaterEqu<HARDWARE_T>, HARDWARE_T> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "Test if greater than or equal to";
  }

  static std::string Name() {
    return "TestGreaterEqu";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
    auto& call_state = hw.GetCurThread().GetExecState().GetTopCallState();
    auto& mem_state = call_state.GetMemory();
    mem_state.SetWorking(
      inst.GetArg(2),
      mem_state.AccessWorking(inst.GetArg(0)) >= mem_state.AccessWorking(inst.GetArg(1))
    );
  }

};

template<
  typename HARDWARE_T,
  typename MaxRatio=std::ratio<1>,
  typename MinRatio=std::ratio<0>
>
struct Inst_Terminal : BaseInst<
  Inst_Terminal<
    HARDWARE_T,
    MaxRatio,
    MinRatio
  >,
  HARDWARE_T
> {
  using hw_t = HARDWARE_T;
  using inst_prop_t = typename HARDWARE_T::inst_prop_t;
  using inst_t = typename HARDWARE_T::inst_t;

  static std::string Desc() {
    return "working[arg0] = tag0 interpreted as a double.";
  }

  static std::string Name() {
    return "Terminal";
  }

  static std::unordered_set<inst_prop_t> Properties() {
    return std::unordered_set<inst_prop_t>{};
  }

  static void Run(hw_t& hw, const inst_t& inst) {
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