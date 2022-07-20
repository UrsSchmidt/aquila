# Aquila

Aquila is a small scripting language that is run by an interpreter written in Java 11. This language has been influenced by JavaScript/ActionScript, Ada/Pascal, Bash, Batch, Groovy, Haskell, Perl, Python, Ruby and Visual Basic 6. Because of the simplicity of the language, no context analysis is necessary when interpreting code.

## Building the interpreter

You will need `ant 1.10.7` and `antlr 4.10.1` to build the interpreter.
Make sure to put the `antlr-4.10.1-complete.jar` file into the parent directory of this directory.
Then simply run `ant package` to build the `aquila.jar` file.

## Installing the interpreter on Ubuntu

Change the path in `aq.sh` to point to your `aquila.jar` file.
Then run `cp aq.sh /usr/local/bin/aq`. You might need sudo rights.
You can then use `#!/usr/local/bin/aq` as the shebang in your scripts, which will call the `aq.sh` script and in turn call the interpreter.

## Running the test suite on Ubuntu

Make sure that you have already built the project.
Then you can simply run `./run-tests.sh` and the whole test suite will be executed.

## Syntax highlighting for gedit

Simply copy the `aquila.lang` file to the `language-specs/` directory of your gedit installation:
`cp aquila.lang /usr/share/gtksourceview-4/language-specs/aquila.lang`. You might need sudo rights.

# Language overview

You can find a lot of example scripts in the `examples/` directory.

## Comments

Comments are written like this: `# this is a comment`

## Types

There are five types as of now: Functions, Dictionaries, Strings, Integers and Booleans. You **cannot** define your own types.

### Functions

Functions are always written as lambdas: `\x, y -> x + y`
You can assign Functions to variables like any other value: `a := \x -> 2 * x;`
You can also write recursive Functions by using the variable that you assign the Function to: `factorial := \n -> if n = 0: 1 else: n * factorial(n - 1);`
You can also add additional (runtime) type-safety by stating what type each of parameters has: `a := \x : Integer -> 2 * x;`
Here you can also use `Any` to explicitly state, that the argument can take any type.

The following predefined functions are available on Functions:

 * `foreach(d : Dictionary, f : Function)`
   `(Dictionary, (Any, Any) -> Void) -> Void`
 * `forall(d : Dictionary, f : Function)`
   `(Dictionary, (Any, Any) -> Boolean)) -> Boolean`

 * `exists(d : Dictionary, f : Function)`
   `(Dictionary, (Any, Any) -> Boolean)) -> Boolean`
 * `filter(d : Dictionary, f : Function)`
   `(Dictionary, (Any, Any) -> Boolean)) -> Dictionary`
 * `map(d : Dictionary, f : Function)`
   `(Dictionary, (Any, Any) -> Any)) -> Dictionary`
 * `fold(d : Dictionary, n : Any, f : Function)`
   `(Dictionary, Any, (Any, Any) -> Any) -> Any`

### Dictionaries

Dictionary aggregates are written like this: `{ key: value }`
If you do not care about the keys, you can also use `{ v1, v2, v3 }` as a shorthand for `{ 0: v1, 1: v2, 2: v3 }`.

You can use either `.key` or `[keyExpression]` to access the values of the Dictionary.

The following operations are available on Dictionaries: `:` (contains).

The following predefined functions are available on Dictionaries:

 * `size(a)` returns the number of key/value pairs in a

 * `dict2str(a)` returns a as a String

Dictionaries are implemented as `TreeMap`.

### Strings

Strings are written like this: `'Hello, world!'`

The following operations are available on Strings: `&` (concatenation), `eq` (equals), `ew` (ends with), `in` (contains), `ne` (not equals) and `sw` (starts with).

