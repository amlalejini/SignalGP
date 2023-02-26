#pragma once

#include <map>
#include <string>
#include <unordered_set>

#include "emp/datastructs/set_utils.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/base/vector.hpp"

#include "../lpbm/inst_impls.hpp"
#include "impls_basic_insts.hpp"
#include "impls_ctrl_insts.hpp"
#include "impls_mem_insts.hpp"
#include "impls_regulation_insts.hpp"

namespace sgp::inst::lfpbm {

template<
  typename HARDWARE_T,
  typename INST_STRUCT_T
>
using BuildInstDef = lpbm::BuildInstDef<HARDWARE_T, INST_STRUCT_T>;

template<typename HARDWARE_T>
struct InstructionDirectory {
  using hw_t = HARDWARE_T;
  using inst_t = typename hw_t::inst_t;
  using inst_lib_t = typename hw_t::inst_lib_t;
  using inst_def_t = typename inst_lib_t::InstructionDef;

  std::map<
    std::string,
    inst_def_t
  > std_instructions = {
    {"Nop", BuildInstDef<hw_t, Inst_Nop<hw_t>>::Gen() },
    {"Inc", BuildInstDef<hw_t, Inst_Inc<hw_t>>::Gen() },
    {"Dec", BuildInstDef<hw_t, Inst_Dec<hw_t>>::Gen() },
    {"Not", BuildInstDef<hw_t, Inst_Not<hw_t>>::Gen() },
    {"Add", BuildInstDef<hw_t, Inst_Add<hw_t>>::Gen() },
    {"Sub", BuildInstDef<hw_t, Inst_Sub<hw_t>>::Gen() },
    {"Mult", BuildInstDef<hw_t, Inst_Mult<hw_t>>::Gen() },
    {"Div", BuildInstDef<hw_t, Inst_Div<hw_t>>::Gen() },
    {"Mod", BuildInstDef<hw_t, Inst_Mod<hw_t>>::Gen() },
    {"TestEqu", BuildInstDef<hw_t, Inst_TestEqu<hw_t>>::Gen() },
    {"TestNEqu", BuildInstDef<hw_t, Inst_TestNEqu<hw_t>>::Gen() },
    {"TestLess", BuildInstDef<hw_t, Inst_TestLess<hw_t>>::Gen() },
    {"TestLessEqu", BuildInstDef<hw_t, Inst_TestLessEqu<hw_t>>::Gen() },
    {"TestGreater", BuildInstDef<hw_t, Inst_TestGreater<hw_t>>::Gen() },
    {"TestGreaterEqu", BuildInstDef<hw_t, Inst_TestGreaterEqu<hw_t>>::Gen() },
    {"If", BuildInstDef<hw_t, Inst_If<hw_t>>::Gen() },
    {"While", BuildInstDef<hw_t, Inst_While<hw_t>>::Gen() },
    {"Countdown", BuildInstDef<hw_t, Inst_Countdown<hw_t>>::Gen() },
    {"Break", BuildInstDef<hw_t, Inst_Break<hw_t>>::Gen() },
    {"Close", BuildInstDef<hw_t, Inst_Close<hw_t>>::Gen() },
    {"Call", BuildInstDef<hw_t, Inst_Call<hw_t>>::Gen() },
    {"Routine", BuildInstDef<hw_t, Inst_Routine<hw_t>>::Gen() },
    {"Return", BuildInstDef<hw_t, Inst_Return<hw_t>>::Gen() },
    {"SetMem", BuildInstDef<hw_t, Inst_SetMem<hw_t>>::Gen() },
    {"CopyMem", BuildInstDef<hw_t, Inst_CopyMem<hw_t>>::Gen() },
    {"SwapMem", BuildInstDef<hw_t, Inst_SwapMem<hw_t>>::Gen() },
    {"InputToWorking", BuildInstDef<hw_t, Inst_InputToWorking<hw_t>>::Gen() },
    {"WorkingToOutput", BuildInstDef<hw_t, Inst_WorkingToOutput<hw_t>>::Gen() },
    {"WorkingToGlobal", BuildInstDef<hw_t, Inst_WorkingToGlobal<hw_t>>::Gen() },
    {"GlobalToWorking", BuildInstDef<hw_t, Inst_GlobalToWorking<hw_t>>::Gen() },
    {"FullWorkingToGlobal", BuildInstDef<hw_t, Inst_FullWorkingToGlobal<hw_t>>::Gen() },
    {"FullGlobalToWorking", BuildInstDef<hw_t, Inst_FullGlobalToWorking<hw_t>>::Gen() },
    {"Fork", BuildInstDef<hw_t, Inst_Fork<hw_t>>::Gen() },
    {"Terminate", BuildInstDef<hw_t, Inst_Terminate<hw_t>>::Gen() },
    {"Terminal", BuildInstDef<hw_t, Inst_Terminal<hw_t>>::Gen() }
  };

  std::map<
    std::string,
    inst_def_t
  > regulation_instructions = {
    { "SetRegulator", BuildInstDef<hw_t, Inst_SetRegulator<hw_t>>::Gen() },
    { "SetOwnRegulator", BuildInstDef<hw_t, Inst_SetOwnRegulator<hw_t>>::Gen() },
    { "ClearRegulator", BuildInstDef<hw_t, Inst_ClearRegulator<hw_t>>::Gen() },
    { "ClearOwnRegulator", BuildInstDef<hw_t, Inst_ClearOwnRegulator<hw_t>>::Gen() },
    { "AdjRegulator", BuildInstDef<hw_t, Inst_AdjRegulator<hw_t>>::Gen() },
    { "AdjOwnRegulator", BuildInstDef<hw_t, Inst_AdjOwnRegulator<hw_t>>::Gen() },
    { "IncRegulator", BuildInstDef<hw_t, Inst_IncRegulator<hw_t>>::Gen() },
    { "IncOwnRegulator", BuildInstDef<hw_t, Inst_IncOwnRegulator<hw_t>>::Gen() },
    { "DecRegulator", BuildInstDef<hw_t, Inst_DecRegulator<hw_t>>::Gen() },
    { "DecOwnRegulator", BuildInstDef<hw_t, Inst_DecOwnRegulator<hw_t>>::Gen() },
    { "SenseRegulator", BuildInstDef<hw_t, Inst_SenseRegulator<hw_t>>::Gen() },
    { "SenseOwnRegulator", BuildInstDef<hw_t, Inst_SenseOwnRegulator<hw_t>>::Gen() }
  };

  void AddAllDefaultInstructions(
    inst_lib_t& inst_lib,
    const std::unordered_set<std::string>& except={}
  ) {
    for (auto& entry : std_instructions) {
      const std::string& name = entry.first;
      if (emp::Has(except, name)) continue;
      inst_lib.AddInst(
        entry.second
      );
    }

  }

  void AddDefaultInstructions(
    inst_lib_t& inst_lib,
    const emp::vector<std::string>& include
  ) {
    for (const std::string& name : include) {
      if (emp::Has(std_instructions, name)) {
        inst_lib.AddInst(
          std_instructions[name]
        );
      }
    }
  }

};

}