# Matrix Calculator

A small interactive matrix calculator written in C. It presents a prompt and
performs addition, subtraction, multiplication, transposition, inversion and
determinants on matrices entered by hand.

## Build

```sh
make            # builds ./puc
make run        # builds, then runs
make test       # builds, then runs the test suite
make debug      # -O0 -g with AddressSanitizer + UBSan
make clean
```

Or without `make`:

```sh
cc -std=c11 -Wall -Wextra -O2 -Isrc -o puc src/*.c -lm
```

The only requirements are a C11 compiler and a POSIX shell for the tests.

## Layout

| File | Contents |
| ---- | -------- |
| `src/matrix.h` / `.c` | The matrix operations. Pure: they read inputs, write outputs and print nothing, so they can be reasoned about without any I/O. |
| `src/io.h` / `.c` | Reading dimensions and matrices from stdin, and printing them back. Owns all input validation and the end-of-input flag. |
| `src/commands.h` / `.c` | One function per interactive command, wiring the I/O layer to the operations, plus the table that names them. |
| `src/main.c` | The banner and the read-dispatch loop. |

Commands are dispatched through a table in `commands.c` that carries each
name, its help text and its handler. The help output is generated from that
same table, so a new command is added in one place and cannot go
undocumented.

## Usage

Run `./puc` and type commands at the `Enter an option:` prompt. Despite the
leading dashes, these are **interactive commands, not command-line arguments** —
passing them as `./puc -add` has no effect.

| Command   | Action |
| --------- | ------ |
| `--help`  | List the available commands |
| `-add`    | Add two matrices |
| `-sub`    | Subtract the second matrix from the first |
| `-multi`  | Multiply two matrices |
| `-mmulti` | Multiply a chain of matrices |
| `-props`  | Print the algebraic properties of matrix multiplication |
| `-trans`  | Transpose a matrix |
| `-inv`    | Invert a square matrix |
| `-det`    | Determinant of a square matrix |
| `-exit`   | Quit |

Each command prompts for the dimensions first, then reads the elements in
row-major order, whitespace-separated. Invalid input is reported and the
command is abandoned, returning you to the prompt.

### Example

```
Enter an option: -add
Enter the number of rows and columns: 2 2
Enter the elements of the matrix (2 x 2):
1 2
3 4
Enter the elements of the matrix (2 x 2):
5 6
7 8
The sum of the matrices is:
6 8
10 12
```

## Implementation notes

- `-inv` uses **Gauss-Jordan elimination with partial pivoting**. Selecting the
  largest available pivot keeps the elimination numerically stable and lets
  matrices with a zero on the diagonal — `[[0,1],[1,0]]`, for instance — be
  inverted rather than being mistaken for singular ones. Singularity is judged
  against an epsilon rather than an exact comparison against zero.
- `-det` uses **Bareiss fraction-free elimination**. Every division in the inner
  loop is exact, so an integer matrix gives an exact integer determinant with
  no floating-point round-off: a singular matrix reports `0`, not `-2.4e-16`.
- Elements are `int` everywhere except `-inv`, which works in `double` and
  prints to two decimal places.

## Limits

- Matrices are capped at **10x10** (`MAX_SIZE` in `src/matrix.h`), and
  `-mmulti` accepts a chain of at most **10** (`MAX_MATRICES`). Both are single
  constants.
- **`-det` overflows silently on larger matrices.** It computes in 64-bit
  integers, and the binding limit is not the determinant but the intermediate
  products Bareiss forms: `work[i][j] * work[k][k]` multiplies two minors, so
  it overflows well before the result would. The largest entry magnitude that
  came back exact for every one of 25 random matrices at each size:

  | size          | 2-4    | 5    | 6   | 7-8 | 9  | 10 |
  | ------------- | ------ | ---- | --- | --- | -- | -- |
  | safe entries  | ±1000+ | ±100 | ±50 | ±20 | ±10 | ±5 |

  Past those the result can be wrong with no indication, because the overflow
  is not detected. Every other command is unaffected: `-add`, `-sub`, `-multi`
  and `-mmulti` are plain `int` arithmetic, and `-inv` works in `double`.

## Tests

`tests/run_tests.sh` drives the compiled binary over stdin and checks what it
prints. It is POSIX shell with no dependencies, and takes the binary as its
optional first argument:

```sh
make test                      # or:
./tests/run_tests.sh ./puc
```

Each case is checked with a timeout, so a hang fails the run instead of
wedging it. Alongside the per-command cases there is a regression test for
every bug fixed in this repository's history, including the read loop that
used to spin forever on end-of-input and the `-multi` path that used to
overrun its arrays on oversized input.

CI builds with both GCC and Clang, runs the suite under AddressSanitizer and
UBSan, and builds once more with `-Wshadow -Wconversion -Werror`.
