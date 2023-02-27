#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "emp/bits/BitSet.hpp"

#include "sgp/inst/InstructionLibrary.hpp"
#include "sgp/cpu/LinearProgramCPU.hpp"
#include "sgp/cpu/linprg/LinearProgram.hpp"
#include "sgp/cpu/mem/BasicMemoryModel.hpp"

#include "sgp/inst/lpbm/InstructionAdder.hpp"
#include "sgp/inst/lpbm/inst_impls.hpp"

template <typename INST_LIB_T>
void AddBasicInstructions(INST_LIB_T& inst_lib) {
  using hardware_t = typename INST_LIB_T::hardware_t;
  namespace inst_impls = sgp::inst::lpbm;
  inst_impls::InstructionAdder<hardware_t> inst_adder;
  inst_adder.AddAllDefaultInstructions(
    inst_lib
  );
}

TEST_CASE("LinearProgram<emp::BitSet<W>,int> - GenRandInst") {
  constexpr size_t TAG_WIDTH = 16;
  using mem_model_t = sgp::cpu::mem::BasicMemoryModel;
  // using tag_t = emp::BitSet<TAG_WIDTH>;
  using arg_t = int;
  using matchbin_t = emp::MatchBin<
    size_t,
    emp::HammingMetric<TAG_WIDTH>,
    emp::RankedSelector<>,
    emp::AdditiveCountdownRegulator<>
  >;
  using hardware_t = sgp::cpu::LinearProgramCPU<
    mem_model_t,
    // tag_t,
    arg_t,
    matchbin_t
  >;
  using inst_lib_t = typename hardware_t::inst_lib_t;
  using inst_t = typename hardware_t::inst_t;

  constexpr int RANDOM_SEED = 1;
  constexpr size_t NUM_TAGS = 10;
  constexpr size_t NUM_ARGS = 3;
  constexpr size_t MIN_ARG_VAL = 0;
  constexpr size_t MAX_ARG_VAL = 15;
  emp::Random random(RANDOM_SEED);

  // Build a limited instruction library.
  inst_lib_t inst_lib;
  AddBasicInstructions(inst_lib);

  // Generate a bunch of random instructions, check that they conform with requested bounds.
  for (size_t i = 0; i < 10000; ++i) {
    inst_t inst(
      sgp::cpu::linprg::GenRandInst<hardware_t, TAG_WIDTH>(
        random,
        inst_lib,
        NUM_TAGS,
        NUM_ARGS,
        {MIN_ARG_VAL, MAX_ARG_VAL}
      )
    );
    REQUIRE(inst.id < inst_lib.GetSize());
    REQUIRE(inst.GetTags().size() == NUM_TAGS);
    REQUIRE(inst.GetArgs().size() == NUM_ARGS);
    for (auto& arg : inst.GetArgs()) {
      REQUIRE(arg >= (int)MIN_ARG_VAL);
      REQUIRE(arg <= (int)MAX_ARG_VAL);
    }
  }
}


TEST_CASE("LinearProgram<emp::BitSet<W>,int> - GenRandLinearProgram") {
  constexpr size_t TAG_WIDTH = 16;
  using mem_model_t = sgp::cpu::mem::BasicMemoryModel;
  using arg_t = int;
  using matchbin_t = emp::MatchBin<
    size_t,
    emp::HammingMetric<TAG_WIDTH>,
    emp::RankedSelector<>,
    emp::AdditiveCountdownRegulator<>
  >;
  using hardware_t = sgp::cpu::LinearProgramCPU<
    mem_model_t,
    arg_t,
    matchbin_t
  >;
  using inst_lib_t = typename hardware_t::inst_lib_t;
  using program_t = typename hardware_t::program_t;

  constexpr int RANDOM_SEED = 1;
  constexpr size_t NUM_TAGS = 10;
  constexpr size_t NUM_ARGS = 3;
  constexpr size_t MIN_ARG_VAL = 0;
  constexpr size_t MAX_ARG_VAL = 15;
  constexpr size_t MIN_INST_CNT = 1;
  constexpr size_t MAX_INST_CNT = 512;

  emp::Random random(RANDOM_SEED);
  // Build a limited instruction library.
  inst_lib_t inst_lib;
  AddBasicInstructions(inst_lib);
  // Generate a bunch of random instructions, check that they conform with requested bounds.
  for (size_t i = 0; i < 10000; ++i) {
    program_t program(
      sgp::cpu::linprg::GenRandLinearProgram<hardware_t, TAG_WIDTH>(
        random,
        inst_lib,
        {MIN_INST_CNT, MAX_INST_CNT},
        NUM_TAGS,
        NUM_ARGS,
        {MIN_ARG_VAL, MAX_ARG_VAL}
      )
    );
    REQUIRE(program.GetSize() >= MIN_INST_CNT);
    REQUIRE(program.GetSize() <= MAX_INST_CNT);
    for (size_t pID = 0; pID < program.GetSize(); ++pID) {
      auto& inst = program[pID];
      REQUIRE(inst.id < inst_lib.GetSize());
      REQUIRE(inst.GetTags().size() == NUM_TAGS);
      REQUIRE(inst.GetArgs().size() == NUM_ARGS);
      for (auto & arg : inst.GetArgs()) {
        REQUIRE(arg >= (int)MIN_ARG_VAL);
        REQUIRE(arg <= (int)MAX_ARG_VAL);
      }
    }
  }
}