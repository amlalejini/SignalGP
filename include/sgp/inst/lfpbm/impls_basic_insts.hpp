#pragma once

#include "../InstructionLibrary.hpp"
#include "../BaseInstructionSpec.hpp"
#include "../lpbm/impls_basic_insts.hpp"

namespace sgp::inst::lfpbm {

template<typename HARDWARE_T>
using Inst_Nop = lpbm::Inst_Nop<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_Inc = lpbm::Inst_Inc<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_Dec = lpbm::Inst_Dec<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_Not = lpbm::Inst_Not<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_Add = lpbm::Inst_Add<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_Sub = lpbm::Inst_Sub<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_Mult = lpbm::Inst_Mult<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_Div = lpbm::Inst_Div<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_Mod = lpbm::Inst_Mod<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_TestEqu = lpbm::Inst_TestEqu<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_TestNEqu = lpbm::Inst_TestNEqu<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_TestLess = lpbm::Inst_TestLess<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_TestLessEqu = lpbm::Inst_TestLessEqu<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_TestGreater = lpbm::Inst_TestGreater<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_TestGreaterEqu = lpbm::Inst_TestGreaterEqu<HARDWARE_T>;

template<typename HARDWARE_T>
using Inst_Terminal = lpbm::Inst_Terminal<HARDWARE_T>;

}