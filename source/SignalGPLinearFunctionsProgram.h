#ifndef EMP_LINEAR_FUNCTIONS_PROGRAM_SIGNALGP_H
#define EMP_LINEAR_FUNCTIONS_PROGRAM_SIGNALGP_H

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

#include "SignalGPBase.h"
#include "LinearFunctionsProgram.h"

namespace emp { namespace signalgp {

  template<typename MEMORY_MODEL_T,
           typename TAG_T=emp::BitSet<16>,
           typename INST_ARGUMENT_T=int,
           typename MATCHBIN_T=emp::MatchBin< size_t, emp::HammingMetric<16>, emp::RankedSelector<std::ratio<16+8, 16> >>,
           typename CUSTOM_COMPONENT_T=emp::signalgp::DefaultCustomComponent>
  class LinearFunctionsProgramSignalGP : public BaseSignalGP<LinearFunctionsProgramSignalGP<MEMORY_MODEL_T,TAG_T,INST_ARGUMENT_T,MATCHBIN_T,CUSTOM_COMPONENT_T>,
                                                             lsgp_utils::ExecState<MEMORY_MODEL_T>,
                                                             TAG_T,
                                                             CUSTOM_COMPONENT_T>

  {
  public:
    // Type aliases :scream:
    using this_t = LinearFunctionsProgramSignalGP<MEMORY_MODEL_T,TAG_T,INST_ARGUMENT_T,MATCHBIN_T,CUSTOM_COMPONENT_T>;
    using exec_state_t = lsgp_utils::ExecState<MEMORY_MODEL_T>;
    using call_state_t = typename exec_state_t::call_state_t;
    using flow_t = lsgp_utils::FlowType;
    using flow_info_t = lsgp_utils::FlowInfo;
    using flow_handler_t = lsgp_utils::FlowHandler<this_t, exec_state_t>;
    using tag_t = TAG_T;
    using arg_t = INST_ARGUMENT_T;
    using matchbin_t = MATCHBIN_T;
    using memory_model_t = MEMORY_MODEL_T;
    using memory_state_t = typename memory_model_t::memory_state_t;
    using program_t = emp::signalgp::LinearFunctionsProgram<tag_t, arg_t>;
    using base_hw_t = BaseSignalGP<this_t, exec_state_t, tag_t, CUSTOM_COMPONENT_T>;
    using thread_t = typename base_hw_t::Thread;
    using event_lib_t = typename base_hw_t::event_lib_t; // EventLibrary<this_t>
    using event_t = typename base_hw_t::event_t;

    enum class InstProperty { BLOCK_CLOSE, BLOCK_DEF }; /// Instruction-definition properties.
    using inst_t = typename program_t::inst_t;
    using inst_lib_t = InstructionLibrary<this_t, inst_t, InstProperty>;

    using fun_end_flow_t = typename flow_handler_t::fun_end_flow_t;
    using fun_open_flow_t = typename flow_handler_t::fun_open_flow_t;

  protected:
    emp::Ptr<inst_lib_t> inst_lib; ///< Library of program instructions.
    flow_handler_t flow_handler;
    memory_model_t memory_model;
    program_t program;
    emp::Random & random;
    matchbin_t matchbin;
    bool is_matchbin_cache_dirty;
    std::function<void()> fun_clear_matchbin_cache = [this](){ this->ResetMatchBin(); }; // todo - can we do a better job baking this in?

    size_t max_call_depth;

    /// Setup default flow control functions for opening, closing, and breaking
    /// each type of control flow: BASIC, WHILE_LOOP, CALL, ROUTINE.
    // TODO
    void SetupDefaultFlowControl() {
      // --- BASIC Flow ---
      // On open:
      flow_handler[flow_t::BASIC].open_flow_fun =
        [](this_t & hw, exec_state_t & exec_state, const flow_info_t & new_flow) {
        };

      // On close
      // - Pop current flow from stack.
      // - Set new top of flow stack (if any)'s IP and MP to returning IP and MP.
      flow_handler[flow_t::BASIC].close_flow_fun =
        [](this_t & hw, exec_state_t & exec_state) {
        };

      // On break!
      flow_handler[flow_t::BASIC].break_flow_fun =
        [](this_t & hw, exec_state_t & exec_state) {
        };

      flow_handler[flow_t::WHILE_LOOP].open_flow_fun =
        [](this_t & hw, exec_state_t & exec_state, const flow_info_t & new_flow) {
        };

      flow_handler[flow_t::WHILE_LOOP].close_flow_fun =
        [](this_t & hw, exec_state_t & exec_state) {
        };

      flow_handler[flow_t::WHILE_LOOP].break_flow_fun =
        [](this_t & hw, exec_state_t & exec_state) {
        };

      flow_handler[flow_t::ROUTINE].open_flow_fun =
        [](this_t & hw, exec_state_t & exec_state, const flow_info_t & new_flow) {
        };

      flow_handler[flow_t::ROUTINE].close_flow_fun =
        [](this_t & hw, exec_state_t & exec_state) {
        };

      // breaking from a routine is the same as closing a routine
      flow_handler[flow_t::ROUTINE].break_flow_fun =
        [](this_t & hw, exec_state_t & exec_state) {
        };

      flow_handler[flow_t::CALL].open_flow_fun =
        [](this_t & hw, exec_state_t & exec_state, const flow_info_t & new_flow) {
        };

      flow_handler[flow_t::CALL].close_flow_fun =
        [](this_t & hw, exec_state_t & exec_state) {
        };

      flow_handler[flow_t::CALL].break_flow_fun =
        [](this_t & hw, exec_state_t & exec_state) {
        };
    }

  public:
    LinearFunctionsProgramSignalGP(emp::Random & rnd, emp::Ptr<inst_lib_t> ilib, emp::Ptr<event_lib_t> elib)
      : base_hw_t(elib),
        inst_lib(ilib),
        random(rnd),
        matchbin(rnd),
        is_matchbin_cache_dirty(true),
        max_call_depth(256)
    {
      // Configure default flow control.
      SetupDefaultFlowControl();
    }

    LinearFunctionsProgramSignalGP(LinearFunctionsProgramSignalGP &&) = default;
    LinearFunctionsProgramSignalGP(const LinearFunctionsProgramSignalGP &) = default;

    void Reset() {}
    void SingleExecutionStep(this_t &, thread_t &) {}
    vector<size_t> FindModuleMatch(const tag_t &, size_t) {return vector<size_t>();}
    void InitThread(thread_t &, size_t) {}

    void ResetMatchBin() {}

  };

}}

#endif