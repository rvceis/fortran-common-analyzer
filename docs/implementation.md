
# IMPLEMENTATION DETAILS

# Data Structures

## VariableInfo

```cpp
struct VariableInfo {
    string name;
    string type;
    int count;
    int totalSize;
    int offset;
    int alignment;
};

Stores:

variable type
memory size
alignment requirements
computed offset
CommonEntry
struct CommonEntry {
    string fileName;
    string blockName;
    vector<VariableInfo> vars;
};

Represents a complete COMMON block layout.

Parsing Strategy

The analyzer fully utilizes the LLVM Flang compiler API (`src/flang_parser.cpp`). It invokes `Fortran::parser::DumpTree` to serialize the Fortran AST into a highly structured string representation, which is then parsed by `main.cpp` to extract variables, `COMMON` blocks, `EQUIVALENCE`, and `SAVE` attributes.

Declaration Parsing

Supported declarations:

REAL X(100)
INTEGER I
DOUBLE PRECISION D
COMMON Parsing

Supported COMMON format:

COMMON /BLOCK/ A, B, C
Memory Layout Algorithm

Offsets are computed sequentially:

offset += variable.totalSize;

Example layout:

Variable	Offset
A	0
B	4
C	8
Validators
Size Mismatch Detector

Detects:

incompatible COMMON block sizes
Type Punning Detector

Detects:

incompatible type aliasing
Alignment Analyzer

Detects:

misaligned DOUBLE PRECISION storage
Complexity

For N COMMON declarations:

Parsing: O(N)
Validation: O(N²) worst-case