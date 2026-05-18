#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <chrono>
#include "flang_parser.h"
using namespace std;

// ============================================================
// DATA STRUCTURES
// ============================================================

struct VariableInfo
{
    string name;
    string type;
    int count = 1;
    int totalSize = 0;
    int offset = 0;
    int alignment = 4;
    int lineNumber = 0;
};

struct CommonEntry
{
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
struct EquivalenceInfo
{

    string fileName;
    string left;
    string right;
};

vector<EquivalenceInfo> equivalenceDB;
// ============================================================
// UTILITY FUNCTIONS
// ============================================================

int getTypeSize(const string &type)
{

    if (type == "REAL")
        return 4;

    if (type == "INTEGER")
        return 4;

    if (type == "DOUBLE PRECISION")
        return 8;

    return 4;
}

int getAlignment(const string &type)
{

    if (type == "DOUBLE PRECISION")
        return 8;

    return 4;
}

int getCommonSize(const CommonEntry &entry)
{

    int total = 0;

    for (const auto &v : entry.vars)
        total += v.totalSize;

    return total;
}

string buildLayoutSignature(const CommonEntry &entry)
{

    string sig;

    for (const auto &v : entry.vars)
    {

        sig += v.type;
        sig += ":";
        sig += to_string(v.offset);
        sig += ":";
        sig += to_string(v.totalSize);
        sig += ";";
    }

    return sig;
}

void printHeader(const string &title)
{

    cout << "\n===== " << title << " =====\n";
}

// ============================================================
// SINGLE GENERIC REPORT FUNCTION
// ============================================================

void printAnalysis(
    const string &category,
    const vector<string> &messages)
{

    printHeader(category);

    if (messages.empty())
    {

        cout << "No issues detected.\n";
        return;
    }

    for (const auto &msg : messages)
        cout << msg << "\n";
}

// ============================================================
// ANALYSIS PASSES
// ============================================================

vector<string> analyzeSizeMismatch()
{

    vector<string> results;

    for (const auto &block : groupedBlocks)
    {

        const auto &entries = block.second;

        if (entries.size() < 2)
            continue;

        int expectedSize =
            getCommonSize(entries[0]);

        bool mismatch = false;

        for (size_t i = 1; i < entries.size(); i++)
        {

            if (getCommonSize(entries[i]) != expectedSize)
            {

                mismatch = true;
                break;
            }
        }

        if (mismatch)
        {

            string msg = "\nERROR: COMMON Block " + block.first + " has inconsistent sizes\n";

            for (const auto &e : entries) {

                msg += "  File: " + e.fileName + " -> " +
                       to_string(getCommonSize(e)) +
                       " bytes\n";
            }

            results.push_back(msg);
        }
    }

    return results;
}

vector<string> analyzeTypePunning()
{

    vector<string> results;

    for (const auto &block : groupedBlocks)
    {

        const auto &entries = block.second;

        if (entries.size() < 2)
            continue;

        for (size_t i = 0; i < entries.size(); i++)
        {

            for (size_t j = i + 1;
                 j < entries.size();
                 j++)
            {

                const auto &vars1 =entries[i].vars;

                const auto &vars2 =entries[j].vars;

                size_t limit =min(vars1.size(), vars2.size());

                for (size_t k = 0;
                     k < limit;
                     k++)
                {

                    if (vars1[k].type != vars2[k].type)
                    {

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

vector<string> analyzeAlignment()
{

    vector<string> results;

    for (const auto &entry : commonDB)
    {

        for (const auto &v : entry.vars)
        {

            if (v.offset % v.alignment != 0)
            {

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

vector<string> analyzeStructuralLayouts()
{

    vector<string> results;

    for (const auto &block : groupedBlocks)
    {

        const auto &entries = block.second;

        if (entries.size() < 2)
            continue;

        string baseLayout =
            buildLayoutSignature(entries[0]);

        for (size_t i = 1;
             i < entries.size();
             i++)
        {

            string currentLayout =
                buildLayoutSignature(entries[i]);

            if (baseLayout != currentLayout)
            {

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

vector<string> analyzeSaveConsistency()
{

    vector<string> results;

    for (const auto &block : groupedBlocks)
    {

        const auto &entries = block.second;

        if (entries.size() < 2)
            continue;

        bool reference =
            entries[0].hasSave;

        bool mismatch = false;

        for (size_t i = 1;
             i < entries.size();
             i++)
        {

            if (entries[i].hasSave != reference)
            {

                mismatch = true;
                break;
            }
        }

        if (mismatch)
        {

            string msg =
                "\nWARNING: COMMON Block " +
                block.first +
                " has inconsistent SAVE usage\n";

            for (const auto &e : entries)
            {

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
vector<string> analyzeEquivalence()
{

    vector<string> results;

    for (const auto &eq : equivalenceDB)
    {

        for (const auto &entry : commonDB)
        {

            // SAME FILE ONLY
            if (entry.fileName != eq.fileName)
                continue;

            for (const auto &v : entry.vars)
            {

                if (v.name == eq.left)
                {

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

vector<string> generateMigrationAdvice()
{

    vector<string> advice;

    for (const auto &block : groupedBlocks)
    {

        const auto &entries = block.second;

        if (entries.size() < 2)
            continue;

        bool issueDetected = false;

        // Check size mismatch
        int baseSize =
            getCommonSize(entries[0]);

        for (size_t i = 1; i < entries.size(); i++)
        {

            if (getCommonSize(entries[i]) != baseSize)
            {

                issueDetected = true;
            }
        }

        // Check layout mismatch
        string baseLayout =
            buildLayoutSignature(entries[0]);

        for (size_t i = 1; i < entries.size(); i++)
        {

            string currentLayout =
                buildLayoutSignature(entries[i]);

            if (currentLayout != baseLayout)
            {

                issueDetected = true;
            }
        }

        if (issueDetected)
        {

            string msg =
                "\nMIGRATION ADVICE FOR COMMON BLOCK: " + block.first + "\n";

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
                "  MODULE " + block.first +
                "_MOD\n";

            for (const auto &v :
                 entries[0].vars)
            {

                msg += "      " + v.type + " :: " + v.name;

                if (v.count > 1)
                {

                    msg += "(" + to_string(v.count) + ")";
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

void printStatistics()
{

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

void printGlobalDatabase()
{

    printHeader("GLOBAL COMMON DATABASE");

    for (const auto &entry : commonDB)
    {

        cout << "\nFile: "
             << entry.fileName
             << "\n";

        cout << "COMMON Block: "
             << entry.blockName
             << "\n\n";

        for (const auto &v : entry.vars)
        {

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

int main(int argc, char *argv[])
{
    auto start =chrono::high_resolution_clock::now();
    if (argc < 2)
    {

        cout << "Usage: ./analyzer <files>\n";
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        cout << "\nProcessing File: "<< argv[i]<< "\n";
        string astStr = parseFortranFile(argv[i]);
        map<string, VariableInfo> variableTable;
        set<string> saveBlocks;
        
        enum class AstState { NONE, TYPE_DECL, SAVE_STMT, EQUIV_STMT, COMMON_STMT, COMMON_BLOCK, COMMON_OBJ };
        
        // PASS 1: Types, Save, Equivalence
        {
            istringstream iss(astStr);
            string line;
            AstState state = AstState::NONE;
            string currentType = "REAL";
            VariableInfo currentVar;
            bool inEntityDecl = false;
            string currentEqLeft = "";

            while (getline(iss, line)) {
                if (line.find("TypeDeclarationStmt") != string::npos) {
                    if (inEntityDecl) {
                        currentVar.totalSize = getTypeSize(currentVar.type) * currentVar.count;
                        currentVar.alignment = getAlignment(currentVar.type);
                        variableTable[currentVar.name] = currentVar;
                        inEntityDecl = false;
                    }
                    state = AstState::TYPE_DECL;
                    currentType = "REAL";
                } else if (line.find("SaveStmt") != string::npos) {
                    state = AstState::SAVE_STMT;
                } else if (line.find("EquivalenceStmt") != string::npos) {
                    state = AstState::EQUIV_STMT;
                    currentEqLeft = "";
                } else if (line.find("->") != string::npos && line.find("SpecificationConstruct") != string::npos) {
                    if (inEntityDecl) {
                        currentVar.totalSize = getTypeSize(currentVar.type) * currentVar.count;
                        currentVar.alignment = getAlignment(currentVar.type);
                        variableTable[currentVar.name] = currentVar;
                        inEntityDecl = false;
                    }
                    state = AstState::NONE;
                }

                if (state == AstState::TYPE_DECL) {
                    if (line.find("IntrinsicTypeSpec -> Real") != string::npos) currentType = "REAL";
                    else if (line.find("IntegerTypeSpec") != string::npos) currentType = "INTEGER";
                    else if (line.find("DoublePrecision") != string::npos) currentType = "DOUBLE PRECISION";
                    
                    if (line.find("EntityDecl") != string::npos) {
                        if (inEntityDecl) {
                            currentVar.totalSize = getTypeSize(currentVar.type) * currentVar.count;
                            currentVar.alignment = getAlignment(currentVar.type);
                            variableTable[currentVar.name] = currentVar;
                        }
                        inEntityDecl = true;
                        currentVar = VariableInfo();
                        currentVar.type = currentType;
                        currentVar.count = 1;
                    }
                    if (inEntityDecl) {
                        if (line.find("Name = '") != string::npos) {
                            size_t start = line.find("'") + 1;
                            size_t end = line.find("'", start);
                            currentVar.name = line.substr(start, end - start);
                            transform(currentVar.name.begin(), currentVar.name.end(), currentVar.name.begin(), ::toupper);
                        }
                        if (line.find("IntLiteralConstant = '") != string::npos) {
                            size_t start = line.find("'") + 1;
                            size_t end = line.find("'", start);
                            currentVar.count = stoi(line.substr(start, end - start));
                        }
                    }
                } else if (state == AstState::SAVE_STMT) {
                    if (line.find("Name = '") != string::npos) {
                        size_t start = line.find("'") + 1;
                        size_t end = line.find("'", start);
                        string name = line.substr(start, end - start);
                        transform(name.begin(), name.end(), name.begin(), ::toupper);
                        saveBlocks.insert(name);
                    }
                } else if (state == AstState::EQUIV_STMT) {
                    if (line.find("Name = '") != string::npos) {
                        size_t start = line.find("'") + 1;
                        size_t end = line.find("'", start);
                        string name = line.substr(start, end - start);
                        transform(name.begin(), name.end(), name.begin(), ::toupper);
                        if (currentEqLeft.empty()) {
                            currentEqLeft = name;
                        } else {
                            EquivalenceInfo eq;
                            eq.fileName = argv[i];
                            eq.left = currentEqLeft;
                            eq.right = name;
                            equivalenceDB.push_back(eq);
                            currentEqLeft = ""; 
                        }
                    }
                }
            }
            if (inEntityDecl) {
                currentVar.totalSize = getTypeSize(currentVar.type) * currentVar.count;
                currentVar.alignment = getAlignment(currentVar.type);
                variableTable[currentVar.name] = currentVar;
            }
        }

        // PASS 2: COMMON blocks
        {
            istringstream iss(astStr);
            string line;
            AstState state = AstState::NONE;
            
            CommonEntry currentEntry;
            bool inBlock = false;
            
            while (getline(iss, line)) {
                if (line.find("CommonStmt") != string::npos) {
                    state = AstState::COMMON_STMT;
                    if (inBlock && !currentEntry.blockName.empty()) {
                        if (saveBlocks.find(currentEntry.blockName) != saveBlocks.end()) {
                            currentEntry.hasSave = true;
                        }
                        commonDB.push_back(currentEntry);
                        groupedBlocks[currentEntry.blockName].push_back(currentEntry);
                    }
                    inBlock = false;
                } else if (state == AstState::COMMON_STMT || state == AstState::COMMON_BLOCK || state == AstState::COMMON_OBJ) {
                    if (line.find("| | | Block") != string::npos) {
                        if (inBlock && !currentEntry.blockName.empty()) {
                            if (saveBlocks.find(currentEntry.blockName) != saveBlocks.end()) {
                                currentEntry.hasSave = true;
                            }
                            commonDB.push_back(currentEntry);
                            groupedBlocks[currentEntry.blockName].push_back(currentEntry);
                        }
                        inBlock = true;
                        currentEntry = CommonEntry();
                        currentEntry.fileName = argv[i];
                        currentEntry.blockName = "_BLANK_"; // default if unnamed
                        state = AstState::COMMON_BLOCK;
                    } else if (line.find("CommonBlockObject") != string::npos) {
                        state = AstState::COMMON_OBJ;
                    } else if (line.find("Name = '") != string::npos) {
                        size_t start = line.find("'") + 1;
                        size_t end = line.find("'", start);
                        string name = line.substr(start, end - start);
                        transform(name.begin(), name.end(), name.begin(), ::toupper);
                        
                        if (state == AstState::COMMON_BLOCK) {
                            currentEntry.blockName = name;
                        } else if (state == AstState::COMMON_OBJ) {
                            VariableInfo v;
                            if (variableTable.find(name) != variableTable.end()) {
                                v = variableTable[name];
                            } else {
                                v.name = name;
                                v.type = "REAL"; // Implicit default
                                v.count = 1;
                                v.totalSize = 4;
                                v.alignment = 4;
                            }
                            int currentOffset = 0;
                            if (!currentEntry.vars.empty()) {
                                const auto& lastVar = currentEntry.vars.back();
                                currentOffset = lastVar.offset + lastVar.totalSize;
                                int align = v.alignment;
                                if (currentOffset % align != 0) {
                                    currentOffset += (align - (currentOffset % align));
                                }
                            }
                            v.offset = currentOffset;
                            currentEntry.vars.push_back(v);
                        }
                    } else if (line.find("->") != string::npos && line.find("Block") == string::npos && line.find("CommonBlockObject") == string::npos) {
                        if (inBlock && !currentEntry.blockName.empty()) {
                            if (saveBlocks.find(currentEntry.blockName) != saveBlocks.end()) {
                                currentEntry.hasSave = true;
                            }
                            commonDB.push_back(currentEntry);
                            groupedBlocks[currentEntry.blockName].push_back(currentEntry);
                        }
                        inBlock = false;
                        state = AstState::NONE;
                    }
                }
            }
            if (inBlock && !currentEntry.blockName.empty()) {
                if (saveBlocks.find(currentEntry.blockName) != saveBlocks.end()) {
                    currentEntry.hasSave = true;
                }
                commonDB.push_back(currentEntry);
                groupedBlocks[currentEntry.blockName].push_back(currentEntry);
            }
        }
    }

    // RUN ANALYSES

    printAnalysis( "SIZE MISMATCH ANALYSIS", analyzeSizeMismatch());

    printAnalysis(  "TYPE PUNNING ANALYSIS", analyzeTypePunning());

    printAnalysis( "ALIGNMENT ANALYSIS", analyzeAlignment());

    printAnalysis("STRUCTURAL LAYOUT ANALYSIS",analyzeStructuralLayouts());

    printAnalysis( "SAVE ATTRIBUTE ANALYSIS",  analyzeSaveConsistency());

    printAnalysis(  "EQUIVALENCE ANALYSIS", analyzeEquivalence());

    // FINAL DATABASE

    printAnalysis( "MIGRATION ADVISOR", generateMigrationAdvice());

    printStatistics();
    auto end =
        chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "\nAnalysis Time: "
         << duration.count()
         << " ms\n";
    return 0;
}