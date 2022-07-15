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
        run(args);
    }

    public static void run(String[] args) throws IOException {
        if (args == null || args.length < 1) {
            System.err.println("ERROR: Usage: aquila <file> arguments...");
            System.exit(1);
            return;
        }

        File file = new File(args[0]);
        if (!file.isFile()) {
            System.err.println("ERROR: File not found " + file);
            System.exit(1);
            return;
        }

        AquilaLexer lexer = new AquilaLexer(CharStreams.fromStream(new FileInputStream(file)));
        AquilaParser parser = new AquilaParser(new CommonTokenStream(lexer));
        ProgramContext program = parser.program();
        final int nose = parser.getNumberOfSyntaxErrors();
        if (nose > 0) {
            System.err.println("ERROR: There were " + nose + " syntax errors");
            System.exit(1);
            return;
        }

        new Interpreter(args).visit(program);
    }

}
