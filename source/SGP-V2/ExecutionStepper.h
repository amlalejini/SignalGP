#ifndef EMP_SIGNALGP_V2_EXECSTEPPER_H
#define EMP_SIGNALGP_V2_EXECSTEPPER_H

#include <iostream>
#include <utility>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/Random.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"

#include "../EventLibrary.h"
#include "../InstructionLibrary.h"

#include "SignalGP.h"
#include "LinearProgram.h"

namespace emp { namespace sgp_v2 {

  // Each program type needs their own 'ExecutionStepper' to manage execution
  // - knows about program structure
  // - knows how to make programs
  // - knows how to execute programs
  // TODO - turn everything into configurable lambdas?
  template<typename MEMORY_MODEL_T,
           typename TAG_T=emp::BitSet<16>,
           typename INST_ARGUMENT_T=int,
           typename MATCHBIN_T=emp::MatchBin< size_t, emp::HammingMetric<16>, emp::RankedSelector<std::ratio<16+8, 16> >>
           >
  class SimpleExecutionStepper {
  public:
    struct ExecState;
    struct Module;
    struct FlowInfo;
    struct CallState;

    // Blocks are within-module flow control segments (e.g., while loops, if statements, etc)
    enum class InstProperty { MODULE, BLOCK_CLOSE, BLOCK_DEF };

    using exec_stepper_t = SimpleExecutionStepper<MEMORY_MODEL_T, TAG_T, INST_ARGUMENT_T, MATCHBIN_T>;
    using exec_state_t = ExecState;

    using tag_t = TAG_T;
    using arg_t = INST_ARGUMENT_T;
    using module_t = Module;
    using matchbin_t = MATCHBIN_T;

    using memory_model_t = MEMORY_MODEL_T;
    using memory_state_t = typename memory_model_t::memory_state_t;

    using program_t = SimpleProgram<tag_t, arg_t>;

    using hardware_t = SignalGP<exec_stepper_t>;
    using thread_t = typename hardware_t::thread_t;

    using inst_t = typename program_t::inst_t;
    using inst_prop_t = InstProperty;
    using inst_lib_t = InstructionLibrary<hardware_t, inst_t, inst_prop_t>;

    /// Library of flow types.
    /// e.g., WHILE, IF, ROUTINE, et cetera
    /// NOTE - I'm not sure that I'm a fan of how this is organized/named/setup.
    /// BASIC: if statements (for now)
    enum class FlowType : size_t { BASIC, WHILE_LOOP, ROUTINE, CALL };
    struct FlowHandler {
      struct FlowControl {
        using end_flow_fun_t = std::function<void(exec_state_t &)>;
        using open_flow_fun_t = std::function<void(exec_state_t &, const FlowInfo &)>;
        open_flow_fun_t open_flow_fun;
        end_flow_fun_t close_flow_fun;
        end_flow_fun_t break_flow_fun;
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

      std::string FlowTypeToString(FlowType type) const {
        switch (type) {
          case FlowType::BASIC: return "BASIC";
          case FlowType::WHILE_LOOP: return "WHILE_LOOP";
          case FlowType::ROUTINE: return "ROUTINE";
          case FlowType::CALL: return "CALL";
          default: return "UNKNOWN";
        }
      }

      void OpenFlow(const FlowInfo & new_flow, exec_state_t & state) {
        FlowType type = new_flow.type;
        emp_assert(Has(lib, type), "FlowType not recognized!");
        lib[type].open_flow_fun(state, new_flow);
      }

      void CloseFlow(FlowType type, exec_state_t & state) {
        emp_assert(Has(lib, type), "FlowType not recognized!");
        lib[type].close_flow_fun(state);
      }
    };

    /// Base flow information
    struct FlowInfo {
      FlowType type;        ///< Flow type ID?
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
    };

    struct CallState {
      // Local memory
      // memory!
      // flow stack
      memory_state_t memory;
      emp::vector<FlowInfo> flow_stack; ///< Stack of 'Flow' (read heads)
      bool circular;  ///< Should call wrap when IP goes off end? Or, implicitly return?

      CallState(const memory_state_t & _mem=memory_state_t(), bool _circular=false)
        : memory(_mem), flow_stack(), circular(_circular) { ; }

      bool IsFlow() const { return !flow_stack.empty(); }

      emp::vector<FlowInfo> & GetFlowStack() { return flow_stack; }

