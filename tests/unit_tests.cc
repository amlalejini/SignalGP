#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

#include "tools/BitSet.h"

#include "InstructionLibrary.h"
#include "EventLibrary.h"
#include "SGP-V2/SignalGP.h"
#include "SGP-V2/MemoryModel.h"
#include "SGP-V2/ExecutionStepper.h"

TEST_CASE( "Hello World", "[general]" ) {
  std::cout << "Hello tests!" << std::endl;
}

// TEST_CASE( "Instruction Library", "[general]" ) {
//   using SignalGP_t = emp::SignalGP<emp::SimpleProgram, emp::SimpleMemory, emp::SimpleEvent>;
//   emp::InstructionLibrary<SignalGP_t> inst_lib;
// }

// TEST_CASE( "Event Library", "[general]" ) {
//   using SignalGP_t = emp::SignalGP<emp::SimpleProgram, emp::SimpleMemory, emp::SimpleEvent>;
//   emp::EventLibrary<SignalGP_t> event_lib;
// }

// TEST_CASE( "SignalGP - v0", "[general]" ) {
//   using SignalGP_t = emp::SignalGP<emp::SimpleProgramModule, emp::SimpleMemoryModule, emp::SimpleEvent>;
//   emp::InstructionLibrary<SignalGP_t> inst_lib;
//   emp::EventLibrary<SignalGP_t> event_lib;
//   emp::Random random(2);

//   std::cout << "Constructing hardware." << std::endl;
//   SignalGP_t hardware(&inst_lib, &event_lib, &random);
//   std::cout << "Hardware constructed." << std::endl;

//   hardware.SingleProcess();

// }

TEST_CASE( "SignalGP - v2", "[general]" ) {
  // using emp::sgp_v2; /

  using mem_model_t = emp::sgp_v2::SimpleMemoryModel;
  using exec_stepper_t = emp::sgp_v2::SimpleExecutionStepper<mem_model_t>;
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

  // Construct a program
  program_t program;
  program.PushInst(inst_lib, "Nop", {0, 0, 0}, {tag_t(), tag_t(), tag_t()});
  program.PushInst(inst_lib, "Nop", {0, 0, 0}, {tag_t(), tag_t(), tag_t()});
  program.PushInst(inst_lib, "Nop", {0, 0, 0}, {tag_t(), tag_t(), tag_t()});
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

  std::cout << "Loading program." << std::endl;
  hardware.SetProgram(program);
  std::cout << "=> Program loaded." << std::endl;

  // Todo - print program to verify!
  // Todo - print modules to verify!
  std::cout << "======= MODULES =======" << std::endl;
  hardware.PrintModules(); std::cout << std::endl;
  std::cout << "=======================" << std::endl;
  std::cout << "======= PROGRAMS =======" << std::endl;
  hardware.PrintProgram();
  std::cout << "========================" << std::endl;
  hardware.SingleProcess();
}
