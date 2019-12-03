#ifndef EMP_SIGNALGP_BASE_H
#define EMP_SIGNALGP_BASE_H

#include <iostream>
#include <utility>
#include <limits>
#include <optional>
#include <queue>
#include <tuple>
#include <memory>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/Random.h"

#include "EventLibrary.h"

// @discussion - where should I put configurable lambdas?
// todo - move function implementations outside of class

namespace emp { namespace signalgp {

  /// Placeholder additional component type.
  struct DefaultCustomComponent { };

  /// @brief Base SignalGP class from which all SignalGP implementations should be derived.
  ///
  /// This version of SignalGP makes use of the curiously recursive template pattern (see: https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).
  /// SignalGPBase manages virtual thread execution, manages event queuing and handling, and provides
  /// provides an interface to a custom hardware component (CUSTOM_COMPONENT_T).
  ///
  /// SignalGPBase has the following template parameters:
  ///   * DERIVED_T - Specifies the type of the SignalGP implementation, which must be derived from
  ///     SignalGPBase.
  ///   * EXEC_STATE_T - Specifies the type of the state information required to execute a thread.
  ///   * TAG_T - Specifies the type that is used to search for modules when spawning a new thread.
  ///   * CUSTOM_COMPONENT_T - Optional template parameter. Specifies type of custom hardware component
  ///     to be added on to the SignalGP virtual hardware.
  ///
  /// SignalGP implementations that inherit from SignalGPBase add functionality to SignalGPBase's.
  /// At a high level, while SignalGPBase manages events and threads, derived implementations of SignalGP
  /// flesh out how the virtual hardware should execute (e.g., what does it mean to execute? what state
  /// information is required to specify the state of a thread? et cetera).
  ///
  /// REQUIREMENTS
  ///   * Derived implementations MUST minimally specify the following methods:
  ///     * ResetImpl()
  ///       - Return type: void
  ///       - Reset state information in DERIVED_T virtual hardware.
  ///     * SingleExecutionStep(DERIVED_T & hw, thread_t & thread)
  ///       - Return type: void
  ///       - Execute the thread (with EXEC_STATE_T state information) on the given DERIVED_T hardware
  ///         object.
  ///     * FindModuleMatch(const tag_t & tag, size_t n)
  ///       - Return type: vector<size_t>
  ///       - Find and return up to n module IDs that match with the given TAG_T tag.
  ///     * InitThread(thread_t & thread, size_t module_id)
  ///       - Return type: void
  ///       - Initialize thread_t thread with given module_id.
  ///   * EXEC_STATE_T
  ///     * EXEC_STATE_T::Reset()
  ///       - Return type: void
  ///       - Reset the EXEC_STATE_T execution state.
  template<typename DERIVED_T,
           typename EXEC_STATE_T,
           typename TAG_T,
           typename CUSTOM_COMPONENT_T=DefaultCustomComponent>
  class BaseSignalGP {  // TODO - BaseSignalGP => SignalGPBase
  public:
    // Forward declarations
    struct Thread;

    // Types that base signalgp functionality needs to know about.
    using hardware_t = DERIVED_T;
    using exec_state_t = EXEC_STATE_T;
    using tag_t = TAG_T;
    using custom_comp_t = CUSTOM_COMPONENT_T;

    using event_t = BaseEvent;
    using event_lib_t = EventLibrary<hardware_t>;

    using thread_t = Thread;

    using fun_print_hardware_state_t = std::function<void(const hardware_t&, std::ostream &)>;
    using fun_print_execution_state_t = std::function<void(const exec_state_t &, const hardware_t&, std::ostream &)>;
    using fun_print_event_t = std::function<void(const event_t &, const hardware_t&, std::ostream &)>;

    /// Thread state information.
    struct Thread {
      enum class ThreadState { RUNNING, DEAD, PENDING };
      exec_state_t exec_state;  ///< Internal state information required by DERIVED_T to execute the thread.
      double priority;          ///< Thread priority. Low priority threads are killed if higher priority threads are pending.
      ThreadState run_state;    ///< Is this thread RUNNING, DEAD, or PENDING?

