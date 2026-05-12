#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <chrono>
using namespace std;

// ============================================================
// DATA STRUCTURES
// ============================================================

struct VariableInfo {
    string name;
    string type;
    int count = 1;
    int totalSize = 0;
    int offset = 0;
    int alignment = 4;
    int lineNumber = 0;
};

struct CommonEntry {
    string fileName;
    string blockName;
    vector<VariableInfo> vars;
    bool hasSave = false;
};

// ============================================================
// GLOBAL DATABASES
// ============================================================

vector<CommonEntry> commonDB;
map<string, vector<CommonEntry>> groupedBlocks;
struct EquivalenceInfo {

    string fileName;
    string left;
    string right;
};

vector<EquivalenceInfo> equivalenceDB;
// ============================================================
// UTILITY FUNCTIONS
// ============================================================

int getTypeSize(const string &type) {

    if(type == "REAL")
        return 4;

    if(type == "INTEGER")
        return 4;

    if(type == "DOUBLE PRECISION")
        return 8;

    return 4;
}

int getAlignment(const string &type) {

    if(type == "DOUBLE PRECISION")
        return 8;

    return 4;
}

int getCommonSize(const CommonEntry &entry) {

    int total = 0;

    for(const auto &v : entry.vars)
        total += v.totalSize;

    return total;
}

string buildLayoutSignature(const CommonEntry &entry) {

    string sig;

    for(const auto &v : entry.vars) {

        sig += v.type;
        sig += ":";
        sig += to_string(v.offset);
        sig += ":";
        sig += to_string(v.totalSize);
        sig += ";";
    }

    return sig;
}

void printHeader(const string &title) {

    cout << "\n===== "
         << title
         << " =====\n";
}

// ============================================================
// SINGLE GENERIC REPORT FUNCTION
// ============================================================

void printAnalysis(
    const string &category,
    const vector<string> &messages
) {

    printHeader(category);

    if(messages.empty()) {

        cout << "No issues detected.\n";
        return;
    }

    for(const auto &msg : messages)
        cout << msg << "\n";
}

// ============================================================
// ANALYSIS PASSES
// ============================================================

vector<string> analyzeSizeMismatch() {

    vector<string> results;

    for(const auto &block : groupedBlocks) {

        const auto &entries = block.second;

        if(entries.size() < 2)
            continue;

        int expectedSize =
            getCommonSize(entries[0]);

        bool mismatch = false;

        for(size_t i = 1; i < entries.size(); i++) {

            if(getCommonSize(entries[i])
                != expectedSize) {

                mismatch = true;
                break;
            }
        }

        if(mismatch) {

            string msg =
                "\nERROR: COMMON Block " +
                block.first +
                " has inconsistent sizes\n";

            for(const auto &e : entries) {

                msg += "  File: " +
                       e.fileName +
                       " -> " +
                       to_string(getCommonSize(e)) +
                       " bytes\n";
            }

            results.push_back(msg);
        }
    }

    return results;
}

vector<string> analyzeTypePunning() {

    vector<string> results;

    for(const auto &block : groupedBlocks) {

        const auto &entries = block.second;

        if(entries.size() < 2)
            continue;

        for(size_t i = 0; i < entries.size(); i++) {

            for(size_t j = i + 1;
                j < entries.size();
                j++) {

                const auto &vars1 =
                    entries[i].vars;

                const auto &vars2 =
                    entries[j].vars;

                size_t limit =
                    min(vars1.size(),
                        vars2.size());

                for(size_t k = 0;
                    k < limit;
                    k++) {

                    if(vars1[k].type
                        != vars2[k].type) {

                        string msg =
                            "\nWARNING: COMMON Block " +
                            block.first +
                            " aliases different types\n";

                        msg += "  Conflict:\n";

                        msg += "    " +
                               entries[i].fileName +
                               " -> " +
                               vars1[k].name +
                               " (" +
                               vars1[k].type +
                               ")\n";

                        msg += "    " +
                               entries[j].fileName +
                               " -> " +
                               vars2[k].name +
                               " (" +
                               vars2[k].type +
                               ")\n";

                        results.push_back(msg);
                    }
                }
            }
        }
    }

    return results;
}

