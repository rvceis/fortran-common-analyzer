# Fortran COMMON Block Memory Safety Analyzer

A static-analysis tool for detecting unsafe memory usage patterns in legacy Fortran COMMON blocks.

This project analyzes multiple Fortran source files and validates COMMON block consistency across translation units. The analyzer detects memory safety issues such as:

- Size mismatches
- Type punning
- Alignment violations
- Inconsistent memory layouts
- Potential storage corruption

The project is inspired by real-world issues found in legacy scientific computing software where COMMON blocks are shared across independently developed modules.

---

# Features

## Implemented

- Multi-file Fortran COMMON block parser
- Global COMMON block database
- Variable declaration extraction
- Memory size computation
- COMMON layout generation
- Size mismatch detection
- Type punning detection
- Offset computation
- Alignment violation detection

## Planned

- EQUIVALENCE conflict analysis
- SAVE attribute consistency checking
- COMMON-to-MODULE migration advisor
- Flang AST integration
- JSON/HTML report generation

---

# Project Structure

```text
src/            -> analyzer source code
testcases/      -> seeded test programs
build/          -> compiled binaries
build.sh        -> build script
run.sh          -> execution script
# Fortran COMMON Block Memory Safety Analyzer

A static-analysis tool for detecting unsafe memory usage patterns in legacy Fortran COMMON blocks.

This project analyzes multiple Fortran source files and validates COMMON block consistency across translation units. The analyzer detects memory safety issues such as:

- Size mismatches
- Type punning
- Alignment violations
- Inconsistent memory layouts
- Potential storage corruption

The project is inspired by real-world issues found in legacy scientific computing software where COMMON blocks are shared across independently developed modules.

---

# Features

## Implemented

- Multi-file Fortran COMMON block parser
- Global COMMON block database
- Variable declaration extraction
- Memory size computation
- COMMON layout generation
- Size mismatch detection
- Type punning detection
- Offset computation
- Alignment violation detection

## Planned

- EQUIVALENCE conflict analysis
- SAVE attribute consistency checking
- COMMON-to-MODULE migration advisor
- Flang AST integration
- JSON/HTML report generation

---

# Project Structure

```text
src/            -> analyzer source code
testcases/      -> seeded test programs
build/          -> compiled binaries
build.sh        -> build script
run.sh          -> execution script