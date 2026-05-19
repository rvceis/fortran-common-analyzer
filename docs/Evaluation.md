# EVALUATION

# Test Cases

| Test | Purpose | Result |
|---|---|---|
| TC1 (`file1.f`, `file2.f`) | Basic COMMON and size mismatch | PASS |
| TC2 (`file3.f`, `file4.f`) | Type punning across files | PASS |
| TC3 (`file5.f`, `file6.f`) | Structural layout and alignment | PASS |
| TC4 (`file8.f`, `file9.f`) | SAVE attribute consistency | PASS |
| TC5 (`file10.f`, `fileD.f`, `fileE.f`) | EQUIVALENCE mapping and complex aliasing | PASS |
| TC6 (`fileC.f`) | Complex grammar (DO loops, IF) AST skipping | PASS |
| TC7 (`fileF.f`, `fileG.f`) | Unnamed / implicitly typed COMMON blocks | PASS |

---

# Metrics

| Metric | Value |
|---|---|
| Files analyzed | 16 |
| COMMON blocks analyzed | 11 |
| Errors detected | 3 |
| Warnings generated | 19 |
| Analysis Time | ~25 ms |

---

# Detected Violations

## Size Mismatch

Detected inconsistent memory sizes across translation units.

## Type Punning

Detected incompatible type aliasing in shared storage.

## Alignment Violations

Detected improperly aligned DOUBLE PRECISION variables.

---

# Comparison Against Naive Approach

| Feature | Naive Parsing | Our Analyzer |
|---|---|---|
| Multi-file analysis | No | Yes |
| Layout analysis | No | Yes |
| Offset tracking | No | Yes |
| Type validation | No | Yes |
| Alignment checking | No | Yes |

---

# Limitations

- Flang AST dump parsing relies on textual layout from `DumpTree` rather than native C++ Visitor nodes.
- Limited semantic analysis; control flow and procedure boundaries are not yet deeply inspected.

---

# Future Evaluation

Planned evaluation on:
- Full grammar parsing using the newly linked LLVM/Flang libraries.
- Validation on LAPACK and BLAS.
- Legacy scientific Fortran codebases.