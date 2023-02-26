#pragma once

#include "../lpbm/BaseInst.hpp"
#include "../lpbm/impls_mem_insts.hpp"

namespace sgp::inst::lfpbm {

template<typename HARDWARE_T>
using Inst_SetMem = lpbm::Inst_SetMem<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_CopyMem = lpbm::Inst_CopyMem<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_SwapMem = lpbm::Inst_SwapMem<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_InputToWorking = lpbm::Inst_InputToWorking<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_WorkingToOutput = lpbm::Inst_WorkingToOutput<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_WorkingToGlobal = lpbm::Inst_WorkingToGlobal<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_GlobalToWorking = lpbm::Inst_GlobalToWorking<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_FullWorkingToGlobal = lpbm::Inst_FullWorkingToGlobal<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_FullGlobalToWorking = lpbm::Inst_FullGlobalToWorking<HARDWARE_T>;

}