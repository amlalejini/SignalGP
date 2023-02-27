#pragma once

#include "../InstructionLibrary.hpp"
#include "../BaseInstructionAdder.hpp"
#include "inst_impls.hpp"

namespace sgp::inst::lfpbm {

template <typename HARDWARE_T>
class InstructionAdder : public BaseInstructionAdder<HARDWARE_T> {
public:
  using base_t = BaseInstructionAdder<HARDWARE_T>;
  using hw_t = HARDWARE_T;
  using inst_t = typename hw_t::inst_t;
  using inst_lib_t = InstructionLibrary<hw_t, inst_t>;
  using inst_def_t = InstructionDef<hw_t, inst_t>;

protected:
  using base_t::default_instructions;

  std::map<
    std::string,
    inst_def_t
  > regulation_instructions = {
    { "SetRegulator", BuildInstructionDef<Inst_SetRegulator<hw_t>, inst_def_t>() },
    { "SetOwnRegulator", BuildInstructionDef<Inst_SetOwnRegulator<hw_t>, inst_def_t>() },
    { "ClearRegulator", BuildInstructionDef<Inst_ClearRegulator<hw_t>, inst_def_t>() },
    { "ClearOwnRegulator", BuildInstructionDef<Inst_ClearOwnRegulator<hw_t>, inst_def_t>() },
    { "AdjRegulator", BuildInstructionDef<Inst_AdjRegulator<hw_t>, inst_def_t>() },
    { "AdjOwnRegulator", BuildInstructionDef<Inst_AdjOwnRegulator<hw_t>, inst_def_t>() },
    { "IncRegulator", BuildInstructionDef<Inst_IncRegulator<hw_t>, inst_def_t>() },
    { "IncOwnRegulator", BuildInstructionDef<Inst_IncOwnRegulator<hw_t>, inst_def_t>() },
    { "DecRegulator", BuildInstructionDef<Inst_DecRegulator<hw_t>, inst_def_t>() },
    { "DecOwnRegulator", BuildInstructionDef<Inst_DecOwnRegulator<hw_t>, inst_def_t>() },
    { "SenseRegulator", BuildInstructionDef<Inst_SenseRegulator<hw_t>, inst_def_t>() },
    { "SenseOwnRegulator", BuildInstructionDef<Inst_SenseOwnRegulator<hw_t>, inst_def_t>() }
  };

public:

  InstructionAdder() {
    // Configure default instructions
    default_instructions = {
      {"Nop", BuildInstructionDef<Inst_Nop<hw_t>, inst_def_t>()},
      {"Inc", BuildInstructionDef<Inst_Inc<hw_t>, inst_def_t>() },
      {"Dec", BuildInstructionDef<Inst_Dec<hw_t>, inst_def_t>() },
      {"Not", BuildInstructionDef<Inst_Not<hw_t>, inst_def_t>() },
      {"Add", BuildInstructionDef<Inst_Add<hw_t>, inst_def_t>() },
      {"Sub", BuildInstructionDef<Inst_Sub<hw_t>, inst_def_t>() },
      {"Mult", BuildInstructionDef<Inst_Mult<hw_t>, inst_def_t>() },
      {"Div", BuildInstructionDef<Inst_Div<hw_t>, inst_def_t>() },
      {"Mod", BuildInstructionDef<Inst_Mod<hw_t>, inst_def_t>() },
      {"TestEqu", BuildInstructionDef<Inst_TestEqu<hw_t>, inst_def_t>() },
      {"TestNEqu", BuildInstructionDef<Inst_TestNEqu<hw_t>, inst_def_t>() },
      {"TestLess", BuildInstructionDef<Inst_TestLess<hw_t>, inst_def_t>() },
      {"TestLessEqu", BuildInstructionDef<Inst_TestLessEqu<hw_t>, inst_def_t>() },
      {"TestGreater", BuildInstructionDef<Inst_TestGreater<hw_t>, inst_def_t>() },
      {"TestGreaterEqu", BuildInstructionDef<Inst_TestGreaterEqu<hw_t>, inst_def_t>() },
      {"If", BuildInstructionDef<Inst_If<hw_t>, inst_def_t>() },
      {"While", BuildInstructionDef<Inst_While<hw_t>, inst_def_t>() },
      {"Countdown", BuildInstructionDef<Inst_Countdown<hw_t>, inst_def_t>() },
      {"Break", BuildInstructionDef<Inst_Break<hw_t>, inst_def_t>() },
      {"Close", BuildInstructionDef<Inst_Close<hw_t>, inst_def_t>() },
      {"Call", BuildInstructionDef<Inst_Call<hw_t>, inst_def_t>() },
      {"Routine", BuildInstructionDef<Inst_Routine<hw_t>, inst_def_t>() },
      {"Return", BuildInstructionDef<Inst_Return<hw_t>, inst_def_t>() },
      {"SetMem", BuildInstructionDef<Inst_SetMem<hw_t>, inst_def_t>() },
      {"CopyMem", BuildInstructionDef<Inst_CopyMem<hw_t>, inst_def_t>() },
      {"SwapMem", BuildInstructionDef<Inst_SwapMem<hw_t>, inst_def_t>() },
      {"InputToWorking", BuildInstructionDef<Inst_InputToWorking<hw_t>, inst_def_t>() },
      {"WorkingToOutput", BuildInstructionDef<Inst_WorkingToOutput<hw_t>, inst_def_t>() },
      {"WorkingToGlobal", BuildInstructionDef<Inst_WorkingToGlobal<hw_t>, inst_def_t>() },
      {"GlobalToWorking", BuildInstructionDef<Inst_GlobalToWorking<hw_t>, inst_def_t>() },
      {"FullWorkingToGlobal", BuildInstructionDef<Inst_FullWorkingToGlobal<hw_t>, inst_def_t>() },
      {"FullGlobalToWorking", BuildInstructionDef<Inst_FullGlobalToWorking<hw_t>, inst_def_t>() },
      {"Fork", BuildInstructionDef<Inst_Fork<hw_t>, inst_def_t>() },
      {"Terminate", BuildInstructionDef<Inst_Terminate<hw_t>, inst_def_t>() },
      {"Terminal", BuildInstructionDef<Inst_Terminal<hw_t>, inst_def_t>() }
    };
  }

  void AddAllRegulationInstructions(
    inst_lib_t& inst_lib,
    const emp::vector<std::string>& except = {}
  ) {
    AddAllInstructions(
      regulation_instructions,
      inst_lib,
      except
    );
  }

};

}