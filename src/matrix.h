/*
 * Matrix operations.
 *
 * Everything here is pure: it reads its inputs, writes its outputs and
 * prints nothing. Reporting a failure to the user is the caller's job, so
 * these can be reasoned about and tested without any I/O.
 */

#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>

/* Largest matrix accepted, in either dimension. */
#define MAX_SIZE 10

/* Largest chain accepted by the repeated-multiplication command. */
#define MAX_MATRICES 10

typedef enum
{
    MATRIX_OK = 0,
    MATRIX_SINGULAR
} MatrixStatus;

/*
 * Determinants are computed in the widest integer the compiler offers.
 * __int128 is a GCC and Clang extension rather than standard C11, so fall
 * back to long long where it is missing; the arithmetic is identical, only
 * the range differs.
 */
#ifdef __SIZEOF_INT128__
typedef __int128 MatrixDet;
typedef unsigned __int128 MatrixDetMagnitude;
#define MATRIX_DET_DIGITS 39
#else
typedef long long MatrixDet;
typedef unsigned long long MatrixDetMagnitude;
#define MATRIX_DET_DIGITS 19
#endif

/* Buffer size that always holds a formatted determinant: sign + digits + NUL. */
#define MATRIX_DET_STRING_MAX (MATRIX_DET_DIGITS + 2)

void matrix_add(const int left[][MAX_SIZE], const int right[][MAX_SIZE],
                int out[][MAX_SIZE], int rows, int columns);

void matrix_subtract(const int left[][MAX_SIZE], const int right[][MAX_SIZE],
                     int out[][MAX_SIZE], int rows, int columns);

/*
 * out = left * right, where left is rows_left x inner and right is
 * inner x columns_right. `out` must not alias either input.
 */
void matrix_multiply(const int left[][MAX_SIZE], const int right[][MAX_SIZE],
                     int out[][MAX_SIZE], int rows_left, int inner,
                     int columns_right);

/* out becomes the columns x rows transpose of a rows x columns input. */
void matrix_transpose(const int in[][MAX_SIZE], int out[][MAX_SIZE],
                      int rows, int columns);

/* Exact integer determinant of an n x n matrix. */
MatrixDet matrix_determinant(const int in[][MAX_SIZE], int n);

/*
 * Render a determinant as decimal. printf has no length modifier for
 * __int128, so the digits are produced by hand. `buffer` must be at least
 * MATRIX_DET_STRING_MAX bytes; returns `buffer`.
 */
const char *matrix_det_to_string(MatrixDet value, char *buffer, size_t size);

/*
 * Inverse of an n x n matrix. Returns MATRIX_SINGULAR and leaves `inverse`
 * unspecified if the matrix cannot be inverted.
 */
MatrixStatus matrix_invert(const double in[][MAX_SIZE],
                           double inverse[][MAX_SIZE], int n);

#endif /* MATRIX_H */