vector<string> analyzeAlignment() {

    vector<string> results;

    for(const auto &entry : commonDB) {

        for(const auto &v : entry.vars) {

            if(v.offset % v.alignment != 0) {

                string msg =
                    "\nWARNING: Alignment violation detected\n";

                msg += "  File: " +
                       entry.fileName +
                       "\n";

                msg += "  COMMON Block: " +
                       entry.blockName +
                       "\n";

                msg += "  Variable: " +
                       v.name +
                       "\n";

                msg += "  Type: " +
                       v.type +
                       "\n";

                msg += "  Offset: " +
                       to_string(v.offset) +
                       "\n";

                msg += "  Required Alignment: " +
                       to_string(v.alignment) +
                       "\n";

                results.push_back(msg);
            }
        }
    }

    return results;
}

vector<string> analyzeStructuralLayouts() {

    vector<string> results;

    for(const auto &block : groupedBlocks) {

        const auto &entries = block.second;

        if(entries.size() < 2)
            continue;

        string baseLayout =
            buildLayoutSignature(entries[0]);

        for(size_t i = 1;
            i < entries.size();
            i++) {

            string currentLayout =
                buildLayoutSignature(entries[i]);

            if(baseLayout != currentLayout) {

                string msg =
                    "\nWARNING: COMMON Block " +
                    block.first +
                    " has incompatible layouts\n";

                msg += "  Reference File: " +
                       entries[0].fileName +
                       "\n";

                msg += "  Conflicting File: " +
                       entries[i].fileName +
                       "\n";

                results.push_back(msg);
            }
        }
    }

    return results;
}

vector<string> analyzeSaveConsistency() {

    vector<string> results;

    for(const auto &block : groupedBlocks) {

        const auto &entries = block.second;

        if(entries.size() < 2)
            continue;

        bool reference =
            entries[0].hasSave;

        bool mismatch = false;

        for(size_t i = 1;
            i < entries.size();
            i++) {

            if(entries[i].hasSave
                != reference) {

                mismatch = true;
                break;
            }
        }

        if(mismatch) {

            string msg =
                "\nWARNING: COMMON Block " +
                block.first +
                " has inconsistent SAVE usage\n";

            for(const auto &e : entries) {

                msg += "  File: " +
                       e.fileName +
                       " -> ";

                msg += (e.hasSave)
                    ? "SAVE enabled\n"
                    : "SAVE missing\n";
            }

            results.push_back(msg);
        }
    }

    return results;
}

// ============================================================
// FIXED EQUIVALENCE ANALYSIS
// ============================================================
vector<string> analyzeEquivalence() {

    vector<string> results;

    for(const auto &eq : equivalenceDB) {

        for(const auto &entry : commonDB) {

            // SAME FILE ONLY
            if(entry.fileName != eq.fileName)
                continue;

            for(const auto &v : entry.vars) {

                if(v.name == eq.left) {

                    string msg =
                        "\nWARNING: EQUIVALENCE detected\n";

                    msg += "  File: " +
                           entry.fileName +
                           "\n";

                    msg += "  COMMON Block: " +
                           entry.blockName +
                           "\n";

                    msg += "  Variable: " +
                           eq.left +
                           "\n";

                    msg += "  Aliased With: " +
                           eq.right +
                           "\n";

                    results.push_back(msg);
                }
            }
        }
    }

    return results;
}


