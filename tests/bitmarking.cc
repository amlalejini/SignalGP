#include <iostream>
#include <chrono>         // For std::clock
#include <array>
#include <tuple>
#include <map>

#include "base/vector.h"

#include "tools/BitSet.h"
#include "tools/BitVector.h"

#include "data/DataFile.h"

#include "tools/Random.h"
#include "tools/random_utils.h"

#include <thread>

// Replicates - 100

// Tests - Sizes
// - 16 Bit
// - 32 Bit
// - 64 Bit
// - 128 Bit
// - 256 Bit

// Tests - Logic operations
// - ==
// - <
// - >
// - GetUInt
// - CountOnes_Mixed
// - NOT
// - AND
// - OR
// - NAND
// - AND
// - NOR
// - XOR
// - EQU
// - SHIFT

size_t SEED = 2;
size_t NUM_REPLICATES = 50;    // How many replicates should we do?
size_t NUM_ITERATIONS = 10000;  // How many times to repeat each operation before measuring time elapsed?


class Benchmark {
public:


private:
  size_t num_iterations;
  size_t num_replicates;

  struct Info {
    size_t replicate=0;
    std::string treatment="";
    size_t bits=0;
    std::string operation="";
    double time=0.0;
    size_t n=0;
    double value=0.0;
  } info;

  emp::DataFile datafile;
  emp::Random random;

  emp::vector< std::tuple<emp::BitVector,emp::BitVector> > bit_vectors;
  emp::vector< std::tuple<emp::BitSet<16>,emp::BitSet<16>> > bit_set_16;
  emp::vector< std::tuple<emp::BitSet<31>,emp::BitSet<31>> > bit_set_31;
  emp::vector< std::tuple<emp::BitSet<32>,emp::BitSet<32>> > bit_set_32;
  emp::vector< std::tuple<emp::BitSet<64>,emp::BitSet<64>> > bit_set_64;
  emp::vector< std::tuple<emp::BitSet<128>,emp::BitSet<128>> > bit_set_128;
  emp::vector< std::tuple<emp::BitSet<256>,emp::BitSet<256>> > bit_set_256;
  emp::vector< std::tuple<emp::BitSet<10000>,emp::BitSet<10000>> > bit_set_10000;


  // Test bitvectors
  template<size_t NUM_BITS>
  void BenchmarkBitVectors() {

    // Resize all bit vectors
    const size_t width = NUM_BITS;
    info.bits = width;
    for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
      std::get<0>(bit_vectors[i]).Resize(width);
      std::get<1>(bit_vectors[i]).Resize(width);
    }

