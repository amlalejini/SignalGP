# Code Review Notes

## Questions/Discussion Points

- SignalGP.h
  - how could I use concepts to clean this up?
  - SignalGP: where should I put configurable lambdas/Signals?
    - would these play nice with copy operators?
  - @discussion - template/organization structure
  - can I assert that execution stepper knows about SignalGP_t?
  - @discussion - any reason signalgp needs to know this (module_t)?
  - @discussion - any reason for signalgp to know about memory model?
  - @discussion - any reason top-level signalgp needs to know about matchbins?
  - Pros/Cons of nesting Thread type in SignalGP class
  - InitExecStepper
    - Best way to handle initializing execution stepper?
- ExecStepper.h
  - // @discussion - SGP_CUSTOM_COMPONENT_T? Gross, but necessary?
  - @discussion - do lambdas with captures by reference (e.g., this) work appropriately
              when copied? => probably not...