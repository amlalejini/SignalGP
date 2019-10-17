#ifndef EMP_TOY_SIGNALGP_H
#define EMP_TOY_SIGNALGP_H

#include <iostream>

#include "base/vector.h"

#include "BaseSignalGP.h"

namespace toy_signalgp_impl {
  struct ExecState {
    size_t value=0;
    void Clear() { value = 0; }
  };
}


template<typename CUSTOM_COMPONET_T=emp::signalgp::DefaultCustomComponent>
class ToySignalGP : virtual public emp::signalgp::BaseSignalGP< ToySignalGP<CUSTOM_COMPONET_T>,
                                                        toy_signalgp_impl::ExecState,
                                                        // typename ToySignalGP<CUSTOM_COMPONET_T>::ExecState,
                                                        size_t,
                                                        CUSTOM_COMPONET_T> {
public:
  using this_t = ToySignalGP<CUSTOM_COMPONET_T>;

  // struct ExecState {
  //   size_t value=0;
  //   void Clear() { value = 0; }
  // };
  using exec_state_t = toy_signalgp_impl::ExecState;            ///< REQUIRED. Thread state information.
  // using exec_state_t = ExecState;

  using base_hw_t = emp::signalgp::BaseSignalGP<this_t,
                                                exec_state_t,
                                                size_t,
                                                CUSTOM_COMPONET_T>;

  using program_t = emp::vector<size_t>;     ///< REQUIRED. What types of programs does this stepper execute?
  using tag_t = size_t;                      ///< REQUIRED. What does this stepper use to reference different modules?


  using event_lib_t = typename base_hw_t::event_lib_t;
  using event_t = typename base_hw_t::event_t;
  using thread_t = typename base_hw_t::Thread;

protected:
  program_t program;

public:
  ToySignalGP(emp::Ptr<event_lib_t> elib)
    : base_hw_t(elib)
  {
    std::cout << "Derived constructor!" << std::endl;
  }

  program_t & GetProgram() { return program; }

  void SetProgram(const program_t & p) {
    program = p;
  }

  /// REQUIRED
  void ResetProgram() {
    program.clear();
  }

  // No extra hardware state to reset.
  /// REQUIRED
  void Reset() { ResetProgram(); }

  /// REQUIRED
  emp::vector<size_t> FindModuleMatch(const tag_t & tag, size_t N) {
    // tag_t => size_t
    // Let tag % program.size() index into program.
    emp::vector<size_t> matches;
    if (!program.size()) { return matches; }
    for (size_t i = 0; i < N; ++i) {
      matches.emplace_back((tag + i) % program.size());
    }
    return matches;
  }

  /// Module ids index into program directly (each module is a size_t)
  /// REQUIRED
  void InitThread(thread_t & thread, size_t module_id) {
    emp_assert(module_id < program.size());
    thread.exec_state.value = program[module_id];
  }

  /// REQUIRED
  void SingleExecutionStep(this_t & hw, thread_t & thread) {
    if (thread.GetExecState().value == 0) {
      thread.SetDead(true);
    } else {
      thread.exec_state.value -= 1;
    }
  }

};

#endif