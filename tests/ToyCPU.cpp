#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <utility>
#include <iostream>
#include <string>

#include "emp/math/Random.hpp"
#include "emp/base/vector.hpp"

#include "sgp/cpu/ToyCPU.hpp"

TEST_CASE("Toy SignalGP", "[general]") {
  using signalgp_t = sgp::cpu::ToyCPU<size_t>;
  using event_lib_t = typename signalgp_t::event_lib_t;

  event_lib_t event_lib;
  emp::Random random(2);
  signalgp_t hardware(event_lib);

  // Configure hardware
  hardware.SetActiveThreadLimit(16);
  // Make a toy program.
  emp::vector<size_t> prog1({0,1,2,3,4,5});
  // Load program onto hardware.
  hardware.SetProgram(prog1);
  hardware.SetPrintHardwareStateFun(
    [](const signalgp_t & hw, std::ostream & os) {
      os << "Thread states: [";
      for (size_t ti = 0; ti < hw.GetThreadExecOrder().size(); ++ti) {
        const size_t thread_id = hw.GetThreadExecOrder()[ti];
        if (!hw.GetThread(thread_id).IsRunning()) continue;
        if (ti) os << ", ";
        os << "{" << thread_id << ": " << hw.GetThread(thread_id).GetExecState().value << "}";
      }
      os << "]\n";
    }
  );

  std::cout << "--- Initial hardware state ---" << std::endl;
  hardware.PrintHardwareState();
  // Spawn a few threads.
  hardware.SpawnThreadWithID(0);
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

// Test SignalGP thread management (using Toy virtual hardware)
// todo - test priority
TEST_CASE("Thread Management (Toy SignalGP)") {
  using signalgp_t = sgp::cpu::ToyCPU<size_t>;
  using event_lib_t = typename signalgp_t::event_lib_t;

  event_lib_t event_lib;
  emp::Random random(2);
  signalgp_t hardware(event_lib);

  // Configure hardware
  hardware.SetActiveThreadLimit(8);
  REQUIRE(hardware.ValidateThreadState());
  hardware.SetThreadCapacity(16);
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetThreads().size() >= 8);
  REQUIRE(hardware.GetThreads().size() <= 16);

  // Make a toy program.
  emp::vector<size_t> prog1({1, 5, 10, 20, 50, 100});
  // Load program onto hardware.
  hardware.SetProgram(prog1);
  hardware.SetPrintHardwareStateFun(
    [](const signalgp_t & hw, std::ostream & os) {
      os << "Thread states: [";
      for (size_t ti = 0; ti < hw.GetThreadExecOrder().size(); ++ti) {
        const size_t thread_id = hw.GetThreadExecOrder()[ti];
        if (!hw.GetThread(thread_id).IsRunning()) continue;
        if (ti) os << ", ";
        os << "{" << thread_id << ": " << hw.GetThread(thread_id).GetExecState().value << "}";
      }
      os << "]\n";
    }
  );

  //////////////////////////////////////////////////////////////////////////////
  // Test that things start empty
  REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetUnusedThreadIDs().size() >= 8);
  REQUIRE(hardware.GetUnusedThreadIDs().size() <= 16);
  REQUIRE(hardware.GetThreadExecOrder().size() == 0);

  //////////////////////////////////////////////////////////////////////////////
  // Test - spawn thread while active & pending < limit
  std::cout << "Test - Spawn thread while active & pending < limit" << std::endl;
  hardware.SpawnThreadWithID(0); // Thread should be pending.
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
  REQUIRE(hardware.GetThreadExecOrder().size() == 0);
  hardware.SingleProcess(); // Thread will get activated and run 1 step (1=>0).
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 1);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 1);
  hardware.SingleProcess(); // Thread will be marked as dead (and cleaned up).
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 0);

  //////////////////////////////////////////////////////////////////////////////
  // Test - spawn thread while active == limit
  std::cout << "Test - Spawn thread while active == limit" << std::endl;
  hardware.ResetHardware();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 0);
  for (size_t i = 0; i < 8; ++i) {
    hardware.SpawnThreadWithID(1);
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.GetPendingThreadIDs().size() == i+1);
    REQUIRE(hardware.GetThreadExecOrder().size() == 0);
  }
  hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);
  hardware.SpawnThreadWithID(1);
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);
  hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);

  //////////////////////////////////////////////////////////////////////////////
  // Test - spawn thread while active & pending == limit
  std::cout << "Test - Spawn thread while active & pending == limit" << std::endl;
  hardware.ResetHardware();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 0);
  for (size_t i = 0; i < 32; ++i) {
    hardware.SpawnThreadWithID(1);
    size_t expected_pending = (i+1 > 16) ? 16 : i + 1;
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.GetPendingThreadIDs().size() == expected_pending);
    REQUIRE(hardware.GetThreadExecOrder().size() == 0);
  }
  hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);
  hardware.SpawnThreadWithID(1);
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 1);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);
  hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);
  for (size_t i = 0; i < 4; ++i) hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 0);

  //////////////////////////////////////////////////////////////////////////////
  // Test - spawn thread while active & pending == limit
  hardware.ResetHardware();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 0);
  for (size_t i = 0; i < 32; ++i) {
    hardware.SpawnThreadWithID(1, 1);
    size_t expected_pending = (i+1 > 16) ? 16 : i + 1;
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.GetPendingThreadIDs().size() == expected_pending);
    REQUIRE(hardware.GetThreadExecOrder().size() == 0);
  }
  hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);

  std::cout << "Test - Spawn different priority threads while active == limit" << std::endl;
  hardware.SpawnThreadWithID(2, 100);
  hardware.SpawnThreadWithID(2, -1);
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 2);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);
  hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);
  for (size_t i = 0; i < 4; ++i) hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 1);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 1);

  //////////////////////////////////////////////////////////////////////////////
  // Test - spawn thread while active & pending == limit
  hardware.ResetHardware();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 0);
  for (size_t i = 0; i < 32; ++i) {
    hardware.SpawnThreadWithID(1, 1);
    size_t expected_pending = (i+1 > 16) ? 16 : i + 1;
    REQUIRE(hardware.ValidateThreadState());
    REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
    REQUIRE(hardware.GetPendingThreadIDs().size() == expected_pending);
    REQUIRE(hardware.GetThreadExecOrder().size() == 0);
  }
  hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);
  // mark all active threads as dead
  for (size_t id : hardware.GetActiveThreadIDs()) {
    hardware.GetThread(id).SetDead();
  }
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 8);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 8);
  hardware.SingleProcess();
  REQUIRE(hardware.ValidateThreadState());
  REQUIRE(hardware.GetActiveThreadIDs().size() == 0);
  REQUIRE(hardware.GetPendingThreadIDs().size() == 0);
  REQUIRE(hardware.GetThreadExecOrder().size() == 0);
}
