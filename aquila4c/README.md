# aquila4c

This is the C++ implementation of Aquila. **This is a work in progress.**

## Building the interpreter

You will need `antlr 4.10.1`, `bash` and `g++` to build the interpreter.
Make sure to put the `antlr-4.10.1-complete.jar` file into the parent directory of this repository.
You will also have to install the [ANTLR C++ runtime](https://github.com/antlr/antlr4/blob/master/doc/cpp-target.md).
Then simply run `./build.sh` to build the `aquila4c` file.

## Installing the interpreter on Ubuntu

Run `cp aquila4c /usr/local/bin/aq`. You might need sudo rights.
You can then use `#!/usr/local/bin/aq` as the shebang in your scripts, which will call the interpreter.
