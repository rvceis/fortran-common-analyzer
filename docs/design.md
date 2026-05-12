
## Objective

The objective of this project is to detect memory safety violations caused by inconsistent Fortran COMMON block declarations across multiple translation units.

---

# System Architecture

```text
Fortran Files
     ↓
Source Parser
     ↓
COMMON Block Extractor
     ↓
Global COMMON Database
     ↓
Validation Engine
     ↓
Diagnostics Reporter
Design Approach

The analyzer uses a multi-pass static-analysis pipeline:

Parse all source files
Extract variable declarations
Build COMMON block layouts
Compute memory sizes and offsets
Compare layouts across files
Report inconsistencies
Memory Layout Model

Each COMMON block is modeled as:

variable sequence
type information
byte offsets
total memory size
alignment requirements
Analysis Passes
Pass 1 — COMMON Collection

Collects:

COMMON name
variables
source file
declaration info
Pass 2 — Layout Construction

Builds:

variable offsets
memory sizes
alignment metadata
Pass 3 — Validation

Performs:

size mismatch analysis
type punning analysis
alignment validation
Alternatives Considered
Direct LLVM IR Analysis

Rejected initially because:

difficult for rapid prototyping
harder debugging
Full Flang AST Traversal

Planned for final integration after prototype stabilization.

Current Implementation Strategy

The current version uses regex-based parsing for:

faster prototyping
easier debugging
incremental validator development