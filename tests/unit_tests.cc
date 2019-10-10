#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

#include "tools/BitSet.h"

#include "InstructionLibrary.h"
#include "EventLibrary.h"
#include "SGP-V2/SignalGP.h"
#include "SGP-V2/MemoryModel.h"
#include "SGP-V2/ExecutionStepper.h"
#include "SGP-V2/LinearProgram.h"
#include "SGP-V2/instructions_impl.h"

TEST_CASE( "Hello World", "[general]" ) {
  std::cout << "Hello tests!" << std::endl;
}

TEST_CASE( "SignalGP_V2::LinearProgram::SimpleMemory - Default Instructions", "[instructions]" ) {
  // General setup.
  using mem_model_t = emp::sgp_v2::SimpleMemoryModel;
  using exec_stepper_t = emp::sgp_v2::SimpleExecutionStepper<mem_model_t>;
  using exec_state_t = typename exec_stepper_t::exec_state_t;
  // using tag_t = typename exec_stepper_t::tag_t;
  using signalgp_t = emp::sgp_v2::SignalGP<exec_stepper_t>;
  using inst_lib_t = typename exec_stepper_t::inst_lib_t;
  using inst_t = typename exec_stepper_t::inst_t;
  using inst_prop_t = typename exec_stepper_t::inst_prop_t;
  using program_t = typename exec_stepper_t::program_t; // SimpleProgram<TAG_T, INST_ARGUMENT_T>

  using mem_buffer_t = typename mem_model_t::mem_buffer_t;

  const size_t THREAD_LIMIT = 8;

  inst_lib_t inst_lib;
  emp::EventLibrary<signalgp_t> event_lib;
  emp::Random random(2);

  // Add some instructions to the instruction library.
  inst_lib.AddInst("Nop", [](signalgp_t & hw, const inst_t & inst) { ; }, "No operation!");
  inst_lib.AddInst("ModuleDef", [](signalgp_t & hw, const inst_t & inst) { ; }, "Module definition", {inst_prop_t::MODULE});
  inst_lib.AddInst("Inc", emp::sgp_v2::inst_impl::Inst_Inc<signalgp_t, inst_t>, "Increment!");
  inst_lib.AddInst("Dec", emp::sgp_v2::inst_impl::Inst_Dec<signalgp_t, inst_t>, "Decrement!");
  inst_lib.AddInst("Not", emp::sgp_v2::inst_impl::Inst_Not<signalgp_t, inst_t>, "Logical not of ARG[0]");
  inst_lib.AddInst("Add", emp::sgp_v2::inst_impl::Inst_Add<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Sub", emp::sgp_v2::inst_impl::Inst_Sub<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Mult", emp::sgp_v2::inst_impl::Inst_Mult<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Div", emp::sgp_v2::inst_impl::Inst_Div<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Mod", emp::sgp_v2::inst_impl::Inst_Mod<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestEqu", emp::sgp_v2::inst_impl::Inst_TestEqu<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestNEqu", emp::sgp_v2::inst_impl::Inst_TestNEqu<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestLess", emp::sgp_v2::inst_impl::Inst_TestLess<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestLessEqu", emp::sgp_v2::inst_impl::Inst_TestLessEqu<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestGreater", emp::sgp_v2::inst_impl::Inst_TestGreater<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestGreaterEqu", emp::sgp_v2::inst_impl::Inst_TestGreaterEqu<signalgp_t, inst_t>, "");
  inst_lib.AddInst("If", emp::sgp_v2::inst_impl::Inst_If<signalgp_t, inst_t>, "");
  inst_lib.AddInst("SetMem", emp::sgp_v2::inst_impl::Inst_SetMem<signalgp_t, inst_t>, "");
  inst_lib.AddInst("While", emp::sgp_v2::inst_impl::Inst_While<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Countdown", emp::sgp_v2::inst_impl::Inst_Countdown<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Break", emp::sgp_v2::inst_impl::Inst_Break<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Close", emp::sgp_v2::inst_impl::Inst_Close<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Call", emp::sgp_v2::inst_impl::Inst_Call<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Routine", emp::sgp_v2::inst_impl::Inst_Routine<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Return", emp::sgp_v2::inst_impl::Inst_Return<signalgp_t, inst_t>, "");
  inst_lib.AddInst("CopyMem", emp::sgp_v2::inst_impl::Inst_CopyMem<signalgp_t, inst_t>, "");
  inst_lib.AddInst("SwapMem", emp::sgp_v2::inst_impl::Inst_SwapMem<signalgp_t, inst_t>, "");
  inst_lib.AddInst("InputToWorking", emp::sgp_v2::inst_impl::Inst_InputToWorking<signalgp_t, inst_t>, "");
  inst_lib.AddInst("WorkingToOutput", emp::sgp_v2::inst_impl::Inst_WorkingToOutput<signalgp_t, inst_t>, "");
  inst_lib.AddInst("WorkingToGlobal", emp::sgp_v2::inst_impl::Inst_WorkingToGlobal<signalgp_t, inst_t>, "");
  inst_lib.AddInst("GlobalToWorking", emp::sgp_v2::inst_impl::Inst_GlobalToWorking<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Fork", emp::sgp_v2::inst_impl::Inst_Fork<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Terminate", emp::sgp_v2::inst_impl::Inst_Terminate<signalgp_t, inst_t>, "");

  signalgp_t hardware(&event_lib, &random);
  hardware.InitExecStepper(&inst_lib, &random);

  // Configure hardware
  hardware.SetThreadLimit(THREAD_LIMIT);

  // Setup custom hardware printing functions.
  hardware.SetPrintProgramFun([&hardware, &inst_lib](std::ostream & os) {
    program_t & program = hardware.GetProgram();
    for (size_t i = 0; i < program.GetSize(); ++i) {
      inst_t & inst = program[i];
      // Name[tags](args)
      os << inst_lib.GetName(inst.id);
      os << "[";
      for (size_t ti = 0; ti < inst.tags.size(); ++ti) {
        if (ti) os << ",";
        os << inst.tags[ti];
      }
      os << "](";
      for (size_t ai = 0; ai < inst.args.size(); ++ai) {
        if (ai) os << ",";
        os << inst.args[ai];
      }
      os << ")\n";
    }
  });

  hardware.SetPrintModulesFun([&hardware](std::ostream & os) {
    hardware.GetExecStepper().PrintModules();
  });

  hardware.SetPrintExecStepperStateFun([&hardware](std::ostream & os) {
    hardware.PrintModules(os);
    os << "\n";
    // todo - print matchbin state
    hardware.GetExecStepper().GetMemoryModel().PrintState(os);
  });

  hardware.SetPrintExecutionStateFun([&hardware](const exec_state_t & state, std::ostream & os) {
    hardware.GetExecStepper().PrintExecutionState(state, os);
  });

  hardware.SetPrintHardwareStateFun([&hardware](std::ostream & os) {
    // -- Print thread usage --
    hardware.PrintThreadUsage(os);
    os << "\n";
    // -- Print event queue --
    hardware.PrintEventQueue(os);
    os << "\n";
    // -- Print state of exec stepper --
    hardware.PrintExecStepperState(os);
    os << "\n";
    // -- Print thread states --
    hardware.PrintActiveThreadStates(os);
  });

  program_t program;

  SECTION ("Inst_Inc") {
    std::cout << "-- Testing Inst_Inc --" << std::endl;
    program.Clear();
    hardware.Reset(); // Reset program & hardware.

    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {0, 0, 0});
    program.PushInst(inst_lib, "Inc", {1, 0, 0});
    program.PushInst(inst_lib, "Inc", {1, 0, 0});
    program.PushInst(inst_lib, "Inc", {2, 0, 0});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});

    // Load program on hardware.
    hardware.SetProgram(program);

    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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
    hardware.SpawnThread(0);

    // Assert state of memory.
    auto & thread_ids = hardware.GetActiveThreadIDs();
    REQUIRE(thread_ids.size() == 1);

    // Assert call stack has only 1 call.
    auto & call_stack = hardware.GetThread(thread_ids[0]).GetExecState().GetCallStack();
    REQUIRE(call_stack.size() == 1);

    auto & call_state = call_stack.back();
    auto & mem_state = call_state.GetMemory();

    // Assert that memory is empty.
    REQUIRE(hardware.GetExecStepper().GetMemoryModel().GetGlobalBuffer().empty());
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

  // SECTION ("Inst_If") {

  // }
  // SECTION ("Inst_While") {

  // }
  // SECTION ("Inst_Countdown") {

  // }
  // SECTION ("Inst_Break") {

  // }
  // SECTION ("Inst_Close") {

  // }
  // SECTION ("Inst_Call") {

  // }
  // SECTION ("Inst_Routine") {

  // }
  // SECTION ("Inst_Return") {

  // }
  // SECTION ("Inst_SetMem") {

  // }
  // SECTION ("Inst_CopyMem") {

  // }
  // SECTION ("Inst_SwapMem") {

  // }
  // SECTION ("Inst_InputToWorking") {

  // }
  // SECTION ("Inst_WorkingToOutput") {

  // }
  // SECTION ("Inst_WorkingToGlobal") {

  // }
  // SECTION ("Inst_GlobalToWorking") {

  // }
  // SECTION ("Inst_Fork") {

  // }
  // SECTION ("Inst_Terminate") {

  // }
  // SECTION ("Inst_Nop") {

  // }
}