      Thread(const exec_state_t & _exec_state=exec_state_t(), double _priority=1.0)
        : exec_state(_exec_state),
          priority(_priority),
          run_state(ThreadState::DEAD) { ; }

      /// Reset thread to default state (priority = 1, DEAD). Calls EXEC_STATE_T::Reset().
      void Reset() {
        // @discussion - How do we want to handle this?
        exec_state.Reset(); // TODO - make this functionality more flexible! Currently assumes exec_state_t has a Clear function!
        run_state = ThreadState::DEAD;
        priority = 1.0;
      }

      exec_state_t & GetExecState() { return exec_state; }
      const exec_state_t & GetExecState() const { return exec_state; }

      /// Set thread state to DEAD.
      void SetDead() { run_state = ThreadState::DEAD; }

      /// Is this thread dead?
      bool IsDead() const { return run_state == ThreadState::DEAD; }

      /// Set thread state to PENDING.
      void SetPending() { run_state = ThreadState::PENDING; }

      /// Is this thread PENDING?
      bool IsPending() const { return run_state == ThreadState::PENDING; }

      /// Set thread state to RUNNING.
      void SetRunning() { run_state = ThreadState::RUNNING; }

      /// Is this thread RUNNING?
      bool IsRunning() const { return run_state == ThreadState::RUNNING; }

      /// Retrieve this thread's priority level.
      double GetPriority() const { return priority; }

      /// Set thread priority.
      void SetPriority(double p) { priority = p; }
    };

  private:
    struct {
      bool valid=false;
      size_t id=(size_t)-1;
      bool IsValid() const { return valid; }
      void Invalidate() { valid = false; }
      void Validate() { valid = true; }
      size_t & ID() { return id; }
    } cur_thread;                       ///< Should always point to currently executing thread.

    bool is_executing=false;            ///< Is this hardware unit currently executing (within a SingleProcess)?
                                        ///< Note that threads are executed inside SingleProcess.

  protected:
    // -- Event management --
    event_lib_t & event_lib;                           ///< Library of events that hardware can handle.
    std::deque<std::unique_ptr<event_t>> event_queue;  ///< Queue of events to be processed every time step.

    // -- Thread management --
    // WARNING: Derived classes can modify these member variables AT THEIR OWN RISK!
    // Modifying thread management members in derived class may have unintended side effects. Use caution.
    size_t max_active_threads=64;         ///< Maximum number of concurrently running (active) threads.
    size_t max_thread_space=512;          ///< Maximum total active + pending threads.
    bool use_thread_priority=true;        ///< Should SignalGP use thread priority when spawning/killing threads?
    emp::vector<thread_t> threads;        /**< All threads (each could be active/inactive/pending).
                                           *   Initially threads.size = MIN(2*max_active_threads, max_thread_space),
                                           *   but vector will grow as necessary up to max_thread_space.
                                           *   NOTE that we can't track threads by priority because
                                           *   thread priorities can be altered on the fly.
                                           **/
    emp::vector<size_t> thread_exec_order;      ///< Thread execution order (not all guaranteed to be
                                                ///<   in RUNNING state).
    std::unordered_set<size_t> active_threads;  ///< Active thread ids, all currently running.
    emp::vector<size_t> unused_threads;         ///< Pool of unused thread ids.
    std::deque<size_t> pending_threads;         ///< Pending (for consideration to be shifted to ACTIVE)
                                                ///<   thread ids.
    // -- Custom component --
    custom_comp_t custom_component;  /**< Custom hardware component. This is convenient for problem-,
                                          environment-, or experiment-specific hardware components that
                                          shouldn't require an entirely new derived implementation of
                                          SignalGP. SignalGPBase provides a basic interface to custom_component.
                                          Tip: to bundle multiple custom components, just set CUSTOM_COMPONENT_T
                                          equal to a struct { with your bundle of components inside }.
                                      */

