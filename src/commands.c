#include "commands.h"

#include "io.h"
#include "matrix.h"

#include <stdio.h>
#include <string.h>

#define DIMENSION_PROMPT "Enter the number of rows and columns: "

/*
 * -add and -sub differ only in the operator applied to each pair of
 * elements, so they share one routine.
 */
static void elementwise(const char *label, int subtract)
{
    int rows, columns;
    int first[MAX_SIZE][MAX_SIZE], second[MAX_SIZE][MAX_SIZE];
    int result[MAX_SIZE][MAX_SIZE];

    if (!io_read_dimensions(DIMENSION_PROMPT, &rows, &columns))
    {
        return;
    }
    if (!io_read_matrix(first, rows, columns) ||
        !io_read_matrix(second, rows, columns))
    {
        return;
    }

    if (subtract)
    {
        matrix_subtract(first, second, result, rows, columns);
    }
    else
    {
        matrix_add(first, second, result, rows, columns);
    }

    printf("%s\n", label);
    io_print_matrix(result, rows, columns);
}

static void command_add(void)
{
    elementwise("The sum of the matrices is:", 0);
}

static void command_subtract(void)
{
    elementwise("The difference of the matrices is:", 1);
}

static void command_multiply(void)
{
    int rows1, columns1, rows2, columns2;
    int first[MAX_SIZE][MAX_SIZE], second[MAX_SIZE][MAX_SIZE];
    int product[MAX_SIZE][MAX_SIZE];

    if (!io_read_dimensions(
            "Enter the number of rows and columns for the first matrix: ",
            &rows1, &columns1))
    {
        return;
    }
    if (!io_read_dimensions(
            "Enter the number of rows and columns for the second matrix: ",
            &rows2, &columns2))
    {
        return;
    }

    if (columns1 != rows2)
    {
        printf("Matrix multiplication not possible. Number of columns in the "
               "first matrix should be equal to the number of rows in the "
               "second matrix.\n");
        return;
    }

    if (!io_read_matrix(first, rows1, columns1) ||
        !io_read_matrix(second, rows2, columns2))
    {
        return;
    }

    matrix_multiply(first, second, product, rows1, columns1, columns2);

    printf("The product of the matrices is:\n");
    io_print_matrix(product, rows1, columns2);
}

static void command_multiply_chain(void)
{
    int count;
    int matrices[MAX_MATRICES][MAX_SIZE][MAX_SIZE];
    int rows[MAX_MATRICES], columns[MAX_MATRICES];
    int result[MAX_SIZE][MAX_SIZE];

    printf("Enter the number of matrices to multiply: ");
    if (!io_read_int(&count))
    {
        return;
    }

    if (count < 2)
    {
        printf("At least two matrices are required for multiplication.\n");
        return;
    }
    if (count > MAX_MATRICES)
    {
        printf("At most %d matrices can be multiplied at once.\n",
               MAX_MATRICES);
        return;
    }

    for (int k = 0; k < count; k++)
    {
        char prompt[64];

        snprintf(prompt, sizeof(prompt),
                 "Enter the number of rows and columns for matrix %d: ",
                 k + 1);
        if (!io_read_dimensions(prompt, &rows[k], &columns[k]))
        {
            return;
        }

        /* Check compatibility before asking for elements the chain
           cannot use. */
        if (k > 0 && columns[k - 1] != rows[k])
        {
            printf("Matrix multiplication not possible between matrix %d "
                   "and %d.\n", k, k + 1);
            return;
        }

        if (!io_read_matrix(matrices[k], rows[k], columns[k]))
        {
            return;
        }
    }

    memcpy(result, matrices[0], sizeof(result));

    for (int k = 1; k < count; k++)
    {
        int temp[MAX_SIZE][MAX_SIZE];

        matrix_multiply(result, matrices[k], temp, rows[0], columns[k - 1],
                        columns[k]);
        memcpy(result, temp, sizeof(result));
    }

    printf("The result of multiple matrix multiplication is:\n");
    io_print_matrix(result, rows[0], columns[count - 1]);
}

