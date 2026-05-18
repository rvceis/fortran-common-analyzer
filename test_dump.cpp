#include "flang/Parser/parsing.h"
#include "flang/Parser/parse-tree.h"
#include "flang/Parser/dump-parse-tree.h"
#include "llvm/Support/raw_ostream.h"

using namespace Fortran;
int main() {
    parser::AllSources allSources;
    parser::AllCookedSources cooked{allSources};
    parser::Parsing parsing{cooked};
    parsing.Prescan("testcases/test_dump2.f", parser::Options{});
    parsing.Parse(llvm::outs());
    if (parsing.parseTree()) {
        parser::DumpTree(llvm::outs(), *parsing.parseTree());
    }
    return 0;
}
