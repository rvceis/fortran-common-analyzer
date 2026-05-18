# Fortran COMMON Block Memory Safety Analyzer

A static-analysis tool for detecting unsafe memory usage patterns in legacy Fortran COMMON blocks.

This project analyzes multiple Fortran source files and validates COMMON block consistency across translation units. The analyzer detects memory safety violations such as:

- Size mismatches
- Type punning
- Alignment violations
- Structural layout incompatibilities
- SAVE attribute inconsistencies
- EQUIVALENCE-based storage aliasing

The tool also provides migration suggestions for converting legacy COMMON blocks into modern MODULE-based Fortran code.

---

# Features

## Implemented Features

- Multi-file COMMON block parser
- Global COMMON block database
- Variable declaration extraction
- Offset and layout generation
- Size mismatch detection
- Type punning detection
- Alignment analysis
- Structural layout comparison
- SAVE attribute consistency analysis
- EQUIVALENCE analysis
- Migration advisor

---

# Project Structure

```text
src/            -> Source code
testcases/      -> Fortran test programs
build/          -> Compiled binaries
docs/           -> Documentation
build.sh        -> Build script
run.sh          -> Run script
```

# Build Instructions
chmod +x build.sh
./build.sh

# Run Instructions
./run.sh testcases/*.f


# Example Output
ERROR: COMMON Block DATA has inconsistent sizes

WARNING: COMMON Block SHARED aliases different types

WARNING: Alignment violation detected

WARNING: EQUIVALENCE detected


# Technologies Used
- C++17
- LLVM 17 / Flang Compiler API
- Static Analysis
- Regex-based Parsing Prototype (Currently migrating to full AST)
- Compiler Design Concepts
- Memory Layout Validation


# Future Work
- Full transition from Regex to LLVM/Flang AST traversal in `main.cpp`
- Enhanced semantic analysis for complex Fortran grammar
- JSON diagnostics
- HTML reporting
- Automatic code refactoring
- CFG-based storage analysis