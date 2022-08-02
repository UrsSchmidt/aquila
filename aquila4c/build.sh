#!/bin/bash

src='src'
gen='gen'
dst='obj'

./clean.sh

# this will make the script exit on the first error
set -e

mkdir -p "$gen/"
mkdir -p "$dst/"

parent=$(dirname "$PWD")
java -jar '../../antlr-4.10.1-complete.jar' "$parent/Aquila.g4" -Dlanguage=Cpp -visitor -no-listener -encoding utf8 -o "$gen/"

comp_ops_gen='-c -std=c++17 -I/usr/local/include/antlr4-runtime'
comp_ops_src="$comp_ops_gen -pedantic -Wall"
g++ $comp_ops_gen -o "$dst/AquilaBaseVisitor.o" "$gen/AquilaBaseVisitor.cpp"
g++ $comp_ops_gen -o "$dst/AquilaLexer.o"       "$gen/AquilaLexer.cpp"
g++ $comp_ops_gen -o "$dst/AquilaParser.o"      "$gen/AquilaParser.cpp"
g++ $comp_ops_gen -o "$dst/AquilaVisitor.o"     "$gen/AquilaVisitor.cpp"
g++ $comp_ops_src -o "$dst/Helper.o"            "$src/Helper.cpp"
g++ $comp_ops_src -o "$dst/Interpreter.o"       "$src/Interpreter.cpp"
g++ $comp_ops_src -o "$dst/Main.o"              "$src/Main.cpp"

g++ -o 'aquila4c' "$dst/"*.o -lantlr4-runtime -lgmp
