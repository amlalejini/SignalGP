# Development notes

For next iteration of refactoring:

- Unify instruction definitions
  - InstDef in instruction library
  - Each instruction's definition struct
    - Move away from static functions, use instruction definitions more directly in the instruction library
  - Centralize instruction properties (not all CPUs need to make use of all properties)
    - Continue to unify language -- Block vs. Flow, etc
- Clean up includes!
  - Many things included that do not need to be included as a result of refactoring, changing made during development, etc.
  - Also might be missing some includes in a few files because of refactoring