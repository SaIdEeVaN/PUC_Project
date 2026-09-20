/*
 * Matrix operations.
 *
 * Everything here is pure: it reads its inputs, writes its outputs and
 * prints nothing. Reporting a failure to the user is the caller's job, so
 * these can be reasoned about and tested without any I/O.
 */

#ifndef MATRIX_H
#define MATRIX_H

/* Largest matrix accepted, in either dimension. */
#define MAX_SIZE 10

/* Largest chain accepted by the repeated-multiplication command. */
#define MAX_MATRICES 10

typedef enum
{
    MATRIX_OK = 0,
    MATRIX_SINGULAR
} MatrixStatus;

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
long long matrix_determinant(const int in[][MAX_SIZE], int n);

/*
 * Inverse of an n x n matrix. Returns MATRIX_SINGULAR and leaves `inverse`
 * unspecified if the matrix cannot be inverted.
 */
MatrixStatus matrix_invert(const double in[][MAX_SIZE],
                           double inverse[][MAX_SIZE], int n);

#endif /* MATRIX_H */
