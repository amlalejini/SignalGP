#ifndef EMP_SIGNALGP_V2_H
#define EMP_SIGNALGP_V2_H

#include <iostream>
#include <utility>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/Random.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"

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

// TODOS
// - [ ] break this file up
// - [ ] clean up matchbin handling
// - [ ] reconcile random num gen ownership

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
    // using memory_buffer_t = std::unordered

    // global memory
    // local memory
    struct SimpleMemoryState {

    };

  };

  /// Base flow information
  /// - should be sufficient for basic flow types
  struct BaseFlowInfo {
    size_t id=(size_t)-1;        ///< Flow type ID?
    size_t ip=(size_t)-1;        ///< Instruction pointer. Which instruction is executed?
    size_t mp=(size_t)-1;        ///< Module pointer. Which module is being executed?

    size_t begin=(size_t)-1;     ///< Where does the flow begin?
    size_t end=(size_t)-1;       ///< Where does the flow end?
  };

  // Simple program:
  // - Linear.
  // - Instructions can have tag or numeric arguments.
  template<typename TAG_T, typename ARGUMENT_T=int>
  class SimpleProgram {
  public:
    struct Instruction;
    using tag_t = TAG_T;
    using arg_t = ARGUMENT_T;
    using inst_t = Instruction;

    // todo => print arg fun
    // todo => print tag fun

    struct Instruction {
      size_t id;                      ///< Instruction ID
      emp::vector<arg_t> args;
      emp::vector<tag_t> tags;

      Instruction(size_t _id,
                  const emp::vector<arg_t> & _args=emp::vector<arg_t>(),
                  const emp::vector<tag_t> & _tags=emp::vector<tag_t>())
        : id(_id), args(_args), tags(_tags) { ; }

      void SetID(size_t _id) { id = _id; }
      size_t GetID() const { return id; }

      emp::vector<arg_t> & GetArgs() { return args; }
      emp::vector<tag_t> & GetTags() { return tags; }

      bool operator==(const Instruction & other) const {
        return std::tie(id, args, tags) == std::tie(other.id, other.args, other.tags);
      }

      bool operator!=(const Instruction & other) const { return !(*this == other); }

      bool operator<(const Instruction & other) const {
        return std::tie(id, args, tags) < std::tie(other.id, other.args, other.tags);
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
                  const emp::vector<tag_t> & tags=emp::vector<tag_t>()) {
      inst_seq.emplace_back(id, args, tags);
    }

    /// Push instruction to program by name.
    template<typename HARDWARE_T, typename INST_PROPERTY_T>
    void PushInst(const InstructionLibrary<HARDWARE_T, Instruction, INST_PROPERTY_T> & ilib,
                  const std::string & name,
                  const emp::vector<arg_t> & args=emp::vector<arg_t>(),
                  const emp::vector<tag_t> & tags=emp::vector<tag_t>()) {
      emp_assert(ilib.IsInst(name), "Uknown instruction name", name);
      PushInst(ilib.GetID(name), args, tags);
    }

    /// Push instruction to program.
    void PushInst(const Instruction & inst) { inst_seq.emplace_back(inst); }

    /// Is the given instruction valid?
    template<typename HARDWARE_T, typename INST_PROPERTY_T>
    bool IsValidInst(const InstructionLibrary<HARDWARE_T, Instruction, INST_PROPERTY_T> & ilib,
                     const Instruction & inst) {
      return inst.id < ilib.GetSize();
    }

  };

  // Each program type needs their own 'ExecutionStepper' to manage execution
  // - knows about program structure
  // - knows how to make programs
  // - knows how to execute programs
  template<typename MEMORY_MODEL_T,
           typename TAG_T=emp::BitSet<16>,
           typename INST_ARGUMENT_T=int,
           typename MATCHBIN_T=emp::MatchBin< size_t, emp::HammingMetric<16>, emp::RankedSelector<std::ratio<16+8, 16> >>
           >
  class SimpleExecutionStepper {
  public:
    struct ExecState;
    struct Module;

    enum class InstProperty { MODULE };

    using exec_stepper_t = SimpleExecutionStepper<MEMORY_MODEL_T, TAG_T, INST_ARGUMENT_T, MATCHBIN_T>;
    using memory_model_t = MEMORY_MODEL_T;
    using memory_state_t = typename memory_model_t::memory_state_t;
    using exec_state_t = ExecState;
    using program_t = SimpleProgram<TAG_T, INST_ARGUMENT_T>;
    using hardware_t = SignalGP<exec_stepper_t>;
    using inst_t = typename program_t::inst_t;
    using inst_lib_t = InstructionLibrary<hardware_t, inst_t, InstProperty>;
    using inst_prop_t = InstProperty;
    using tag_t = TAG_T;
    using module_t = Module;
    using matchbin_t = MATCHBIN_T;

    /// Library of flow types.
    /// e.g., WHILE, IF, ROUTINE, et cetera
    /// NOTE - I'm not sure that I'm a fan of how this is organized/named/setup.
    enum class FlowType : size_t { BASIC, WHILE_LOOP, ROUTINE, CALL };
    struct FlowHandler {
      struct FlowControl {
        using flow_control_fun_t = std::function<void(exec_state_t &)>;
        flow_control_fun_t open_flow_fun;
        flow_control_fun_t close_flow_fun;
        flow_control_fun_t break_flow_fun;
      };
      std::map<FlowType, FlowControl> lib = { {FlowType::BASIC, FlowControl()},
                                              {FlowType::WHILE_LOOP, FlowControl()},
                                              {FlowType::ROUTINE, FlowControl()},
                                              {FlowType::CALL, FlowControl()} };

      FlowControl & operator[](FlowType type) {
        emp_assert(Has(lib, type), "FlowType not recognized!");
        return lib[type];
      }

      const FlowControl & operator[](FlowType type) const {
        emp_assert(Has(lib, type), "FlowType not recognized!");
        return lib[type];
      }
    };

    struct CallState {
      // Local memory
      // memory!
      // flow stack
      emp::vector<BaseFlowInfo> flow_stack; ///< Stack of 'Flow' (read heads)
    };

    /// Execution State.
    struct ExecState {
      // CallStack
      emp::vector<CallState> call_stack;   ///< Program call stack.
    };

    /// Module definition.
    struct Module {
      size_t id;      ///< Module ID. Used to call/reference module.
      size_t begin;   ///< First instruction in module (will be the module definition instruction).
      size_t end;     ///< The last instruction in the module.
      tag_t tag;      ///< Module tag. Used to call/reference module.
      std::unordered_set<size_t> in_module; ///< instruction positions belonging to this module.
      // todo

      Module(size_t _id, size_t _begin=0, size_t _end=0, const tag_t & _tag=tag_t())
        : id(_id), begin(_begin), end(_end), tag(_tag), in_module() { ; }

      size_t GetSize() const { return in_module.size(); }

      size_t GetID() const { return id; }

      tag_t & GetTag() { return tag; }
      const tag_t & GetTag() const { return tag; }

      bool InModule(size_t ip) const { return Has(in_module, ip); }
    };

  protected:
    Ptr<inst_lib_t> inst_lib;
    FlowHandler flow_handler;

    memory_model_t memory_model;

    program_t program;

    emp::vector<module_t> modules;

    tag_t default_module_tag;

    Ptr<Random> random_ptr;         // TODO - does signalgp need a random number generator anymore?

    matchbin_t matchbin;
    bool is_matchbin_cache_dirty;
    std::function<void()> fun_clear_matchbin_cache = [this](){ this->ResetMatchBin(); };

    void SetupDefaultFlowControl() { // TODO!
      flow_handler[FlowType::BASIC].open_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::BASIC].close_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::BASIC].break_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::WHILE_LOOP].open_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::WHILE_LOOP].close_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::WHILE_LOOP].break_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::ROUTINE].open_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::ROUTINE].close_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::ROUTINE].break_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::CALL].open_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::CALL].close_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::CALL].break_flow_fun = [](exec_state_t & exec_state) { ; };
    }

  public:
    SimpleExecutionStepper(Ptr<inst_lib_t> ilib,
                           Ptr<Random> rnd)
      : inst_lib(ilib),
        flow_handler(),
        memory_model(),
        program(),
        modules(),
        default_module_tag(),
        random_ptr(rnd),
        matchbin(rnd ? *rnd : *emp::NewPtr<emp::Random>()),
        is_matchbin_cache_dirty(true)
    {
      // Configure default flow control
      SetupDefaultFlowControl();
    }

    void ResetMatchBin() {
      // todo!
      matchbin.Clear();
      is_matchbin_cache_dirty = false;
      // TODO - reset match bin!
      for (size_t i = 0; i < modules.size(); ++i) {
        matchbin.Set(i, modules[i].GetTag(), i);
      }
    }

    void SingleExecutionStep(hardware_t & hardware, exec_state_t & exec_state) {
      std::cout << "SingleExecutionStep!" << std::endl;
    }

    // Load program!
    // - program = in_program
    // - initialize program modules! ('compilation')
    /// Set program for this hardware object.
    /// After updating hardware's program, run 'UpdateModules'.
    void SetProgram(const program_t & _program) {
      program = _program;
      UpdateModules();
    }

    void SetDefaultTag(const tag_t & _tag) {
      default_module_tag = _tag;
    }

    // todo - check to see if this works
    void UpdateModules() {
      std::cout << "Update modules!" << std::endl;
      // Clear out the current modules.
      modules.clear();
      // Do nothing if there aren't any instructions to look at.
      if (!program.GetSize()) return;
      // Scan program for module definitions.
      std::unordered_set<size_t> dangling_instructions;
      for (size_t pos = 0; pos < program.GetSize(); ++pos) {
        inst_t & inst = program[pos];
        // Is this a module definition?
        if (inst_lib->HasProperty(inst.GetID(), inst_prop_t::MODULE)) {
          // If this isn't the first module we've found, mark this position as the
          // last position of the previous module.
          if (modules.size()) { modules.back().end = pos; }
          emp_assert(inst.GetTags().size(), "MODULE-defining instructions must have tag arguments to be used with this execution stepper.");
          const size_t mod_id = modules.size(); // Module ID for new module.
          modules.emplace_back(mod_id, ( (pos+1) < program.GetSize() ) ? pos+1 : 0, -1, inst.GetTags()[0]);
        } else {
          // We didn't find a new module. Track which module this instruction belongs to:
          // - If we've found a module, add it to the current module.
          // - If we haven't found a module, note that this instruction is dangling.
          if (modules.size()) { modules.back().in_module.emplace(pos); }
          else { dangling_instructions.emplace(pos); }
        }
      }
      // At this point, we know about all of the modules (if any).
      // First, we need to set the end point for the last module we found.
      if (modules.size()) {
        // If the first module begins at the beginning of the instruction, the last
        // module must end at the end of the program.
        // Otherwise, the last module ends where the first module begins.
        // if (modules[0].begin == 0) modules.back().end = program.GetSize();
        // else
        modules.back().end = (modules[0].begin - 1 > 0) ? modules[0].begin - 1 : program.GetSize();
      } else {
        // Found no modules. Add a default module that starts at the beginning and
        // ends at the end.
        modules.emplace_back(0, 0, program.GetSize(), default_module_tag);
      }
      // Now, we need to take care of the dangling instructions.
      // - We're going to assume the program is circular, so dangling instructions
      //   belong to the last module we found.
      for (size_t val : dangling_instructions) modules.back().in_module.emplace(val);

      // Reset matchbin
      ResetMatchBin();
    }

    emp::vector<module_t> & GetModules() { return modules;  }
    size_t GetNumModules() const { return modules.size(); }

    program_t & GetProgram() { return program; }

    void PrintModules(std::ostream & os=std::cout) const {
      os << "Modules: {";
      for (size_t i = 0; i < modules.size(); ++i) {
        if (i) os << ",";
        os << "[id: " << modules[i].id << ", begin: " << modules[i].begin << ", end: " << modules[i].end << "]";
        os << "(" << modules[i].tag << ")"; // TODO - make this generic!
      }
      os << "}";
    }
  };


  // todo - move function implementations outside of class
  template<typename EXEC_STEPPER_T>
  class SignalGP {
  public:
    struct Thread;
    // struct CallState;

    using exec_stepper_t = EXEC_STEPPER_T;
    using exec_state_t = typename exec_stepper_t::exec_state_t;
    using program_t = typename exec_stepper_t::program_t;
    using tag_t = typename exec_stepper_t::tag_t;
    using module_t = typename exec_stepper_t::module_t;
    using memory_model_t = typename exec_stepper_t::memory_model_t;
    using memory_state_t = typename memory_model_t::memory_state_t;
    using matchbin_t = typename exec_stepper_t::matchbin_t;

    // using custom_comp_t = CUSTOM_COMPONENT_T;

    using hardware_t = SignalGP<exec_stepper_t>;

    using event_t = BaseEvent;
    using event_lib_t = EventLibrary<hardware_t>;

    using thread_t = Thread;

    using fun_print_program_t = std::function<void(std::ostream &)>;
    using fun_print_modules_t = std::function<void(std::ostream &)>;

    struct Thread {
      size_t id;
      // label?
      exec_state_t exec_state;
    };

    struct ModuleDescriptor {
      size_t id;
      tag_t tag;
    };

  protected:
    Ptr<const event_lib_t> event_lib; /// These are the events this hardware knows about.

    std::deque<event_t> event_queue;

    Ptr<Random> random_ptr;         // TODO - does signalgp need a random number generator anymore?
    bool random_owner;

    Ptr<exec_stepper_t> exec_stepper;

    // memory_model_t memory_model;

    emp::vector<thread_t> threads;

    emp::vector<size_t> thread_exec_order;
    emp::vector<size_t> pending_threads;
    size_t cur_thread_id=(size_t)-1;
    bool is_executing=false;

    size_t max_threads=(size_t)-1;

    bool initialized=false;

    fun_print_program_t fun_print_program = [](std::ostream & os) { os << "Print program function not set!"; };
    fun_print_modules_t fun_print_modules = [](std::ostream & os) { os << "Print modules function not set!"; };

  public:

    SignalGP(Ptr<const event_lib_t> elib,
             Ptr<Random> rnd=nullptr)
      : event_lib(elib),
        random_ptr(rnd),
        random_owner(false)
    {
      // If no random provided, create one.
      if (!rnd) NewRandom();
    }

    ~SignalGP() {
      if (random_owner) random_ptr.Delete();
      if (initialized) {
        exec_stepper.Delete();
      }
    }

    /// NOTE - Also not a huge fan of this
    template<typename... ARGS>
    void InitExecStepper(ARGS&&... args) {
      if (initialized) exec_stepper.Delete();
      exec_stepper = emp::NewPtr<exec_stepper_t>(std::forward<ARGS>(args)...);
      initialized = true;
    }

    // Todo - Resets
    void ResetMatchBin() {
      // todo!
      if (initialized) {
        exec_stepper->ResetMatchBin();
      }
    }

    // todo - get execution stepper

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

    program_t & GetProgram() {
      emp_assert(initialized, "Hardware must be initialized!");
      return exec_stepper->GetProgram();
    }

    exec_stepper_t & GetExecStepper() { return *exec_stepper; }

    void SetProgram(const program_t & program) {
      emp_assert(initialized, "Hardware must be initialized!");
      // todo - clear the matchbin
      exec_stepper->SetProgram(program); // TODO - finish this function!
      // => get modules for match bin!
    }

    void SetPrintProgramFun(const fun_print_program_t & print_fun) {
      fun_print_program = print_fun;
    }

    void SetPrintModulesFun(const fun_print_modules_t & print_fun) {
      fun_print_modules = print_fun;
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
      emp_assert(initialized, "SignalGP Hardware has not been properly initialized!");
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
        exec_stepper->SingleExecutionStep(*this, threads[cur_thread_id].exec_state);

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

    // todo - call module
    // todo - spawn thread

    void PrintProgram(std::ostream & os=std::cout) const { fun_print_program(os); }
    void PrintModules(std::ostream & os=std::cout) const { fun_print_modules(os); }

  };

}
}

#endif