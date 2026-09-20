#include "matrix.h"

#include <math.h>

/* Pivots smaller than this in magnitude are treated as zero. */
#define PIVOT_EPSILON 1e-12

void matrix_add(const int left[][MAX_SIZE], const int right[][MAX_SIZE],
                int out[][MAX_SIZE], int rows, int columns)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            out[i][j] = left[i][j] + right[i][j];
        }
    }
}

void matrix_subtract(const int left[][MAX_SIZE], const int right[][MAX_SIZE],
                     int out[][MAX_SIZE], int rows, int columns)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            out[i][j] = left[i][j] - right[i][j];
        }
    }
}

void matrix_multiply(const int left[][MAX_SIZE], const int right[][MAX_SIZE],
                     int out[][MAX_SIZE], int rows_left, int inner,
                     int columns_right)
{
    for (int i = 0; i < rows_left; i++)
    {
        for (int j = 0; j < columns_right; j++)
        {
            int total = 0;
            for (int k = 0; k < inner; k++)
            {
                total += left[i][k] * right[k][j];
            }
            out[i][j] = total;
        }
    }
}

void matrix_transpose(const int in[][MAX_SIZE], int out[][MAX_SIZE],
                      int rows, int columns)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            out[j][i] = in[i][j];
        }
    }
}

/*
 * Determinant by Bareiss fraction-free elimination.
 *
 * Every division in the inner loop is exact, so an integer matrix yields an
 * exact integer determinant with none of the round-off that a floating-point
 * LU decomposition would introduce.
 *
 * The binding limit is the intermediate products rather than the
 * determinant: work[i][j] * work[k][k] multiplies two minors, so it
 * overflows long before the result would. The elimination therefore runs in
 * MatrixDet, which is __int128 wherever the compiler has it. Overflow is
 * still not detected, but the range it takes to provoke is far out of reach
 * of anything this calculator can be typed; the README has the measured
 * figures.
 */
MatrixDet matrix_determinant(const int in[][MAX_SIZE], int n)
{
    MatrixDet work[MAX_SIZE][MAX_SIZE];
    MatrixDet previous = 1;
    int sign = 1;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            work[i][j] = in[i][j];
        }
    }

    for (int k = 0; k < n - 1; k++)
    {
        if (work[k][k] == 0)
        {
            int swap = -1;
            for (int row = k + 1; row < n; row++)
            {
                if (work[row][k] != 0)
                {
                    swap = row;
                    break;
                }
            }

            /* A wholly zero column below the diagonal means a zero
               determinant, and also guards the division by `previous`. */
            if (swap < 0)
            {
                return 0;
            }

            for (int j = 0; j < n; j++)
            {
                MatrixDet temp = work[k][j];
                work[k][j] = work[swap][j];
                work[swap][j] = temp;
            }
            sign = -sign;
        }

        for (int i = k + 1; i < n; i++)
        {
            for (int j = k + 1; j < n; j++)
            {
                work[i][j] = (work[i][j] * work[k][k]
                              - work[i][k] * work[k][j]) / previous;
            }
        }
        previous = work[k][k];
    }

    return sign * work[n - 1][n - 1];
}

const char *matrix_det_to_string(MatrixDet value, char *buffer, size_t size)
{
    char digits[MATRIX_DET_DIGITS + 1];
    int count = 0;
    int negative = value < 0;

    /* Build the magnitude unsigned, so the most negative value - which has
       no positive counterpart - negates without overflowing. */
    MatrixDetMagnitude magnitude = negative
        ? (MatrixDetMagnitude)0 - (MatrixDetMagnitude)value
        : (MatrixDetMagnitude)value;

    do
    {
        digits[count++] = (char)('0' + (int)(magnitude % 10));
        magnitude /= 10;
    } while (magnitude != 0);

    if (size < (size_t)count + (negative ? 2u : 1u))
    {
        if (size > 0)
        {
            buffer[0] = '\0';
        }
        return buffer;
    }

    char *out = buffer;
    if (negative)
    {
        *out++ = '-';
    }
    while (count > 0)
    {
        *out++ = digits[--count];
    }
    *out = '\0';

    return buffer;
}

/* Exchange two rows of an n-column matrix. */
static void swap_rows(double matrix[][MAX_SIZE], int a, int b, int n)
{
    for (int j = 0; j < n; j++)
    {
        double temp = matrix[a][j];
        matrix[a][j] = matrix[b][j];
        matrix[b][j] = temp;
    }
}

/*
 * Gauss-Jordan elimination with partial pivoting.
 *
 * Taking the largest available pivot keeps the elimination numerically
 * stable, and it is what lets a matrix with a zero on the diagonal -
 * [[0,1],[1,0]], say - be inverted at all rather than being mistaken for a
 * singular one.
 */
MatrixStatus matrix_invert(const double in[][MAX_SIZE],
                           double inverse[][MAX_SIZE], int n)
{
    double work[MAX_SIZE][MAX_SIZE];

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            work[i][j] = in[i][j];
            inverse[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    for (int col = 0; col < n; col++)
    {
        int pivot = col;
        for (int row = col + 1; row < n; row++)
        {
            if (fabs(work[row][col]) > fabs(work[pivot][col]))
            {
                pivot = row;
            }
        }

        if (fabs(work[pivot][col]) < PIVOT_EPSILON)
        {
            return MATRIX_SINGULAR;
        }

        if (pivot != col)
        {
            swap_rows(work, pivot, col, n);
            swap_rows(inverse, pivot, col, n);
        }

        double diagonal = work[col][col];
        for (int j = 0; j < n; j++)
        {
            work[col][j] /= diagonal;
            inverse[col][j] /= diagonal;
        }

        for (int row = 0; row < n; row++)
        {
            if (row == col)
            {
                continue;
            }

            double factor = work[row][col];
            if (factor == 0.0)
            {
                continue;
            }

            for (int j = 0; j < n; j++)
            {
                work[row][j] -= work[col][j] * factor;
                inverse[row][j] -= inverse[col][j] * factor;
            }
        }
    }

    return MATRIX_OK;
}
