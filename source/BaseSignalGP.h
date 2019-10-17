/*******************************************************************************
 * BaseSignalGP<EXEC_STEPPER_T, CUSTOM_COMPONENT_T>
 * ..
 * EXEC_STEPPER_T - Execution Stepper
 *   - The execution stepper knows how to execute programs. What type of programs?
 *     That's entirely up to the particular implementation of the execution stepper.
 *     So long as the execution stepper provides an appropriate interface, SignalGP
 *     does not care about the particulars.
 *   - Execution stepper interface requirements:
 *     - Required types:
 *       - exec_state_t => execution state information
 *       - program_t    => what type of program does the execution stepper run?
 *       - tag_t        => what type of tag does the execution stepper use to reference modules?
 *       - ?module_t?
 *     - Required function signatures:
 *       - EXEC_STEPPER_T(const EXEC_STEPPER_T&)
 *         - Copy constructor.
 *       - program_t & GetProgram()
 *         - Returns the program currently loaded on the execution stepper.
 *       - void SetProgram(const program_t&)
 *         - Loads a new program on the execution stepper. Handles all appropriate
 *           cleanup and internal state resetting necessary when switching to running
 *           a new program.
 *       - void ResetProgram()
 *         - Clear out the old program (if any). Reset internal state as appropriate.
 *       - void ResetHardwareState()
 *         - Reset internal state of execution stepper without resetting the program.
 *       - vector<size_t> FindModuleMatch(const tag_t&, size_t N)
 *         - Return a vector (max size N) of module IDs that match with specified
 *           tag.
 *       - void InitThread(Thread &, size_t module_id)
 *         - Initialize given thread by calling the module specified by module_id.
 *       - void SingleExecutionStep(SignalGP<EXEC_STEPPER_T, CUSTOM_COMPONENT_T> &, exec_state_t&)
 *         - Advance a single execution stepper on the given execution state using
 *           the given SignalGP hardware state.
 *******************************************************************************/

#ifndef EMP_SIGNALGP_BASE_H
#define EMP_SIGNALGP_BASE_H

#include <iostream>
#include <utility>
#include <limits>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/Random.h"

#include "EventLibrary.h"
#include "InstructionLibrary.h"

// @discussion - how could I use concepts to clean this up?
// @discussion - where should I put configurable lambdas?

namespace emp { namespace signalgp {

  struct BaseEvent {
    size_t id;

    size_t GetID() const { return id; }

    void Print(std::ostream & os) const {
      os << "{id:" << GetID() << "}";
    }
  };

  /// Placeholder additional component type.
  struct DefaultCustomComponent { };

  // todo - move function implementations outside of class
  // todo - make signalgp hardware not awful (& safe) to make copies of
  // @discussion - template/organization structure
  // What about program_t?
  template<typename DERIVED_T,
           typename EXEC_STATE_T,
           typename TAG_T,
           typename CUSTOM_COMPONENT_T=DefaultCustomComponent>  // @DISCUSSION - additional component here versus in derived? arg for: signaling to derived devs?
  class BaseSignalGP {
  public:
    struct Thread;

    /// Theoretical thread limit for hardware.
    /// Some function use max size_t to indicate no threads... TODO - internal thread_id struct
    static constexpr size_t THREAD_LIMIT = std::numeric_limits<size_t>::max() - 1;

    // Types that base signalgp functionality needs to know about.
    using hardware_t = DERIVED_T;
    using exec_state_t = EXEC_STATE_T;
    // using program_t = PROGRAM_T; @discussion - ??
    using tag_t = TAG_T;
    using custom_comp_t = CUSTOM_COMPONENT_T;

    using event_t = BaseEvent;
    using event_lib_t = EventLibrary<hardware_t>;

    using thread_t = Thread;