      FlowInfo & GetTopFlow() {
        emp_assert(flow_stack.size());
        return flow_stack.back();
      }

      bool IsCircular() const { return circular; }

      memory_state_t & GetMemory() { return memory; }

      void SetIP(size_t i) { emp_assert(flow_stack.size()); flow_stack.back().ip = i; }

      void SetMP(size_t m) { emp_assert(flow_stack.size()); flow_stack.back().mp = m; }

      size_t GetIP() const { emp_assert(flow_stack.size()); return flow_stack.back().ip; }

      size_t GetMP() const { emp_assert(flow_stack.size()); return flow_stack.back().mp; }

      size_t & IP() { emp_assert(flow_stack.size()); return flow_stack.back().ip; }

      size_t & MP() { emp_assert(flow_stack.size()); return flow_stack.back().mp; }
    };

    /// Execution State.
    struct ExecState {
      // CallStack
      emp::vector<CallState> call_stack;   ///< Program call stack.

      void Clear() {
        call_stack.clear();
      }

      CallState & GetTopCallState() {
        emp_assert(call_stack.size(), "Cannot get top call state from empty call stack.");
        return call_stack.back();
      }
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

      size_t GetBegin() const { return begin; }
      size_t GetEnd() const { return end; }

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

    size_t max_call_depth;

