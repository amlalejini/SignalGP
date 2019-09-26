#ifndef EMP_SIGNALGP_V2_H
#define EMP_SIGNALGP_V2_H

#include <iostream>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/Random.h"

#include "EventLibrary.h"
#include "InstructionLibrary.h"

/**
 * Terminology
 *
 * References
*/

// Some thoughts on terminology:
// - events = labeled payload
//   - What is the label? And, what is the payload? (want to be flexible if possible)
// - programs contain 'instructions'
//   - Instructions know how they are executed

// Things that SignalGP may need to be templated on:
// - event library type (==> event type)
// - instruction library type (==> instruction type)
// - program type

// Notes
// - Currently events housed/handled in queue (order of reception), should we make
//   this more flexible?
// - 'Modules'
//    - Custom module (i.e., trait)
//    - Memory module
//    - Program module

namespace emp {
namespace sgp_v2 {

  template<typename EXEC_STEPPER_T>
  class SignalGP;

  struct BaseEvent {
    size_t id;

    size_t GetID() const { return id; }
  };


  struct SimpleMemoryModel {
    struct SimpleMemoryState;
    using memory_state_t = SimpleMemoryState;

    struct SimpleMemoryState {

    };
  };

  /// Base flow information
  /// - should be sufficient for basic flow types
  struct BaseFlowInfo {
    size_t id;        ///< Flow type ID?
    size_t ip;        ///< Instruction pointer. Which instruction is executed?
    size_t mp;        ///< Module pointer. Which module is being executed?

    size_t begin;     ///< Where does the flow begin?
    size_t end;       ///< Where does the flow end?
  };

  // Simple program:
  // - Linear.
  // - Instructions can have tag or numeric arguments.
  template<typename LABEL_T, typename ARGUMENT_T=int>
  class SimpleProgram {
  public:
    using label_t = LABEL_T;
    using arg_t = ARGUMENT_T;

    struct Instruction {
      size_t id;                      ///< Instruction ID
      emp::vector<arg_t> args;
      emp::vector<label_t> labels;

      Instruction(size_t _id,
                  const emp::vector<arg_t> & _args=emp::vector<arg_t>(),
                  const emp::vector<label_t> & _labels=emp::vector<label_t>())
        : id(_id), args(_args), labels(_labels) { ; }

      void SetID(size_t _id) { id = _id; }
      size_t GetID() const { return id; }

      emp::vector<arg_t> & GetArgs() { return args; }
      emp::vector<label_t> & GetLabels() { return labels; }

      bool operator==(const Instruction & other) const {
        return std::tie(id, args, labels) == std::tie(other.id, other.args, other.labels);
      }

      bool operator!=(const Instruction & other) const { return !(*this == other); }

      bool operator<(const Instruction & other) const {
        return std::tie(id, args, labels) < std::tie(other.id, other.args, other.labels);
      }
    };

  protected:
    emp::vector<Instruction> inst_seq;

  public:
    SimpleProgram(const emp::vector<Instruction> & iseq=emp::vector<Instruction>())
      : inst_seq(iseq) { ; }

    SimpleProgram(const SimpleProgram &) = default;

    bool operator==(const SimpleProgram & other) const { return inst_seq == other.inst_seq; }
    bool operator!=(const SimpleProgram & other) const { return !(*this == other); }
    bool operator<(const SimpleProgram & other) const { return inst_seq < other.inst_seq; }

    /// Allow program's instruction sequence to be indexed as if a vector.
    Instruction & operator[](size_t id) {
      emp_assert(id < inst_seq.size());
      return inst_seq[id];
    }

    /// Allow program's instruction sequence to be indexed as if a vector.
    const Instruction & operator[](size_t id) const {
      emp_assert(id < inst_seq.size());
      return inst_seq[id];
    }

    /// Clear the program's instruction sequence.
    void Clear() { inst_seq.clear(); }

    /// Get program size.
    size_t GetSize() const { return inst_seq.size(); }

    /// Is a given position valid in this program?
    bool IsValidPosition(size_t pos) const { return pos < GetSize(); }

    // ----- Program modifications -----

    /// Set program's instruction sequence to the one given.
    void SetProgram(const emp::vector<Instruction> & p) { inst_seq = p; }

    /// Push instruction to instruction set.
    /// - No validation! We're trusting that 'id' is legit!
    void PushInst(size_t id,
                  const emp::vector<arg_t> & args=emp::vector<arg_t>(),
                  const emp::vector<label_t> & labels=emp::vector<label_t>()) {
      inst_seq.emplace_back(id, args, labels);
    }

