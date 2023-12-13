# Aquila

Aquila is a small scripting language that is run on an interpreter. This language has been influenced by JavaScript/ActionScript, Ada/Pascal, Bash, Batch, Groovy, Haskell, Perl, Python, Ruby, VBScript and Visual Basic 6. Because of the simplicity of the language, no context analysis is necessary when interpreting code.

## aquila4c

This is the C++ 17 implementation of Aquila. See [here](aquila4c/README.md) for details on how to build `aquila4c`. **This is a work in progress.**

## aquila4j

This is the Java 17 implementation of Aquila. See [here](aquila4j/README.md) for details on how to build `aquila4j`.

## Running the test suite on Ubuntu

Make sure that you have already installed an Aquila interpreter as `aq`.
Then you can simply run `./run-tests.sh` and the whole test suite will be executed.

## Syntax highlighting for gedit

Simply copy the `aquila.lang` file to the `language-specs/` directory of your gedit installation:
`cp aquila.lang /usr/share/gtksourceview-4/language-specs/aquila.lang`. You might need sudo rights.

# Language overview

You can find a lot of example scripts in the `examples/` directory.

## Comments

Comments are written like this: `# this is a comment`

## Types

There are five types as of now: `Boolean`, `Integer`, `String`, `Function` and `Dictionary`. You **cannot** define your own types. There is also the `Any` type, which can be used to explicitly signify that a functions argument can be of any of the above types.

### Boolean

The two `Boolean` literals are written as `true` and `false`.

The following operations are available on booleans:
 * `not` a
 * a `and` b
 * a `or` b
 * a `xor` b

The following predefined functions are available on booleans:

Type conversion functions:
 * `bool2str(a)` returns `a` as a `String`

### Integer

Integer literals are written like this:
 * Decimal: `12345`
 * Binary: `0b10101010`
 * Octal: `0o755`
 * Hexadecimal: `0xabcd`

The following operations are available on integers:
 * `+` a
 * `-` a
 * a `+` b
 * a `-` b
 * a `*` b
 * a `/` b
 * a `mod` b
 * a `rem` b
 * a `=` b
 * a `<>` b
 * a `<` b
 * a `<=` b
 * a `>` b
 * a `>=` b
 * a `!`: the factorial of `a`
 * `|` a `|`: the absolute value of `a`

The following predefined functions are available on integers:

Type conversion functions:
 * `int2str(a)` returns `a` as a `String`
 * `ord2char(a)` returns a `String` containing the Unicode character with codepoint `a`

Numerical functions:
 * `gcd(a, b)` returns the greatest common divisor of `a` and `b`
 * `pow(a, b)` returns `a` to the power of `b`
 * `sgn(a)` returns the sign of `a`
 * `sqrt(a)` returns the square root of `a`

There are also predefined `Integer` functions available under `prelude/integerfunctions.aq`, which can be used by first running `run 'prelude/integerfunctions.aq';`.

### String

String literals are written like this: `'Hello, world!'`

The following operations are available on strings:
 * a `&` b: concatenating `a` and `b`
 * a `eq` b: whether `a` equals `b`
 * a `ew` b: whether `a` ends with `b`
 * a `in` b: whether `b` contains `a`
 * a `ne` b: whether `a` does not equal `b`
 * a `sw` b: whether `a` starts with `b`

The following predefined functions are available on strings:

General functions:
 * `length(a)` returns the number of characters in `a`

Type conversion functions:
 * `char2ord(a)` returns the Unicode value of the first character of `a` as an `Integer`
 * `str2bool(a)` parsing `a` to a `Boolean`
 * `str2dict(a)` parsing `a` to a `Dictionary`
 * `str2int(a)` parsing `a` to an `Integer`

Java style charat/substring functions:
 * `charat(a, b)` returns the character with the index `b` in `a`
 * `substring1(a, b)` returns the substring from `a` that starts at `b`
 * `substring2(a, b, c)` returns the substring from `a` that starts at `b` and ends at `c`

