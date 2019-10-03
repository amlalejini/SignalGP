#ifndef EMP_SIGNALGP_V2_MEMORY_MODEL_H
#define EMP_SIGNALGP_V2_MEMORY_MODEL_H

#include <iostream>
#include <utility>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/Random.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"

#include "../EventLibrary.h"
#include "../InstructionLibrary.h"

namespace emp { namespace sgp_v2 {

  // Demonstrative memory model based on original version of SignalGP.
  class SimpleMemoryModel {
  public:
    struct SimpleMemoryState;
    using memory_state_t = SimpleMemoryState;
    // using memory_buffer_t = std::unordered
    using mem_buffer_t = std::unordered_map<int, double>;
    // global memory
    // local memory
    struct SimpleMemoryState {
      mem_buffer_t working_mem;      // Single buffer!
      mem_buffer_t input_mem;
      mem_buffer_t output_mem;
      // NOTE - memory state does not own this pointer!
      // Ptr<mem_buffer_t> global_mem_ptr;   // Check with others that this is a somewhat safe thing to do...

      SimpleMemoryState(const mem_buffer_t & w=mem_buffer_t(),
                        const mem_buffer_t & i=mem_buffer_t(),
                        const mem_buffer_t & o=mem_buffer_t())
        : working_mem(w), input_mem(i), output_mem(o) { ; }
      SimpleMemoryState(const SimpleMemoryState &) = default;
      SimpleMemoryState(SimpleMemoryState &&) = default;
    };

  protected:
    mem_buffer_t global_mem=mem_buffer_t();

  public:

    SimpleMemoryState CreateMemoryState(const mem_buffer_t & working=mem_buffer_t(),
                                        const mem_buffer_t & input=mem_buffer_t(),
                                        const mem_buffer_t & output=mem_buffer_t())
    { return {working, input, output}; }

    void PrintMemoryBuffer(const mem_buffer_t & buffer, std::ostream & os=std::cout) const {
      os << "[";
      bool comma = false;
      for (auto mem : buffer) {
        if (comma) os << ", ";
        os << "{" << mem.first << ":" << mem.second << "}";
        comma = true;
      }
      os << "]";
    }

    void PrintMemoryState(const memory_state_t & state, std::ostream & os=std::cout) const {
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

    void PrintState(std::ostream & os=std::cout) const {
      os << "Global memory (" << global_mem.size() << "): ";
      PrintMemoryBuffer(global_mem, os);
    }

  };

}}

#endif