    // -- Configurable print functions --
    /// Function to print given hardware state of DERIVED_T to given ostream.
    fun_print_hardware_state_t fun_print_hardware_state = [](const hardware_t& hw, std::ostream & os) { os << "Print hardware state not configured."; };

    /// Function to print given execution state of EXEC_STATE_T to given ostream.
    fun_print_execution_state_t fun_print_execution_state = [](const exec_state_t & e, const hardware_t& hw, std::ostream & os) { os << "Print execution state not configured."; };

    /// Function to print given event to given ostream.
    fun_print_event_t fun_print_event = [](const event_t & e, const hardware_t& hw, std::ostream & os) { e.Print(os); };

    // -- Internally-used thread management functions --
    /// Activate thread:
    /// - (1) Move given thread id to active threads,
    /// - (2) add to thread id to execution order,
    /// - (3) mark thread as RUNNING.
    void ActivateThread(size_t thread_id) {
      emp_assert(thread_id < threads.size(), "Cannot activate invalid thread_id", thread_id);
      emp_assert(!emp::Has(thread_exec_order, thread_id), "Duplicate thread ids in thread_exec_order", thread_id);
      active_threads.emplace(thread_id);
      thread_exec_order.emplace_back(thread_id);
      threads[thread_id].SetRunning();
    }

    /// notice - does not remove from execution order! (SingleProcess will clean that up)
    /// todo - Make public?
    void KillThread(size_t thread_id) {
      emp_assert(thread_id < threads.size());
      // assert thread_id is in active_threads, not in pending
      active_threads.erase(thread_id);
      threads[thread_id].SetDead();
      unused_threads.emplace_back(thread_id);
    }

    // @discussion - move to public? But protect from being called if executing?
    void ActivatePendingThreads() {
      // NOTE: Assumes active threads is accurate!
      // NOTE: all pending threads + active threads should have unique ids

      // Are there pending threads to activate?
      if (pending_threads.empty()) return;

      // Spawn pending threads (in order of arrival) until no more room.
      while (pending_threads.size() && (active_threads.size() < max_active_threads)) {
        const size_t thread_id = pending_threads.front();
        emp_assert(thread_id < threads.size());
        emp_assert(threads[thread_id].IsPending());
        // threads[thread_id].SetRunning();
        // active_threads.emplace(pending_threads.front());
        ActivateThread(thread_id);
        pending_threads.pop_front();
      }

      // Have we finished handling pending threads?
      if (pending_threads.empty()) return;

      // If we're here, we hit max thread capacity. We'll need to kill active
      // threads to make room for pending threads.
      if (use_thread_priority) { // TODO - implement this!

      } else {

      }

      // Find max pending priority, use it to bound which active threads we consider
      // killing.
      double max_pending_priority = threads[pending_threads.front()].GetPriority();
      for (size_t thread_id : pending_threads) {
        emp_assert(thread_id < threads.size());
        emp_assert(threads[thread_id].IsPending());
        const double priority = threads[thread_id].GetPriority();
        if (priority > max_pending_priority) max_pending_priority = priority;
      }

      // Create a MIN heap of the active threads (only include threads with priority < MAX_PENDING_PRIORITY)
      // - todo => make a member variable to avoid re-allocation every time?
      std::priority_queue<std::tuple<double, size_t>,
                          std::vector<std::tuple<double, size_t>>,
                          std::greater<std::tuple<double, size_t>>> active_priorities;
      for (size_t active_id : active_threads) {
        emp_assert(active_id < threads.size());
        thread_t & thread = threads[active_id];
        if (thread.GetPriority() < max_pending_priority) {
          active_priorities.emplace(std::make_tuple(thread.GetPriority(), active_id));
        }
      }

      // Reminder: we're considering active threads in order of least priority &
      //           pending threads in order of arrival.
      while(active_priorities.size() && pending_threads.size()) {
        const size_t pending_id = pending_threads.front();
        const size_t active_id = std::get<1>(active_priorities.top());
        if (threads[pending_id].GetPriority() > threads[active_id].GetPriority()) {
          active_priorities.pop();      // Remove active priority from heap.
          // Kill active thread.
          KillThread(active_id);
          // Activate pending thread.
          ActivateThread(pending_id);
          pending_threads.pop_front(); // Pop pending thread.
        } else {
          // This pending thread won't replace any active threads (not high enough priority).
          pending_threads.pop_front();
          threads[pending_id].SetDead(); // no longer pending
          unused_threads.emplace_back(pending_id); // Reclaim pending_id for future use
        }
      }

      // Any leftover pending threads do not have sufficient priority to kill
      // an active thread.
      while (pending_threads.size()) {
        const size_t pending_id = pending_threads.front();
        threads[pending_id].SetDead();  // no longer pending
        pending_threads.pop_front();
        unused_threads.emplace_back(pending_id); // Reclaim pending id for future use.
      }
    }

