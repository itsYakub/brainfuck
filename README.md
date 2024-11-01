<div align="center">

# BRAINFUCK                                                              

</div>

## README
This is a simple, overcomplicated interpreter for brainfuck, esoteric, imperative and structural minimalistic language, originally created in 1993 by Urban Müller.
It consist's of only 8 instructions represented by single ASCII characters.
Interpreter allows for direct file execution (without compilation), piping file or code into the interpreter via the command-line, or executing strings of code via the command-line.

## GIT
You can get the source code of the interpreter from GitHub gits: [link here](https://gist.github.com/itsYakub/0182664074d29d4050aa6e683809dab1)

You can also get the source code by cloning this repository:
```console
$ git clone git@github.com:itsYakub/brainfuck.git
$ cd brainfuck/
```

## BUILD
Interpreter doesn't require any external dependencies apart from libc. Building the executable require a single command:
```console
$ cc bfi.c -o bfi
```

## LICENCE
This project is under the [MIT LICENCE](./LICENCE)
