#ifndef EMP_SIGNALGP_V2_VHARDWARE_H
#define EMP_SIGNALGP_V2_VHARDWARE_H

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

  struct BaseEvent {
    size_t id;

    size_t GetID() const { return id; }

    void Print(std::ostream & os) const {
      os << "{id:" << GetID() << "}";
    }
  };


  // todo - move function implementations outside of class
  // todo - make signalgp hardware not awful (& safe) to make copies of
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

    // using fun_print_program_t = std::function<void(std::ostream &)>;
    // using fun_print_modules_t = std::function<void(std::ostream &)>;
    // using fun_print_hardware_state_t = std::function<void(std::ostream &)>;
    using fun_print_event_t = std::function<void(const event_t &, std::ostream &)>;
    using fun_print_execution_state_t = std::function<void(const exec_state_t &, std::ostream &)>;

    struct Thread {
      // label?
      exec_state_t exec_state;
      // bool dead;

      Thread(const exec_state_t & _exec_state=exec_state_t())
        : exec_state(_exec_state) { ; }

      void Reset() {
        exec_state.Clear(); // TODO - make this functionality more flexible! Currently assumes exec_state_t has a Clear function!
      }

      exec_state_t & GetExecState() { return exec_state; }
      const exec_state_t & GetExecState() const { return exec_state; }

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

    // Thread management
    size_t max_threads=64;
    emp::vector<thread_t> threads;
    emp::vector<size_t> active_threads;
    emp::vector<size_t> unused_threads;
    std::deque<size_t> pending_threads;

    // emp::vector<size_t> thread_exec_order;
    // emp::vector<size_t> pending_threads;
    size_t cur_thread_id=(size_t)-1;
    bool is_executing=false;

    bool initialized=false;

    std::function<void(std::ostream &)> fun_print_program = [](std::ostream & os) { return; };
    std::function<void(std::ostream &)> fun_print_modules = [](std::ostream & os) { return; };
    std::function<void(std::ostream &)> fun_print_hardware_state = [](std::ostream & os) { return; };
    std::function<void(std::ostream &)> fun_print_exec_stepper_state = [](std::ostream & os) { return; };
    fun_print_execution_state_t fun_print_execution_state = [](const exec_state_t & state, std::ostream & os) { return; };
    fun_print_event_t fun_print_event = [](const event_t & event, std::ostream & os) { event.Print(os); };


  public:

    SignalGP(Ptr<const event_lib_t> elib,
             Ptr<Random> rnd=nullptr)
      : event_lib(elib),
        random_ptr(rnd),
        random_owner(false),
        threads(max_threads), active_threads(), unused_threads(max_threads), pending_threads()
    {
      // If no random provided, create one.
      if (!rnd) NewRandom();
      // Set all threads to unused.
      for (int i = unused_threads.size() - 1; i > 0; --i) {
        unused_threads[i] = i;
      }
    }

    ~SignalGP() {
      if (random_owner) random_ptr.Delete();
      if (initialized) {
        exec_stepper.Delete();
      }
    }

    /// NOTE - Also not a huge fan of this
    // Do we want execution steppers to have a setup funtion that we pass SignalGP
    // hardware to and the stepper will wire things up to the hardware (e.g., on
    // thread creation)?
    // Should we pass reference to hardware to execution stepper?
    //  - would allow execution stepper to hook up to lambdas
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

    void SetPrintProgramFun(const std::function<void(std::ostream &)> & print_fun) {
      fun_print_program = print_fun;
    }

    void SetPrintModulesFun(const std::function<void(std::ostream &)> & print_fun) {
      fun_print_modules = print_fun;
    }

    void SetPrintExecStepperStateFun(const std::function<void(std::ostream &)> & print_fun) {
      fun_print_exec_stepper_state = print_fun;
    }

    void SetPrintHardwareStateFun(const std::function<void(std::ostream &)> & print_fun) {
      fun_print_hardware_state = print_fun;
    }

    void SetPrintExecutionStateFun(const fun_print_execution_state_t & print_fun) {
      fun_print_execution_state = print_fun;
    }

    void SetPrintEventFun(const fun_print_event_t & print_fun) {
      fun_print_event = print_fun;
    }

    /// TODO - TEST => PULLED FROM ORIGINAL SIGNALGP
    // Warning: If you decrease max threads, you may kill actively running threads.
    // Warning: If you decrease max threads
    void SetThreadLimit(size_t n) {
      emp_assert(n, "Max thread count must be greater than 0.");
      emp_assert(!is_executing, "Cannot adjust SignalGP hardware max thread count while executing.");
      // (1) Resize threads to new max thread count.
      threads.resize(n);
      if (n > max_threads) {
        // Increase total available threads. Add new threads to inactive threads
        // vector.
        for (size_t i = max_threads; i < n; ++i) {
          unused_threads.insert(unused_threads.begin(), i);
        }
      } else if (n < max_threads) {
        // Decreasing total threads, adjust active and inactive threads (maintaining
        // relative ordering in each).

        // (1) Fix active threads (maintaining relative ordering).
        size_t active_idx = 0;
        size_t active_cnt = active_threads.size();
        size_t active_adjust = 0;
        while (active_idx < active_cnt) {
          size_t thread_id = active_threads[active_idx];
          if (thread_id >= n) { // Do we need to eliminate this thread id from active threads?
            // If yes, set to -1 and increment adjust.
            active_threads[active_idx - active_adjust] = (size_t)-1;
            ++active_adjust;
          } else if (active_adjust) { // Still valid thread ID, so do we need to defragment?
            active_threads[active_idx] = (size_t)-1;
            active_threads[active_idx - active_adjust] = thread_id;
          }
          ++active_idx;
        }
        active_threads.resize(active_cnt - active_adjust);

        // (2) Fix inactive threads (maintain relative ordering).
        size_t unused_idx = 0;
        size_t unused_cnt = unused_threads.size();
        size_t unused_adjust = 0;
        while (unused_idx < unused_cnt) {
          size_t thread_id = unused_threads[unused_idx];
          if (thread_id >= n) { // Do we need to eliminate this thread id from inactive cores?
            // If yes, set to -1 and increment adjust.
            unused_threads[unused_idx - unused_adjust] = (size_t)-1;
            ++unused_adjust;
          } else if (unused_adjust) { // Still valid thread ID, so do we need to defragment?
            unused_threads[unused_idx] = (size_t)-1;
            unused_threads[unused_idx - unused_adjust] = thread_id;
          }
          ++unused_idx;
        }
        unused_threads.resize(unused_cnt - unused_adjust);

        // (3) Fix pending threads (maintain relative ordering).
        std::deque<size_t> fixed_pending;
        for (size_t i = 0; i < pending_threads.size(); ++i) {
          if (pending_threads[i] < n) fixed_pending.emplace_back(pending_threads[i]);
        }
        pending_threads = fixed_pending;
      }

      max_threads = n; // update max threads
    }

    /// ...
    void NewRandom(int seed=-1) {
      if (random_owner) random_ptr.Delete();
      else random_ptr = nullptr;
      random_ptr.New(seed);
      random_owner = true;
    }

    // ----- Hardware Execution -----
    // todo - add signals throughout?
    // Spawn number (<= n) of threads, using tag to select which modules.
    // return vector of spawned thread ids
    emp::vector<size_t> SpawnThreads(const tag_t & tag, size_t n=1) {
      emp::vector<size_t> matches(exec_stepper->FindModuleMatch(tag, n));
      emp::vector<size_t> thread_ids;
      for (size_t match : matches) {
        const size_t thread_id = SpawnThread(match);
        if (thread_id >= threads.size()) break;
        thread_ids.emplace_back(thread_id);
      }
      return thread_ids;
    }

    // returns thread_id of spawned thread (-1 if none spawned)
    size_t SpawnThread(size_t module_id) {
      if (!unused_threads.size()) return (size_t)-1; // If no unused threads, return.
      // Which thread should we spawn?
      size_t thread_id = unused_threads.back();
      unused_threads.pop_back();

      // Reset thread.
      thread_t & thread = threads[thread_id];
      thread.Reset();

      // Initialize thread state.
      exec_stepper->InitThread(thread, module_id); // TODO - this is somewhat gross; requires yet another function of the exec_stepper.

      // Mark thread as pending if hardware is currently executing. Otherwise,
      // mark it is active.
      if (is_executing) { pending_threads.emplace_back(thread_id); }
      else { active_threads.emplace_back(thread_id); }

      return thread_id;
    }

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
      size_t active_thread_id = 0;
      size_t thread_cnt = active_threads.size();
      size_t adjust = 0;
      // std::cout << "Thread count: " << thread_cnt << std::endl;
      // Run each active thread.
      while (active_thread_id < thread_cnt) {
        // Set the current thread to the thread ID we want to execute.
        cur_thread_id = active_threads[active_thread_id];
        // Do we need to move the current core over in the execution order to make
        // our order tracker contiguous?
        if (adjust) {
          // If we need to adjust, clear out current position, move cur thread ID up by adjust.
          active_threads[active_thread_id] = (size_t)-1;
          active_threads[active_thread_id - adjust] = cur_thread_id;
        }
        // Execute the thread (outsourced to execution stepper)!
        exec_stepper->SingleExecutionStep(*this, threads[cur_thread_id].exec_state);

        // TODO - is this thread dead?
        if (false) {
          active_threads[active_thread_id - adjust] = (size_t)-1;
          unused_threads.emplace_back(cur_thread_id);
          ++adjust;
        }
        ++active_thread_id;
      }

      // No longer executing.
      is_executing = false;
      // Update execution stack size to be accurate.
      active_threads.resize(thread_cnt - adjust);
      // todo - spawn any new threads
      while (pending_threads.size()) {
        active_threads.emplace_back(pending_threads.front());
        pending_threads.pop_front();
      }
      cur_thread_id = (size_t)-1; // Update current thread (to be nonsense while not executing)
    }

    /// Advance hardware by some arbitrary number of steps.
    void Process(size_t num_steps) {
      for (size_t i = 0; i < num_steps; ++i) {
        SingleProcess();
      }
    }

    // todo - call module
    void PrintActiveThreadStates(std::ostream & os=std::cout) const {
      for (size_t i = 0; i < active_threads.size(); ++i) {
        size_t thread_id = active_threads[i];
        const thread_t & thread = threads[thread_id];
        os << "Thread " << i << " (ID="<< thread_id << "):\n";
        PrintExecutionState(thread.GetExecState(), os);
        os << "\n";
      }
    }

    void PrintProgram(std::ostream & os=std::cout) const { fun_print_program(os); }

    void PrintModules(std::ostream & os=std::cout) const { fun_print_modules(os); }

    void PrintExecStepperState(std::ostream & os=std::cout) const { fun_print_exec_stepper_state(os); }

    void PrintHardwareState(std::ostream & os=std::cout) const { fun_print_hardware_state(os); }

    void PrintThreadUsage(std::ostream & os=std::cout) const {
      // Active threads
      os << "Active threads (" << active_threads.size() << "): [";
      for (size_t i = 0; i < active_threads.size(); ++i) {
        if (i) os << ", ";
        os << active_threads[i];
      }
      os << "]\n";
      // Unused threads
      os << "Unused threads (" << unused_threads.size() << "): [";
      for (size_t i = 0; i < unused_threads.size(); ++i) {
        if (i) os << ", ";
        os << unused_threads[i];
      }
      os << "]\n";
      // Pending threads
      os << "Pending threads (" << pending_threads.size() << "): [";
      for (size_t i = 0; i < pending_threads.size(); ++i) {
        if (i) os << ", ";
        os << pending_threads[i];
      }
      os << "]";
    }

    void PrintEventQueue(std::ostream & os=std::cout) const {
      os << "Event queue (" << event_queue.size() << "): [";
      for (size_t i = 0; i < event_queue.size(); ++i) {
        if (i) os << ", ";
        fun_print_event(event_queue[i], os);
      }
      os << "]";
    }

    void PrintExecutionState(const exec_state_t & state, std::ostream & os=std::cout) const {
      fun_print_execution_state(state, os);
    }

  };

}}

#endif