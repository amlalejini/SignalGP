#ifndef EMP_SIGNALGP_V0_H
#define EMP_SIGNALGP_V0_H

/**
 * Terminology
 *
 * References
*/

// Some thoughts on terminology:
// - events = labeled payload
//   - What is the label? And, what is the payload? (want to be flexible if possible)
// - programs contain 'instructions'
//   - Instructions know how they are executed

// Things that SignalGP may need to be templated on:
// - event library type (==> event type)
// - instruction library type (==> instruction type)
// - program type

namespace emp {

  struct SimpleInstruction {
    size_t id;

    size_t GetID() const { return id; }
  };

  struct SimpleEvent {
    size_t id;

    size_t GetID() const { return id; }
  };

  class SignalGP {
  public:
    using instruction_t = SimpleInstruction;
    using event_t = SimpleEvent;

  protected:

  public:

  };

}

#endif