vector<string> generateMigrationAdvice() {

    vector<string> advice;

    for(const auto &block : groupedBlocks) {

        const auto &entries = block.second;

        if(entries.size() < 2)
            continue;

        bool issueDetected = false;

        // Check size mismatch
        int baseSize =
            getCommonSize(entries[0]);

        for(size_t i = 1; i < entries.size(); i++) {

            if(getCommonSize(entries[i])
                != baseSize) {

                issueDetected = true;
            }
        }

        // Check layout mismatch
        string baseLayout =
            buildLayoutSignature(entries[0]);

        for(size_t i = 1; i < entries.size(); i++) {

            string currentLayout =
                buildLayoutSignature(entries[i]);

            if(currentLayout != baseLayout) {

                issueDetected = true;
            }
        }

        if(issueDetected) {

            string msg =
                "\nMIGRATION ADVICE FOR COMMON BLOCK: "
                + block.first + "\n";

            msg +=
                "  Recommendation:\n";

            msg +=
                "  - Replace COMMON block with MODULE\n";

            msg +=
                "  - Use explicit type declarations\n";

            msg +=
                "  - Remove storage aliasing\n";

            msg +=
                "  - Ensure identical layouts across files\n";

            msg +=
                "\n  Suggested MODULE:\n";

            msg +=
                "  MODULE "
                + block.first +
                "_MOD\n";

            for(const auto &v :
                entries[0].vars) {

                msg += "      "
                    + v.type
                    + " :: "
                    + v.name;

                if(v.count > 1) {

                    msg += "("
                        + to_string(v.count)
                        + ")";
                }

                msg += "\n";
            }

            msg +=
                "  END MODULE\n";

            advice.push_back(msg);
        }
    }

    return advice;
}

void printStatistics() {

    int totalWarnings = 0;
    int totalErrors = 0;

    totalErrors += analyzeSizeMismatch().size();

    totalWarnings += analyzeTypePunning().size();
    totalWarnings += analyzeAlignment().size();
    totalWarnings += analyzeStructuralLayouts().size();
    totalWarnings += analyzeSaveConsistency().size();
    totalWarnings += analyzeEquivalence().size();

    printHeader("ANALYSIS STATISTICS");

    cout << "Files analyzed: "
         << commonDB.size()
         << "\n";

    cout << "COMMON blocks analyzed: "
         << groupedBlocks.size()
         << "\n";

    cout << "Warnings generated: "
         << totalWarnings
         << "\n";

    cout << "Errors generated: "
         << totalErrors
         << "\n";
}

// ============================================================
// GLOBAL DATABASE PRINTER
// ============================================================

void printGlobalDatabase() {

    printHeader("GLOBAL COMMON DATABASE");

    for(const auto &entry : commonDB) {

        cout << "\nFile: "
             << entry.fileName
             << "\n";

        cout << "COMMON Block: "
             << entry.blockName
             << "\n\n";

        for(const auto &v : entry.vars) {

            cout << "  Variable: "
                 << v.name
                 << "\n";

            cout << "  Type: "
                 << v.type
                 << "\n";

            cout << "  Offset: "
                 << v.offset
                 << "\n";

            cout << "  Size: "
                 << v.totalSize
                 << " bytes\n\n";
        }
    }
}

// ============================================================
// MAIN
// ============================================================

