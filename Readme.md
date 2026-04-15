# V8086

This project is an emulator written in C++ of the classic Intel 8086 cpu.

# Dependencies

- [g++]()

# Build

To build the project a *NoBuild* script has been provided.

First compile the builder

```sh

g++ nob.cpp -o nob

```

From this point on to compile the project execute *nob* from the project root. 
> [!NOTE]
> You do not need to recompile nob.c

To compile the sources

```sh

./nob

```

A program will be generated *main*. This is your emulator

# Run

The emulator has a little CLI. To check how it works just pass `-h` to the program.

```sh

./main -h

```

