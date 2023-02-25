#pragma once

#include <iostream>
#include <utility>
#include <unordered_map>

#include "emp/base/vector.hpp"
#include "emp/datastructs/set_utils.hpp"

namespace sgp::cpu::mem {

// TODO - make on return/on call re-configurable
// Demonstrative memory model based on original version of SignalGP.
/// Memory is represented as an unordered map with integer addresses that map to double values.
///   Addresses: int
///   Values: double
/// The memory model defines  "memory state", which is the memory local to calls on a CPU's call stack.
/// This BasicMemoryModel also includes a global memory buffer, which is shared across entire CPU.
class BasicMemoryModel {
public:
  struct BasicMemoryState;
  using memory_state_t = BasicMemoryState;
  using address_t = int;
  using value_t = double;
  using mem_buffer_t = std::unordered_map<address_t, value_t>;

  /// Memory state for a call on a CPU's call stack.
  /// - Consists of: working, input, and output memory buffers.
  struct BasicMemoryState {
    mem_buffer_t working_mem;      // Working memory buffer!
    mem_buffer_t input_mem;        // Input memory buffer!
    mem_buffer_t output_mem;       // Output memory buffer!

    BasicMemoryState(
      const mem_buffer_t& w=mem_buffer_t(),
      const mem_buffer_t& i=mem_buffer_t(),
      const mem_buffer_t& o=mem_buffer_t()
    ) :
      working_mem(w),
      input_mem(i),
      output_mem(o)
    { ; }

    BasicMemoryState(const BasicMemoryState&) = default;
    BasicMemoryState(BasicMemoryState&&) = default;

    /// Set value at given key in working memory. No questions asked.
    void SetWorking(int address, double value) {
      working_mem[address] = value;
    }

    /// Set value at given key in input memory. No questions asked.
    void SetInput(int address, double value) {
      input_mem[address] = value;
    }

    /// Set value at given key in output memory. No questions asked.
    void SetOutput(int address, double value) {
      output_mem[address] = value;
    }

    /// Get a reference to value at particular key in working memory. If key
    /// not yet in buffer, add key w/value of 0.
    double& AccessWorking(int address) {
      if (!emp::Has(working_mem, address)) {
        working_mem[address] = 0;
      }
      return working_mem[address];
    }

    /// Get a reference to value at particular key in input memory. If key
    /// not yet in buffer, add key w/value of 0.
    double& AccessInput(int address) {
      if (!emp::Has(input_mem, address)) {
        input_mem[address] = 0;
      }
      return input_mem[address];
    }

    /// Get a reference to value at particular key in output memory. If key
    /// not yet in buffer, add key w/value of 0.
    double & AccessOutput(int address) {
      if (!emp::Has(output_mem, address)) {
        output_mem[address] = 0;
      }
      return output_mem[address];
    }

    double GetWorking(int address) {
      return emp::Find(working_mem, address, 0.0);
    }

    double GetInput(int address) {
      return emp::Find(input_mem, address, 0.0);
    }

    double GetOutput(int address) {
      return emp::Find(output_mem, address, 0.0);
    }

    mem_buffer_t& GetWorkingMemory() { return working_mem; }
    const mem_buffer_t& GetWorkingMemory() const { return working_mem; }
    mem_buffer_t& GetInputMemory() { return input_mem; }
    const mem_buffer_t& GetInputMemory() const { return input_mem; }
    mem_buffer_t& GetOutputMemory() { return output_mem; }
    const mem_buffer_t& GetOutputMemory() const { return output_mem; }
  };

protected:
  mem_buffer_t global_mem = mem_buffer_t(); /// 'Global memory' buffer.

public:

  /// @brief Create new memory state.
  /// @param working - working memory buffer
  /// @param input - input memory buffer
  /// @param output - output memory buffer
  /// @return
  BasicMemoryState CreateMemoryState(
    const mem_buffer_t& working=mem_buffer_t(),
    const mem_buffer_t& input=mem_buffer_t(),
    const mem_buffer_t& output=mem_buffer_t()
  ) {
    return {working, input, output};
  }

  /// Reset memory model state.
  void Reset() {
    global_mem.clear();
  }

  /// Print a single memory buffer.
  void PrintMemoryBuffer(
    const mem_buffer_t& buffer,
    std::ostream& os=std::cout
  ) const {
    os << "[";
    bool comma = false;
    for (auto mem : buffer) {
      if (comma) os << ", ";
      os << "{" << mem.first << ":" << mem.second << "}";
      comma = true;
    }
    os << "]";
  }

  /// Print the state of memory.
  void PrintMemoryState(
    const memory_state_t& state,
    std::ostream& os=std::cout
  ) const {
    os << "Working memory (" << state.working_mem.size() << "): ";
    PrintMemoryBuffer(state.working_mem, os);
    os << "\n";
    os << "Input memory (" << state.input_mem.size() << "): ";
    PrintMemoryBuffer(state.input_mem, os);
    os << "\n";
    os << "Output memory (" << state.output_mem.size() << "): ";
    PrintMemoryBuffer(state.output_mem, os);
    os << "\n";
  }

  void PrintState(std::ostream& os=std::cout) const {
    os << "Global memory (" << global_mem.size() << "): ";
    PrintMemoryBuffer(global_mem, os);
  }

  mem_buffer_t& GetGlobalBuffer() { return global_mem; }
  const mem_buffer_t& GetGlobalBuffer() const { return global_mem; }

  void SetGlobal(int address, double val) {
    global_mem[address] = val;
  }

  double GetGlobal(int address) {
    return emp::Find(global_mem, address, 0.0);
  }

  double& AccessGlobal(int address) {
    if (!emp::Has(global_mem, address)) {
      global_mem[address] = 0;
    }
    return global_mem[address];
  }

  void OnModuleCall(
    memory_state_t& caller_mem,
    memory_state_t& callee_mem
  ) {
    for (auto& mem : caller_mem.working_mem) {
      callee_mem.SetInput(mem.first, mem.second);
    }
  }

  // Handle Module return
  void OnModuleReturn(
    memory_state_t& returning_mem,
    memory_state_t& caller_mem
  ) {
    for (auto& mem : returning_mem.output_mem) {
      caller_mem.SetWorking(mem.first, mem.second);
    }
  }

};

}