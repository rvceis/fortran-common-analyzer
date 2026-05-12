
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

The current prototype uses regex-based parsing.

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