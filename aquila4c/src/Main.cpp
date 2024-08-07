#include <iostream>

#include "antlr4-runtime.h"
#include "../gen/AquilaLexer.h"
#include "../gen/AquilaParser.h"
#include "Interpreter.hpp"

#include <filesystem>
#include <fstream>

int run(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "ERROR: Usage: aquila4c <file> arguments..." << std::endl;
        return EXIT_FAILURE;
    }

    char* file = argv[1];
    if (!std::filesystem::exists(file)) {
        std::cerr << "ERROR: File not found! " << file << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream infile(file);
    antlr4::ANTLRInputStream* input = new antlr4::ANTLRInputStream(infile);
    AquilaLexer* lexer = new AquilaLexer(input);
    antlr4::CommonTokenStream* tokens = new antlr4::CommonTokenStream(lexer);
    AquilaParser* parser = new AquilaParser(tokens);
    antlr4::tree::ParseTree* tree = parser->program();

    const size_t nose = parser->getNumberOfSyntaxErrors();
    if (nose > 0) {
        std::cerr << "ERROR: There were " << nose << " syntax errors!" << std::endl;
        return EXIT_FAILURE;
    }

    Interpreter interpreter(argc, argv);
    interpreter.visit(tree);
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    run(argc, argv);
}
