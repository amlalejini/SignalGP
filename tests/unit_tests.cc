#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

#include "tools/BitSet.h"

#include "InstructionLibrary.h"
#include "EventLibrary.h"
#include "SignalGP_v2.h"

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

  using label_t = emp::BitSet<8>;
  using mem_model_t = emp::sgp_v2::SimpleMemoryModel;
  using exec_stepper_t = emp::sgp_v2::SimpleExecutionStepper<mem_model_t, label_t>;
  using signalgp_t = emp::sgp_v2::SignalGP<exec_stepper_t>;

  emp::InstructionLibrary<signalgp_t, exec_stepper_t::inst_t> inst_lib;
  emp::EventLibrary<signalgp_t> event_lib;
  emp::Random random(2);

  std::cout << "Constructing hardware." << std::endl;
  signalgp_t hardware(&event_lib, &random);
  std::cout << "Hardware constructed." << std::endl;

  hardware.SingleProcess();

}
