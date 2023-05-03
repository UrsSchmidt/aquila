package aquila4j;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import aquila4j.antlr.AquilaLexer;
import aquila4j.antlr.AquilaParser;
import aquila4j.antlr.AquilaParser.ProgramContext;

import org.antlr.v4.runtime.CharStreams;
import org.antlr.v4.runtime.CommonTokenStream;

public class Main {

    public static void run(String[] args) throws IOException {
        if (args == null || args.length < 1) {
            System.err.println("ERROR: Usage: java -jar aquila4j.jar <file> arguments...");
            System.exit(1);
            return;
        }

        File file = new File(args[0]);
        if (!file.isFile()) {
            System.err.println("ERROR: File not found! " + file);
            System.exit(1);
            return;
        }

        AquilaLexer lexer = new AquilaLexer(CharStreams.fromStream(new FileInputStream(file)));
        AquilaParser parser = new AquilaParser(new CommonTokenStream(lexer));
        ProgramContext program = parser.program();

        final int nose = parser.getNumberOfSyntaxErrors();
        if (nose > 0) {
            System.err.println("ERROR: There were " + nose + " syntax errors!");
            System.exit(1);
            return;
        }

        new Interpreter(args).visit(program);
    }

    public static void main(String[] args) throws IOException {
        run(args);
    }

}
