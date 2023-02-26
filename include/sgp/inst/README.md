# Instruction set implementations

The `sgp::inst` namespace contains implementations of common instructions for commonly used virtual CPU configurations. **NOTE** that instruction implementations are often specific particular CPU + memory model configuration.

Instruction sets:

- `sgp::inst::lfpbm` - Instruction implementations for the LinearFunctionsProgramCPU + BasicMemoryModel
- `sgp::inst::lpbm` - Instruction implementations for the LinearFunctionsProgramCPU + BasicMemoryModel