    /// Push instruction to program by name.
    template<typename HARDWARE_T>
    void PushInst(const InstructionLibrary<HARDWARE_T, Instruction> & ilib,
                  const std::string & name,
                  const emp::vector<arg_t> & args=emp::vector<arg_t>(),
                  const emp::vector<label_t> & labels=emp::vector<label_t>()) {
      emp_assert(ilib.IsInst(name), "Uknown instruction name", name);
      PushInst(ilib.GetID(name), args, labels);
    }

    /// Push instruction to program.
    void PushInst(const Instruction & inst) { inst_seq.emplace_back(inst); }

    /// Is the given instruction vaid?
    template<typename HARDWARE_T>
    bool IsValidInst(const InstructionLibrary<HARDWARE_T, Instruction> & ilib,
                     const Instruction & inst) {
      return inst.id < ilib.GetSize();
    }

  };

  // Each program type needs their own 'ExecutionStepper' to manage execution
  template<typename MEMORY_MODEL_T, typename LABEL_T, typename INST_ARGUMENT_T=int>
  class SimpleExecutionStepper {
  public:
    struct ExecState;

    using exec_stepper_t = SimpleExecutionStepper<MEMORY_MODEL_T, LABEL_T, INST_ARGUMENT_T>;
    using memory_model_t = MEMORY_MODEL_T;
    using memory_state_t = typename memory_model_t::memory_state_t;
    using exec_state_t = ExecState;
    using program_t = SimpleProgram<LABEL_T, INST_ARGUMENT_T>;
    using hardware_t = SignalGP<exec_stepper_t>;
    using inst_t = typename program_t::Instruction;
    using inst_lib_t = InstructionLibrary<hardware_t, inst_t>;
    using module_label_t = LABEL_T;

    struct CallState {
      exec_state_t exec_state;
      memory_state_t mem_state;
    };

    /// Library of flow types.
    /// e.g., WHILE, IF, ROUTINE, et cetera
    struct FlowLib {
      struct FlowDef {
        std::string name;
        std::function<void()> open_flow_fun;
        std::function<void()> close_flow_fun;
        std::function<void()> break_flow_fun;
        std::string desc;

        FlowDef(const std::string & _name,
                const std::string & _desc)
          : name(_name), desc(_desc)
        {

        }

        FlowDef(const FlowDef &) = default;

      };
      std::map<std::string, size_t> flow_lib;

    };

    /// Execution State.
    struct ExecState {
      emp::vector<BaseFlowInfo> flow_stack; ///< Stack of 'Flow' (read heads)

      // size_t GetIP() const {}

      // size_t GetMP() const {}
    };

    // struct Module {

    // };

  private:
    Ptr<inst_lib_t> inst_lib;

    memory_model_t memory_model;

    program_t program;

  public:

    void SingleExecutionStep(hardware_t & hardware, exec_state_t & exec_state) {
      //
    }

  };

  // class BaseSignalGP {
  //   // virtual functions:
  //   // - access trait
  //   // - do memory => but how?
  //   // - call/fork/return/terminate

  // };

  // todo - move function implementations outside of class
  template<typename EXEC_STEPPER_T>
  class SignalGP {
  public:
    struct Thread;
    // struct CallState;

    using exec_stepper_t = EXEC_STEPPER_T;
    using exec_state_t = typename exec_stepper_t::exec_state_t;

    // using custom_comp_t = CUSTOM_COMPONENT_T;

    using hardware_t = SignalGP<exec_stepper_t>;

    using memory_model_t = typename exec_stepper_t::memory_model_t;
    using memory_state_t = typename memory_model_t::memory_state_t;

    using module_label_t = typename exec_stepper_t::module_label_t;

    using event_t = BaseEvent;
    using event_lib_t = EventLibrary<hardware_t>;

    // using call_state_t = CallState;
    // using call_state_t = CallState;
    using thread_t = Thread;

    struct Thread {
      size_t id;
      // label?
      exec_state_t exec_state;
    };

    struct ModuleDescriptor {
      size_t id;
      module_label_t label;
    };

  protected:
    // Ptr<const inst_lib_t> inst_lib;   /// These are the instructions this hardware knows about.
    Ptr<const event_lib_t> event_lib; /// These are the events this hardware knows about.

    std::deque<event_t> event_queue;