    // Compute timings for each replicate.
    for (size_t rep = 0; rep < num_replicates; ++rep) {
      info.replicate = rep;
      // Do bit vector:
      info.treatment = "bit_vector";
      // (1) Randomize bit_vectors
      for (size_t i = 0; i < num_iterations; ++i) {
        emp::RandomizeBitVector(std::get<0>(bit_vectors[i]), random);
        emp::RandomizeBitVector(std::get<1>(bit_vectors[i]), random);
      }
      emp::BitVector recipient;

      // (2) Time each operation!

      // ------------ CountOnes_Mixed ------------
      info.operation = "CountOnes_Mixed";
      auto start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
        size_t num_ones = std::get<0>(bit_vectors[i]).CountOnes_Mixed();
        info.value = (double)num_ones;
      }
      auto end_time = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ NOT ------------
      info.operation = "NOT";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).NOT();
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ AND ------------
      info.operation = "AND";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).AND(std::get<1>(bit_vectors[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ OR ------------
      info.operation = "OR";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).OR(std::get<1>(bit_vectors[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ NAND ------------
      info.operation = "NAND";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).NAND(std::get<1>(bit_vectors[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();


      // ------------ NOR ------------
      info.operation = "NOR";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).NOR(std::get<1>(bit_vectors[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ XOR ------------
      info.operation = "XOR";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).XOR(std::get<1>(bit_vectors[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ EQU ------------
      info.operation = "EQU";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).EQU(std::get<1>(bit_vectors[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ SHIFT ------------
      info.operation = "R_SHIFT_0.25l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).SHIFT(width/4);
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "R_SHIFT_0.5l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).SHIFT(width/2);
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "R_SHIFT_1l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).SHIFT(width);
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "L_SHIFT_0.25l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).SHIFT(-1*((int)width/4));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "L_SHIFT_0.5l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).SHIFT(-1*((int)width/2));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "L_SHIFT_1l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_vectors.size(); ++i) {
         recipient = std::get<0>(bit_vectors[i]).SHIFT(-1*(int)width);
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();
    }
  }

  // Test bit sets
  template<size_t NUM_BITS>
  void BenchmarkBitSets(emp::vector< std::tuple<emp::BitSet<NUM_BITS>,emp::BitSet<NUM_BITS>> > & bit_sets) {

    // Resize all bit vectors
    const size_t width = NUM_BITS;
    info.bits = width;

    // Compute timings for each replicate.
    for (size_t rep = 0; rep < num_replicates; ++rep) {
      info.replicate = rep;
      // Do bit set:
      info.treatment = "bit_set";
      // (1) Randomize bit_sets
      for (size_t i = 0; i < num_iterations; ++i) {
        std::get<0>(bit_sets[i]).Randomize(random);
        std::get<1>(bit_sets[i]).Randomize(random);
      }
      emp::BitSet<NUM_BITS> recipient;

      // (2) Time each operation!
      // ------------ CountOnes_Mixed ------------
      info.operation = "CountOnes_Mixed";
      auto start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
        size_t num_ones = std::get<0>(bit_sets[i]).CountOnes_Mixed();
        info.value = (double)num_ones;
      }
      auto end_time = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ NOT ------------
      info.operation = "NOT";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).NOT();
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ AND ------------
      info.operation = "AND";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).AND(std::get<1>(bit_sets[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ OR ------------
      info.operation = "OR";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).OR(std::get<1>(bit_sets[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ NAND ------------
      info.operation = "NAND";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).NAND(std::get<1>(bit_sets[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();


      // ------------ NOR ------------
      info.operation = "NOR";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).NOR(std::get<1>(bit_sets[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ XOR ------------
      info.operation = "XOR";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).XOR(std::get<1>(bit_sets[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ EQU ------------
      info.operation = "EQU";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).EQU(std::get<1>(bit_sets[i]));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      // ------------ SHIFT ------------
      info.operation = "R_SHIFT_0.25l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).SHIFT(width/4);
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "R_SHIFT_0.5l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).SHIFT(width/2);
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "R_SHIFT_1l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).SHIFT(width);
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "L_SHIFT_0.25l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).SHIFT(-1*((int)width/4));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "L_SHIFT_0.5l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).SHIFT(-1*((int)width/2));
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();

      info.operation = "L_SHIFT_1l";
      start_time = std::chrono::high_resolution_clock::now();
      for (size_t i = 0; i < bit_sets.size(); ++i) {
         recipient = std::get<0>(bit_sets[i]).SHIFT(-1*(int)width);
      }
      end_time = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
      info.time = duration;
      datafile.Update();
    }
  }

public:
  Benchmark(size_t seed, size_t n_iterations, size_t n_replicates)
    : num_iterations(n_iterations), num_replicates(n_replicates),
      #ifdef EMP_NDEBUG
        datafile("performance.csv"),
      #else
        datafile("performance-debug.csv"),
      #endif
      random(seed),
      bit_vectors(NUM_ITERATIONS),
      bit_set_16(NUM_ITERATIONS),
      bit_set_31(NUM_ITERATIONS),
      bit_set_32(NUM_ITERATIONS),
      bit_set_64(NUM_ITERATIONS),
      bit_set_128(NUM_ITERATIONS),
      bit_set_256(NUM_ITERATIONS),
      bit_set_10000(NUM_ITERATIONS)
  {
    info.n = NUM_ITERATIONS;

    // -- Setup data file --
    // File headers:
    // - rep        => replicate ID
    // - treatment  => treatment ID
    // - operation  => operation
    // - time       => time (s)
    // - n          => number iterations
    datafile.AddFun<size_t>([this]() { return info.replicate; }, "replicate_id");
    datafile.AddFun<std::string>([this]() { return info.treatment; }, "treatment");
    datafile.AddFun<size_t>([this]() { return info.bits; }, "bits");
    datafile.AddFun<std::string>([this]() { return info.operation; }, "operation");
    datafile.AddFun<double>([this]() { return info.time; }, "time");
    datafile.AddFun<size_t>([this]() { return info.n; }, "iterations");
    #ifdef EMP_NDEBUG
      datafile.AddFun<bool>([this]() { return true; }, "compile_time_optimizations");
    #else
      datafile.AddFun<bool>([this]() { return false; }, "compile_time_optimizations");
    #endif


    datafile.PrintHeaderKeys();
  }

  void Run() {
    BenchmarkBitVectors<31>();
    BenchmarkBitSets<31>(bit_set_31);

    BenchmarkBitVectors<16>();
    BenchmarkBitSets<16>(bit_set_16);

    BenchmarkBitVectors<32>();
    BenchmarkBitSets<32>(bit_set_32);

    BenchmarkBitVectors<64>();
    BenchmarkBitSets<64>(bit_set_64);

    // BenchmarkBitVectors<128>();
    // BenchmarkBitSets<128>(bit_set_128);

    // BenchmarkBitVectors<256>();
    // BenchmarkBitSets<256>(bit_set_256);

    // BenchmarkBitVectors<10000>();
    // BenchmarkBitSets<10000>(bit_set_10000);
  }

};

int main() {
  Benchmark benchmark(SEED, NUM_ITERATIONS, NUM_REPLICATES);
  benchmark.Run();
}