#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
using namespace std;


struct VariableInfo {
    string name;
    string type;
    int count;
    int totalSize;
    int offset;
};

struct CommonEntry {
    string fileName;
    string blockName;
    vector<VariableInfo> vars;
};

vector<CommonEntry> commonDB;
map<string, vector<CommonEntry>> groupedBlocks;

void PrintDb()
{
     cout << "\n===== GLOBAL COMMON DATABASE =====\n";

        for(auto &entry : commonDB) {

            cout << "\nFile: "
                << entry.fileName
                << "\n";

            cout << "COMMON Block: "
                << entry.blockName
                << "\n\n";

            for(auto &v : entry.vars) {

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

int getCommonBlockSize(const CommonEntry& entry) {

    int total = 0;

    for(const auto &v : entry.vars) {
        total += v.totalSize;
    }

    return total;
}
int main(int argc, char* argv[]) {

    if(argc < 2) {
        cout << "Usage: ./analyzer <files>\n";
        return 1;
    }

    regex commonRegex(R"(COMMON\s*/(\w+)/\s*(.*))");
    regex declRegex(R"((REAL|INTEGER|DOUBLE PRECISION)\s+(\w+)(\((\d+)\))?)");

    for(int i = 1; i < argc; i++) {

        cout << "\nProcessing File: "
             << argv[i] << "\n";

        ifstream file(argv[i]);

        if(!file) {
            cout << "Cannot open file\n";
            continue;
        }
        map<string, VariableInfo> variableTable;
        string line;

        while(getline(file, line)) {

            smatch match;
            smatch declMatch;

            if(regex_search(line, declMatch, declRegex)) {

            VariableInfo info;

            info.type = declMatch[1];
            info.name = declMatch[2];

            if(declMatch[4].matched)
                info.count = stoi(declMatch[4]);
            else
                info.count = 1;

            int typeSize = 0;

            if(info.type == "REAL")
                typeSize = 4;

            else if(info.type == "INTEGER")
                typeSize = 4;

            else if(info.type == "DOUBLE PRECISION")
                typeSize = 8;

            info.totalSize = typeSize * info.count;

            variableTable[info.name] = info;
            }

          
            if(regex_search(line, match, commonRegex)) {

                CommonEntry entry;

                entry.fileName = argv[i];
                entry.blockName = match[1];

                string variableList = match[2];

                stringstream ss(variableList);

                string varName;

                while(getline(ss, varName, ',')) {

                    // Remove spaces
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

                // Compute offsets
                int currentOffset = 0;

                for(auto &v : entry.vars) {

                    v.offset = currentOffset;

                    currentOffset += v.totalSize;
                }

                commonDB.push_back(entry);
            }
        }
    }

    for(auto &entry : commonDB) {
    groupedBlocks[entry.blockName].push_back(entry);
    }

                cout << "\n===== SIZE MISMATCH ANALYSIS =====\n";

            for(auto &block : groupedBlocks) {

                auto &entries = block.second;

                if(entries.size() < 2)
                    continue;

                int expectedSize =
                    getCommonBlockSize(entries[0]);

                bool mismatchFound = false;

                for(size_t i = 1; i < entries.size(); i++) {

                    int currentSize =
                        getCommonBlockSize(entries[i]);

                    if(currentSize != expectedSize) {

                        mismatchFound = true;
                        break;
                    }
                }

                if(mismatchFound) {

                    cout << "\nERROR: COMMON Block "
                        << block.first
                        << " has inconsistent sizes\n";

                    for(auto &e : entries) {

                        cout << "  File: "
                            << e.fileName
                            << " -> "
                            << getCommonBlockSize(e)
                            << " bytes\n";
                    }
                }
            }
            cout << "\n===== TYPE PUNNING ANALYSIS =====\n";

    for(auto &block : groupedBlocks) {

        auto &entries = block.second;

        if(entries.size() < 2)
            continue;

        bool warningFound = false;

        for(size_t i = 0; i < entries.size(); i++) {

            for(size_t j = i + 1; j < entries.size(); j++) {

                auto &vars1 = entries[i].vars;
                auto &vars2 = entries[j].vars;

                size_t minVars =
                    min(vars1.size(), vars2.size());

                for(size_t k = 0; k < minVars; k++) {

                    if(vars1[k].type != vars2[k].type) {

                        if(!warningFound) {

                            cout << "\nWARNING: COMMON Block "
                                << block.first
                                << " aliases different types\n";

                        warningFound = true;
                    }

                    cout << "  Conflict:\n";

                    cout << "    "
                         << entries[i].fileName
                         << " -> "
                         << vars1[k].name
                         << " ("
                         << vars1[k].type
                         << ")\n";

                    cout << "    "
                         << entries[j].fileName
                         << " -> "
                         << vars2[k].name
                         << " ("
                         << vars2[k].type
                         << ")\n";
                }
            }
        }
    }
}
    

    PrintDb();

   
    return 0;
}