#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "InstructionLibrary.h"
#include "EventLibrary.h"
#include "SignalGP_v0.h"

TEST_CASE( "Hello World", "[general]" ) {
  std::cout << "Hello tests!" << std::endl;
}

TEST_CASE( "Instruction Library", "[general]" ) {
  emp::InstructionLibrary<emp::SignalGP> inst_lib;
}

TEST_CASE( "Event Library", "[general]" ) {
  emp::EventLibrary<emp::SignalGP> event_lib;
}