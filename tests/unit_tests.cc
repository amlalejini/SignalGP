#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

#include <limits>

#include "tools/BitSet.h"

#include "InstructionLibrary.h"
#include "EventLibrary.h"

#include "BaseSignalGP.h"
#include "LinearProgramSignalGP.h"
#include "instructions_impl.h"
#include "ToySignalGP.h"
#include "MemoryModel.h"

TEST_CASE( "Hello World", "[general]" ) {
  std::cout << "Hello tests!" << std::endl;
}

TEST_CASE("Toy SignalGP", "[general]") {
  using signalgp_t = ToySignalGP<size_t>;
  using event_lib_t = typename signalgp_t::event_lib_t;


  event_lib_t event_lib;

  emp::Random random(2);

  signalgp_t hardware(&event_lib);

  // Configure hardware
  hardware.SetThreadLimit(128);
  // Make a toy program.
  emp::vector<size_t> prog1({0,1,2,3,4,5});
  // Load program onto hardware.
  hardware.SetProgram(prog1);
  hardware.SetPrintHardwareStateFun([](const signalgp_t & hw, std::ostream & os) {
    os << "Thread states: [";
    for (size_t ti = 0; ti < hw.GetActiveThreadIDs().size(); ++ti) {
      if (ti) os << ", ";
      size_t thread_id = hw.GetActiveThreadIDs()[ti];
      os << "{" << thread_id << ": " << hw.GetThread(thread_id).GetExecState().value << "}";
    }
    os << "]\n";
  });

  std::cout << "--- Initial hardware state ---" << std::endl;
  hardware.PrintHardwareState();
  // Spawn a few threads.
  hardware.SpawnThread(0);
  hardware.SpawnThreads(1, 2);
  hardware.SpawnThreads(6, 1);
  // print hardware state.
  std::cout << "--- after spawning threads ---" << std::endl;
  hardware.PrintHardwareState();
  std::cout << "--- single process ---" << std::endl;
  hardware.SingleProcess();
  hardware.PrintHardwareState();
  std::cout << "--- single process ---" << std::endl;
  hardware.SingleProcess();
  hardware.PrintHardwareState();
  std::cout << "--- single process ---" << std::endl;
  hardware.SingleProcess();
  hardware.PrintHardwareState();
  std::cout << "!!" << std::endl;
}

TEST_CASE("SignalGP", "[general]") {
  using mem_model_t = emp::signalgp::SimpleMemoryModel;
  using signalgp_t = emp::signalgp::LinearProgramSignalGP<mem_model_t,
                                                          emp::BitSet<16>,
                                                          int,
                                                          emp::MatchBin< size_t, emp::HammingMetric<16>, emp::RankedSelector<std::ratio<16+8, 16> >>,
                                                          emp::signalgp::DefaultCustomComponent>;
  using inst_lib_t = typename signalgp_t::inst_lib_t;
  using inst_t = typename signalgp_t::inst_t;
  using inst_prop_t = typename signalgp_t::InstProperty;

  using event_lib_t = typename signalgp_t::event_lib_t;
  using program_t = typename signalgp_t::program_t;
  using tag_t = typename signalgp_t::tag_t;

  using mem_buffer_t = typename mem_model_t::mem_buffer_t;

  inst_lib_t inst_lib;
  event_lib_t event_lib;

  // Add some instructions to the instruction library.
  inst_lib.AddInst("Nop", [](signalgp_t & hw, const inst_t & inst) { ; }, "No operation!");
  // inst_lib.AddInst("ModuleDef", [](signalgp_t & hw, const inst_t & inst) { ; }, "Module definition", {inst_prop_t::MODULE});
  inst_lib.AddInst("Inc", emp::signalgp::inst_impl::Inst_Inc<signalgp_t, inst_t>, "Increment!");
  // inst_lib.AddInst("Dec", emp::signalgp::inst_impl::Inst_Dec<signalgp_t, inst_t>, "Decrement!");
  // inst_lib.AddInst("Not", emp::signalgp::inst_impl::Inst_Not<signalgp_t, inst_t>, "Logical not of ARG[0]");
  // inst_lib.AddInst("Add", emp::signalgp::inst_impl::Inst_Add<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("Sub", emp::signalgp::inst_impl::Inst_Sub<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("Mult", emp::signalgp::inst_impl::Inst_Mult<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("Div", emp::signalgp::inst_impl::Inst_Div<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("Mod", emp::signalgp::inst_impl::Inst_Mod<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("TestEqu", emp::signalgp::inst_impl::Inst_TestEqu<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("TestNEqu", emp::signalgp::inst_impl::Inst_TestNEqu<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("TestLess", emp::signalgp::inst_impl::Inst_TestLess<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("TestLessEqu", emp::signalgp::inst_impl::Inst_TestLessEqu<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("TestGreater", emp::signalgp::inst_impl::Inst_TestGreater<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("TestGreaterEqu", emp::signalgp::inst_impl::Inst_TestGreaterEqu<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("If", emp::signalgp::inst_impl::Inst_If<signalgp_t, inst_t>, "", {inst_prop_t::BLOCK_DEF});
  // inst_lib.AddInst("SetMem", emp::signalgp::inst_impl::Inst_SetMem<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("While", emp::signalgp::inst_impl::Inst_While<signalgp_t, inst_t>, "", {inst_prop_t::BLOCK_DEF});
  // inst_lib.AddInst("Countdown", emp::signalgp::inst_impl::Inst_Countdown<signalgp_t, inst_t>, "", {inst_prop_t::BLOCK_DEF});
  // inst_lib.AddInst("Break", emp::signalgp::inst_impl::Inst_Break<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("Close", emp::signalgp::inst_impl::Inst_Close<signalgp_t, inst_t>, "", {inst_prop_t::BLOCK_CLOSE});
  // inst_lib.AddInst("Call", emp::signalgp::inst_impl::Inst_Call<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("Routine", emp::signalgp::inst_impl::Inst_Routine<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("Return", emp::signalgp::inst_impl::Inst_Return<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("CopyMem", emp::signalgp::inst_impl::Inst_CopyMem<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("SwapMem", emp::signalgp::inst_impl::Inst_SwapMem<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("InputToWorking", emp::signalgp::inst_impl::Inst_InputToWorking<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("WorkingToOutput", emp::signalgp::inst_impl::Inst_WorkingToOutput<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("WorkingToGlobal", emp::signalgp::inst_impl::Inst_WorkingToGlobal<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("GlobalToWorking", emp::signalgp::inst_impl::Inst_GlobalToWorking<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("Fork", emp::signalgp::inst_impl::Inst_Fork<signalgp_t, inst_t>, "");
  // inst_lib.AddInst("Terminate", emp::signalgp::inst_impl::Inst_Terminate<signalgp_t, inst_t>, "");


  emp::Random random(2);

  signalgp_t hardware(random, &inst_lib, &event_lib);
  hardware.SetThreadLimit(64);

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

}

