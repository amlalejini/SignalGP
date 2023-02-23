#pragma once

#include <utility>
#include <functional>
#include <map>

#include "emp/datastructs/set_utils.hpp"

namespace sgp {
namespace cpu {
namespace linear_prog {

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

  FlowInfo(
    FlowType _type,
    size_t _mp=(size_t)-1,
    size_t _ip=(size_t)-1,
    size_t _begin=(size_t)-1,
    size_t _end=(size_t)-1
  ) :
    type(_type),
    mp(_mp),
    ip(_ip),
    begin(_begin),
    end(_end)
  { ; }

  size_t GetBegin() const { return begin; }
  size_t GetEnd() const { return end; }
  size_t GetMP() const { return mp; }
  size_t GetIP() const { return ip; }
  FlowType GetType() const { return type; }
  bool IsBasic() const { return type == FlowType::BASIC; }
  bool IsWhileLoop() const { return type == FlowType::WHILE_LOOP; }
  bool IsRoutine() const { return type == FlowType::ROUTINE; }
  bool IsCall() const { return type == FlowType::CALL; }
};

// NEED: hardware_t, flow_t, exec_state_t,
template<
  typename HARDWARE_T,
  typename EXEC_STATE_T
>
struct FlowHandler {
  using exec_state_t = EXEC_STATE_T;
  using hardware_t = HARDWARE_T;
  using fun_end_flow_t = std::function<void(hardware_t&, exec_state_t&)>;
  using fun_open_flow_t = std::function<void(hardware_t&, exec_state_t&, const FlowInfo&)>;

  struct FlowControl {
    fun_open_flow_t open_flow_fun;
    fun_end_flow_t close_flow_fun;
    fun_end_flow_t break_flow_fun;
  };

  /// Mapping from flow type to flow control structure.
  // todo - could change this to an array because we know things at compile time
  std::map<FlowType, FlowControl> lib = {
    {FlowType::BASIC, FlowControl()},
    {FlowType::WHILE_LOOP, FlowControl()},
    {FlowType::ROUTINE, FlowControl()},
    {FlowType::CALL, FlowControl()}
  };

  FlowControl& operator[](FlowType type) {
    emp_assert(emp::Has(lib, type), "FlowType not recognized!");
    return lib[type];
  }

  const FlowControl& operator[](FlowType type) const {
    emp_assert(emp::Has(lib, type), "FlowType not recognized!");
    return lib[type];
  }

  std::string FlowTypeToString(FlowType type) const {
    switch (type) {
      case FlowType::BASIC: return "BASIC";
      case FlowType::WHILE_LOOP: return "WHILE_LOOP";
      case FlowType::ROUTINE: return "ROUTINE";
      case FlowType::CALL: return "CALL";
      default: return "UNKNOWN";
    }
  }

  void OpenFlow(
    hardware_t& hw,
    const FlowInfo& new_flow,
    exec_state_t& state
  ) {
    FlowType type = new_flow.type;
    emp_assert(emp::Has(lib, type), "FlowType not recognized!");
    lib[type].open_flow_fun(hw, state, new_flow);
  }

  void CloseFlow(
    hardware_t& hw,
    FlowType type,
    exec_state_t& state
  ) {
    emp_assert(emp::Has(lib, type), "FlowType not recognized!");
    lib[type].close_flow_fun(hw, state);
  }

  void BreakFlow(
    hardware_t& hw,
    FlowType type,
    exec_state_t& state
  ) {
    emp_assert(emp::Has(lib, type), "FlowType not recognized!");
    lib[type].break_flow_fun(hw, state);
  }

};

} // End linear_prog namespace
} // End cpu namespace
} // End sgp namespace