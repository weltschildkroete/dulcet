# dulcet

An interpreter for the untyped lambda calculus written in C.

## Installation

Requirements are an Unix-like environment with a C11 compiler and POSIX make.

Edit `config.mk` to your liking and run the following commands to build:

```console
$ make
```

and to install the executable (as root, if necessary):

```console
$ make install
```

`dulceti` is installed in `/usr/local` by default.

## Usage

This project includes both an interpreter executable and a library that can be
called from C code.

The executable receives some lambda expression in De Bruijn index notation from
stdin and beta reduces it with the normal order reduction strategy, printing
the result to stdout in the end. For example, one may call

```console
$ echo '(\\\\4 2 (3 2 1)) (\\2 (2 1)) (\\2 (2 (2 1)))' | ./dulceti # lambda expression for PLUS 2 3
\\2 (2 (2 (2 (2 1)))) # expected output is 5
```

To use this as a library in your C code, simply include the `dulcet.h` and
`dulcet_parser.h` headers and link against the object files from the respective
source files.