int main(int argc, char* argv[]) {
    auto start =
    chrono::high_resolution_clock::now();
    if(argc < 2) {

        cout << "Usage: ./analyzer <files>\n";
        return 1;
    }

    regex commonRegex(
        R"(COMMON\s*/(\w+)/\s*(.*))"
    );

    regex declRegex(
        R"((REAL|INTEGER|DOUBLE PRECISION)\s+(\w+)(\((\d+)\))?)"
    );

    regex saveRegex(
        R"(SAVE\s*/(\w+)/)"
    );

    regex equivalenceRegex(
    R"(EQUIVALENCE\s*\(\s*([A-Za-z]\w*)\s*,\s*([A-Za-z]\w*)\s*\))",
    regex_constants::icase
);

    for(int i = 1; i < argc; i++) {

        cout << "\nProcessing File: "
             << argv[i]
             << "\n";

        ifstream file(argv[i]);

        if(!file) {

            cout << "Cannot open file\n";
            continue;
        }

        vector<string> lines;
        string line;

        while(getline(file, line))
            lines.push_back(line);

        file.close();

        map<string, VariableInfo> variableTable;
        // map<string, string> equivalenceMap;
        set<string> saveBlocks;

        // PASS 1 → SAVE

       


        for(const auto &line : lines) {

            smatch saveMatch;

            if(regex_search(
                line,
                saveMatch,
                saveRegex
            )) {

                saveBlocks.insert(
                    saveMatch[1]
                );
            }
        }

        // PASS 2 → Declarations + COMMON

        for(size_t lineNo = 0; lineNo < lines.size(); lineNo++)
        {
             string line = lines[lineNo];
            smatch declMatch;
            smatch eqMatch;

            if(regex_search(
                line,
                eqMatch,
                equivalenceRegex
            )) {

                string left = eqMatch[1];
                string right = eqMatch[2];

                left.erase(
                    remove_if(
                        left.begin(),
                        left.end(),
                        ::isspace
                    ),
                    left.end()
                );

                right.erase(
                    remove_if(
                        right.begin(),
                        right.end(),
                        ::isspace
                    ),
                    right.end()
                );

                transform(
                    left.begin(),
                    left.end(),
                    left.begin(),
                    ::toupper
                );

                transform(
                    right.begin(),
                    right.end(),
                    right.begin(),
                    ::toupper
                );
               EquivalenceInfo eq;

                eq.fileName = argv[i];
                eq.left = left;
                eq.right = right;

                equivalenceDB.push_back(eq);
                // cout << "Detected EQUIVALENCE: "
                // << left
                // << " <-> "
                // << right
                // << "\n";
            }

            if(regex_search(
                line,
                declMatch,
                declRegex
            )) {

                VariableInfo info;

                info.type = declMatch[1];
                info.name = declMatch[2];
                info.lineNumber = lineNo + 1;
                transform(
                    info.name.begin(),
                    info.name.end(),
                    info.name.begin(),
                    ::toupper
                );

                if(declMatch[4].matched)
                    info.count =
                        stoi(declMatch[4]);
                else
                    info.count = 1;

                info.totalSize =
                    getTypeSize(info.type)
                    * info.count;

                info.alignment =
                    getAlignment(info.type);

                variableTable[info.name]
                    = info;
            }

            smatch commonMatch;

            if(regex_search(
                line,
                commonMatch,
                commonRegex
            )) {

                CommonEntry entry;

                entry.fileName = argv[i];
                entry.blockName = commonMatch[1];

                if(saveBlocks.find(
                    entry.blockName
                ) != saveBlocks.end()) {

                    entry.hasSave = true;
                }

                string variableList =
                    commonMatch[2];

                stringstream ss(variableList);

                string varName;

                while(getline(
                    ss,
                    varName,
                    ','
                )) {

                    transform(
                    varName.begin(),
                    varName.end(),
                    varName.begin(),
                    ::toupper
                );
                    varName.erase(
                        remove_if(
                            varName.begin(),
                            varName.end(),
                            ::isspace
                        ),
                        varName.end()
                    );

                    if(variableTable.find(varName)
                        != variableTable.end()) {

                        entry.vars.push_back(
                            variableTable[varName]
                        );
                    }
                }

                int currentOffset = 0;

                for(auto &v : entry.vars) {

                    v.offset = currentOffset;

                    currentOffset +=
                        v.totalSize;
                }

                commonDB.push_back(entry);
            }
        }
    }

    // GROUP COMMON BLOCKS

    for(const auto &entry : commonDB) {

        groupedBlocks[
            entry.blockName
        ].push_back(entry);
    }

    // RUN ANALYSES

    printAnalysis(
        "SIZE MISMATCH ANALYSIS",
        analyzeSizeMismatch()
    );

    printAnalysis(
        "TYPE PUNNING ANALYSIS",
        analyzeTypePunning()
    );

    printAnalysis(
        "ALIGNMENT ANALYSIS",
        analyzeAlignment()
    );

    printAnalysis(
        "STRUCTURAL LAYOUT ANALYSIS",
        analyzeStructuralLayouts()
    );

    printAnalysis(
        "SAVE ATTRIBUTE ANALYSIS",
        analyzeSaveConsistency()
    );

    printAnalysis(
    "EQUIVALENCE ANALYSIS",
    analyzeEquivalence()
    );

    // FINAL DATABASE

    printAnalysis(
    "MIGRATION ADVISOR",
    generateMigrationAdvice()
    );

    printStatistics();
    auto end =
    chrono::high_resolution_clock::now();

    auto duration =chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "\nAnalysis Time: "
        << duration.count()
        << " ms\n";
    return 0;
}