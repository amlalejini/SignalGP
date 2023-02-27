#pragma once

#include <iostream>
#include <utility>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/matching/MatchBin.hpp"
#include "emp/matching/matchbin_utils.hpp"
#include "emp/math/Range.hpp"

#include "../../EventLibrary.hpp"
#include "../../inst/InstructionLibrary.hpp"
#include "../../utils/random_utils.hpp"
#include "Instruction.hpp"

namespace sgp::cpu::linprg {

// Simple program:
// - Linear.
// - Instructions can have tag or numeric arguments.
template<
  typename TAG_T,
  typename ARGUMENT_T=int
>
class LinearProgram {
public:

  using tag_t = TAG_T;
  using arg_t = ARGUMENT_T;
  using inst_t = Instruction<tag_t, arg_t>;

protected:
  emp::vector<inst_t> inst_seq;

public:

  LinearProgram(
    const emp::vector<inst_t>& iseq=emp::vector<inst_t>()
  ) :
    inst_seq(iseq)
  { ; }

  LinearProgram(const LinearProgram&) = default;

  bool operator==(const LinearProgram& other) const {
    return inst_seq == other.inst_seq;
  }
  bool operator!=(const LinearProgram& other) const {
    return !(*this == other);
  }
  bool operator<(const LinearProgram& other) const {
    return inst_seq < other.inst_seq;
  }

  /// Allow program's instruction sequence to be indexed as if a vector.
  inst_t & operator[](size_t id) {
    emp_assert(id < inst_seq.size());
    return inst_seq[id];
  }

  /// Allow program's instruction sequence to be indexed as if a vector.
  const inst_t & operator[](size_t id) const {
    emp_assert(id < inst_seq.size());
    return inst_seq[id];
  }

  /// Clear the program's instruction sequence.
  void Clear() {
    inst_seq.clear();
  }

  /// Get program size.
  size_t GetSize() const { return inst_seq.size(); }

  /// Is a given position valid in this program?
  bool IsValidPosition(size_t pos) const { return pos < GetSize(); }

  /// Set program's instruction sequence to the one given.
  void SetProgram(const emp::vector<inst_t>& p) { inst_seq = p; }

  /// Push instruction to instruction set.
  /// - No validation! We're trusting that 'id' is legit!
  void PushInst(
    size_t id,
    const emp::vector<arg_t>& args=emp::vector<arg_t>(),
    const emp::vector<tag_t>& tags=emp::vector<tag_t>()
  ) {
    inst_seq.emplace_back(id, args, tags);
  }

  /// Push instruction to program by name.
  template<typename HARDWARE_T>
  void PushInst(
    const inst::InstructionLibrary<HARDWARE_T, inst_t>& ilib,
    const std::string& name,
    const emp::vector<arg_t>& args=emp::vector<arg_t>(),
    const emp::vector<tag_t>& tags=emp::vector<tag_t>()
  ) {
    emp_assert(ilib.IsInst(name), "Unknown instruction name", name);
    PushInst(ilib.GetID(name), args, tags);
  }

  /// Push instruction to program.
  void PushInst(const inst_t& inst) {
    inst_seq.emplace_back(inst);
  }

  /// Is the given instruction valid?
  template<typename HARDWARE_T>
  static bool IsValidInst(
    const inst::InstructionLibrary<HARDWARE_T, inst_t>& ilib,
    const inst_t& inst
  ) {
    return inst.id < ilib.GetSize();
  }

  // Print each instruction out
  template<typename HARDWARE_T>
  void Print(
    std::ostream& out,
    const inst::InstructionLibrary<HARDWARE_T, inst_t>& ilib
  ) const {
    for(auto const& inst : inst_seq){
      inst.Print(out, ilib);
    }
  }

};

//////////////////////////////////////////////////////////////////////////////
// LinearProgram utilities

/// Generate random instruction.
template<typename HARDWARE_T, size_t TAG_WIDTH>
Instruction<emp::BitSet<TAG_WIDTH>, int> GenRandInst(
  emp::Random& rnd,
  const inst::InstructionLibrary<
    HARDWARE_T,
    Instruction<emp::BitSet<TAG_WIDTH>, int>
  >& inst_lib,
  size_t num_tags=1,
  size_t num_args=3,
  const emp::Range<int>& arg_val_range={0, 15}
) {
  emp_assert(inst_lib.GetSize() > 0, "Instruction library must have at least one instruction definition before being used to generate a random instruction.");
  emp::vector<int> args(num_args);
  for (size_t i = 0; i < args.size(); ++i) {
    args[i] = rnd.GetInt(arg_val_range.GetLower(), arg_val_range.GetUpper()+1);
  }
  return {
    rnd.GetUInt(inst_lib.GetSize()),
    args,
    sgp::utils::RandomBitSets<TAG_WIDTH>(rnd, num_tags)
  };
}

/// @brief Generate a random linear program.
/// @tparam HARDWARE_T
/// @tparam TAG_WIDTH
/// @param rnd
/// @param inst_lib
/// @param inst_cnt_range
/// @param num_inst_tags
/// @param num_inst_args
/// @param arg_val_range
/// @return
template<typename HARDWARE_T, size_t TAG_WIDTH>
LinearProgram<emp::BitSet<TAG_WIDTH>, int> GenRandLinearProgram(
  emp::Random& rnd,
  const inst::InstructionLibrary<
    HARDWARE_T,
    Instruction<emp::BitSet<TAG_WIDTH>, int>
  >& inst_lib,
  const emp::Range<size_t>& inst_cnt_range={1, 32},
  size_t num_inst_tags=1,
  size_t num_inst_args=3,
  const emp::Range<int>& arg_val_range={0, 15}
) {
  emp_assert(
    inst_lib.GetSize() > 0,
    "Instruction library must have at least one instruction definition before being used to generate a random instruction."
  );
  LinearProgram<emp::BitSet<TAG_WIDTH>, int> new_program;
  size_t inst_cnt = rnd.GetUInt(inst_cnt_range.GetLower(), inst_cnt_range.GetUpper()+1);
  for (size_t i = 0; i < inst_cnt; ++i) {
    new_program.PushInst(
      GenRandInst<HARDWARE_T, TAG_WIDTH>(
        rnd,
        inst_lib,
        num_inst_tags,
        num_inst_args,
        arg_val_range
      )
    );
  }
  return new_program;
}

}