VB style left/mid/right functions:
 * `left(a, b)` returns the substring from `a` that starts at `0` and has length `b`
 * `mid(a, b, c)` returns the substring from `a` that starts at `b` and has length `c`
 * `right(a, b)` returns the substring from `a` that starts at `length - b` and has length `b`

Haskell style head/tail functions:
 * `head(a)` returns the first character of `a`
 * `tail(a)` returns everything after the first character of `a`

Finding substrings in strings:
 * `findleft(a, b, c)` returns the first occurrence of `b` in `a` starting from the left at `c`
 * `findright(a, b, c)` returns the first occurrence of `b` in `a` starting from the right at `c`

Splitting and joining strings:
 * `split(a, b)` returns the `String` `b` splitted at `String` `a` as a `Dictionary`
 * `join(a, b)` returns the `Dictionary` `b` joined with `String` `a` as a `String`

Other functions:
 * `repeat(a, b)` returns `a` repeated `b` times
 * `replace(a, b, c)` replace all occurrences of `b` in `a` with `c`

There are also predefined `String` functions available under `prelude/stringfunctions.aq`, which can be used by first running `run 'prelude/stringfunctions.aq';`.

### Function

Functions are always written as lambdas: `\x, y -> x + y`

You can assign functions to variables like any other value: `a := \x -> 2 * x;`

You can write recursive functions using the variable that you assign the function to: `factorial := \n -> if n = 0: 1 else: n * factorial(n - 1);`

You can add additional runtime type-safety by stating what type each of parameters has: `a := \x : Integer -> 2 * x;`

Here you can also use `Any` to explicitly state, that the argument can take any type: `\x : Any -> x`

You can write void functions using code blocks: `a := \s -> (write s;);`

The following predefined functions are available on functions:
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

### Dictionary

The keys of dictionaries are always of type `String`.

Dictionary aggregates are written like this: `{ key: value }`

If you do not care about the keys, you can also use `{ v1, v2, v3 }` as a shorthand for `{ 0: v1, 1: v2, 2: v3 }`.

The following operations are available on dictionaries:
 * a `.` b: access value of `Dictionary` `a` via key name `b`
 * a `[` b `]`: access value of `Dictionary` `a` via key expression `b`
 * a `:` b: whether `Dictionary` `b` contains value `a`

The following predefined functions are available on dictionaries:

General functions:
 * `size(a)` returns the number of key/value pairs in `a`
 
Type conversion functions:
 * `dict2str(a)` returns `a` as a `String`

### Checking for types

You can check for types using the predefined functions `boolean`, `dictionary`, `function`, `integer` and `string`:
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

 * `error(a)` exits the current script with the error message `a` and exit code 1

## Assignments

You can assign values to variables using: `myVariable := myValue;`

## Simple statements

### Call statements

You can call a void `Function` using: `call myVoidFunction(p1, p2);`

### Exit statements

You can exit the current script with a specific exit code using: `exit myExitCode;`

### Now statements

You can put the current time in milliseconds into an `Integer` using: `now myTimeVariable;`

### Random statements

You can put a random number that is in a certain range into an `Integer` using: `random myVariable from 1 to 10;`

### Read statements

You can read a line from the command line into a `String` using: `read myVariable;`

### Remove statements

You can remove variables or key/value pairs from dictionaries using: `remove myVariable;` and `remove myDictionary.myKey;` respectively

### Run statements

You can run other external Aquila scripts using: `run 'script.aq';`

### Sleep statements

You can sleep/wait for a specific amount of milliseconds using: `sleep myVariableHoldingMilliseconds;`

### Write statements

You can write a `String` as a line to the command line using: `write 'Hello, world!';`

## Compound statements

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

Omitting the loop part:
```
while expression (
    # code
)
```

Omitting the while part:
```
loop (
    # code
) while expression;
```

### For statements

`for` statements are written like this (in this example iterating over the odd numbers from 1 to 100):
```
for i from 1 to 100 step 2 (
    # code
)
```
If not needed, the `step` part can be omitted, it will default to `step 1`.

## Using command line arguments

Command line arguments are stored in the `args` variable.

## Using environment variables

Environment variables are stored in the `env` variable. You **cannot** set environment variables from inside your script.
