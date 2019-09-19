#ifndef EMP_INSTRUCTION_LIBRARY_H
#define EMP_INSTRUCTION_LIBRARY_H

#include <map>
#include <unordered_set>
#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/map_utils.h"
#include "tools/string_utils.h"

// Comments:
// - @AML: Why are instruction functions stored in multiple places? (inst def & fun_call lib)

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

namespace emp {

  template<typename HARDWARE_T>
  class InstructionLibrary {

  public:
    using hardware_t = HARDWARE_T;
    using instruction_t = typename hardware_t::instruction_t;
    using instruction_fun_t = std::function<void(hardware_t &, const instruction_t &)>;

    struct InstructionDef {
      std::string name;           ///< Name of this instruction.
      instruction_fun_t fun_call; ///< Function to call when instruction is executed.
      std::string desc;           ///< Description of instruction.
      // Maybe need an instruction category?

      InstructionDef(const std::string & _name, instruction_fun_t _fun_call, const std::string & _desc)
        : name(_name), fun_call(_fun_call), desc(_desc) { ; }
      InstructionDef(const InstructionDef &) = default;
    };

  protected:

    emp::vector<InstructionDef> inst_lib;      ///< Full definitions for instructions.
    std::map<std::string, size_t> name_map;    ///< How do names link to instructions?

  public:

    InstructionLibrary() : inst_lib(), name_map() { ; }
    InstructionLibrary(const InstructionLibrary &) = delete;
    InstructionLibrary(InstructionLibrary &&) = delete;
    ~InstructionLibrary() { ; }

    InstructionLibrary & operator=(const InstructionLibrary &) = default;                   ///< Copy Operator
    InstructionLibrary & operator=(InstructionLibrary &&) = default;

    /// Return the name associated with the specified instruction ID.
    const std::string & GetName(size_t id) const { return inst_lib[id].name; }

    /// Return the function associated with the specified instruction ID.
    const instruction_fun_t & GetFunction(size_t id) const { return inst_lib[id].fun_call; }

    /// Return the provided description for the provided instruction ID.
    const std::string & GetDesc(size_t id) const { return inst_lib[id].desc; }

    /// Get the number of instructions in this set.
    size_t GetSize() const { return inst_lib.size(); }

    /// Retrieve a unique letter associated with the specified instruction ID.
    static constexpr char GetSymbol(size_t id) {
      if (id < 26) return ('a' + id);
      if (id < 52) return ('A' + (id - 26));
      if (id < 62) return ('0' + (id - 52));
      return '+';
    }

    /// Return the ID of the instruction associated with the specified symbol.
    static constexpr size_t GetID(char symbol) {
      if (symbol >= 'a' && symbol <= 'z') return (size_t) (symbol - 'a');
      if (symbol >= 'A' && symbol <= 'Z') return (size_t) (symbol - 'A' + 26);
      if (symbol >= '0' && symbol <= '9') return (size_t) (symbol - '0' + 52);
      return (size_t) 62;
    }

    /// Is the given instruction (specified by name) in the instruction library?
    bool IsInst(const std::string & name) const {
        return Has(name_map, name);
    }

    /// Return the ID of the instruction that has the specified name.
    size_t GetID(const std::string & name) const {
      emp_assert(Has(name_map, name), name);
      return Find(name_map, name, (size_t) -1);
    }

    /// Add a new instruction to the instruction set.
    void AddInst(const std::string & name,
                 const instruction_fun_t & fun_call,
                 const std::string & desc="") {

      const size_t id = inst_lib.size();
      inst_lib.emplace_back(name, fun_call, desc);
      name_map[name] = id;
    }

    /// Process a specified instruction in the provided hardware.
    void ProcessInst(hardware_t & hw, const instruction_t & inst) const {
      inst_lib[inst.GetID()].fun_call(hw, inst);
    }

    /// Process a specified instruction on hardware that can be converted to the correct type.
    template <typename IN_HW>
    void ProcessInst(emp::Ptr<IN_HW> hw, const instruction_t & inst) const {
      emp_assert( dynamic_cast<hardware_t*>(hw.Raw()) );
      inst_lib[inst.id].fun_call(*(hw.template Cast<hardware_t>()), inst);
    }
  };
}

#endif