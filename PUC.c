/*
 * Matrix Calculator
 *
 * An interactive calculator for small matrices. Commands are read from
 * stdin at a prompt; see help() for the list.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

/* Largest matrix the calculator will accept, in either dimension. */
#define MAX_SIZE 5

/* Largest chain length accepted by -mmulti. */
#define MAX_MATRICES 10

/* Pivots smaller than this in magnitude are treated as zero. */
#define PIVOT_EPSILON 1e-12

/* Keep the scanf field width in step with the buffer size. */
#define OPTION_MAX 99
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define OPTION_FORMAT "%" STRINGIFY(OPTION_MAX) "s"

/*
 * Set once stdin is exhausted. Every read goes through the helpers below,
 * so a single flag is enough for each command to unwind and for the main
 * loop to stop. Without it, a failed scanf left the caller's variables
 * untouched and the loop spun forever.
 */
static int input_closed = 0;

/* ------------------------------------------------------------------ */
/* Input                                                              */
/* ------------------------------------------------------------------ */

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

/* Read one integer. Returns 1 on success, 0 on EOF or malformed input. */
static int read_int(int *value)
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

/* As read_int, but for the double-precision values used by -inv. */
static int read_double(double *value)
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

/* Prompt for a validated rows/columns pair. Returns 1 on success. */
static int read_dimensions(const char *prompt, int *rows, int *columns)
{
    printf("%s", prompt);
    if (!read_int(rows) || !read_int(columns))
    {
        return 0;
    }
    return dimensions_valid(*rows, *columns);
}

static int read_matrix(int matrix[][MAX_SIZE], int rows, int columns)
{
    printf("Enter the elements of the matrix (%d x %d):\n", rows, columns);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            if (!read_int(&matrix[i][j]))
            {
                return 0;
            }
        }
    }
    return 1;
}

static int read_matrix_double(double matrix[][MAX_SIZE], int rows, int columns)
{
    printf("Enter the elements of the matrix (%d x %d):\n", rows, columns);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            if (!read_double(&matrix[i][j]))
            {
                return 0;
            }
        }
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Output                                                             */
/* ------------------------------------------------------------------ */

static void print_matrix(const int matrix[][MAX_SIZE], int rows, int columns)
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

static void print_matrix_double(const double matrix[][MAX_SIZE], int rows,
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

/* ------------------------------------------------------------------ */
/* Commands                                                           */
/* ------------------------------------------------------------------ */

static void help(void)
{
    printf("Options:\n");
    printf("  --help    Display this help message\n");
    printf("  -add      Perform matrix addition\n");
    printf("  -sub      Perform matrix subtraction\n");
    printf("  -multi    Perform matrix multiplication\n");
    printf("  -mmulti   Perform multiple matrix multiplication\n");
    printf("  -props    Display properties of matrices\n");
    printf("  -trans    Perform matrix transpose\n");
    printf("  -inv      Perform matrix inversion\n");
    printf("  -det      Compute the determinant of a square matrix\n");
    printf("  -exit     Exit the program\n");
}

/*
 * -add and -sub differ only in the operator applied to each pair of
 * elements, so they share one routine.
 */
static void elementwise(const char *label, int subtract)
{
    int rows, columns;
    int first[MAX_SIZE][MAX_SIZE], second[MAX_SIZE][MAX_SIZE];
    int result[MAX_SIZE][MAX_SIZE];

    if (!read_dimensions("Enter the number of rows and columns: ",
                         &rows, &columns))
    {
        return;
    }
    if (!read_matrix(first, rows, columns) ||
        !read_matrix(second, rows, columns))
    {
        return;
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            result[i][j] = subtract ? first[i][j] - second[i][j]
                                    : first[i][j] + second[i][j];
        }
    }

    printf("%s\n", label);
    print_matrix(result, rows, columns);
}

static void sum_matrix(void)
{
    elementwise("The sum of the matrices is:", 0);
}

static void diff_matrix(void)
{
    elementwise("The difference of the matrices is:", 1);
}

/*
 * out = left * right, where left is rows_left x inner and right is
 * inner x columns_right. Shared by -multi and -mmulti.
 */
