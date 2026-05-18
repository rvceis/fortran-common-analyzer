#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "flang_parser.h"

#include "flang/Parser/parsing.h"
#include "flang/Parser/parse-tree.h"
#include "flang/Parser/source.h"
#include "flang/Parser/dump-parse-tree.h"

#include "llvm/Support/raw_ostream.h"

#include <iostream>

using namespace Fortran;
void parseFortranFile(const std::string &fileName) {

    parser::AllSources allSources;

    parser::AllCookedSources cooked{
        allSources
    };

    parser::Parsing parsing{cooked};

    parsing.Prescan(
        fileName,
        parser::Options{}
    );

    parsing.Parse(llvm::outs());

    std::cout
        << "Successfully parsed: "
        << fileName
        << "\n";
}