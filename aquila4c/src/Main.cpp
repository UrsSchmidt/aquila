#include <iostream>

#include "antlr4-runtime.h"
#include "../gen/AquilaLexer.h"
#include "../gen/AquilaParser.h"
#include "Interpreter.h"

using namespace antlr4;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "ERROR: Usage: aquila4c <file> arguments..." << std::endl;
        return 1;
    }

    std::ifstream infile(argv[1]);
    ANTLRInputStream input(infile);
    AquilaLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    AquilaParser parser(&tokens);
    tree::ParseTree* tree = parser.program();

    const size_t nose = parser.getNumberOfSyntaxErrors();
    if (nose > 0) {
        std::cerr << "ERROR: There were syntax errors!" << std::endl;
        return 1;
    }

    Interpreter interpreter;
    interpreter.visit(tree);
    return 0;
}
