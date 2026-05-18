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
Full Flang C++ AST Traversal

The project previously used regex and is now driven by parsing the Flang AST dump. Writing a full C++ `Visitor` pattern over the heavily templated Flang AST was deemed too complex for this lab scope when `DumpTree` provides identical structural accuracy.

LLVM IR Analysis

Rejected because:

COMMON-level semantics are easier to analyze before IR lowering

Current Limitations
While the Flang parser perfectly extracts AST declarations, the analyzer relies on parsing the `DumpTree` string output instead of using native C++ template Visitor patterns.
No CFG analysis
No interprocedural flow analysis