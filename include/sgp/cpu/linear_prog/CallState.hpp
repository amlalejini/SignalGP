#pragma once

#include "emp/base/vector.hpp"

#include "Flow.hpp"

namespace sgp {
namespace cpu {
namespace linear_prog {

/// State information for a function call.
template<typename MEMORY_STATE_T>
struct CallState {

  MEMORY_STATE_T memory;            ///< Memory local to this call state.
  emp::vector<FlowInfo> flow_stack; ///< Stack of 'Flow' (a stack of fancy read heads)
  bool circular;                    ///< Should call wrap when IP goes off end? Or, implicitly return?

  CallState(
    const MEMORY_STATE_T& _mem=MEMORY_STATE_T(),
    bool _circular=false
  ) :
    memory(_mem),
    flow_stack(),
    circular(_circular)
  { ; }

  bool IsFlow() const { return !flow_stack.empty(); }

  emp::vector<FlowInfo>& GetFlowStack() { return flow_stack; }

  FlowInfo& GetTopFlow() {
    emp_assert(flow_stack.size());
    return flow_stack.back();
  }

  const FlowInfo& GetTopFlow() const {
    emp_assert(flow_stack.size());
    return flow_stack.back();
  }

  bool IsCircular() const { return circular; }

  MEMORY_STATE_T& GetMemory() { return memory; }

  // --- For your convenience shortcuts: ---
  /// Set the instruction pointer of the 'flow' at the top of the flow stack.
  void SetIP(size_t i) {
    emp_assert(flow_stack.size());
    flow_stack.back().ip = i;
  }

  /// Set the module pointer at the top of the flow stack.
  void SetMP(size_t m) {
    emp_assert(flow_stack.size());
    flow_stack.back().mp = m;
  }

  /// Get the instruction pointer at the top of the flow stack.
  size_t GetIP() const {
    emp_assert(flow_stack.size());
    return flow_stack.back().ip;
  }

  /// Get the module pointer at the top of the flow stack.
  size_t GetMP() const {
    emp_assert(flow_stack.size());
    return flow_stack.back().mp;
  }

  /// Get a mutable reference to the instruction pointer from the top of the flow stack.
  size_t& IP() {
    emp_assert(flow_stack.size());
    return flow_stack.back().ip;
  }

  /// Get a mutable reference to the module pointer from the top of the flow stack.
  size_t& MP() {
    emp_assert(flow_stack.size());
    return flow_stack.back().mp;
  }
};

} // End linear_prog namespace
} // End cpu namespace
} // End sgp namespace