    void SetupDefaultFlowControl() { // TODO!
      // --- BASIC Flow ---
      // On open:
      flow_handler[FlowType::BASIC].open_flow_fun =
        [](exec_state_t & exec_state, const FlowInfo & new_flow) {
          emp_assert(exec_state.call_stack.size(), "Failed to open BASIC flow. No calls on call stack.");
          CallState & call_state = exec_state.call_stack.back();
          call_state.flow_stack.emplace_back(new_flow);
        };

      // On close
      // - Pop current flow from stack.
      // - Set new top of flow stack (if any)'s IP and MP to returning IP and MP.
      flow_handler[FlowType::BASIC].close_flow_fun =
        [](exec_state_t & exec_state) {
          emp_assert(exec_state.call_stack.size(), "Failed to close BASIC flow. NO calls on call stack.");
          CallState & call_state = exec_state.call_stack.back();
          emp_assert(call_state.IsFlow(), "Failed to close BASIC flow. No flow to close.");
          const size_t ip = call_state.GetTopFlow().ip;
          const size_t mp = call_state.GetTopFlow().mp;
          call_state.flow_stack.pop_back();
          if (call_state.IsFlow()) {
            FlowInfo & top = call_state.GetTopFlow();
            top.ip = ip;
            top.mp = mp;
          }
        };

      // On break!
      flow_handler[FlowType::BASIC].break_flow_fun = [](exec_state_t & exec_state) { ; };

      flow_handler[FlowType::WHILE_LOOP].open_flow_fun =
        [](exec_state_t & exec_state, const FlowInfo & new_flow) {
          emp_assert(exec_state.call_stack.size(), "Failed to open WHILE_LOOP flow. No calls on call stack.");
          CallState & call_state = exec_state.call_stack.back();
          call_state.flow_stack.emplace_back(new_flow);
        };

      flow_handler[FlowType::WHILE_LOOP].close_flow_fun =
        [](exec_state_t & exec_state) {
          emp_assert(exec_state.call_stack.size(), "Failed to close WHILE_LOOP flow. No calls on call stack.");
          // Move IP to start of block
          CallState & call_state = exec_state.call_stack.back();
          const size_t loop_begin = call_state.GetTopFlow().begin;
          const size_t mp = call_state.GetTopFlow().mp;
          call_state.flow_stack.pop_back();
          if (call_state.IsFlow()) {
            call_state.SetIP(loop_begin);
            call_state.SetMP(mp);
          }
        };

      flow_handler[FlowType::WHILE_LOOP].break_flow_fun = [](exec_state_t & exec_state) { ; };

      flow_handler[FlowType::ROUTINE].open_flow_fun = [](exec_state_t & exec_state, const FlowInfo & new_flow) { ; };
      flow_handler[FlowType::ROUTINE].close_flow_fun = [](exec_state_t & exec_state) { ; };
      flow_handler[FlowType::ROUTINE].break_flow_fun = [](exec_state_t & exec_state) { ; };

      flow_handler[FlowType::CALL].open_flow_fun =
        [](exec_state_t & exec_state, const FlowInfo & new_flow) {
          emp_assert(exec_state.call_stack.size(), "Failed to open CALL flow. No calls on call stack.");
          CallState & call_state = exec_state.call_stack.back();
          call_state.flow_stack.emplace_back(new_flow);
        };

      flow_handler[FlowType::CALL].close_flow_fun =
        [](exec_state_t & exec_state) {
          emp_assert(exec_state.call_stack.size(), "Failed to close CALL flow. No calls on call stack.");
          CallState & call_state = exec_state.call_stack.back();
          emp_assert(call_state.IsFlow(), "Failed to close CALL flow. No flow to close.");
          // Closing a CALL flow:
          // - Pop call flow from flow stack.
          // - No need to pass IP and MP down (presumably, this was the bottom
          //   of the flow stack).
          if (call_state.IsCircular()) {
            FlowInfo & top = call_state.GetTopFlow();
            top.ip = top.begin;
          } else {
            call_state.GetFlowStack().pop_back();
          }
        };

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
        is_matchbin_cache_dirty(true),
        max_call_depth(256)
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

    bool IsValidProgramPosition(size_t mp, size_t ip) const {
      if (mp < modules.size()) {
        if (modules[mp].InModule(ip)) return true;
      }
      return false;
    }

    void SingleExecutionStep(hardware_t & hardware, exec_state_t & exec_state) {
      std::cout << "SingleExecutionStep!" << std::endl;
      // If there's a call state on the call stack, execute an instruction.
      while (exec_state.call_stack.size()) {
        // There's something on the call stack.
        CallState & call_state = exec_state.call_stack.back();
        // Is there anything on the flow stack?
        if (call_state.IsFlow()) {
          std::cout << "- There's some flow." << std::endl;
          FlowInfo & flow_info = call_state.flow_stack.back();
          size_t mp = flow_info.mp;
          size_t ip = flow_info.ip;
          std::cout << ">> MP=" << mp << "; IP=" << ip << std::endl;
          emp_assert(mp < GetNumModules(), "Invalid module pointer: ", mp);
          // Process current instruction (if any)!
          if (modules[mp].InModule(ip)) {
            // The instruction pointer is looking at a valid instruction in the current module.
            // NOTE - should we increment the IP before or after executing?
            inst_lib->ProcessInst(hardware, program[ip]);
            ++flow_info.ip; // Move instruction pointer forward (might be invalid location).
          } else if (ip >= program.GetSize()
                    && modules[mp].InModule(0)
                    && modules[mp].end < modules[mp].begin) {
            // The instruction pointer is off the edge of the program.
            // HERE, we handle if this module wraps back to the beginning of the program.
            // in which case, we need to move the IP.
            ip = 0;
            inst_lib->ProcessInst(hardware, program[ip]);
            ++flow_info.ip;
          } else {
            // IP not valid for this module. Close flow.
            flow_handler.CloseFlow(flow_info.type, exec_state);
            continue;
          }
        } else {
          // No flow!
          // todo - return from call?
          ReturnCall(exec_state);
        }
        break; // We executed *something*, break from loop.
      }
    }

    void InitThread(thread_t & thread, size_t module_id) {
      emp_assert(module_id < modules.size(), "Invalid module ID.");
      // Initialize new thread!
      std::cout << "InitThread!" << std::endl;
      exec_state_t & state = thread.GetExecState();
      if (state.call_stack.size()) { state.Clear(); }
      // TODO - switch code below to just use call module function!
      CallModule(module_id, state);
      // (1) create fresh memory state
      // state.call_stack.emplace_back(memory_model.CreateMemoryState());
      // (2) Set exec state up
      // CallState & call_state = state.call_stack.back();
      // flow type, ip, mp, begin, end
      // module_t & module_info = modules[module_id];
      // call_state.flow_stack.emplace_back(FlowType::CALL, module_info.begin, module_id, module_info.begin, module_info.end);
      // flow_handler.OpenFlow({FlowType::CALL, module_id, module_info.begin, module_info.begin, module_info.end}, state);
    }

    FlowHandler & GetFlowHandler() {
      return flow_handler;
    }

    /// Find end of code block (i.e., internal flow control code segment).
    // todo - debug!
    // BLOCK_CLOSE
    // BLOCK_DEF
    size_t FindEndOfBlock(size_t mp, size_t ip) {
      emp_assert(mp < modules.size(), "Invalid module!");
      int depth = 1;
      std::unordered_set<size_t> seen;
      while (true) {
        if (!IsValidProgramPosition(mp, ip)) break;
        const inst_t & inst = program[ip];
        if (inst_lib->HasProperty(inst.GetID(), inst_prop_t::BLOCK_DEF)) {
          ++depth;
        } else if (inst_lib->HasProperty(inst.GetID(), inst_prop_t::BLOCK_CLOSE)) {
          --depth;
          if (depth == 0) break;
        }
        seen.emplace(ip);
        ++ip;
        if (ip >= program.GetSize() && seen.size() < modules[mp].GetSize()) ip %= program.GetSize();
      }
      return ip;
    }

    // Find best module given a tag.
    emp::vector<size_t> FindModuleMatch(const tag_t & tag, size_t n=1) {
      // Find n matches.
      if(is_matchbin_cache_dirty){
        ResetMatchBin();
      }
      // no need to transform to values because we're using
      // matchbin uids equivalent to function uids
      return matchbin.Match(tag, n);
    }

    void CallModule(const tag_t & tag, exec_state_t & exec_state, bool circular=false) {
      // todo!
      emp::vector<size_t> matches(FindModuleMatch(tag));
      if (matches.size()) {
        CallModule(matches[0], exec_state, circular);
      }
    }

    // call a module on given execution state
    void CallModule(size_t module_id, exec_state_t & exec_state, bool circular=false) {
      emp_assert(module_id < modules.size());
      if (exec_state.call_stack.size() >= max_call_depth) return;
      // Push new state onto stack.
      exec_state.call_stack.emplace_back(memory_model.CreateMemoryState(), circular);

      // todo - open flow
      module_t & module_info = modules[module_id];
      flow_handler.OpenFlow({FlowType::CALL, module_id, module_info.begin, module_info.begin, module_info.end}, exec_state);

      if (exec_state.call_stack.size() > 1) {
        // todo - memory
        CallState & caller_state = exec_state.call_stack[exec_state.call_stack.size() - 2];
        CallState & new_state = exec_state.call_stack.back();
        memory_model.OnModuleCall(caller_state.GetMemory(), new_state.GetMemory());
      }
    }

    // todo - test!
    void ReturnCall(exec_state_t & exec_state) {
      std::cout << "Return!" << std::endl;
      // TODO - finish
      if (exec_state.call_stack.empty()) return; // Nothing to return from.
      // Get the current call state.
      CallState & returning_state = exec_state.call_stack.back();

      // Is there anything to return to?
      if (exec_state.call_stack.size() > 1) {
        // Yes! Copy the returning state's output memory into the caller state's local memory.
        CallState & caller_state = exec_state.call_stack[exec_state.call_stack.size() - 2];
        // TODO - setup configurable memory return! (lambda)
        memory_model.OnModuleReturn(returning_state.GetMemory(), caller_state.GetMemory());
      }
      // Pop the returning state from call stack.
      exec_state.call_stack.pop_back();
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

    memory_model_t & GetMemoryModel() { return memory_model; }

    void PrintModules(std::ostream & os=std::cout) const {
      os << "Modules: [";
      for (size_t i = 0; i < modules.size(); ++i) {
        if (i) os << ",";
        os << "{id:" << modules[i].id << ", begin:" << modules[i].begin << ", end:" << modules[i].end << ", tag:" << modules[i].tag << "}";
      }
      os << "]";
    }

    void PrintExecutionState(const exec_state_t & state, std::ostream & os=std::cout) const {
      // -- Call stack --
      // todo
      os << "Call stack (" << state.call_stack.size() << "):\n";
      os << "------ TOP ------\n";
      for (auto it = state.call_stack.rbegin(); it != state.call_stack.rend(); ++it) {
        const CallState & call_state = *it;
        if (call_state.flow_stack.size()) {
          const FlowInfo & top_flow = call_state.flow_stack.back();
          // MP, IP, ...
          // type, mp, ip, begin, end
          // todo - print full flow stack!
          os << "Call: {mp:" << top_flow.mp
             << ", ip:" << top_flow.ip
             << ", flow-begin:" << top_flow.begin
             << ", flow-end:" << top_flow.end
             << ", flow-type:" << flow_handler.FlowTypeToString(top_flow.type)
             << "}\n";
        }
        memory_model.PrintMemoryState(call_state.memory, os);
        os << "---\n";
      }
      os << "-----------------";
    }

  };

}}

#endif