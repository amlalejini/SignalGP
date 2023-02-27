#pragma once

#include <utility>

namespace sgp::inst {

struct BaseInstruction {
  size_t id;
  BaseInstruction(size_t _id) : id(_id) { ; }

  size_t GetID() const { return id; }
  void SetID(size_t new_id) { id = new_id; }
};

}