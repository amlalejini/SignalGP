#pragma once

#include "CallState.hpp"

namespace sgp::cpu::linprg {

/// Execution State. TODO - add label?
template<typename MEMORY_MODEL_T>
struct ExecState {
  using memory_state_t = typename MEMORY_MODEL_T::memory_state_t;
  using call_state_t = CallState<memory_state_t>;
  emp::vector<call_state_t> call_stack;   ///< Program call stack.

  /// Empty out the call stack.
  void Clear() { call_stack.clear(); }
  void Reset() { call_stack.clear(); }

  /// Get a reference to the current (top) call state on the call stack.
  /// Requires the call stack to be not empty.
  call_state_t & GetTopCallState() {
    emp_assert(call_stack.size(), "Cannot get top call state from empty call stack.");
    return call_stack.back();
  }

  /// Get a mutable reference to the entire call stack.
  emp::vector<call_state_t> & GetCallStack() { return call_stack; }
};

} // End sgp::cpu::linprg namespace