#pragma once

#include "../BaseInstructionSpec.hpp"
#include "../lpbm/impls_regulation_insts.hpp"

namespace sgp::inst::lfpbm {

template<typename HARDWARE_T>
using Inst_SetRegulator = lpbm::Inst_SetRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_SetOwnRegulator = lpbm::Inst_SetOwnRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_ClearRegulator = lpbm::Inst_ClearRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_ClearOwnRegulator = lpbm::Inst_ClearOwnRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_AdjRegulator = lpbm::Inst_AdjRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_AdjOwnRegulator = lpbm::Inst_AdjOwnRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_IncRegulator = lpbm::Inst_IncRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_IncOwnRegulator = lpbm::Inst_IncOwnRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_DecRegulator = lpbm::Inst_DecRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_DecOwnRegulator = lpbm::Inst_DecOwnRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_SenseRegulator = lpbm::Inst_SenseRegulator<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_SenseOwnRegulator = lpbm::Inst_SenseOwnRegulator<HARDWARE_T>;

}