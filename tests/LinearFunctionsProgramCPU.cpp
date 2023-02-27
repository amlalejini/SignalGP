#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "emp/bits/BitSet.hpp"

#include "sgp/inst/InstructionLibrary.hpp"
#include "sgp/cpu/LinearFunctionsProgramCPU.hpp"
#include "sgp/cpu/lfunprg/LinearFunctionsProgram.hpp"
#include "sgp/cpu/mem/BasicMemoryModel.hpp"

#include "sgp/inst/lfpbm/InstructionAdder.hpp"
#include "sgp/inst/lfpbm/inst_impls.hpp"

template <typename INST_LIB_T>
void AddBasicInstructions(INST_LIB_T& inst_lib) {
  using hardware_t = typename INST_LIB_T::hardware_t;
  namespace inst_impls = sgp::inst::lfpbm;
  inst_impls::InstructionAdder<hardware_t> inst_directory;
  inst_directory.AddAllDefaultInstructions(
    inst_lib
  );
}

TEST_CASE("SignalGP - Linear Functions Program") {
  using mem_model_t = sgp::cpu::mem::BasicMemoryModel;
  using signalgp_t = sgp::cpu::LinearFunctionsProgramCPU<
    mem_model_t,
    int,
    emp::MatchBin<
      size_t,
      emp::HammingMetric<16>,
      emp::RankedSelector<std::ratio<16+8, 16>>,
      emp::AdditiveCountdownRegulator<>
    >,
    sgp::cpu::DefaultCustomComponent
  >;
  using inst_lib_t = typename signalgp_t::inst_lib_t;

  using event_lib_t = typename signalgp_t::event_lib_t;
  using program_t = typename signalgp_t::program_t;
  using tag_t = typename signalgp_t::tag_t;

  using mem_buffer_t = typename mem_model_t::mem_buffer_t;

  inst_lib_t inst_lib;
  event_lib_t event_lib;
  AddBasicInstructions(inst_lib);

  emp::Random random(2);
  signalgp_t hardware(random, inst_lib, event_lib);
  hardware.SetActiveThreadLimit(16);
  REQUIRE(hardware.ValidateThreadState());
  program_t program;

  SECTION ("Inst_Inc") {
    std::cout << "-- Testing Inst_Inc --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    emp_assert(hardware.ValidateThreadState());

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {0, 0, 0});
    program.PushInst(inst_lib, "Inc", {1, 0, 0});
    program.PushInst(inst_lib, "Inc", {1, 0, 0});
    program.PushInst(inst_lib, "Inc", {2, 0, 0});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // Inc(0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Inc(1)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 1.0}}));
    hardware.SingleProcess(); // Inc(1)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}}));
    hardware.SingleProcess(); // Inc(2)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 1.0}}));
    hardware.SingleProcess(); // Inc(3)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 1.0}, {3, 1.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_Dec") {
    std::cout << "-- Testing Inst_Dec --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {0, 0, 0});
    program.PushInst(inst_lib, "Dec", {0, 0, 0});
    program.PushInst(inst_lib, "Dec", {0, 0, 0});
    program.PushInst(inst_lib, "Dec", {1, 0, 0});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // Inc(0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Dec(0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}}));
    hardware.SingleProcess(); // Dec(0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, -1.0}}));
    hardware.SingleProcess(); // Dec(1)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, -1.0}, {1, -1.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_Not") {
    std::cout << "-- Testing Inst_Not --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {0, 0, 0});
    program.PushInst(inst_lib, "Not", {0, 0, 0});
    program.PushInst(inst_lib, "Not", {1, 0, 0});
    program.PushInst(inst_lib, "Dec", {0, 0, 0});
    program.PushInst(inst_lib, "Not", {0, 0, 0});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // Inc(0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Not(0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}}));
    hardware.SingleProcess(); // Not(1)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 1.0}}));
    hardware.SingleProcess(); // Dec(0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, -1.0}, {1, 1.0}}));
    hardware.SingleProcess(); // Not(0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 1.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_Add") {
    std::cout << "-- Testing Inst_Add --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {0, 0, 0}); // [0] = 1
    program.PushInst(inst_lib, "Add", {0, 0, 1}); // [1] = 2
    program.PushInst(inst_lib, "Add", {1, 1, 2}); // [2] = 4
    program.PushInst(inst_lib, "Add", {1, 2, 3}); // [3] = 6

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // Inc(0, 0, 0) // [0] = 1
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Add(0, 0, 1) // [1] = 2
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}}));
    hardware.SingleProcess(); // Add(1, 1, 2) // [2] = 4
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 4.0}}));
    hardware.SingleProcess(); // Add(1, 2, 3) // [3] = 6
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 4.0}, {3, 6.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_Sub") {
    std::cout << "-- Testing Inst_Sub --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {0, 0, 0}); // [0] = 1
    program.PushInst(inst_lib, "Add", {0, 0, 1}); // [1] = 2
    program.PushInst(inst_lib, "Add", {1, 1, 2}); // [2] = 4
    program.PushInst(inst_lib, "Add", {1, 2, 3}); // [3] = 6

    program.PushInst(inst_lib, "Sub", {3, 4, 3}); // [4] = 0; [3] = 6
    program.PushInst(inst_lib, "Sub", {3, 2, 4}); // [4] = 2
    program.PushInst(inst_lib, "Sub", {2, 3, 4}); // [4] = -2

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    // hardware.SingleProcess(); // Inc(0)
    // REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}}));

    hardware.SingleProcess(); // Inc(0, 0, 0) // [0] = 1
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Add(0, 0, 1) // [1] = 2
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}}));
    hardware.SingleProcess(); // Add(1, 1, 2) // [2] = 4
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 4.0}}));
    hardware.SingleProcess(); // Add(1, 2, 3) // [3] = 6
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 4.0}, {3, 6.0}}));
    hardware.SingleProcess(); // Sub(3, 4, 3) // [4] = 0; [3] = 6
    REQUIRE((mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 4.0}, {3, 6.0}, {4, 0.0}})));
    hardware.SingleProcess(); // Sub(3, 2, 4) // [4] = 2
    REQUIRE((mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 4.0}, {3, 6.0}, {4, 2.0}})));
    hardware.SingleProcess(); // Sub(2, 3, 4) // [4] = -2
    REQUIRE((mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 4.0}, {3, 6.0}, {4, -2.0}})));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_Mult") {
    std::cout << "-- Testing Inst_Mult --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {0, 0, 0}); // [0] = 1
    program.PushInst(inst_lib, "Add", {0, 0, 1}); // [1] = 2
    program.PushInst(inst_lib, "Dec", {-1, 0, 0}); // [-1] = -1
    program.PushInst(inst_lib, "Mult", {0, 0, 2}); // [2] = 1
    program.PushInst(inst_lib, "Mult", {-1, -1, 2}); // [2] = 1
    program.PushInst(inst_lib, "Mult", {1, 1, 2}); // [2] = 4
    program.PushInst(inst_lib, "Mult", {-1, 1, 2}); // [2] = -2

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // Inc(0, 0, 0) // [0] = 1
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Add(0, 0, 1) // [1] = 2
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}}));
    hardware.SingleProcess(); // Dec(-1, 0, 0) // [-1] = -1
    REQUIRE(mem_state.working_mem == mem_buffer_t({{-1, -1.0}, {0, 1.0}, {1, 2.0}}));
    hardware.SingleProcess(); // Mult(0, 0, 2) // [2] = 1
    REQUIRE(mem_state.working_mem == mem_buffer_t({{-1, -1.0}, {0, 1.0}, {1, 2.0}, {2, 1.0}}));
    hardware.SingleProcess(); // Mult(-1, -1, 2) // [2] = 1
    REQUIRE(mem_state.working_mem == mem_buffer_t({{-1, -1.0}, {0, 1.0}, {1, 2.0}, {2, 1.0}}));
    hardware.SingleProcess(); // Mult(1, 1, 2) // [2] = 4
    REQUIRE(mem_state.working_mem == mem_buffer_t({{-1, -1.0}, {0, 1.0}, {1, 2.0}, {2, 4.0}}));
    hardware.SingleProcess(); // Mult(-1, 1, 2) // [2] = -2
    REQUIRE(mem_state.working_mem == mem_buffer_t({{-1, -1.0}, {0, 1.0}, {1, 2.0}, {2, -2.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_Div") {
    std::cout << "-- Testing Inst_Div --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {0, 0, 0}); // [0] = 1
    program.PushInst(inst_lib, "Add", {0, 0, 1}); // [1] = 2
    program.PushInst(inst_lib, "Div", {2, 2, 2}); // [2] = 0   Do nothing.
    program.PushInst(inst_lib, "Div", {2, 2, 2}); // [2] = 0   Do nothing.
    program.PushInst(inst_lib, "Div", {0, 1, 3}); // [3] = 0.5 Do nothing.

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // Inc(0, 0, 0) // [0] = 1
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Add(0, 0, 1) // [1] = 2
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}}));
    hardware.SingleProcess(); // Div(2, 2, 2); // [2] = 0   Do nothing.
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 0.0}}));
    hardware.SingleProcess(); // Div(2, 2, 2); // [2] = 0   Do nothing.
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 0.0}}));
    hardware.SingleProcess(); // Div(0, 1, 3); // [3] = 0.5 Do nothing.
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 0.0}, {3, 0.5}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_Mod") {
    std::cout << "-- Testing Inst_Mod --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {0, 0, 0}); // [0] = 1
    program.PushInst(inst_lib, "Add", {0, 0, 1}); // [1] = 2
    program.PushInst(inst_lib, "Mod", {2, 2, 2}); // [2] = 0   Do nothing.
    program.PushInst(inst_lib, "Mod", {2, 2, 2}); // [2] = 0   Do nothing.
    program.PushInst(inst_lib, "Mod", {0, 1, 3}); // [3] = 1

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // Inc(0, 0, 0) // [0] = 1
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Add(0, 0, 1) // [1] = 2
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}}));
    hardware.SingleProcess(); // Mod(2, 2, 2); // [2] = 0   Do nothing.
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 0.0}}));
    hardware.SingleProcess(); // Mod(2, 2, 2); // [2] = 0   Do nothing.
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 0.0}}));
    hardware.SingleProcess(); // Mod(0, 1, 3); // [3] = 1
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 1.0}, {1, 2.0}, {2, 0.0}, {3, 1.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_SetMem") {
    std::cout << "-- Testing Inst_SetMem --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "SetMem", {0, 0, 0});
    program.PushInst(inst_lib, "SetMem", {1, 1, 0});
    program.PushInst(inst_lib, "SetMem", {2, 2, 0});
    program.PushInst(inst_lib, "SetMem", {3, -128, 0});
    program.PushInst(inst_lib, "SetMem", {4, 256, 0});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // SetMem(0, 0, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}}));
    hardware.SingleProcess(); // SetMem(1, 1, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 1.0}}));
    hardware.SingleProcess(); // SetMem(2, 2, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 2.0}}));
    hardware.SingleProcess(); // SetMem(3, -128, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 2.0}, {3, -128.0}}));
    hardware.SingleProcess(); // SetMem(4, 256, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 2.0}, {3, -128.0}, {4, 256}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_TestEqu") {
    std::cout << "-- Testing Inst_TestEqu --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "SetMem", {0, 0, 0});
    program.PushInst(inst_lib, "SetMem", {1, 0, 0});
    program.PushInst(inst_lib, "SetMem", {2, 2, 0});
    program.PushInst(inst_lib, "TestEqu", {0, 1, 3});
    program.PushInst(inst_lib, "TestEqu", {0, 2, 3});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // SetMem(0, 0, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}}));
    hardware.SingleProcess(); // SetMem{1, 0, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}}));
    hardware.SingleProcess(); // SetMem{2, 2, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}}));
    hardware.SingleProcess(); // TestEqu{0, 1, 3}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 1.0}}));
    hardware.SingleProcess(); // TestEqu{0, 2, 3}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 0.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_TestNEqu") {
    std::cout << "-- Testing Inst_TestNEqu --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "SetMem", {0, 0, 0});
    program.PushInst(inst_lib, "SetMem", {1, 0, 0});
    program.PushInst(inst_lib, "SetMem", {2, 2, 0});
    program.PushInst(inst_lib, "TestNEqu", {0, 1, 3});
    program.PushInst(inst_lib, "TestNEqu", {0, 2, 3});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // SetMem(0, 0, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}}));
    hardware.SingleProcess(); // SetMem{1, 0, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}}));
    hardware.SingleProcess(); // SetMem{2, 2, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 0.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 1.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_TestLess") {
    std::cout << "-- Testing Inst_TestLess --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "SetMem", {0, 0, 0});
    program.PushInst(inst_lib, "SetMem", {1, 0, 0});
    program.PushInst(inst_lib, "SetMem", {2, 2, 0});
    program.PushInst(inst_lib, "TestLess", {0, 1, 3});
    program.PushInst(inst_lib, "TestLess", {0, 2, 3});
    program.PushInst(inst_lib, "TestLess", {2, 0, 3});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // SetMem(0, 0, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}}));
    hardware.SingleProcess(); // SetMem{1, 0, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}}));
    hardware.SingleProcess(); // SetMem{2, 2, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 0.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 0.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_TestLessEqu") {
    std::cout << "-- Testing Inst_TestLessEqu --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "SetMem", {0, 0, 0});
    program.PushInst(inst_lib, "SetMem", {1, 0, 0});
    program.PushInst(inst_lib, "SetMem", {2, 2, 0});
    program.PushInst(inst_lib, "TestLessEqu", {0, 1, 3});
    program.PushInst(inst_lib, "TestLessEqu", {0, 2, 3});
    program.PushInst(inst_lib, "TestLessEqu", {2, 0, 3});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // SetMem(0, 0, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}}));
    hardware.SingleProcess(); // SetMem{1, 0, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}}));
    hardware.SingleProcess(); // SetMem{2, 2, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 0.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_TestGreater") {
    std::cout << "-- Testing Inst_TestGreater --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "SetMem", {0, 0, 0});
    program.PushInst(inst_lib, "SetMem", {1, 0, 0});
    program.PushInst(inst_lib, "SetMem", {2, 2, 0});
    program.PushInst(inst_lib, "TestGreater", {0, 1, 3});
    program.PushInst(inst_lib, "TestGreater", {0, 2, 3});
    program.PushInst(inst_lib, "TestGreater", {2, 0, 3});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // SetMem(0, 0, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}}));
    hardware.SingleProcess(); // SetMem{1, 0, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}}));
    hardware.SingleProcess(); // SetMem{2, 2, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 0.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 0.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 1.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_TestGreaterEqu") {
    std::cout << "-- Testing Inst_TestGreaterEqu --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "SetMem", {0, 0, 0});
    program.PushInst(inst_lib, "SetMem", {1, 0, 0});
    program.PushInst(inst_lib, "SetMem", {2, 2, 0});
    program.PushInst(inst_lib, "TestGreaterEqu", {0, 1, 3});
    program.PushInst(inst_lib, "TestGreaterEqu", {0, 2, 3});
    program.PushInst(inst_lib, "TestGreaterEqu", {2, 0, 3});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();

    emp_assert(hardware.GetPendingThreadIDs().size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());

    hardware.SingleProcess(); // SetMem(0, 0, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}}));
    hardware.SingleProcess(); // SetMem{1, 0, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}}));
    hardware.SingleProcess(); // SetMem{2, 2, 0}
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 0.0}}));
    hardware.SingleProcess();
    REQUIRE(mem_state.working_mem == mem_buffer_t({{0, 0.0}, {1, 0.0}, {2, 2.0}, {3, 1.0}}));

    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_If") {
    std::cout << "-- Testing Inst_TestIf --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    // If(true), ____, ____, ____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "If", {1, 0, 0});
    program.PushInst(inst_lib, "Inc", {2, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // If
    hardware.SingleProcess(); // Inc
    hardware.SingleProcess(); // Nop
    hardware.SingleProcess(); // Nop
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 1.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , If(true), ____, ____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {1, 0, 0});
    program.PushInst(inst_lib, "Inc", {2, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 1.0}, {3, 1.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , ____ , ____, If(true)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {3, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 3.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // if(false) , ____ , ____, ____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "If",  {0, 0, 0});
    program.PushInst(inst_lib, "Inc", {2, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , if(false) , ____, ____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {0, 0, 0});
    program.PushInst(inst_lib, "Inc", {2, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , ____ , ____, If(false)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 3.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , ____ , DEF, _____, if(true) , _____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {4, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Nop", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {4, 1.0}, {5, 2.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ ,CLOSE, ____ , DEF, _____, if(true) , _____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {4, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Close", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {4, 1.0}, {5, 2.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ ,CLOSE, ____ , DEF, _____, if(true)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Close", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {5, 2.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ ,CLOSE, ____ , DEF, _____, if(false) , _____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {0, 0, 0});
    program.PushInst(inst_lib, "Inc", {4, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Close", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {5, 1.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ ,CLOSE, ____ , DEF, _____, if(false)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {0, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Close", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    program.PushInst(inst_lib, "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {5, 1.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////
  }

  SECTION ("Inst_While") {
    std::cout << "-- Testing Inst_While --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    // While(true), ____, ____, ____  => Infinite loop.
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "While", {1, 0, 0});
    program.PushInst(inst_lib, "Inc",   {2, 0, 0});
    program.PushInst(inst_lib, "Inc",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc",   {4, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 1.0}, {3, 1.0}, {4, 1.0}}));
    for (size_t i = 0; i < 4; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 2.0}, {3, 2.0}, {4, 2.0}}));
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , While(true), ____, ____   => Infinite loop.
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc",   {2, 0, 0});
    program.PushInst(inst_lib, "While", {1, 0, 0});
    program.PushInst(inst_lib, "Inc",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc",   {4, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 1.0}, {3, 1.0}, {4, 1.0}}));
    for (size_t i = 0; i < 3; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 1.0}, {3, 2.0}, {4, 2.0}}));
    for (size_t i = 0; i < 32; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetActiveThreadIDs().size());
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , ____ , ____, While(true)   => Infinite loop.
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc",   {3, 0, 0});
    program.PushInst(inst_lib, "While", {3, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 3.0}}));
    for (size_t i = 0; i < 32; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetActiveThreadIDs().size());
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 3.0}}));
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // While(false) , ____ , ____, ____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "While", {0, 0, 0});
    program.PushInst(inst_lib, "Inc",   {2, 0, 0});
    program.PushInst(inst_lib, "Inc",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc",   {4, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , While(false) , ____, ____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc",   {2, 0, 0});
    program.PushInst(inst_lib, "While", {0, 0, 0});
    program.PushInst(inst_lib, "Inc",   {2, 0, 0});
    program.PushInst(inst_lib, "Inc",   {2, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , ____ , ____, While(false)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc",   {2, 0, 0});
    program.PushInst(inst_lib, "Inc",   {2, 0, 0});
    program.PushInst(inst_lib, "Inc",   {2, 0, 0});
    program.PushInst(inst_lib, "While", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{0, 0.0}, {1, 1.0}, {2, 3.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ , ____ , DEF, _____, While(true) , _____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc",   {3, 0, 0});
    program.PushInst(inst_lib, "While", {3, 0, 0});
    program.PushInst(inst_lib, "Inc",   {4, 0, 0});
    program.PushInst(inst_lib, "Inc",   {5, 0, 0});
    program.PushInst(inst_lib, "Nop",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",   {5, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc
    hardware.SingleProcess(); // While(true)
    hardware.SingleProcess(); // Inc
    hardware.SingleProcess(); // Inc
    hardware.SingleProcess(); // Nop
    hardware.SingleProcess(); // Inc
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {4, 1.0}, {5, 2.0}}));
    for (size_t i = 0; i < 5; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {4, 2.0}, {5, 4.0}}));
    for (size_t i = 0; i < 32; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetActiveThreadIDs().size());
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ ,CLOSE, ____ , DEF, _____, While(true) , _____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc",     {4, 0, 0});
    program.PushInst(inst_lib, "Inc",     {5, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc
    hardware.SingleProcess(); // While(true)
    hardware.SingleProcess(); // Inc(4)
    hardware.SingleProcess(); // Inc(5)
    hardware.SingleProcess(); // Close
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {4, 1.0}, {5, 1.0}}));
    for (size_t i = 0; i < 4; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {4, 2.0}, {5, 2.0}}));
    for (size_t i = 0; i < 32; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetActiveThreadIDs().size());
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ ,CLOSE, ____ , DEF, _____, While(true)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc",     {5, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // While(true)
    hardware.SingleProcess(); // Inc(5)
    hardware.SingleProcess(); // Close
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {5, 1.0}}));
    for (size_t i = 0; i < 3; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {5, 2.0}}));
    for (size_t i = 0; i < 32; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetActiveThreadIDs().size());
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ ,CLOSE, ____ , DEF, _____, While(false) , _____
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {4, 0, 0});
    program.PushInst(inst_lib, "Inc",     {5, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // While(true)
    hardware.SingleProcess(); // Inc(6)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {6, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // ____ ,CLOSE, ____ , DEF, _____, While(false)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {5, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // While(true)
    hardware.SingleProcess(); // Inc(6)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {6, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // CLOSE, ____ , DEF, _____, While(false)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {0, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // While(true)
    hardware.SingleProcess(); // Inc(6)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {6, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // if(false), INC, CLOSE, INC, CLOSE, ____ , DEF, _____, While(True)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {1, 0, 0});
    program.PushInst(inst_lib, "If",      {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {7, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "Close",   {0, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // While(true)
    hardware.SingleProcess(); // If(false)
    hardware.SingleProcess(); // Inc(6)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {6, 1.0}}));
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // if(false), INC, CLOSE, INC, CLOSE, ____ , DEF, _____, While(false)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {0, 0, 0});
    program.PushInst(inst_lib, "If",      {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {7, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "Close",   {0, 0, 0});
    program.PushFunction(tag_t());
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    program.PushInst(inst_lib, "Nop",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // While(false)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}}));
    ////////////////////////////////////////////////////////////////////////////
  }

  SECTION ("Inst_Countdown") {
    std::cout << "-- Testing Inst_Countdown --" << std::endl;
    // Countdown instruction is pretty much the same as the While.
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "Countdown",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // Countdown(true)
    hardware.SingleProcess(); // Inc(6)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}}));
    for (size_t i = 0; i < 10; ++i) { hardware.SingleProcess(); }
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}}));
    REQUIRE(hardware.GetActiveThreadIDs().size());
  }

  SECTION ("Inst_Break") {
    std::cout << "-- Testing Inst_Break --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {1, 0, 0});
    program.PushInst(inst_lib, "If",      {1, 0, 0});
    program.PushInst(inst_lib, "Break",   {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {7, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "Close",   {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {8, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    size_t thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // While(true)
    hardware.SingleProcess(); // If(true)
    hardware.SingleProcess(); // Break
    hardware.SingleProcess(); // Inc(8)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {8, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc",       {3, 0, 0});
    program.PushInst(inst_lib, "If",        {1, 0, 0});
    program.PushInst(inst_lib, "If",        {1, 0, 0});
    program.PushInst(inst_lib, "Break",     {0, 0, 0});
    program.PushInst(inst_lib, "Inc",       {7, 0, 0});
    program.PushInst(inst_lib, "Close",     {5, 0, 0});
    program.PushInst(inst_lib, "Inc",       {6, 0, 0});
    program.PushInst(inst_lib, "Close",     {0, 0, 0});
    program.PushInst(inst_lib, "Inc",       {8, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    emp_assert(spawned);
    thread_id = spawned.value();
    emp_assert(hardware.GetPendingThreadIDs().size() == 1);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);
    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // If(true)
    hardware.SingleProcess(); // If(true)
    hardware.SingleProcess(); // Break
    hardware.SingleProcess(); // Inc(7)
    hardware.SingleProcess(); // Close
    hardware.SingleProcess(); // Inc(6)
    hardware.SingleProcess(); // Close
    hardware.SingleProcess(); // Inc(8)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{0, 0.0}, {1, 1.0}, {3, 1.0}, {6, 1.0}, {7, 1.0}, {8, 1.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////
  }

  SECTION ("Inst_Call") {
    std::cout << "-- Testing Inst_Call --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    tag_t zeros, ones;
    ones.SetUInt(0, (uint16_t)-1);
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {2, 2});
    program.PushInst(inst_lib,   "SetMem", {3, 3});
    program.PushInst(inst_lib,   "Call", {0, 0, 0}, {ones});
    program.PushFunction(ones);
    program.PushInst(inst_lib,   "InputToWorking", {2, 1, 0});
    program.PushInst(inst_lib,   "InputToWorking", {3, 2, 0});
    program.PushInst(inst_lib,   "Inc", {1, 0, 0});
    program.PushInst(inst_lib,   "Inc", {2, 0, 0});
    program.PushInst(inst_lib,   "WorkingToOutput", {1, 4, 0});
    program.PushInst(inst_lib,   "WorkingToOutput", {2, 5, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    size_t thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    for (size_t i = 0; i < 10; ++i) hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{4, 3.0}, {5, 4.0}, {2, 2.0}, {3, 3.0}}));
    ////////////////////////////////////////////////////////////////////////////
  }

  SECTION ("Inst_Routine") {
    std::cout << "-- Testing Inst_Routine --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    tag_t zeros, ones;
    ones.SetUInt(0, (uint16_t)-1);
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {2, 2});
    program.PushInst(inst_lib,   "SetMem", {3, 3});
    program.PushInst(inst_lib,   "Routine", {0, 0, 0}, {ones});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    program.PushFunction(ones);
    program.PushInst(inst_lib,   "Inc", {0, 0, 0});
    program.PushInst(inst_lib,   "Inc", {1, 0, 0});
    program.PushInst(inst_lib,   "Inc", {2, 0, 0});
    program.PushInst(inst_lib,   "Inc", {3, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    size_t thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // SetMem
    hardware.SingleProcess(); // Routine
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 2);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    for (size_t i = 0; i < 4; ++i) hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 2);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{0, 1.0}, {1, 1.0}, {2, 3.0}, {3, 4.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{0, 1.0}, {1, 1.0}, {2, 3.0}, {3, 4.0}, {4, 4.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////
  }

  SECTION ("Inst_Return") {
    std::cout << "-- Testing Inst_Return --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    // Return from 1 Call (=> thread should die)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    tag_t zeros, ones;
    ones.SetUInt(0, (uint16_t)-1);
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {2, 2});
    program.PushInst(inst_lib,   "SetMem", {3, 3});
    program.PushInst(inst_lib,   "Return", {0, 0, 0});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    size_t thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}}));
    hardware.SingleProcess(); // Return
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 0);
    REQUIRE(hardware.GetThread(thread_id).IsDead());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.ValidateThreadState());
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Return from 2 Calls (=> should only return from top call)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    ones.SetUInt(0, (uint16_t)-1);
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {2, 2});
    program.PushInst(inst_lib,   "SetMem", {3, 3});
    program.PushInst(inst_lib,   "Call", {0, 0, 0}, {ones});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    program.PushFunction(ones);
    program.PushInst(inst_lib,   "SetMem", {2, 4});
    program.PushInst(inst_lib,   "SetMem", {5, 10});
    program.PushInst(inst_lib,   "Return", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}}));
    hardware.SingleProcess(); // Call
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 2);
    hardware.SingleProcess(); // SetMem
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 4.0}, {5, 10.0}}));
    hardware.SingleProcess(); // Return
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
            == mem_buffer_t({{2, 2.0}, {3, 3.0}, {4, 4.0}}));
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    REQUIRE(hardware.ValidateThreadState());
    hardware.SingleProcess(); // EOP
    REQUIRE(hardware.GetThread(thread_id).IsDead());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.ValidateThreadState());
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Return from Call from within BASIC block
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "Inc", {0, 0, 0});
    program.PushInst(inst_lib,   "If",  {0, 0, 0});
    program.PushInst(inst_lib,   "Return", {0, 0, 0});
    program.PushInst(inst_lib,   "Close",  {0, 0, 0});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // Inc
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // If
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Return
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 0);
    REQUIRE(hardware.GetThread(thread_id).IsDead());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.ValidateThreadState());
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Return from Call from within 2 BASIC blocks
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "Inc", {0, 0, 0});
    program.PushInst(inst_lib,   "If",  {0, 0, 0});
    program.PushInst(inst_lib,   "While",  {0, 0, 0});
    program.PushInst(inst_lib,   "Return", {0, 0, 0});
    program.PushInst(inst_lib,   "Close",  {0, 0, 0});
    program.PushInst(inst_lib,   "Close",  {0, 0, 0});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // Inc
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // If
    hardware.SingleProcess(); // While
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{0, 1.0}}));
    hardware.SingleProcess(); // Return
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 0);
    REQUIRE(hardware.GetThread(thread_id).IsDead());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.ValidateThreadState());
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Return from 1 Routine (=> thread should live on)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {2, 2});
    program.PushInst(inst_lib,   "SetMem", {3, 3});
    program.PushInst(inst_lib,   "Routine", {0, 0, 0}, {ones});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    program.PushFunction(ones);
    program.PushInst(inst_lib,   "SetMem", {5, 5});
    program.PushInst(inst_lib,   "Return", {0, 0, 0});
    program.PushInst(inst_lib,   "SetMem", {6, 6});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}}));
    hardware.SingleProcess(); // Routine
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 2);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // SetMem(5,5)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {5, 5.0}}));
    hardware.SingleProcess(); // Return
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {5, 5.0}}));
    hardware.SingleProcess(); // SetMem(4, 4)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {5, 5.0}, {4, 4.0}}));
    hardware.SingleProcess(); // EOP
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 0);
    REQUIRE(hardware.GetThread(thread_id).IsDead());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Return from 2 Routines (=> should just return from top)
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    tag_t two;
    two.SetUInt(0, (uint32_t)2);
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {2, 2});
    program.PushInst(inst_lib,   "SetMem", {3, 3});
    program.PushInst(inst_lib,   "Routine", {0, 0, 0}, {ones});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    program.PushFunction(ones);
    program.PushInst(inst_lib,   "SetMem", {5, 5});
    program.PushInst(inst_lib,   "Routine", {0, 0, 0}, {two});
    program.PushInst(inst_lib,   "SetMem", {6, 6});
    program.PushFunction(two);
    program.PushInst(inst_lib,   "SetMem", {7, 7});
    program.PushInst(inst_lib,   "Return", {0, 0, 0});
    program.PushInst(inst_lib,   "SetMem", {8, 8});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}}));
    hardware.SingleProcess(); // Routine
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 2);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // SetMem(5,5)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {5, 5.0}}));
    hardware.SingleProcess(); // Routine
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {5, 5.0}}));
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 3);
    hardware.SingleProcess(); // SetMem(7,7)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {5, 5.0}, {7, 7.0}}));
    hardware.SingleProcess(); // Return
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 2);
    hardware.SingleProcess(); // SetMem(6,6)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {5, 5.0}, {7, 7.0}, {6, 6.0}}));
    hardware.SingleProcess(); // EOM, SetMem(4,4)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {5, 5.0}, {7, 7.0}, {6, 6.0}, {4, 4.0}}));
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    hardware.SingleProcess(); // EOP
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 0);
    REQUIRE(hardware.GetThread(thread_id).IsDead());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Return from Routine from within BASIC block
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {1, 1});
    program.PushInst(inst_lib,   "Routine", {0, 0, 0}, {ones});
    program.PushInst(inst_lib,   "SetMem", {5, 5});
    program.PushFunction(ones);
    program.PushInst(inst_lib,   "If",  {1});
    program.PushInst(inst_lib,   "Return", {0, 0, 0});
    program.PushInst(inst_lib,   "Close",  {0, 0, 0});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    hardware.SingleProcess(); // Routine
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 2);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // If
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{1, 1.0}}));
    hardware.SingleProcess(); // Return
    hardware.SingleProcess(); // SetMem(5,5)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{1, 1.0}, {5, 5.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 0);
    REQUIRE(hardware.GetThread(thread_id).IsDead());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.ValidateThreadState());
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Return from Routine from within BASIC block
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {1, 1});
    program.PushInst(inst_lib,   "Routine", {0, 0, 0}, {ones});
    program.PushInst(inst_lib,   "SetMem", {5, 5});
    program.PushFunction(ones);
    program.PushInst(inst_lib,   "If",  {1});
    program.PushInst(inst_lib,   "While",  {1});
    program.PushInst(inst_lib,   "Return", {0, 0, 0});
    program.PushInst(inst_lib,   "Close",  {0, 0, 0});
    program.PushInst(inst_lib,   "Close",  {0, 0, 0});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    hardware.SingleProcess(); // Routine
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 2);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // If
    hardware.SingleProcess(); // While
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{1, 1.0}}));
    hardware.SingleProcess(); // Return
    hardware.SingleProcess(); // SetMem(5,5)
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{1, 1.0}, {5, 5.0}}));
    hardware.SingleProcess();
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 0);
    REQUIRE(hardware.GetThread(thread_id).IsDead());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.ValidateThreadState());
    ////////////////////////////////////////////////////////////////////////////
  }

  SECTION ("Inst_CopyMem") {
    std::cout << "-- Testing Inst_CopyMem --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushInst(inst_lib, "SetMem", {1, 1, 0});
    program.PushInst(inst_lib, "CopyMem", {1, 2, 0});
    program.PushInst(inst_lib, "CopyMem", {2, 3, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    size_t thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);
    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();
    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());
    hardware.SingleProcess(); // SetMem(1, 1, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{1, 1.0}}));
    hardware.SingleProcess(); // CopyMem(1, 2, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{1, 1.0}, {2, 1.0}}));
    hardware.SingleProcess(); // CopyMem(2, 3, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{1, 1.0}, {2, 1.0}, {3, 1.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_SwapMem") {
    std::cout << "-- Testing Inst_SwapMem --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    program.PushInst(inst_lib, "SetMem", {1, 1, 0});
    program.PushInst(inst_lib, "SetMem", {3, 3, 0});
    program.PushInst(inst_lib, "SwapMem", {1, 2, 0});
    program.PushInst(inst_lib, "SwapMem", {2, 3, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    size_t thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_id).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);
    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();
    // Assert that memory is empty.
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer().empty());
    REQUIRE(mem_state.working_mem.empty());
    REQUIRE(mem_state.input_mem.empty());
    REQUIRE(mem_state.output_mem.empty());
    hardware.SingleProcess(); // SetMem(1, 1, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{1, 1.0}}));
    hardware.SingleProcess(); // SetMem(3, 3, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{1, 1.0}, {3, 3.0}}));
    hardware.SingleProcess(); // SwapMem(1, 2, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{1, 0.0}, {2, 1.0}, {3, 3.0}}));
    hardware.SingleProcess(); // SwapMem(2, 3, 0)
    REQUIRE(mem_state.working_mem == mem_buffer_t({{1, 0.0}, {3, 1.0}, {2, 3.0}}));
    hardware.SingleProcess(); // IP off edge of program
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  }

  SECTION ("Inst_WorkingToGlobal") {
    std::cout << "-- Testing Inst_WorkingToGlobal --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    tag_t zeros, ones;
    ones.SetUInt(0, (uint16_t)-1);
    program.PushFunction(zeros);
    program.PushInst(inst_lib, "SetMem", {2, 2});
    program.PushInst(inst_lib, "WorkingToGlobal", {2, 1, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    size_t thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}}));
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer() == mem_buffer_t());
    hardware.SingleProcess(); // WorkingToGlobal
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}}));
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer() == mem_buffer_t({{1, 2.0}}));
  }

  SECTION ("Inst_GlobalToWorking") {
    std::cout << "-- Testing Inst_GlobalToWorking --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    tag_t zeros, ones;
    ones.SetUInt(0, (uint16_t)-1);
    program.PushFunction(zeros);
    program.PushInst(inst_lib, "SetMem", {2, 2});
    program.PushInst(inst_lib, "WorkingToGlobal", {2, 1, 0});
    program.PushInst(inst_lib, "GlobalToWorking", {1, 3, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    size_t thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}}));
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer() == mem_buffer_t());
    hardware.SingleProcess(); // WorkingToGlobal
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}}));
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer() == mem_buffer_t({{1, 2.0}}));
    hardware.SingleProcess(); // GlobalToWorking
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 2.0}}));
    REQUIRE(hardware.GetMemoryModel().GetGlobalBuffer() == mem_buffer_t({{1, 2.0}}));
  }

  SECTION ("Inst_Fork") {
    std::cout << "-- Testing Inst_Fork --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    tag_t zeros, ones, two;
    ones.SetUInt(0, (uint16_t)-1);
    two.SetUInt(0, (uint16_t)2);
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {2, 2});
    program.PushInst(inst_lib,   "SetMem", {3, 3});
    program.PushInst(inst_lib,   "Fork", {0, 0, 0}, {ones});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    program.PushFunction(ones);
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushInst(inst_lib,   "Fork", {0, 0, 0}, {two});
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushFunction(two);
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    size_t thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}}));
    hardware.SingleProcess(); // Fork
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 1);
    REQUIRE(hardware.ValidateThreadState());
    hardware.SingleProcess(); // [0]: SetMem(4,4), [1]: Nop
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {4, 4.0}}));
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 2);
    REQUIRE(hardware.ValidateThreadState());
    hardware.SingleProcess(); // [0]: DEAD, [1]: Fork
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 1);
    REQUIRE(hardware.ValidateThreadState());
    hardware.SingleProcess(); // [0]: DEAD, [1]: Nop, [2]: Nop
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 2);
    REQUIRE(hardware.ValidateThreadState());
    hardware.SingleProcess(); // [0]: DEAD, [1]: DEAD, [2]: Nop
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 1);
    REQUIRE(hardware.ValidateThreadState());
    hardware.SingleProcess(); // [0]: DEAD, [1]: DEAD, [2]: Nop
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 1);
    REQUIRE(hardware.ValidateThreadState());
    hardware.SingleProcess(); // [0]: DEAD, [1]: DEAD, [2]: DEAD
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.ValidateThreadState());
    ////////////////////////////////////////////////////////////////////////////
  }

  SECTION ("Inst_Terminate") {
    std::cout << "-- Testing Inst_Terminate --" << std::endl;
    ////////////////////////////////////////////////////////////////////////////
    program.Clear();
    hardware.Reset(); // Reset program & hardware.
    tag_t zeros, ones, two;
    ones.SetUInt(0, (uint16_t)-1);
    two.SetUInt(0, (uint16_t)2);
    program.PushFunction(zeros);
    program.PushInst(inst_lib,   "SetMem", {2, 2});
    program.PushInst(inst_lib,   "SetMem", {3, 3});
    program.PushInst(inst_lib,   "Fork", {0, 0, 0}, {ones});
    program.PushInst(inst_lib,   "SetMem", {4, 4});
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushFunction(ones);
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushInst(inst_lib,   "Fork", {0, 0, 0}, {two});
    program.PushInst(inst_lib,   "Terminate", {0, 0, 0});
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushFunction(two);
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    program.PushInst(inst_lib,   "Terminate", {0, 0, 0});
    program.PushInst(inst_lib,   "Nop", {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    auto spawned = hardware.SpawnThreadWithID(0);
    REQUIRE(spawned);
    size_t thread_id = spawned.value();
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetFlowStack().size() == 1);
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetCallStack().size() == 1);
    hardware.SingleProcess(); // SetMem
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}}));
    hardware.SingleProcess(); // Fork
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 1);
    REQUIRE(hardware.ValidateThreadState());
    hardware.SingleProcess(); // [0]: SetMem(4,4), [1]: Nop
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{2, 2.0}, {3, 3.0}, {4, 4.0}}));
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 2);
    REQUIRE(hardware.ValidateThreadState());

    hardware.SingleProcess(); // [0]: Nop, [1]: Fork
    REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 2);
    REQUIRE(hardware.ValidateThreadState());

    hardware.SingleProcess(); // [0]: Nop, [1]: Terminate - DEAD, [2]: Nop
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 2);
    REQUIRE(hardware.ValidateThreadState());

    hardware.SingleProcess(); // [0]: Nop, [1]: DEAD, [2]: Terminate - DEAD
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 1);
    REQUIRE(hardware.ValidateThreadState());

    hardware.SingleProcess(); // [0]: Nop, [1]: DEAD, [2]: Terminate - DEAD
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 1);
    REQUIRE(hardware.ValidateThreadState());

    hardware.SingleProcess(); // [0]: EOM, [1]: DEAD, [2]: Terminate - DEAD
    REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.ValidateThreadState());
    ////////////////////////////////////////////////////////////////////////////
  }
}