    using fun_print_hardware_state_t = std::function<void(const hardware_t&, std::ostream &)>;
    // using fun_print_program_t = std::function<void(const program_t&, const hardware_t&, std::ostream &)>;
    using fun_print_execution_state_t = std::function<void(const exec_state_t &, const hardware_t&, std::ostream &)>;
    using fun_print_event_t = std::function<void(const event_t &, const hardware_t&, std::ostream &)>;

    // QUESTION - Pros/cons of nesting Thread type in SignalGP class?
    struct Thread {
      // comment => labels can exist inside execution state.
      exec_state_t exec_state;
      bool dead;
      double priority;

      Thread(const exec_state_t & _exec_state=exec_state_t())
        : exec_state(_exec_state), dead(true) { ; }

      void Reset() {
        // @discussion - How do we want to handle this?
        exec_state.Clear(); // TODO - make this functionality more flexible! Currently assumes exec_state_t has a Clear function!
        dead = true;
      }

      exec_state_t & GetExecState() { return exec_state; }
      const exec_state_t & GetExecState() const { return exec_state; }

      void SetDead(bool val=true) { dead = val; }
      bool IsDead() const { return dead; }
    };

  protected:
    Ptr<event_lib_t> event_lib;         ///< These are the events this hardware knows about.
    std::deque<event_t> event_queue;    ///< Queue of events to be processed every time step.

    // Ptr<Random> random_ptr;             ///< Random number generator. (TODO - make this a smart pointer)
    // bool random_owner;                  ///< Is this hardware unit responsible for cleaning up the random number generator memory?

    // Thread management
    size_t max_threads=64;              ///< Maximum number of concurrently running threads.
    emp::vector<thread_t> threads;      ///< All threads (each could be active/inactive/pending).
    emp::vector<size_t> active_threads; ///< Active thread ids.
    emp::vector<size_t> unused_threads; ///< Unused thread ids.
    std::deque<size_t> pending_threads; ///< Pending thread ids.

    size_t cur_thread_id=(size_t)-1;    ///< Currently executing thread.
    bool is_executing=false;            ///< Is this hardware unit currently executing (within a SingleProcess)?

    custom_comp_t custom_component;

    // Configurable print functions. @NOTE: should these emp_assert(false)?
    fun_print_hardware_state_t fun_print_hardware_state = [](const hardware_t& hw, std::ostream & os) { return; };
    // fun_print_program_t fun_print_program = [](const program_t& p, const hardware_t& hw, std::ostream & os) { return; };
    fun_print_execution_state_t fun_print_execution_state = [](const exec_state_t & e, const hardware_t& hw, std::ostream & os) { return; };
    fun_print_event_t fun_print_event = [](const event_t & e, const hardware_t& hw, std::ostream & os) { e.Print(os); };

  public:
    BaseSignalGP(Ptr<event_lib_t> elib)
      : event_lib(elib),
        event_queue(),
        threads(max_threads),
        active_threads(),
        unused_threads(max_threads),
        pending_threads()
    {
      std::cout << "Base constructor" << std::endl;
      // If no random provided, create one.
      // if (!rnd) NewRandom();
      // Set all threads to unused.
      for (int i = unused_threads.size() - 1; i > 0; --i) {
        unused_threads[i] = i;
      }
    }

    // Todo - test!
    /// Move constructor.
    BaseSignalGP(BaseSignalGP && in) = default;

    /// todo - test!
    /// Copy constructor.
    BaseSignalGP(const BaseSignalGP & in) = default;

    /// Destructor.
    // todo - test!
    ~BaseSignalGP() { }

    /// Full virtual hardware reset:
    /// Required
    virtual void Reset() = 0;

    /// Required
    virtual void SingleExecutionStep(DERIVED_T &, thread_t &) = 0;

    /// Required
    virtual vector<size_t> FindModuleMatch(const tag_t &, size_t) = 0;

    /// Required
    virtual void InitThread(thread_t &, size_t) = 0;

