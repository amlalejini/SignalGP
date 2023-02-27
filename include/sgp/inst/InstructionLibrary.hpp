#pragma once

#include <map>
#include <unordered_set>
#include <string>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/control/Signal.hpp"

// Requirements:
// - instruction_t MUST have a valid GetID() function

// Instruction library associates instruction IDs with definitions
// - Pretty name
// - unique ID
// - arguments (e.g., arguments)
// - fun_call (what does it do when executed?)
// - category(ies?)
//   - Instructions can have a broad type (tracked by an internal inst type library)

// Hardware specifies instruction type? => specifies argument type?

namespace sgp::inst {

/// Special properties that can be associated with an instruction definition.
enum class InstProperty {
  MODULE,
  BLOCK_CLOSE,
  BLOCK_DEF
};

template<typename HARDWARE_T, typename INSTRUCTION_T>
struct InstructionDef {
  using inst_fun_t = std::function<void(HARDWARE_T&, const INSTRUCTION_T&)>;
  std::string name;     ///< Name of this instruction.
  std::string desc;     ///< Description of the instruction.
  inst_fun_t fun_call;  ///< Function to call when the instruction is executed.
  std::unordered_set<InstProperty> properties; ///< Properties specific to this instruction.

  InstructionDef(
    const std::string& _name,
    inst_fun_t _fun_call,
    const std::string& _desc,
    const std::unordered_set<InstProperty>& _properties={}
  ) :
    name(_name),
    desc(_desc),
    fun_call(_fun_call),
    properties(_properties)
  { ; }

  InstructionDef(const InstructionDef&) = default;
};

template<typename INST_SPEC_T,typename INST_DEF_T>
INST_DEF_T BuildInstructionDef() {
  return {
    INST_SPEC_T::name(),
    INST_SPEC_T::run,
    INST_SPEC_T::desc(),
    INST_SPEC_T::properties()
  };
}

template<typename HARDWARE_T, typename INSTRUCTION_T>
class InstructionLibrary {
public:

  using hardware_t = HARDWARE_T;
  using inst_t = INSTRUCTION_T;
  using inst_fun_t = std::function<void(hardware_t&, const inst_t&)>;
  using inst_prop_t = InstProperty;
  using inst_def_t = InstructionDef<HARDWARE_T, INSTRUCTION_T>;

protected:

  emp::vector<inst_def_t> inst_lib;      ///< Full definitions for instructions.
  std::map<std::string, size_t> name_map;    ///< How do names link to instructions?
  emp::Signal<void(hardware_t&, const inst_t&)> before_inst_exec;

public:

  InstructionLibrary() : inst_lib(), name_map(), before_inst_exec() { ; }
  InstructionLibrary(const InstructionLibrary&) = delete;
  InstructionLibrary(InstructionLibrary&&) = delete;
  ~InstructionLibrary() { ; }

  InstructionLibrary & operator=(const InstructionLibrary&) = default; ///< Copy Operator
  InstructionLibrary & operator=(InstructionLibrary&&) = default;

  /// Remove all instructions from the instruction library.
  void Clear() {
    inst_lib.clear();
    name_map.clear();
  }

  /// Return the name associated with the specified instruction ID.
  const std::string& GetName(size_t id) const { return inst_lib[id].name; }

  /// Return the function associated with the specified instruction ID.
  const inst_fun_t& GetFunction(size_t id) const { return inst_lib[id].fun_call; }

  /// Return the provided description for the provided instruction ID.
  const std::string& GetDesc(size_t id) const { return inst_lib[id].desc; }

  /// Get the number of instructions in this set.
  size_t GetSize() const { return inst_lib.size(); }

  /// Does instruction have a particular property?
  bool HasProperty(size_t id, const inst_prop_t& prop) {
    emp_assert(id < GetSize());
    return inst_lib[id].properties.count(prop);
  }

  /// Is the given instruction (specified by name) in the instruction library?
  bool IsInst(const std::string& name) const {
    return emp::Has(name_map, name);
  }

  /// Return the ID of the instruction that has the specified name.
  size_t GetID(const std::string& name) const {
    emp_assert(emp::Has(name_map, name), name);
    return emp::Find(name_map, name, (size_t) -1);
  }

  /// Add a new instruction to the instruction set.
  void AddInst(
    const std::string& name,
    const inst_fun_t& fun_call,
    const std::string& desc="",
    const std::unordered_set<inst_prop_t>& properties=std::unordered_set<inst_prop_t>()
  ) {
    const size_t id = inst_lib.size();
    inst_lib.emplace_back(name, fun_call, desc, properties);
    name_map[name] = id;
  }

  void AddInst(
    const inst_def_t& definition
  ) {
    const size_t id = inst_lib.size();
    inst_lib.emplace_back(definition);
    name_map[definition.name] = id;
  }

  template<typename INST_SPEC_T>
  void AddInst(
    const INST_SPEC_T& inst_spec
  ) {
    AddInst(
      INST_SPEC_T::name(),
      INST_SPEC_T::run,
      INST_SPEC_T::desc(),
      INST_SPEC_T::properties()
    );
  }

  template<typename INST_SPEC_T>
  void AddInst() {
    AddInst(
      INST_SPEC_T::name(),
      INST_SPEC_T::run,
      INST_SPEC_T::desc(),
      INST_SPEC_T::properties()
    );
  }

  /// Process a specified instruction in the provided hardware.
  void ProcessInst(hardware_t& hw, const inst_t& inst) {
    before_inst_exec.Trigger(hw, inst);
    inst_lib[inst.GetID()].fun_call(hw, inst);
  }

  /// Process a specified instruction on hardware that can be converted to the correct type.
  template <typename IN_HW>
  void ProcessInst(emp::Ptr<IN_HW> hw, const inst_t& inst) {
    emp_assert( dynamic_cast<hardware_t*>(hw.Raw()) );
    ProcessInst(*(hw.template Cast<hardware_t>()), inst);
  }

  emp::SignalKey OnBeforeInstExec(
    const std::function<void(hardware_t&, const inst_t&)>& fun
  ) {
    return before_inst_exec.AddAction(fun);
  }

  void ResetBeforeInstExecSignal() {
    before_inst_exec.Clear();
  }

};

} // End sgp namespace