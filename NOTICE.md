# Notice and modification record

This project incorporates the NUN_DX STM32F103 library attributed in its source files to YCZ / 杨长治 and preserves the original source snapshot under `legacy/nun_dx_original/`.

Integration modifications made on 2026-09-29 include:

- merged the dart load-cell CubeMX/FreeRTOS board project with the NUN_DX module library;
- added a stable application API and a narrow board port abstraction;
- allowed NUN_DX I2C/UART drivers to attach to CubeMX-owned HAL handles;
- made optional interrupt handlers coexist with CubeMX-generated vectors;
- added central feature switches and per-module resource selection;
- fixed compiler/linker compatibility issues found during GCC and Keil builds;
- created a new Keil project plus a GCC/CMake build;
- restored matching official HAL and FreeRTOS port files that were absent from the supplied archive;
- added deployment, pinout, module and compliance documentation.

Original authors and third-party copyright holders retain all rights stated in their files. See `LICENSE` and `THIRD_PARTY_NOTICES.md`.