/*
TEST_CASE( "SignalGP_V2::LinearProgram::SimpleMemory - Default Instructions", "[instructions]" ) {
  // General setup.
  using mem_model_t = emp::sgp_v2::SimpleMemoryModel;
  using exec_stepper_t = emp::sgp_v2::LinearProgramExecutionStepper<mem_model_t>;
  using exec_state_t = typename exec_stepper_t::exec_state_t;
  // using tag_t = typename exec_stepper_t::tag_t;
  using signalgp_t = emp::sgp_v2::SignalGP<exec_stepper_t>;
  using inst_lib_t = typename exec_stepper_t::inst_lib_t;
  using inst_t = typename exec_stepper_t::inst_t;
  using inst_prop_t = typename exec_stepper_t::inst_prop_t;
  using program_t = typename exec_stepper_t::program_t; // SimpleProgram<TAG_T, INST_ARGUMENT_T>
  using tag_t = typename signalgp_t::tag_t;

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
  inst_lib.AddInst("If", emp::sgp_v2::inst_impl::Inst_If<signalgp_t, inst_t>, "", {inst_prop_t::BLOCK_DEF});
  inst_lib.AddInst("SetMem", emp::sgp_v2::inst_impl::Inst_SetMem<signalgp_t, inst_t>, "");
  inst_lib.AddInst("While", emp::sgp_v2::inst_impl::Inst_While<signalgp_t, inst_t>, "", {inst_prop_t::BLOCK_DEF});
  inst_lib.AddInst("Countdown", emp::sgp_v2::inst_impl::Inst_Countdown<signalgp_t, inst_t>, "", {inst_prop_t::BLOCK_DEF});
  inst_lib.AddInst("Break", emp::sgp_v2::inst_impl::Inst_Break<signalgp_t, inst_t>, "");
  inst_lib.AddInst("Close", emp::sgp_v2::inst_impl::Inst_Close<signalgp_t, inst_t>, "", {inst_prop_t::BLOCK_CLOSE});
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
    hardware.SpawnThread(0);
    size_t thread_id = hardware.GetActiveThreadIDs()[0];
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);

    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
    hardware.SingleProcess();
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
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);
    thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Nop", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {4, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Close", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {4, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Close", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {3, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Close", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {0, 0, 0});
    program.PushInst(inst_lib, "Inc", {4, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "Close", {5, 0, 0});
    program.PushInst(inst_lib, "Inc", {5, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    program.PushInst(inst_lib, "If",  {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);
    size_t thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);
    thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);
    thread_id = hardware.GetActiveThreadIDs()[0];
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
    hardware.SpawnThread(0);
    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "Inc",   {5, 0, 0});
    program.PushInst(inst_lib, "Nop",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",   {5, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",   {3, 0, 0});
    program.PushInst(inst_lib, "While", {3, 0, 0});
    program.PushInst(inst_lib, "Inc",   {4, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc",     {5, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc",     {4, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Inc",     {5, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {3, 0, 0});

    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "Inc",     {5, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {4, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "Inc",     {5, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "If",      {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {7, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "Close",   {0, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {1, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "If",      {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {7, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "Close",   {0, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {0, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "Countdown",   {3, 0, 0});
    program.PushInst(inst_lib, "Inc", {3, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    size_t thread_id = hardware.GetActiveThreadIDs()[0];
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);

    hardware.SingleProcess(); // Inc(3)
    hardware.SingleProcess(); // While(true)
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
    // Build program to test inc instruction.
    program.PushInst(inst_lib, "If",      {1, 0, 0});
    program.PushInst(inst_lib, "Break",     {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {7, 0, 0});
    program.PushInst(inst_lib, "Close",   {5, 0, 0});
    program.PushInst(inst_lib, "Inc",     {6, 0, 0});
    program.PushInst(inst_lib, "Close",   {0, 0, 0});
    program.PushInst(inst_lib, "Inc",     {8, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",     {3, 0, 0});
    program.PushInst(inst_lib, "While",   {1, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    size_t thread_id = hardware.GetActiveThreadIDs()[0];
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
    program.PushInst(inst_lib, "If",        {1, 0, 0});
    program.PushInst(inst_lib, "Break",     {0, 0, 0});
    program.PushInst(inst_lib, "Inc",       {7, 0, 0});
    program.PushInst(inst_lib, "Close",     {5, 0, 0});
    program.PushInst(inst_lib, "Inc",       {6, 0, 0});
    program.PushInst(inst_lib, "Close",     {0, 0, 0});
    program.PushInst(inst_lib, "Inc",       {8, 0, 0});
    program.PushInst(inst_lib, "ModuleDef", {0, 0, 0}, {tag_t()});
    program.PushInst(inst_lib, "Inc",       {3, 0, 0});
    program.PushInst(inst_lib, "If",        {1, 0, 0});
    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    thread_id = hardware.GetActiveThreadIDs()[0];
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

    // Build program to test inc instruction.
    tag_t zeros, ones;
    ones.SetUInt(0, (uint32_t)-1);
    // SetUInt
    program.PushInst(inst_lib, "ModuleDef",  {0, 0, 0}, {zeros});
    program.PushInst(inst_lib,   "SetMem", {2, 2});
    program.PushInst(inst_lib,   "SetMem", {3, 3});
    program.PushInst(inst_lib,   "Call", {0, 0, 0}, {ones});

    program.PushInst(inst_lib, "ModuleDef",  {0, 0, 0}, {ones});
    program.PushInst(inst_lib,   "InputToWorking", {2, 1, 0});
    program.PushInst(inst_lib,   "InputToWorking", {3, 2, 0});
    program.PushInst(inst_lib,   "Inc", {1, 0, 0});
    program.PushInst(inst_lib,   "Inc", {2, 0, 0});
    program.PushInst(inst_lib,   "WorkingToOutput", {1, 4, 0});
    program.PushInst(inst_lib,   "WorkingToOutput", {2, 5, 0});

    // Load program on hardware.
    hardware.SetProgram(program);
    // Spawn a thread to run the program.
    hardware.SpawnThread(0);

    size_t thread_id = hardware.GetActiveThreadIDs()[0];
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(0, 0);
    hardware.GetThread(thread_id).GetExecState().GetTopCallState()
            .GetMemory().SetWorking(1, 1);

    for (size_t i = 0; i < 10; ++i) hardware.SingleProcess();
    REQUIRE(hardware.GetThread(thread_id).GetExecState().GetTopCallState().GetMemory().working_mem
        == mem_buffer_t({{4, 3.0}, {1, 1.0}, {0, 0.0}, {5, 4.0}, {2, 2.0}, {3, 3.0}}));
    ////////////////////////////////////////////////////////////////////////////
  }

  // SECTION ("Inst_Routine") {

  // }

  // SECTION ("Inst_Return") {

  // }

  // SECTION ("Inst_CopyMem") {

  // }

  // SECTION ("Inst_SwapMem") {

  // }

  // SECTION ("Inst_WorkingToGlobal") {

  // }

  // SECTION ("Inst_GlobalToWorking") {

  // }

  // SECTION ("Inst_Fork") {

  // }

  // SECTION ("Inst_Terminate") {

  // }
}
*/
