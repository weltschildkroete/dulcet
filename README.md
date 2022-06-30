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

The executable receives some lambda expression in classic notation from stdin
or a file given by the `-f` flag, beta reduces it with the normal order
reduction strategy, printing the result to stdout in the end, or a file given
by the `-o` flag. For example,

```console
$ ./dulceti <<< '(\m.\n.\f.\x.m f (n f x)) (\f.\x.f (f x)) (\f.\x.f (f (f x)))' # lambda expression for PLUS 2 3
λa.λb.a (a (a (a (a b)))) # expected output is church-encoded 5
```

For more information on different input or output notations and reduction
strategies, see `dulceti --help`.

To use this as a library in your C code, simply include the `dulcet.h` and
`dulcet_parser.h` headers and link against the object files from the respective
source files.

See the [examples](examples/) for other sample cases.
