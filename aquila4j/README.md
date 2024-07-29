# aquila4j

This is the Java 17 implementation of Aquila.

## Building the interpreter

You will need `ant 1.10.7` and `antlr 4.12.0` to build the interpreter.
Make sure to put the `antlr-4.12.0-complete.jar` file into the parent directory of this repository.
Then run `ant package` to build the `aquila4j.jar` file.

## Installing the interpreter on Ubuntu

Run `./install.sh`. You might need sudo rights.
You can then use `#!/usr/local/bin/aq` as the shebang in your scripts, which will call the interpreter.

## Implementation details

Types are implemented using the following Java types:

| Aquila     | Java                                                  |
|------------|-------------------------------------------------------|
| Any        | `java.lang.Object`                                    |
| Boolean    | `java.lang.Boolean`                                   |
| Integer    | `java.math.BigInteger`                                |
| String     | `java.lang.String`[^1]                                |
| Function   | `aquila4j.antlr.AquilaParser.LambdaExpressionContext` |
| Dictionary | `java.util.TreeMap<Object, Object>`                   |

[^1]: Since this interpreter is written in Java and Strings in Java are Unicode-compatible, you can use Unicode characters in your Strings as well.
