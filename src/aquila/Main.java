package aquila;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import aquila.antlr.AquilaLexer;
import aquila.antlr.AquilaParser;
import aquila.antlr.AquilaParser.ProgramContext;

import org.antlr.v4.runtime.CharStreams;
import org.antlr.v4.runtime.CommonTokenStream;

public class Main {

    public static void main(String[] args) throws IOException {
        if (args.length < 1) {
            System.err.println("Usage: aquila <file>");
            return;
        }

        File file = new File(args[0]);
        if (!file.isFile()) {
            System.err.println("Error: File not found " + file);
            return;
        }

        AquilaLexer lexer = new AquilaLexer(CharStreams.fromStream(new FileInputStream(file)));
        AquilaParser parser = new AquilaParser(new CommonTokenStream(lexer));
        ProgramContext program = parser.program();
        if (parser.getNumberOfSyntaxErrors() > 0) {
            return;
        }

        new Interpreter(args).visit(program);
    }

}
