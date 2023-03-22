#pragma once

#include "emp/base/vector.hpp"

#include "../../inst/BaseInstruction.hpp"

namespace sgp::cpu::linprg {

template<typename TAG_T, typename ARG_T>
struct Instruction : public inst::BaseInstruction {
  using base_t = inst::BaseInstruction;
  using arg_t = ARG_T;
  using tag_t = TAG_T;

  // size_t id;                      ///< Instruction ID
  emp::vector<arg_t> args;
  emp::vector<tag_t> tags;

  Instruction(
    size_t _id,
    const emp::vector<arg_t>& _args=emp::vector<arg_t>(),
    const emp::vector<tag_t>& _tags=emp::vector<tag_t>()
  ) :
    base_t(_id),
    args(_args),
    tags(_tags)
  { ; }

  bool operator==(const Instruction& other) const {
    return std::tie(id, args, tags) == std::tie(other.id, other.args, other.tags);
  }

  bool operator!=(const Instruction& other) const {
    return !(*this == other);
  }

  bool operator<(const Instruction& other) const {
    return std::tie(id, args, tags) < std::tie(other.id, other.args, other.tags);
  }

  emp::vector<arg_t>& GetArgs() { return args; }
  const emp::vector<arg_t>& GetArgs() const { return args; }
  emp::vector<tag_t>& GetTags() { return tags; }
  const emp::vector<tag_t>& GetTags() const { return tags; }

  const arg_t& GetArg(size_t i) const { return args[i]; }
  const tag_t& GetTag(size_t i) const { return tags[i]; }

  // Print each of the instruction's tag followed by the instruction and its arguments
  template<typename HARDWARE_T>
  void Print(
    std::ostream& out,
    const inst::InstructionLibrary<HARDWARE_T, Instruction>& ilib
  ) const {
    out << "\t";
    // Skip last tag & arg so we dont get an extra delimiter.
    out << "(";
    std::copy(tags.begin(), tags.end() - 1, std::ostream_iterator<tag_t>(out, ","));
    out << tags.back() << ") " << ilib.GetName(id) << " [";
    std::copy(args.begin(), args.end() - 1, std::ostream_iterator<arg_t>(out, ", "));
    out << args.back() << "]\n";
  }
};

}
