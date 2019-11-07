#ifndef EMP_LINEAR_SIGNALGP_UTILS
#define EMP_LINEAR_SIGNALGP_UTILS

#include <iostream>
#include <utility>
#include <memory>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/Random.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"

#include "EventLibrary.h"
#include "InstructionLibrary.h"

namespace emp { namespace signalgp { namespace lsgp_utils {


  /// Library of flow types.
  /// e.g., WHILE, IF, ROUTINE, et cetera
  /// NOTE - I'm not sure that I'm a fan of how this is organized/named/setup.
  /// BASIC: if statements (for now)
  enum class FlowType : size_t { BASIC, WHILE_LOOP, ROUTINE, CALL };

  /// Everything the execution stepper needs to know to manage (open, close, break)
  /// any of the execution flow types.
  struct FlowInfo {
    FlowType type;    ///< Flow type ID?
    size_t mp;        ///< Module pointer. Which module is being executed?
    size_t ip;        ///< Instruction pointer. Which instruction is executed?
    size_t begin;     ///< Where does the flow begin?
    size_t end;       ///< Where does the flow end?

    FlowInfo(FlowType _type, size_t _mp=(size_t)-1, size_t _ip=(size_t)-1,
              size_t _begin=(size_t)-1, size_t _end=(size_t)-1)
      : type(_type), mp(_mp), ip(_ip), begin(_begin), end(_end) { ; }

    size_t GetBegin() const { return begin; }
    size_t GetEnd() const { return end; }
    size_t GetMP() const { return mp; }
    size_t GetIP() const { return ip; }
    FlowType GetType() const { return type; }
  };

  /// State information for a function call.
  template<typename MEMORY_STATE_T>
  struct CallState {
    MEMORY_STATE_T memory;            ///< Memory local to this call state.
    emp::vector<FlowInfo> flow_stack; ///< Stack of 'Flow' (a stack of fancy read heads)
    bool circular;                    ///< Should call wrap when IP goes off end? Or, implicitly return?

    CallState(const MEMORY_STATE_T & _mem=MEMORY_STATE_T(), bool _circular=false)
      : memory(_mem), flow_stack(), circular(_circular) { ; }

    bool IsFlow() const { return !flow_stack.empty(); }

    emp::vector<FlowInfo> & GetFlowStack() { return flow_stack; }

    FlowInfo & GetTopFlow() {
      emp_assert(flow_stack.size());
      return flow_stack.back();
    }

    bool IsCircular() const { return circular; }

    MEMORY_STATE_T & GetMemory() { return memory; }

    // --- For your convenience shortcuts: ---
    /// Set the instruction pointer of the 'flow' at the top of the flow stack.
    void SetIP(size_t i) { emp_assert(flow_stack.size()); flow_stack.back().ip = i; }

    /// Set the module pointer at the top of the flow stack.
    void SetMP(size_t m) { emp_assert(flow_stack.size()); flow_stack.back().mp = m; }

    /// Get the instruction pointer at the top of the flow stack.
    size_t GetIP() const { emp_assert(flow_stack.size()); return flow_stack.back().ip; }

    /// Get the module pointer at the top of the flow stack.
    size_t GetMP() const { emp_assert(flow_stack.size()); return flow_stack.back().mp; }

    /// Get a mutable reference to the instruction pointer from the top of the flow stack.
    size_t & IP() { emp_assert(flow_stack.size()); return flow_stack.back().ip; }

    /// Get a mutable reference to the module pointer from the top of the flow stack.
    size_t & MP() { emp_assert(flow_stack.size()); return flow_stack.back().mp; }
  };

  /// Execution State. TODO - add label?
  template<typename MEMORY_MODEL_T>
  struct ExecState {
    using memory_state_t = typename MEMORY_MODEL_T::memory_state_t;
    using call_state_t = CallState<memory_state_t>;
    emp::vector<call_state_t> call_stack;   ///< Program call stack.

    /// Empty out the call stack.
    void Clear() { call_stack.clear(); }

    /// Get a reference to the current (top) call state on the call stack.
    /// Requires the call stack to be not empty.
    call_state_t & GetTopCallState() {
      emp_assert(call_stack.size(), "Cannot get top call state from empty call stack.");
      return call_stack.back();
    }

    /// Get a mutable reference to the entire call stack.
    emp::vector<call_state_t> & GetCallStack() { return call_stack; }
  };

}}}

#endif