static void command_transpose(void)
{
    int rows, columns;
    int matrix[MAX_SIZE][MAX_SIZE], transpose[MAX_SIZE][MAX_SIZE];

    if (!io_read_dimensions(DIMENSION_PROMPT, &rows, &columns))
    {
        return;
    }
    if (!io_read_matrix(matrix, rows, columns))
    {
        return;
    }

    matrix_transpose(matrix, transpose, rows, columns);

    printf("The transpose of the matrix is:\n");
    io_print_matrix(transpose, columns, rows);
}

static void command_inverse(void)
{
    int rows, columns;
    double matrix[MAX_SIZE][MAX_SIZE], inverse[MAX_SIZE][MAX_SIZE];

    if (!io_read_dimensions(DIMENSION_PROMPT, &rows, &columns))
    {
        return;
    }
    if (rows != columns)
    {
        printf("Inverse of a matrix can only be found for a square matrix.\n");
        return;
    }
    if (!io_read_matrix_double(matrix, rows, rows))
    {
        return;
    }

    if (matrix_invert(matrix, inverse, rows) == MATRIX_SINGULAR)
    {
        printf("Matrix is singular and cannot be inverted.\n");
        return;
    }

    printf("The inverse of the matrix is:\n");
    io_print_matrix_double(inverse, rows, rows);
}

static void command_determinant(void)
{
    int rows, columns;
    int matrix[MAX_SIZE][MAX_SIZE];

    if (!io_read_dimensions(DIMENSION_PROMPT, &rows, &columns))
    {
        return;
    }
    if (rows != columns)
    {
        printf("Determinant is only defined for a square matrix.\n");
        return;
    }
    if (!io_read_matrix(matrix, rows, rows))
    {
        return;
    }

    printf("The determinant of the matrix is: %lld\n",
           matrix_determinant(matrix, rows));
}

static void command_properties(void)
{
    printf("Properties of Matrix Multiplication:\n");
    printf("1. Associative: (AB)C = A(BC)\n");
    printf("2. Distributive: A(B + C) = AB + AC\n");
    printf("3. Not Commutative: AB != BA (in general)\n");
    printf("4. Identity Matrix: AI = IA = A\n");
    printf("5. Zero Matrix: A0 = 0 = 0A\n");
}

/*
 * The command table. It drives both dispatch and the help text, so a new
 * command is added in exactly one place and cannot go undocumented.
 * A NULL handler marks the command that ends the session.
 */
static const struct
{
    const char *name;
    const char *description;
    void (*run)(void);
} COMMANDS[] = {
    {"--help",  "Display this help message",                  command_help},
    {"-add",    "Perform matrix addition",                    command_add},
    {"-sub",    "Perform matrix subtraction",                 command_subtract},
    {"-multi",  "Perform matrix multiplication",              command_multiply},
    {"-mmulti", "Perform multiple matrix multiplication",
     command_multiply_chain},
    {"-props",  "Display properties of matrices",             command_properties},
    {"-trans",  "Perform matrix transpose",                   command_transpose},
    {"-inv",    "Perform matrix inversion",                   command_inverse},
    {"-det",    "Compute the determinant of a square matrix", command_determinant},
    {"-exit",   "Exit the program",                           NULL},
};

#define COMMAND_COUNT ((int)(sizeof(COMMANDS) / sizeof(COMMANDS[0])))

void command_help(void)
{
    printf("Options:\n");
    for (int i = 0; i < COMMAND_COUNT; i++)
    {
        printf("  %-10s%s\n", COMMANDS[i].name, COMMANDS[i].description);
    }
}

CommandResult command_run(const char *name)
{
    for (int i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcmp(name, COMMANDS[i].name) != 0)
        {
            continue;
        }
        if (COMMANDS[i].run == NULL)
        {
            return COMMAND_EXIT;
        }
        COMMANDS[i].run();
        return COMMAND_RAN;
    }
    return COMMAND_UNKNOWN;
}
