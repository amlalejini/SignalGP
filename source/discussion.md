# Code Review Notes

## Questions/Discussion Points

## Discussion Points

- Goal: solidify interface. Want to start using. Okay for us to improve internals
  as we go.

- How should SignalGP virtual hardware be named?
- How should namespaces/file structure/etc be named & organized?
  - e.g., signalgp namespace? instruction implementations for a particular
    hardware loadout?
- Should implementations of functions be at bottom of file? Pro: cleans up class
  definition. Con: implementation signatures are DISGUSTING (because of template madness).

- Getting into Empirical master
  - Do we even want to? Would it be better to have a SignalGP stand-alone repository?
    - It's nice to have it packaged with Empirical. But, hardware directory will
      get really gross.
  - What's the status on MatchBin? Can we get it merged in?
  - What to do with EventDrivenGP? => Move to OLD? Leave alone? Conflicts with updated
    versions of things like

- Implementation details
  - BaseSignalGP
    - Big downside: Template hell.
    - Hard cap on total thread space (pending + active) threads
    - Where do we want configurable lambdas/emp::Signals (for data collect, modification, etc)?
      - think emp::World signals like, OnBirth, OnPlacement, etc
    - CUSTOM_COMPONENT_T
      - Arg for including it in Base class: want all SignalGP classes to have capacity
        for this.
      - Arg against: would clean up templates ever so slightly. Derived classes
        can make this decision for themselves (but this is sort of annoying).
    - struct Thread
      - Nested in BaseSignalGP (don't really want the type to bleed out of scope)
    - Thread management
      - Data structures for each.
      - Management interface.
      - Management visibility.
      - Management consolidation.

    - Can concepts be used to clean anything up?