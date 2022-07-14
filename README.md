# Aquila

Aquila is a small scripting language that is run by an interpreter written in Java 11. This language has been influenced by JavaScript/ActionScript, Ada/Pascal, Bash, Batch, Python and Visual Basic 6. Because of the simplicity of the language, no context analysis is necessary when interpreting code.

## Building the interpreter

You will need `ant` to build the interpreter.
Then simply run `ant package` to build the `aquila.jar` file.

## Installing the interpreter on Ubuntu

Change the path in `aq.sh` to point to your `aquila.jar` file.
Then run `cp aq.sh /usr/local/bin/aq`. You might need sudo rights.
You can then use `#!/usr/local/bin/aq` as the shebang in your scripts, which will call the `aq.sh` script and in turn call the interpreter.

## Syntax highlighting for gedit

Simply copy the `aquila.lang` file to the `language-specs/` directory of your gedit installation:
`cp aquila.lang /usr/share/gtksourceview-4/language-specs/aquila.lang`. You might need sudo rights.

# Language overview

You can find a lot of example scripts in the `examples/` directory.

## Comments

Comments are written like this: `# this is a comment`

## Types

There are four types as of now: Dictionaries, Strings, Integers and Booleans. You **cannot** define your own types.

### Dictionaries

Dictionary aggregates are written like this: `{ key : value; }`
If you do not care about the keys, you can also use `{ v1; v2; v3; }` as a shorthand for `{ 0 : v1; 1 : v2; 2 : v3; }`.

You can use either `.key` or `[keyExpression]` to access the values of the dictionary.
The following predefined functions are available on Dictionaries:
 * `size(a)` returns the number of key/value pairs in a
 * `dict2str(a)` returns a as a String

Dictionaries are implemented as `TreeMap`.

### Strings

Strings are written like this: `'Hello, world!'`

The following operations are available on Strings: `&` (concatenation), `eq` (equals) and `ne` (not equals).
The following predefined functions are available on Strings:
 * `length(a)` returns the number of characters in a
 * `ord(a)` returns the Unicode value of the first character of a
 * `str2bool(a)` parsing a to a Boolean
 * `str2dict(a)` parsing a to a Dictionary
 * `str2int(a)` parsing a to an Integer

Since the interpreter is written in Java and Strings in Java are Unicode-compatible, you can use Unicode characters in your Strings as well.

### Integers

Integers are written like this:
 * Decimal: `12345`
 * Binary: `0b10101010`
 * Octal: `0o755`
 * Hexadecimal: `0xabcd`

The following operations are available on Integers: `+`, `-`, `*`, `/`, `mod`, `rem`, `=`, `<>`, `<`, `<=`, `>`, `>=` and `|x|`.
The following predefined functions are available on Integers:
 * `chr(a)` returns a String containing a single Unicode character
 * `gcd(a, b)` returns the greatest common divisor of a and b
 * `int2str(a)` returns a as a String
 * `pow(a, b)` returns the a to the power of b
 * `sgn(a)` returns the sign of a
 * `sqrt(a)` returns the square root of a

Integers are implemented as `BigInteger`.

### Booleans

The two Booleans are `true` and `false`.

The following operations are available on Booleans: `not`, `and`, `or` and `xor`.
The following predefined functions are available on Booleans:
 * `bool2str(a)` returns a as a String

### Checking for types

You can check for types by using the predefined functions `boolean`, `dictionary`, `integer` and `string`:
```
a := 123;
write bool2str(integer(a)); # true
```

## Statements

### If statements

If statements are written like this:
```
if expression (
    code;
) elif expression (
    code;
) else (
    code;
)
```
If not needed, the else part can be omitted and there can be 0..n elif (short for else if) parts.

### Loop statements

Loop statements are written like this:
```
loop (
    code;
) while expression (
    code;
)
```
If not needed, either the loop part or the while part can be omitted.

### For statements

For statements are written like this (in this example iterating over the odd numbers from 1 to 100):
```
for i from 1 to 100 step 2 (
    code;
)
```
If not needed, the step part can be omitted, it will default to `step 1`.

### Foreach statements

Foreach statements are written like this (in this example iterating over all elements of myDictionary and printing their keys and values):
```
foreach key k value v in myDictionary (
    write k & ' : ' & v;
)
```
If not needed, the key and/or the value can both be omitted.

### Read statements

You can read a line from the command line into a String using: `read myVariable;`

### Write statements

You can write a String as a line to the command line using: `write 'Hello, world!';`

### Assignments

You can assign values to variables using: `myVariable := myValue;`

### Remove statements

You can remove key/value pairs from Dictionaries using: `remove myDictionary.myKey;`

## Using command line arguments

Command line arguments are stored in the `args` variable.

## Using environment variables

Environment variables are stored in the `env` variable. You **cannot** set environment variables from inside your script.