    /// HardwareState reset:
    /// - Reset execution stepper hardware state.
    /// - Clear event queue.
    /// - Reset all threads, move all to unused; clear pending.
    void BaseResetState() {
      emp_assert(!is_executing, "Cannot reset hardware while executing.");
      event_queue.clear();
      for (auto & thread : threads) {
        thread.Reset();
      }
      active_threads.clear();
      pending_threads.clear();
      unused_threads.resize(max_threads);
      // Add all available threads to unused.
      for (size_t i = 0; i < unused_threads.size(); ++i) {
        unused_threads[i] = (unused_threads.size() - 1) - i;
      }
      cur_thread_id = (size_t)-1;
      is_executing = false;
    }

    /// Get event library associated with hardware.
    Ptr<const event_lib_t> GetEventLib() const { return event_lib; }

    /// Get reference to random number generator used by this hardware.
    // Random & GetRandom() { return *random_ptr; }

    /// Get pointer to random number generator used by this hardware.
    // Ptr<Random> GetRandomPtr() { return random_ptr; }

    /// Get reference to this hardware's execution stepper object.
    DERIVED_T & GetHardware() { return static_cast<DERIVED_T>(*this); }
    const DERIVED_T & GetHardware() const { return static_cast<DERIVED_T>(*this); }

    custom_comp_t & GetCustomComponent() { return custom_component; }
    const custom_comp_t & GetCustomComponent() const { return custom_component; }
    void SetCustomComponent(const custom_comp_t & val) { custom_component = val; }

    /// Get the maximum number of threads allowed to run simultaneously on this hardware
    /// object.
    size_t GetMaxThreads() const { return max_threads; }

    /// Get the number of currently running threads.
    size_t GetNumActiveThreads() const { return active_threads.size(); }

    /// Get a reference to active threads.
    /// NOTE: use responsibly! No safety gloves here!
    emp::vector<thread_t> & GetThreads() { return threads; }

    /// Get a reference to a particular thread.
    thread_t & GetThread(size_t i) { emp_assert(i < threads.size()); return threads[i]; }

    /// Get const reference to vector of currently active threads active.
    const emp::vector<size_t> & GetActiveThreadIDs() const { return active_threads; }

    /// Get const reference to threads that are not currently active.
    const emp::vector<size_t> & GetUnusedThreadIDs() const { return unused_threads; }

    /// Get const reference to thread ids of pending threads.
    const std::deque<size_t> & GetPendingThreadIDs() const { return pending_threads; }

    /// Get the ID of the currently executing thread. If hardware is not in midst
    /// of an execution cycle, this will return (size_t)-1.
    size_t GetCurThreadID() { return cur_thread_id; }

    /// Get the currently executing thread. Only valid to call this while virtual
    /// hardware is executing. Otherwise, will error out.
    thread_t & GetCurThread() {
      emp_assert(is_executing, "Hardware is not executing! No current thread.");
      emp_assert(cur_thread_id < threads.size());
      return threads[cur_thread_id];
    }

