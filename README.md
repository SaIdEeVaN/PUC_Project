# Matrix Calculator

A small interactive matrix calculator written in C. It presents a prompt and
performs addition, subtraction, multiplication, transposition and inversion on
matrices entered by hand.

## Build

```sh
make            # builds ./puc
make run        # builds, then runs
make debug      # -O0 -g with AddressSanitizer + UBSan
make clean
```

Or without `make`:

```sh
cc -std=c11 -Wall -Wextra -O2 -o puc PUC.c
```

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
| `-inv`    | Invert a square matrix (Gauss-Jordan elimination) |
| `-exit`   | Quit |

Each command prompts for the dimensions first, then reads the elements in
row-major order, whitespace-separated.

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

## Limits and known issues

- Matrices are capped at **5x5** (`MAX_SIZE` in `PUC.c`).
- All operations use `int` elements except `-inv`, which uses `double` and
  prints to two decimal places.
- Input is **not validated**. The 11 `-Wunused-result` warnings from the `-O2`
  build are real: no `scanf` return value is checked, so non-numeric input
  leaves variables uninitialised rather than producing an error.
- `-inv` prints a message for a non-square or singular matrix but then
  continues anyway instead of returning, so the output that follows is
  meaningless in those cases.
- `-multi` validates that the inner dimensions agree but does not check the
  operands against `MAX_SIZE`, unlike every other command. Asking it for two
  6x6 matrices overruns the arrays; `make debug` reports it as a
  `stack-buffer-overflow`.
- Negative dimensions are accepted everywhere.

These are pre-existing behaviours, documented here rather than changed.
