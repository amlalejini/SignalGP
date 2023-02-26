#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "emp/bits/BitSet.hpp"

#include "sgp/InstructionLibrary.hpp"
#include "sgp/cpu/LinearFunctionsProgramCPU.hpp"
#include "sgp/cpu/lfunprg/LinearFunctionsProgram.hpp"
#include "sgp/cpu/mem/BasicMemoryModel.hpp"

#include "sgp/inst/lfpbm/inst_impls.hpp"

template <typename INST_LIB_T>
void AddBasicInstructions(INST_LIB_T& inst_lib) {
  using hardware_t = typename INST_LIB_T::hardware_t;
  namespace inst_impls = sgp::inst::lfpbm;
  inst_impls::InstructionDirectory<hardware_t> inst_directory;
  inst_directory.AddAllDefaultInstructions(
    inst_lib
  );
}


/// Instantiate a linear function program.
TEST_CASE("LinearFunctionsProgram instantiation") {
  using tag_t = emp::BitSet<8>;
  using arg_t = int;
  using program_t = sgp::cpu::lfunprg::LinearFunctionsProgram<tag_t, arg_t>;
  program_t program1;
}

TEST_CASE("LinearFunction<emp::BitSet<W>, int> - GenRandLinearFunction") {
  constexpr size_t TAG_WIDTH = 16;
  using mem_model_t = sgp::cpu::mem::BasicMemoryModel;
  using arg_t = int;
  using matchbin_t = emp::MatchBin<
    size_t,
    emp::HammingMetric<TAG_WIDTH>,
    emp::RankedSelector<>,
    emp::AdditiveCountdownRegulator<>
  >;
  using hardware_t = sgp::cpu::LinearFunctionsProgramCPU<
    mem_model_t,
    arg_t,
    matchbin_t
  >;
  using inst_lib_t = typename hardware_t::inst_lib_t;
  using program_t = typename hardware_t::program_t;
  using function_t = typename program_t::function_t;

  constexpr int RANDOM_SEED = 1;
  constexpr size_t NUM_FUNC_TAGS = 3;
  // constexpr size_t MIN_NUM_FUNC = 1;
  // constexpr size_t MAX_NUM_FUNC = 32;
  constexpr size_t NUM_INST_TAGS = 10;
  constexpr size_t NUM_INST_ARGS = 3;
  constexpr size_t MIN_ARG_VAL = 0;
  constexpr size_t MAX_ARG_VAL = 15;
  constexpr size_t MIN_INST_CNT = 1;
  constexpr size_t MAX_INST_CNT = 512;

  emp::Random random(RANDOM_SEED);
  // Build a limited instruction library.
  inst_lib_t inst_lib;
  AddBasicInstructions(inst_lib);

  // Generate a bunch of random instructions, check that they conform with requested bounds.
  for (size_t i = 0; i < 1000; ++i) {
    function_t function(
      sgp::cpu::lfunprg::GenRandLinearFunction<hardware_t, TAG_WIDTH>(
        random,
        inst_lib,
        NUM_FUNC_TAGS,
        {MIN_INST_CNT, MAX_INST_CNT},
        NUM_INST_TAGS,
        NUM_INST_ARGS,
        {MIN_ARG_VAL, MAX_ARG_VAL}
      )
    );
    REQUIRE(function.GetSize() >= MIN_INST_CNT);
    REQUIRE(function.GetSize() <= MAX_INST_CNT);
    REQUIRE(function.GetTags().size() == NUM_FUNC_TAGS);
    for (size_t pID = 0; pID < function.GetSize(); ++pID) {
      auto& inst = function[pID];
      REQUIRE(inst.id < inst_lib.GetSize());
      REQUIRE(inst.GetTags().size() == NUM_INST_TAGS);
      REQUIRE(inst.GetArgs().size() == NUM_INST_ARGS);
      for (auto& arg : inst.GetArgs()) {
        REQUIRE(arg >= (int)MIN_ARG_VAL);
        REQUIRE(arg <= (int)MAX_ARG_VAL);
      }
    }
  }
}