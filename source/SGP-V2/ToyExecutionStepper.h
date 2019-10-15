#ifndef EMP_SIGNALGP_V2_TOY_EXECSTEPPER_H
#define EMP_SIGNALGP_V2_TOY_EXECSTEPPER_H

#include <iostream>

#include "base/vector.h"

#include "../SignalGP.h"

template<typename ...SGP_Ts>
class ToyExecutionStepper {
public:
  struct ExecState;

  using exec_stepper_t = ToyExecutionStepper<SGP_Ts...>;

  using program_t = emp::vector<size_t>;     ///< REQUIRED. What types of programs does this stepper execute?
  using tag_t = size_t;                      ///< REQUIRED. What does this stepper use to reference different modules?
  using exec_state_t = ExecState;            ///< REQUIRED. Thread state information.

  using signalgp_t = emp::sgp_v2::SignalGP<exec_stepper_t, SGP_Ts...>;
  using thread_t = typename signalgp_t::Thread;

  struct ExecState {
    size_t value=0;
    void Clear() { value = 0; }
  };

protected:
  program_t program;

public:

  /// REQUIRED
  program_t & GetProgram() { return program; }

  /// REQUIRED
  void SetProgram(const program_t & p) {
    program = p;
  }

  /// REQUIRED
  void ResetProgram() {
    program.clear();
  }

  // No extra hardware state to reset.
  /// REQUIRED
  void ResetHardwareState() { ; }

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
  void SingleExecutionStep(signalgp_t & hw, thread_t & thread) {
    if (thread.GetExecState().value == 0) {
      thread.SetDead(true);
    } else {
      thread.exec_state.value -= 1;
    }
  }
};

#endif