  public:
    // todo - why are we initializing thread this way?
    // BaseSignalGP(Ptr<event_lib_t> elib) // @discussion - okay practice to rely on default constructors for other member variables?
    BaseSignalGP(event_lib_t & elib) // @discussion - okay practice to rely on default constructors for other member variables?
      : event_lib(elib),
        threads( (2*max_active_threads < max_thread_space) ? 2*max_active_threads : max_thread_space ),
        unused_threads( threads.size() )
    {
      // Set all threads to unused.
      for (size_t i = 0; i < unused_threads.size(); ++i) {
        unused_threads[i] = (unused_threads.size() - 1) - i;
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
    virtual ~BaseSignalGP() {};

    /// Full virtual hardware reset:
    /// Required
    /// - NOTE: derived class should call BaseResetState
    virtual void Reset() = 0;

    /// Required
    virtual void SingleExecutionStep(DERIVED_T &, thread_t &) = 0;

    /// Required - @discussion is vector<size_t> really the return type we want here?
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
      thread_exec_order.clear(); // No threads to execute.
      active_threads.clear();    // No active threads.
      pending_threads.clear();   // No pending threads.
      unused_threads.resize(threads.size());
      // unused_threads.resize(max); TODO - fix
      // Add all available threads to unused.
      for (size_t i = 0; i < unused_threads.size(); ++i) {
        unused_threads[i] = (unused_threads.size() - 1) - i;
      }
      cur_thread.Invalidate();
      cur_thread.id = max_thread_space;
      // cur_thread_id = (size_t)-1;
      is_executing = false;
    }

    /// Get event library associated with hardware.
    event_lib_t & GetEventLib() { return event_lib; }
    const event_lib_t & GetEventLib() const { return event_lib; }

    /// Get reference to this hardware's execution stepper object.
    DERIVED_T & GetHardware() { return static_cast<DERIVED_T&>(*this); }
    const DERIVED_T & GetHardware() const { return static_cast<const DERIVED_T&>(*this); }

    /// Access hardware custom component.
    custom_comp_t & GetCustomComponent() { return custom_component; }
    const custom_comp_t & GetCustomComponent() const { return custom_component; }
    void SetCustomComponent(const custom_comp_t & val) { custom_component = val; }

    /// Get the maximum number of threads allowed to run simultaneously on this hardware
    /// object.
    size_t GetMaxActiveThreads() const { return max_active_threads; }

    /// Get maximum number of active + pending threads allowed to exist simultaneously
    /// on this hardware object.
    size_t GetMaxThreadSpace() const { return max_thread_space; }

    /// Get the number of currently running threads.
    size_t GetNumActiveThreads() const { return active_threads.size(); }

    /// Get number of threads being considered for activation.
    size_t GetNumPendingThreads() const { return pending_threads.size(); }

    /// Get number of unused threads. May be larger than max number of active threads.
    size_t GetNumUnusedThreads() const { return unused_threads.size(); }

    /// Get a reference to active threads.
    /// NOTE: use responsibly! No safety gloves here!
    /// TODO - emp_assert, emp_warn?
    emp::vector<thread_t> & GetThreads() { return threads; }

    /// Get a reference to a particular thread.
    thread_t & GetThread(size_t i) { emp_assert(i < threads.size()); return threads[i]; }
    const thread_t & GetThread(size_t i) const { emp_assert(i < threads.size()); return threads[i]; }

    /// Get const reference to vector of currently active threads active.
    const std::unordered_set<size_t> & GetActiveThreadIDs() const { return active_threads; }

    /// Get const reference to threads that are not currently active.
    const emp::vector<size_t> & GetUnusedThreadIDs() const { return unused_threads; }

    /// Get const reference to thread ids of pending threads.
    const std::deque<size_t> & GetPendingThreadIDs() const { return pending_threads; }

    /// Get const reference to thread execution order. Note, not all threads in exec
    /// order list guaranteed to be active.
    const emp::vector<size_t> & GetThreadExecOrder() const { return thread_exec_order; }

    /// Get the ID of the currently executing thread. If hardware is not in midst
    /// of an execution cycle, this will return (size_t)-1.
    /// Instructions are executed in SingleProcess
    size_t GetCurThreadID() {
      emp_assert(is_executing);
      emp_assert(cur_thread.IsValid(), "There is no currently executing thread.");
      emp_assert(cur_thread.ID() < threads.size(), "Current thread ID is invalid.");
      // emp_assert(cur_thread_id < threads.size());
      return cur_thread.ID();
    }

    /// Get the currently executing thread. Only valid to call this while virtual
    /// hardware is executing. Otherwise, will error out.
    /// Instructions are executed in SingleProcess
    thread_t & GetCurThread() {
      emp_assert(is_executing, "Hardware is not executing! No current thread.");
      emp_assert(cur_thread.IsValid(), "There is no currently executing thread.");
      emp_assert(cur_thread.ID() < threads.size(), "Current thread ID is invalid.");
      // emp_assert(cur_thread_id < threads.size());
      return threads[cur_thread.ID()];
    }

    /// Are we inside of a 'SingleProcess'. Note, instructoins are executed in SingleProcess.
    bool IsExecuting() const { return is_executing; }

    /// TODO - TEST
    // Warning: If you decrease max threads, you may kill actively running threads.
    // Slow operation.
    // TODO - fix set thread limit function
    void SetActiveThreadLimit(size_t n) {
      emp_assert(n, "Max thread count must be greater than 0.");
      emp_assert(!is_executing, "Cannot adjust SignalGP hardware max thread count while executing.");
      // NOTE - this cannot DECREASE the capacity of the 'threads' member variable.
      //        It can, however, INCREASE the capacity of the 'threads' member variable.
      // Adjust max_thread_space if necessary.

      // TODO
      // temp = vector(active.begin(), active.end())
      // partial sort(temp, [](){get priority})

      max_thread_space = std::max(n, max_thread_space); //(n > max_thread_space) ? n : max_thread_space;
      if (n > max_active_threads) {
        // Increasing total threads able to run simultaneously. This might increase
        // thread storage (threads).
        for (size_t i = threads.size(); i < n; ++i) {
          // add any new thread ids to unused threads.
          unused_threads.emplace_back(i);
        }
        // If requesting more possible active threads than space, resize.
        if (n > threads.size()) threads.resize(n);
      } else if (n < active_threads.size()) {
        // new thread limit is lower than current number of active threads.
        emp_assert(thread_exec_order.size() >= active_threads.size());
        // need to kill active threads to abide by new active thread limit.
        size_t num_kill = active_threads.size() - n;
        while (num_kill) {
          const size_t thread_id = thread_exec_order.back();
          if (threads[thread_id].IsRunning()) {
            KillThread(thread_id);
            --num_kill;
          }
          // Either thread was running and we killed it or thread was already dead.
          // - either way, we needed to remove the thread from exec order.
          thread_exec_order.pop_back();
        }
      }
      max_active_threads = n;
    }

    /// Maximum allowed number of pending + active threads.
    // todo - test!
    void SetThreadCapacity(size_t n) {
      emp_assert(n, "Max thread count must be greater than 0.");
      emp_assert(!is_executing, "Cannot adjust SignalGP hardware max thread count while executing.");
      // This function can both decrease AND increase the size of the threads
      // vector.

      // If new thread cap < max active threads, decrease max active threads to new cap.
      if (n < max_active_threads) {
        SetActiveThreadLimit(n);
      }

      // If new thread cap < current thread storage, decrease thread storage, and
      // update thread tracking.
      if (n < threads.size()) {
        // Lazily update
        emp::vector<size_t> new_thread_exec_order;
        emp::vector<size_t> new_unused_threads;
        std::deque<size_t> new_pending_threads;
        for (size_t id : thread_exec_order) {
          if (id < n && !threads[id].IsDead()) new_thread_exec_order.emplace_back(id);
        }
        for (size_t id : unused_threads) {
          if (id < n) new_unused_threads.emplace_back(id);
        }
        for (size_t id : new_pending_threads) {
          if (id < n) new_pending_threads.emplace_back(id);
        }
        for (size_t id : active_threads) {
          if (id >= n) active_threads.erase(n);
        }
        thread_exec_order = new_thread_exec_order;
        unused_threads = new_unused_threads;
        pending_threads = new_pending_threads;
        threads.resize(n); // Decrease thread storage.
      }

      max_thread_space = n;
    }

    /// @discussion - needs better name
    // - Mark all pending threads as dead
    void RemoveAllPendingThreads() {
      while (pending_threads.size()) {
        const size_t thread_id = pending_threads.back();
        pending_threads.pop_back();
        // threads[thread_id].SetDead();
        threads[thread_id].Reset(); // this should be safe - todo - think on it
        unused_threads.emplace_back(thread_id);
      }
    }

    /// Spawn a number of threads (<= n). Use tag to select which modules to call.
    /// Return a vector of spawned thread IDs.
    ///  - If hardware is executing, these threads will be marked as pending.
    ///  - If hardware is not executing, each requested new thread will:
    ///    - Be initialized if # active threads < max_threads
    ///    - Be initialized if # active threads == max_threads && priority level
    ///      is greater than lowest priority level of active threads.
    ///    - Not be initialized if # active threads == max-threads && priority level
    ///      is less than lowest priority level of active threads.
    emp::vector<size_t> SpawnThreads(const tag_t & tag, size_t n, double priority=1.0) {
      emp::vector<size_t> matches(GetHardware().FindModuleMatch(tag, n));
      emp::vector<size_t> thread_ids;
      for (size_t match : matches) {
        const auto thread_id = SpawnThreadWithID(match, priority);
        if (thread_id) {
          thread_ids.emplace_back(thread_id.value());
        }
      }
      return thread_ids;
    }

    std::optional<size_t> SpawnThreadWithTag(const tag_t & tag, double priority=1.0) {
      emp::vector<size_t> match(GetHardware().FindModuleMatch(tag, 1));
      return (match.size()) ? SpawnThreadWithID(match[0], priority) : std::nullopt;
    }

    /// Spawn a new thread with given ID.
    /// If no unused threads & already maxed out thread space, will not spawn new
    /// thread.
    /// Otherwise, mark thread as pending.
    std::optional<size_t> SpawnThreadWithID(size_t module_id, double priority=1.0, bool priority_override=true) {
      size_t thread_id;
      // Is there an unused thread to commandeer?
      if (unused_threads.size()) {
        // Unused thread is available, use it.
        thread_id = unused_threads.back();
        unused_threads.pop_back();
      } else if (threads.size() < max_thread_space) {
        // No unused threads available, but we have space to make a new one.
        thread_id = threads.size();
        threads.emplace_back();
      } else {
        // No unused threads available, and no more space to make a new one.
        return std::nullopt;
      }
      // If we make it here, we have a valid thread_id to use.
      emp_assert(thread_id < threads.size());

      // We've identified a thread to commandeer. Reset it, initialize it, and
      // mark it appropriately.
      thread_t & thread = threads[thread_id];
      thread.Reset();
      thread.SetPriority(priority);

      // Let derived hardware initialize thread w/appropriate module.
      GetHardware().InitThread(thread, module_id);
      // emp_assert()

      // Mark thread as pending.
      thread.SetPending();
      pending_threads.emplace_back(thread_id);

      return std::optional<size_t>{thread_id}; // this could mess with thread priority level!
    }

    /// Handle an event (on this hardware) now!.
    void HandleEvent(const event_t & event) { event_lib.HandleEvent(GetHardware(), event); }

    /// Trigger an event (from this hardware).
    void TriggerEvent(const event_t & event) { event_lib.TriggerEvent(GetHardware(), event); }

    /// Queue an event (to be handled by this hardware) next time this hardware
    /// unit is executed.
    template<typename EVENT_T>
    void QueueEvent(const EVENT_T & event) {
      event_queue.emplace_back(std::make_unique<EVENT_T>(event));
    }

    /// Advance the hardware by a single step.
    void SingleProcess() {
      // Handle events (which may spawn threads)
      while (!event_queue.empty()) {
        HandleEvent(*(event_queue.front()));
        event_queue.pop_front();
      }

      // Activate all pending threads. (which may kill currently active threads)
      ActivatePendingThreads();
      emp_assert(active_threads.size() <= max_active_threads);

      // Begin execution!
      is_executing = true;
      cur_thread.Validate();    // cur_thread is valid during execution.
      size_t exec_order_id = 0;
      size_t thread_exec_cnt = thread_exec_order.size();
      size_t adjust = 0;
      while (exec_order_id < thread_exec_cnt) {
        emp_assert(exec_order_id < threads.size()); // Exec order ID should always be valid thread.
        // cur_thread_id = thread_exec_order[exec_order_id];
        cur_thread.id = thread_exec_order[exec_order_id];

        // Do we need to move the current thread id over in the execution ordering
        // to make our execution order contiguous?
        if (adjust) {
          // If we need to adjust, invalidate current position and move current
          // thread id up by 'adjust'.
          thread_exec_order[exec_order_id] = max_thread_space; // Invalid position!
          thread_exec_order[exec_order_id - adjust] = cur_thread.ID();
        }

        // Is this thread dead?
        if (threads[cur_thread.ID()].IsDead()) {
          // If this thread is active, kill it.
          if (emp::Has(active_threads, cur_thread.ID())) KillThread(cur_thread.ID());
          ++adjust;
          ++exec_order_id;
          continue;
        }

        // Execute the thread (defined by derived class)
        GetHardware().SingleExecutionStep(GetHardware(), threads[cur_thread.ID()]);

        // Did the thread die?
        if (threads[cur_thread.ID()].IsDead()) {
          KillThread(cur_thread.ID());
          ++adjust;
        }
        ++exec_order_id;
      }
      is_executing = false;

      // Update the thread execution ordering to be accurate.
      thread_exec_order.resize(thread_exec_cnt - adjust);
      emp_assert(thread_exec_order.size() == active_threads.size()); // discussion - right?

      // Invalidate the current thread id.
      cur_thread.id = max_thread_space;
      cur_thread.Invalidate();
    }

    /// Advance hardware by some arbitrary number of steps.
    void Process(size_t num_steps) {
      for (size_t i = 0; i < num_steps; ++i) {
        SingleProcess();
      }
    }

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
      for (size_t thread_id : active_threads) {
        const thread_t & thread = threads[thread_id];
        os << "Thread ID = " << thread_id << "):\n";
        PrintExecutionState(thread.GetExecState(), GetHardware(), os);
        os << "\n";
      }
    }