    Ptr<Random> random_ptr;
    bool random_owner;

    exec_stepper_t exec_stepper;

    // memory_model_t memory_model;

    emp::vector<thread_t> threads;

    emp::vector<size_t> thread_exec_order;
    emp::vector<size_t> pending_threads;
    size_t cur_thread_id=(size_t)-1;
    bool is_executing=false;

    size_t max_threads=(size_t)-1;

  public:

    SignalGP(Ptr<const event_lib_t> elib,
             Ptr<Random> rnd=nullptr)
      : event_lib(elib),
        random_ptr(rnd), random_owner(false)
    {
      // If no random provided, create one.
      if (!rnd) NewRandom();
    }

    ~SignalGP() {
      if (random_owner) random_ptr.Delete();
    }

    // Todo - Resets

    // Accessors

    /// Get the maximum number of threads allowed to run simultaneously on this hardware
    /// object.
    size_t GetMaxThreads() const { return max_threads; }

    /// Get event library associated with hardware.
    Ptr<const event_lib_t> GetEventLib() const { return event_lib; }

    /// Get reference to random number generator used by this hardware.
    Random & GetRandom() { return *random_ptr; }

    /// Get pointer to random number generator used by this hardware.
    Ptr<Random> GetRandomPtr() { return random_ptr; }

    /// Get a reference to active threads.
    /// NOTE: use responsibly! No safety gloves here!
    emp::vector<thread_t> & GetThreads() { return threads; }

    /// Get the ID of the currently executing thread. If hardware is not in midst
    /// of an execution cycle, this will return (size_t)-1.
    size_t GetCurThreadID() { return cur_thread_id; }

    thread_t & GetCurThread() {
      emp_assert(is_executing, "Hardware is not executing! No current thread.");
      emp_assert(cur_thread_id < threads.size());
      return threads[cur_thread_id];
    }

    exec_state_t & GetCurExecState() {
      return GetCurThread().exec_state;
    }

    /// todo
    void SetMaxThreads(size_t n) { ; }

    /// ...
    void NewRandom(int seed=-1) {
      if (random_owner) random_ptr.Delete();
      else random_ptr = nullptr;
      random_ptr.New(seed);
      random_owner = true;
    }

    // ----- Hardware Execution -----
    // todo - add signals throughout?

    // thread_t & SpawnThread() {
    //   ;
    // }

    /// Handle an event (on this hardware) now!.
    void HandleEvent(const event_t & event) { event_lib->HandleEvent(*this, event); }

    /// Trigger an event (from this hardware).
    void TriggerEvent(const event_t & event) { event_lib->TriggerEvent(*this, event); }

    /// Queue an event (to be handled by this hardware) next time this hardware
    /// unit is executed.
    void QueueEvent(const event_t & event) {
      event_queue.emplace_back(event);
    }

    /// Advance the hardware by a single step.
    void SingleProcess() {
      // todo - validate that program exists!

      // Handle events
      while (!event_queue.empty()) {
        HandleEvent(event_queue.front());
        event_queue.pop_front();
      }
      // Distribute one unit of computational time to each thread.
      is_executing = true;
      size_t thread_cnt = threads.size();
      size_t adjust = 0;
      emp_assert(thread_cnt == thread_exec_order.size()); // Number of threads should match size of thread execution order

      size_t thread_order_index = 0;
      while (thread_order_index < thread_cnt) {
        // Set the current thread to the thread ID we want to execute.
        cur_thread_id = thread_exec_order[thread_order_index];
        // Do we need to move the current core over in the execution order to make
        // our order tracker contiguous?
        if (adjust) {
          // If we need to adjust, clear out current position, move cur thread ID up by adjust.
          thread_exec_order[thread_order_index] = (size_t)-1;
          thread_exec_order[thread_order_index - adjust] = cur_thread_id;
        }
        // Execute the thread (outsourced to execution stepper)!
        exec_stepper.SingleExecutionStep(*this, threads[cur_thread_id].exec_state);

        // TODO - is this thread dead?

        ++thread_order_index;
      }
      is_executing = false;
      thread_exec_order.resize(thread_cnt - adjust);
      // todo - spawn any new threads
      cur_thread_id = (size_t)-1;
    }

    /// Advance hardware by some arbitrary number of steps.
    void Process(size_t num_steps) {
      for (size_t i = 0; i < num_steps; ++i) {
        SingleProcess();
      }
    }

  };

}
}

#endif