    /// TODO - TEST => PULLED FROM ORIGINAL SIGNALGP
    // Warning: If you decrease max threads, you may kill actively running threads.
    // Warning: If you decrease max threads
    void SetThreadLimit(size_t n) {
      emp_assert(n, "Max thread count must be greater than 0.");
      emp_assert(n <= THREAD_LIMIT, "Max thread count must be less than or equal to", THREAD_LIMIT);
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

    /// Create a new random number generator with the given seed.
    /// If already own number generator, delete old and create new.
    /// If this does not own number generator, make new and mark ownership.
    // void NewRandom(int seed=-1) {
    //   if (random_owner) random_ptr.Delete();
    //   else random_ptr = nullptr;
    //   random_ptr.New(seed);
    //   random_owner = true;
    // }

    /// Spawn number (<= n) of threads, using tag to select which modules.
    /// return vector of spawned thread ids
    emp::vector<size_t> SpawnThreads(const tag_t & tag, size_t n) {
      emp::vector<size_t> matches(GetHardware().FindModuleMatch(tag, n));
      emp::vector<size_t> thread_ids;
      for (size_t match : matches) {
        const size_t thread_id = SpawnThread(match);
        if (thread_id >= threads.size()) break;
        thread_ids.emplace_back(thread_id);
      }
      return thread_ids;
    }

    /// Spawn a single thread with a particular module (specified by id).
    /// Returns thread_id of spawned thread (-1 if none spawned).
    size_t SpawnThread(size_t module_id) {
      if (!unused_threads.size()) return (size_t)-1; // If no unused threads, return.
      // Which thread should we spawn?
      size_t thread_id = unused_threads.back();
      unused_threads.pop_back();

      // Reset thread.
      thread_t & thread = threads[thread_id];
      thread.Reset();

      // Initialize thread state.
      GetHardware().InitThread(thread, module_id); // TODO - this is somewhat gross; requires yet another function of the exec_stepper.
      thread.SetDead(false);

      // Mark thread as pending if hardware is currently executing. Otherwise,
      // mark it is active.
      if (is_executing) { pending_threads.emplace_back(thread_id); }
      else { active_threads.emplace_back(thread_id); }

      return thread_id;
    }


    /// Handle an event (on this hardware) now!.
    void HandleEvent(const event_t & event) { event_lib->HandleEvent(GetHardware(), event); }

    /// Trigger an event (from this hardware).
    void TriggerEvent(const event_t & event) { event_lib->TriggerEvent(GetHardware(), event); }

    /// Queue an event (to be handled by this hardware) next time this hardware
    /// unit is executed.
    void QueueEvent(const event_t & event) { event_queue.emplace_back(event); }

    /// Advance the hardware by a single step.
    void SingleProcess() {
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
        GetHardware().SingleExecutionStep(GetHardware(), threads[cur_thread_id]); //@discussion should just pass the entire thread?

        // Is this thread dead?
        if (threads[cur_thread_id].IsDead()) {
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

    /// How should programs be printed?
    // void SetPrintProgramFun(const fun_print_program_t & print_fun) {
    //   fun_print_program = print_fun;
    // }

    /// How does the hardware state get printed?
    void SetPrintHardwareStateFun(const fun_print_hardware_state_t & print_fun) {
      fun_print_hardware_state = print_fun;
    }

    /// How does a single execution state get printed?
    void SetPrintExecutionStateFun(const fun_print_execution_state_t & print_fun) {
      fun_print_execution_state = print_fun;
    }

    /// How do we print a single event?
    void SetPrintEventFun(const fun_print_event_t & print_fun) {
      fun_print_event = print_fun;
    }

    /// Print active threads.
    void PrintActiveThreadStates(std::ostream & os=std::cout) const {
      for (size_t i = 0; i < active_threads.size(); ++i) {
        size_t thread_id = active_threads[i];
        const thread_t & thread = threads[thread_id];
        os << "Thread " << i << " (ID="<< thread_id << "):\n";
        PrintExecutionState(thread.GetExecState(), GetHardware(), os);
        os << "\n";
      }
    }

    /// Print loaded program.
    // void PrintProgram(std::ostream & os=std::cout) const { fun_print_program(os); }

    /// Print overall state of hardware.
    void PrintHardwareState(std::ostream & os=std::cout) const { fun_print_hardware_state(GetHardware(), os); }

    /// Print thread usage status (active, unused, and pending thread ids).
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

    /// Print everything in the event queue.
    void PrintEventQueue(std::ostream & os=std::cout) const {
      os << "Event queue (" << event_queue.size() << "): [";
      for (size_t i = 0; i < event_queue.size(); ++i) {
        if (i) os << ", ";
        fun_print_event(event_queue[i], GetHardware(), os);
      }
      os << "]";
    }

  };

}}

#endif