/*
TEST_CASE( "SignalGP - v2", "[general]" ) {

  using mem_model_t = emp::sgp_v2::SimpleMemoryModel;
  using exec_stepper_t = emp::sgp_v2::SimpleExecutionStepper<mem_model_t>;
  using exec_state_t = typename exec_stepper_t::exec_state_t;
  using tag_t = typename exec_stepper_t::tag_t;
  using signalgp_t = emp::sgp_v2::SignalGP<exec_stepper_t>;
  using inst_lib_t = typename exec_stepper_t::inst_lib_t;
  using inst_t = typename exec_stepper_t::inst_t;
  using inst_prop_t = typename exec_stepper_t::inst_prop_t;
  using program_t = typename exec_stepper_t::program_t; // SimpleProgram<TAG_T, INST_ARGUMENT_T>

  inst_lib_t inst_lib;
  emp::EventLibrary<signalgp_t> event_lib;
  emp::Random random(2);

  // Add some instructions to the instruction library.
  inst_lib.AddInst("Nop", [](signalgp_t & hw, const inst_t & inst) { ; }, "No operation!");
  inst_lib.AddInst("ModuleDef", [](signalgp_t & hw, const inst_t & inst) { ; }, "Module definition", {inst_prop_t::MODULE});
  inst_lib.AddInst("Inc", emp::sgp_v2::inst_impl::Inst_Inc<signalgp_t, inst_t>, "Increment!");
  inst_lib.AddInst("Dec", emp::sgp_v2::inst_impl::Inst_Dec<signalgp_t, inst_t>, "Decrement!");
  inst_lib.AddInst("Not", emp::sgp_v2::inst_impl::Inst_Not<signalgp_t, inst_t>, "Logical not of ARG[0]");
  inst_lib.AddInst("Add", emp::sgp_v2::inst_impl::Inst_Add<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Sub", emp::sgp_v2::inst_impl::Inst_Sub<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Mult", emp::sgp_v2::inst_impl::Inst_Mult<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Div", emp::sgp_v2::inst_impl::Inst_Div<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Mod", emp::sgp_v2::inst_impl::Inst_Mod<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestEqu", emp::sgp_v2::inst_impl::Inst_TestEqu<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestNEqu", emp::sgp_v2::inst_impl::Inst_TestNEqu<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestLess", emp::sgp_v2::inst_impl::Inst_TestLess<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestLessEqu", emp::sgp_v2::inst_impl::Inst_TestLessEqu<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestGreater", emp::sgp_v2::inst_impl::Inst_TestGreater<signalgp_t, inst_t>, "");
  inst_lib.AddInst("TestGreaterEqu", emp::sgp_v2::inst_impl::Inst_TestGreaterEqu<signalgp_t, inst_t>, "");
  inst_lib.AddInst("If", emp::sgp_v2::inst_impl::Inst_If<signalgp_t, inst_t>, "");

  // Construct a program
  program_t program;
  program.PushInst(inst_lib, "Nop", {0, 0, 0}, {tag_t(), tag_t(), tag_t()});
  program.PushInst(inst_lib, "Inc", {0, 0, 0}, {tag_t(), tag_t(), tag_t()});
  program.PushInst(inst_lib, "Dec", {1, 0, 0}, {tag_t(), tag_t(), tag_t()});
  program.PushInst(inst_lib, "Sub", {0, 1, 2}, {tag_t(), tag_t(), tag_t()});
  program.PushInst(inst_lib, "Nop", {0, 0, 0}, {tag_t(), tag_t(), tag_t()});

  std::cout << "Constructing hardware." << std::endl;
  signalgp_t hardware(&event_lib, &random);
  std::cout << "=> Hardware constructed." << std::endl;

  std::cout << "Initializing execution stepper." << std::endl;
  hardware.InitExecStepper(&inst_lib, &random);
  std::cout << "=> Execution stepper initialized." << std::endl;

  hardware.SetPrintProgramFun([&hardware, &inst_lib](std::ostream & os) {
    program_t & program = hardware.GetProgram();
    for (size_t i = 0; i < program.GetSize(); ++i) {
      inst_t & inst = program[i];
      // Name[tags](args)
      os << inst_lib.GetName(inst.id);
      os << "[";
      for (size_t ti = 0; ti < inst.tags.size(); ++ti) {
        if (ti) os << ",";
        os << inst.tags[ti];
      }
      os << "](";
      for (size_t ai = 0; ai < inst.args.size(); ++ai) {
        if (ai) os << ",";
        os << inst.args[ai];
      }
      os << ")\n";
    }
  });
  hardware.SetPrintModulesFun([&hardware](std::ostream & os) {
    hardware.GetExecStepper().PrintModules();
  });
  hardware.SetPrintExecStepperStateFun([&hardware](std::ostream & os) {
    hardware.PrintModules(os);
    os << "\n";
    // todo - print matchbin state
    hardware.GetExecStepper().GetMemoryModel().PrintState(os);
  });
  hardware.SetPrintExecutionStateFun([&hardware](const exec_state_t & state, std::ostream & os) {
    hardware.GetExecStepper().PrintExecutionState(state, os);
  });
  hardware.SetPrintHardwareStateFun([&hardware](std::ostream & os) {
    // -- Print thread usage --
    hardware.PrintThreadUsage(os);
    os << "\n";
    // -- Print event queue --
    hardware.PrintEventQueue(os);
    os << "\n";
    // -- Print state of exec stepper --
    hardware.PrintExecStepperState(os);
    os << "\n";
    // -- Print thread states --
    hardware.PrintActiveThreadStates(os);
  });

  std::cout << "Loading program." << std::endl;
  hardware.SetProgram(program);
  std::cout << "=> Program loaded." << std::endl;
  hardware.SetThreadLimit(8);
  // Todo - print program to verify!
  // Todo - print modules to verify!
  std::cout << "======= MODULES =======" << std::endl;
  hardware.PrintModules(); std::cout << std::endl;
  std::cout << "=======================" << std::endl;
  std::cout << "======= PROGRAMS =======" << std::endl;
  hardware.PrintProgram();
  std::cout << "========================" << std::endl;

  // hardware.SingleProcess(); // This should do nothing!

  std::cout << "============= HARDWARE STATE =============" << std::endl;
  hardware.PrintHardwareState();
  std::cout << "==========================================" << std::endl;

  // Spawn a thread!
  std::cout << ">>Spawn a thread (module 0)" << std::endl;
  hardware.SpawnThread(0);
  std::cout << "============= HARDWARE STATE =============" << std::endl;
  hardware.PrintHardwareState();
  std::cout << "==========================================" << std::endl;

  hardware.SpawnThreads(tag_t(), 1);
  std::cout << ">>Spawn a thread (module ?)" << std::endl;
  std::cout << "============= HARDWARE STATE =============" << std::endl;
  hardware.PrintHardwareState();
  std::cout << "==========================================" << std::endl;

  while (true) {
    char cmd;
    std::cin >> cmd;
    if (cmd == 'q') {
      break;
    }
    hardware.SingleProcess();
    std::cout << "============= HARDWARE STATE =============" << std::endl;
    hardware.PrintHardwareState();
    std::cout << "==========================================" << std::endl;
  }
}
*/