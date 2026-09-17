#include "io.h"

#include <stdio.h>

/*
 * Set once stdin is exhausted. Every read goes through this file, so a
 * single flag is enough for each command to unwind and for the main loop to
 * stop. Without it, a failed scanf left the caller's variables untouched and
 * the loop spun forever.
 */
static int input_closed = 0;

int io_input_closed(void)
{
    return input_closed;
}

/* Drop the remainder of the current line, so a bad token is not re-read. */
static void discard_line(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        /* discard */
    }
}

/* Report a malformed token once, in one place. */
static void report_invalid(void)
{
    printf("Invalid input. Expected a number.\n");
}

int io_read_int(int *value)
{
    int result = scanf("%d", value);

    if (result == 1)
    {
        return 1;
    }
    if (result == EOF)
    {
        input_closed = 1;
        return 0;
    }
    discard_line();
    report_invalid();
    return 0;
}

int io_read_double(double *value)
{
    int result = scanf("%lf", value);

    if (result == 1)
    {
        return 1;
    }
    if (result == EOF)
    {
        input_closed = 1;
        return 0;
    }
    discard_line();
    report_invalid();
    return 0;
}

/* Reject dimensions the fixed-size arrays cannot hold. */
static int dimensions_valid(int rows, int columns)
{
    if (rows < 1 || columns < 1)
    {
        printf("Matrix dimensions must be at least 1.\n");
        return 0;
    }
    if (rows > MAX_SIZE || columns > MAX_SIZE)
    {
        printf("Matrix size exceeds the maximum allowed (%d x %d).\n",
               MAX_SIZE, MAX_SIZE);
        return 0;
    }
    return 1;
}

int io_read_dimensions(const char *prompt, int *rows, int *columns)
{
    printf("%s", prompt);
    if (!io_read_int(rows) || !io_read_int(columns))
    {
        return 0;
    }
    return dimensions_valid(*rows, *columns);
}

int io_read_matrix(int matrix[][MAX_SIZE], int rows, int columns)
{
    printf("Enter the elements of the matrix (%d x %d):\n", rows, columns);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            if (!io_read_int(&matrix[i][j]))
            {
                return 0;
            }
        }
    }
    return 1;
}

int io_read_matrix_double(double matrix[][MAX_SIZE], int rows, int columns)
{
    printf("Enter the elements of the matrix (%d x %d):\n", rows, columns);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            if (!io_read_double(&matrix[i][j]))
            {
                return 0;
            }
        }
    }
    return 1;
}

void io_print_matrix(const int matrix[][MAX_SIZE], int rows, int columns)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void io_print_matrix_double(const double matrix[][MAX_SIZE], int rows,
                            int columns)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            double value = matrix[i][j];

            /* Round-off can leave a tiny negative, which prints "-0.00". */
            if (value > -0.005 && value < 0.005)
            {
                value = 0.0;
            }
            printf("%.2f ", value);
        }
        printf("\n");
    }
}
