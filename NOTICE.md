# Notice and modification record

This project incorporates the NUN_DX STM32F103 library attributed in its source files to YCZ / 杨长治. The integrated source is under `legacy/nun_dx_original/components/nun_dx/`; original README material and source hashes are retained for traceability.

Integration modifications made from 2026-09-29 through 2026-10-01 include:

- merged the dart load-cell CubeMX board project with the NUN_DX module library;
- replaced the RTOS-dependent load-cell task path with direct bare-metal Modbus polling;
- added a stable application API and a narrow board port abstraction;
- allowed NUN_DX I2C/UART drivers to attach to CubeMX-owned HAL handles;
- made optional interrupt handlers coexist with CubeMX-generated vectors;
- added central feature switches and per-module resource selection;
- fixed compiler/linker compatibility issues found during GCC and Keil builds;
- created a new Keil project plus a GCC/CMake build;
- restored matching official HAL files that were absent from the supplied archive;
- removed duplicate projects, unused platform examples, incompatible STM32F4 drafts and generated artifacts;
- added deployment, pinout, module and compliance documentation.

Original authors and third-party copyright holders retain all rights stated in their files. See `LICENSE` and `THIRD_PARTY_NOTICES.md`.