static void multiply(const int left[][MAX_SIZE], const int right[][MAX_SIZE],
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

static void multiplication_matrix(void)
{
    int rows1, columns1, rows2, columns2;
    int first[MAX_SIZE][MAX_SIZE], second[MAX_SIZE][MAX_SIZE];
    int product[MAX_SIZE][MAX_SIZE];

    if (!read_dimensions(
            "Enter the number of rows and columns for the first matrix: ",
            &rows1, &columns1))
    {
        return;
    }
    if (!read_dimensions(
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

    if (!read_matrix(first, rows1, columns1) ||
        !read_matrix(second, rows2, columns2))
    {
        return;
    }

    multiply(first, second, product, rows1, columns1, columns2);

    printf("The product of the matrices is:\n");
    print_matrix(product, rows1, columns2);
}

static void multiple_matrix_multiplication(void)
{
    int count;
    int matrices[MAX_MATRICES][MAX_SIZE][MAX_SIZE];
    int rows[MAX_MATRICES], columns[MAX_MATRICES];
    int result[MAX_SIZE][MAX_SIZE];

    printf("Enter the number of matrices to multiply: ");
    if (!read_int(&count))
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
        if (!read_dimensions(prompt, &rows[k], &columns[k]))
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

        if (!read_matrix(matrices[k], rows[k], columns[k]))
        {
            return;
        }
    }

    memcpy(result, matrices[0], sizeof(result));

    for (int k = 1; k < count; k++)
    {
        int temp[MAX_SIZE][MAX_SIZE];

        multiply(result, matrices[k], temp, rows[0], columns[k - 1],
                 columns[k]);
        memcpy(result, temp, sizeof(result));
    }

    printf("The result of multiple matrix multiplication is:\n");
    print_matrix(result, rows[0], columns[count - 1]);
}

static void matrix_properties(void)
{
    printf("Properties of Matrix Multiplication:\n");
    printf("1. Associative: (AB)C = A(BC)\n");
    printf("2. Distributive: A(B + C) = AB + AC\n");
    printf("3. Not Commutative: AB != BA (in general)\n");
    printf("4. Identity Matrix: AI = IA = A\n");
    printf("5. Zero Matrix: A0 = 0 = 0A\n");
}

static void transpose_matrix(void)
{
    int rows, columns;
    int matrix[MAX_SIZE][MAX_SIZE], transpose[MAX_SIZE][MAX_SIZE];

    if (!read_dimensions("Enter the number of rows and columns: ",
                         &rows, &columns))
    {
        return;
    }
    if (!read_matrix(matrix, rows, columns))
    {
        return;
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            transpose[j][i] = matrix[i][j];
        }
    }

    printf("The transpose of the matrix is:\n");
    print_matrix(transpose, columns, rows);
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

static void inverse_matrix(void)
{
    int rows, columns;
    double matrix[MAX_SIZE][MAX_SIZE], inverse[MAX_SIZE][MAX_SIZE];

    if (!read_dimensions("Enter the number of rows and columns: ",
                         &rows, &columns))
    {
        return;
    }
    if (rows != columns)
    {
        printf("Inverse of a matrix can only be found for a square matrix.\n");
        return;
    }

    int n = rows;
    if (!read_matrix_double(matrix, n, n))
    {
        return;
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            inverse[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    /* Gauss-Jordan elimination with partial pivoting. Taking the largest
       available pivot keeps the elimination numerically stable, and it is
       what lets a matrix with a zero on the diagonal - [[0,1],[1,0]], say -
       be inverted at all rather than being mistaken for a singular one. */
    for (int col = 0; col < n; col++)
    {
        int pivot = col;
        for (int row = col + 1; row < n; row++)
        {
            if (fabs(matrix[row][col]) > fabs(matrix[pivot][col]))
            {
                pivot = row;
            }
        }

        if (fabs(matrix[pivot][col]) < PIVOT_EPSILON)
        {
            printf("Matrix is singular and cannot be inverted.\n");
            return;
        }

        if (pivot != col)
        {
            swap_rows(matrix, pivot, col, n);
            swap_rows(inverse, pivot, col, n);
        }

        double diagonal = matrix[col][col];
        for (int j = 0; j < n; j++)
        {
            matrix[col][j] /= diagonal;
            inverse[col][j] /= diagonal;
        }

        for (int row = 0; row < n; row++)
        {
            if (row == col)
            {
                continue;
            }

            double factor = matrix[row][col];
            if (factor == 0.0)
            {
                continue;
            }

            for (int j = 0; j < n; j++)
            {
                matrix[row][j] -= matrix[col][j] * factor;
                inverse[row][j] -= inverse[col][j] * factor;
            }
        }
    }

    printf("The inverse of the matrix is:\n");
    print_matrix_double(inverse, n, n);
}

/*
 * Determinant by Bareiss fraction-free elimination.
 *
 * Every division in the inner loop is exact, so an integer matrix yields an
 * exact integer determinant with none of the round-off that a floating-point
 * LU decomposition would introduce. Intermediate values are bounded by the
 * Hadamard bound, which for a 5x5 keeps entries up to roughly +/-2700 exact
 * in 64 bits; beyond that the arithmetic can overflow, which is not detected.
 */
static long long determinant(const int matrix[][MAX_SIZE], int n)
{
    long long work[MAX_SIZE][MAX_SIZE];
    long long previous = 1;
    int sign = 1;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            work[i][j] = matrix[i][j];
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
                long long temp = work[k][j];
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

static void determinant_matrix(void)
{
    int rows, columns;
    int matrix[MAX_SIZE][MAX_SIZE];

    if (!read_dimensions("Enter the number of rows and columns: ",
                         &rows, &columns))
    {
        return;
    }
    if (rows != columns)
    {
        printf("Determinant is only defined for a square matrix.\n");
        return;
    }
    if (!read_matrix(matrix, rows, rows))
    {
        return;
    }

    printf("The determinant of the matrix is: %lld\n",
           determinant(matrix, rows));
}

/* ------------------------------------------------------------------ */

int main(void)
{
    printf(" __  __       _        _         _____      _            _       _             \n");
    printf("|  \\/  |     | |      (_)       / ____|    | |          | |     | |            \n");
    printf("| \\  / | __ _| |_ _ __ ___  __ | |     __ _| | ___ _   _| | __ _| |_ ___  _ __ \n");
    printf("| |\\/| |/ _` | __| '__| \\ \\/ / | |    / _` | |/ __| | | | |/ _` | __/ _ \\| '__|\n");
    printf("| |  | | (_| | |_| |  | |>  <  | |___| (_| | | (__| |_| | | (_| | || (_) | |   \n");
    printf("|_|  |_|\\__,_|\\__|_|  |_/_/\\_\\  \\_____|\\__,_|_|\\___|\\__,_|_|\\__,_|\\__\\___/|_|   \n");
    printf("                                                                               \n");

    printf("Use --help to see the available options.\n");

    while (!input_closed)
    {
        char option[OPTION_MAX + 1];

        printf("\nEnter an option: ");
        if (scanf(OPTION_FORMAT, option) != 1)
        {
            break;
        }

        if (strcmp(option, "--help") == 0)
        {
            help();
        }
        else if (strcmp(option, "-add") == 0)
        {
            sum_matrix();
        }
        else if (strcmp(option, "-sub") == 0)
        {
            diff_matrix();
        }
        else if (strcmp(option, "-multi") == 0)
        {
            multiplication_matrix();
        }
        else if (strcmp(option, "-mmulti") == 0)
        {
            multiple_matrix_multiplication();
        }
        else if (strcmp(option, "-props") == 0)
        {
            matrix_properties();
        }
        else if (strcmp(option, "-trans") == 0)
        {
            transpose_matrix();
        }
        else if (strcmp(option, "-inv") == 0)
        {
            inverse_matrix();
        }
        else if (strcmp(option, "-det") == 0)
        {
            determinant_matrix();
        }
        else if (strcmp(option, "-exit") == 0)
        {
            break;
        }
        else
        {
            printf("Unknown option: %s. Use --help for usage information.\n",
                   option);
        }
    }

    printf("Exiting the program.\n");
    return 0;
}
