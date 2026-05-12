# DESIGN DOCUMENT

# Objective

The objective of this project is to detect unsafe memory usage patterns caused by inconsistent COMMON block declarations across multiple Fortran translation units.

---

# System Architecture

```text
Fortran Source Files
        ↓
Source Parser
        ↓
COMMON Block Extractor
        ↓
Global COMMON Database
        ↓
Memory Layout Builder
        ↓
Validation Engine
        ↓
Diagnostics + Migration Advisor
Design Approach

The analyzer uses a multi-pass static-analysis pipeline.

Pass 1 — Metadata Collection

Collects:

SAVE attributes
EQUIVALENCE relations
Pass 2 — Semantic Parsing

Extracts:

variable declarations
COMMON blocks
types
array sizes
offsets
Pass 3 — Validation

Performs:

size mismatch analysis
type punning analysis
alignment analysis
structural layout validation
SAVE consistency analysis
EQUIVALENCE analysis
Memory Layout Model

Each COMMON block is represented as:

variable sequence
type information
total size
byte offsets
alignment requirements
Design Decisions
Regex-Based Prototype

The current implementation uses regex-based parsing for:

rapid prototyping
easier debugging
incremental development
Modular Analysis Passes

Each validator is implemented independently:

improves maintainability
simplifies debugging
supports future LLVM integration
Alternatives Considered
Full Flang AST Traversal

Planned for future work.

Rejected initially because:

higher implementation complexity
slower prototyping
LLVM IR Analysis

Rejected because:

COMMON-level semantics are easier to analyze before IR lowering
Current Limitations
Partial Fortran grammar support
Prototype-level parser
No CFG analysis
No interprocedural flow analysis