The following predefined functions are available on Strings:

 * `length(a)` returns the number of characters in a

 * `char2ord(a)` returns the Unicode value of the first character of a
 * `str2bool(a)` parsing a to a Boolean
 * `str2dict(a)` parsing a to a Dictionary
 * `str2int(a)` parsing a to an Integer

 * `charat(a, b)` returns the character with the index b in a
 * `substring1(a, b)` returns the substring from a that starts at b
 * `substring2(a, b, c)` returns the substring from a that starts at b and ends at c

 * `left(a, b)` returns the substring from a that starts at 0 and has length b
 * `mid(a, b, c)` returns the substring from a that starts at b and has length c
 * `right(a, b)` returns the substring from a that starts at length - b and has length b

 * `head(a)` returns the first character of the String
 * `tail(a)` returns everything after the first character of the String

 * `findleft(a, b, c)` returns the first occurrence of b in a starting from the left at c
 * `findright(a, b, c)` returns the first occurrence of b in a starting from the right at c
 * `split(a, b)` returns the String b splitted at String a as a Dictionary
 * `join(a, b)` returns the Dictionary b joined with String a as a String
 * `repeat(a, b)` returns a repeated b times
 * `replace(a, b, c)` replace all occurrences of b in a with c

Since the interpreter is written in Java and Strings in Java are Unicode-compatible, you can use Unicode characters in your Strings as well.

### Integers

Integers are written like this:
 * Decimal: `12345`
 * Binary: `0b10101010`
 * Octal: `0o755`
 * Hexadecimal: `0xabcd`

The following operations are available on Integers: `+`, `-`, `*`, `/`, `mod`, `rem`, `=`, `<>`, `<`, `<=`, `>`, `>=`, `n!` and `|x|`.

The following predefined functions are available on Integers:

 * `int2str(a)` returns a as a String
 * `ord2char(a)` returns a String containing a single Unicode character

 * `gcd(a, b)` returns the greatest common divisor of a and b
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

You can check for types by using the predefined functions `boolean`, `dictionary`, `function`, `integer` and `string`:
```
a := 123;
write bool2str(integer(a)); # true
```

## Other operations and functions

### If expressions

`if` expressions are written like this:
```
if expression:
    # expression
elif expression:
    # expression
else:
    # expression
```
There can be 0..n `elif` (short for else if) parts and the `else` part is mandatory.

### Let expressions

`let` expressions are written like this:
```
let a = 1 + 2, b = 3 * 4:
    # expression
```
There can be 1..n bindings after the `let`.

### Switch expressions

`switch` expressions are written like this:
```
switch expression:
case expression1, expression2:
    # expression
default:
    # expression
```
There can be 0..n `case` parts and the `default` part is mandatory.

### Misc. built-in functions

 * `error(a)` exits the current script with the error message a and exit code 1
 * `exit(a)` exits the current script with exit code a
 * `sleep(a)` sleeps/waits for a milliseconds

## Statements

### If statements

`if` statements are written like this:
```
if expression (
    # code
) elif expression (
    # code
) else (
    # code
)
```
There can be 0..n `elif` (short for else if) parts and if not needed, the `else` part can be omitted.

### Switch statements

`switch` statements are written like this:
```
switch expression:
case expression1, expression2 (
    # code
)
default (
    # code
)
```
There can be 0..n `case` parts and if not needed, the `default` part can be omitted.

### Loop statements

`loop` statements are written like this:
```
loop (
    # code
) while expression (
    # code
)
```
If not needed, either the `loop` part or the `while` part can be omitted, but not both.

### For statements

`for` statements are written like this (in this example iterating over the odd numbers from 1 to 100):
```
for i from 1 to 100 step 2 (
    # code
)
```
If not needed, the `step` part can be omitted, it will default to `step 1`.

### Read statements

You can read a line from the command line into a String using: `read myVariable;`

### Write statements

You can write a String as a line to the command line using: `write 'Hello, world!';`

### Assignments

You can assign values to variables using: `myVariable := myValue;`

### Remove statements

You can remove key/value pairs from Dictionaries using: `remove myDictionary.myKey;`

### Run statements

You can run other Aquila scripts using: `run 'script.aq';`

## Using command line arguments

Command line arguments are stored in the `args` variable.

## Using environment variables

Environment variables are stored in the `env` variable. You **cannot** set environment variables from inside your script.