    /// Print overall state of hardware.
    void PrintHardwareState(std::ostream & os=std::cout) const { fun_print_hardware_state(GetHardware(), os); }

    /// Print thread usage status (active, unused, and pending thread ids).
    void PrintThreadUsage(std::ostream & os=std::cout) const {
      // All threads (and state)
      os << "All allocated (" << threads.size() << "); [";
      for (size_t i = 0; i < threads.size(); ++i) {
        if (i) os << ", ";
        char state;
        if (threads[i].IsDead()) state = 'D';
        else if (threads[i].IsRunning()) state = 'A';
        else if (threads[i].IsPending()) state = 'P';
        else state = '?';
        os << i << " (" << state << ":" << threads[i].GetPriority() << ")";
      }
      os << "]\n";
      // Active threads
      os << "Active threads (" << active_threads.size() << "): [";
      bool comma = false;
      for (size_t id : active_threads) {
        if (comma) os << ", ";
        else comma = true;
        os << id;
      }
      os << "]\n";
      // Unused Threads
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
      os << "]\n";
      // Execution order
      os << "Execution order (" << thread_exec_order.size() << "): [";
      for (size_t i = 0; i < thread_exec_order.size(); ++i) {
        if (i) os << ", ";
        size_t thread_id = thread_exec_order[i];
        char state;
        if (threads[thread_id].IsDead()) state = 'D';
        else if (threads[thread_id].IsRunning()) state = 'A';
        else if (threads[thread_id].IsPending()) state = 'P';
        else state = '?';
        os << thread_id << " (" << state << ")";
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

    /// Does the current state of thread management make sense?
    /// Primarily used for testing.
    bool ValidateThreadState() {
      emp_assert(!is_executing);
      // std::cout << "--VALIDATE THREAD STATE--" << std::endl;
      // (1) Thread storage should not exceed max_thread_capacity
      if (threads.size() > max_thread_space) return false;
      // std::cout << "(1) OKAY" << std::endl;
      // (2) # of active threads should not exceed max_active_threads
      if (active_threads.size() > max_active_threads) return false;
      // std::cout << "(2) OKAY" << std::endl;
      // (3) No thread ID should appear more than once in the execution order.
      std::unordered_set<size_t> exec_order_set(thread_exec_order.begin(), thread_exec_order.end());
      if (exec_order_set.size() != thread_exec_order.size()) return false;
      // std::cout << "(3) OKAY" << std::endl;
      // (4) No thread ID should appear more than once in the unused threads tracker.
      std::unordered_set<size_t> unused_set(unused_threads.begin(), unused_threads.end());
      if (unused_set.size() != unused_threads.size()) return false;
      // std::cout << "(4) OKAY" << std::endl;
      // (5) No thread ID should appear more than once in the pending threads tracker.
      std::unordered_set<size_t> pending_set(pending_threads.begin(), pending_threads.end());
      if (pending_set.size() != pending_threads.size()) return false;
      // std::cout << "(5) OKAY" << std::endl;
      // (6) No thread ID should appear more than once across ACTIVE, UNUSED, & PENDING threads.
      //     - Also, all thread IDs should be valid (id < threads.size())!
      emp::vector<size_t> id_appearances(threads.size(), 0);
      for (size_t id : active_threads) {
        if (id >= threads.size()) return false;
        id_appearances[id] += 1;
      }
      for (size_t id : unused_threads) {
        if (id >= threads.size()) return false;
        id_appearances[id] += 1;
      }
      for (size_t id : pending_threads) {
        if (id >= threads.size()) return false;
        id_appearances[id] += 1;
      }
      for (size_t id = 0; id < id_appearances.size(); ++id) {
        if (id_appearances[id] != 1) return false;
      }
      // std::cout << "(6) OKAY" << std::endl;
      // (7) Every thread in active threads should NOT be marked as pending (either dead or active OKAY)
      for (size_t id : active_threads) {
        if (threads[id].IsPending()) return false;
      }
      // std::cout << "(7) OKAY" << std::endl;
      // If all of that passed, return true (i.e., thread management is valid).
      return true;
    }

  };

}}

#endif