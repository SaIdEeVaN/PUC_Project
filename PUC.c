#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_SIZE 5

void sum_matrix();
void diff_matrix();
void multiplication_matrix();
void multiple_matrix_multiplication();
void matrix_properties();
void transpose_matrix();
void inverse_matrix();
void help();

int determinant(int matrix[MAX_SIZE][MAX_SIZE], int n);
void adjoint(int matrix[MAX_SIZE][MAX_SIZE], int adj[MAX_SIZE][MAX_SIZE]);

int main() 
{

    printf(" __  __       _        _         _____      _            _       _             \n");
    printf("|  \\/  |     | |      (_)       / ____|    | |          | |     | |            \n");
    printf("| \\  / | __ _| |_ _ __ ___  __ | |     __ _| | ___ _   _| | __ _| |_ ___  _ __ \n");
    printf("| |\\/| |/ _` | __| '__| \\ \\/ / | |    / _` | |/ __| | | | |/ _` | __/ _ \\| '__|\n");
    printf("| |  | | (_| | |_| |  | |>  <  | |___| (_| | | (__| |_| | | (_| | || (_) | |   \n");
    printf("|_|  |_|\\__,_|\\__|_|  |_/_/\\_\\  \\_____|\\__,_|_|\\___|\\__,_|_|\\__,_|\\__\\___/|_|   \n");
    printf("                                                                               \n");

    printf("Use --help to see the available options.\n");

    while (1) 
    {
        char option[100];
        printf("\nEnter an option: ");
        scanf("%s", option);

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
        else if (strcmp(option, "-exit") == 0) 
        {
            printf("Exiting the program.\n");
            break;
        } 
        else 
        {
            printf("Unknown option: %s. Use --help for usage information.\n", option);
        }
    }
    return 0;
}

void help() 
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
    printf("  -exit     Exit the program\n");
}

void input_matrix(int matrix[MAX_SIZE][MAX_SIZE], int rows, int columns) 
{
    printf("Enter the elements of the matrix (%d x %d):\n", rows, columns);
    for (int i = 0; i < rows; i++) 
    {
        for (int j = 0; j < columns; j++) 
        {
            scanf("%d", &matrix[i][j]);
        }
    }
}

void display_matrix(int matrix[MAX_SIZE][MAX_SIZE], int rows, int columns) 
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

void sum_matrix() 
{
    int rows, columns;
    int matrix1[MAX_SIZE][MAX_SIZE], matrix2[MAX_SIZE][MAX_SIZE], sum[MAX_SIZE][MAX_SIZE];

    printf("Enter the number of rows and columns: ");
    scanf("%d %d", &rows, &columns);

    if (rows > MAX_SIZE || columns > MAX_SIZE) 
    {
        printf("Matrix size exceeds the maximum allowed (%d x %d).\n", MAX_SIZE, MAX_SIZE);
        return;
    }

    input_matrix(matrix1, rows, columns);
    input_matrix(matrix2, rows, columns);

    for (int i = 0; i < rows; i++) 
    {
        for (int j = 0; j < columns; j++) 
        {
            sum[i][j] = matrix1[i][j] + matrix2[i][j];
        }
    }

    printf("The sum of the matrices is:\n");
    display_matrix(sum, rows, columns);
}

void diff_matrix() 
{
    int rows, columns;
    int matrix1[MAX_SIZE][MAX_SIZE], matrix2[MAX_SIZE][MAX_SIZE], diff[MAX_SIZE][MAX_SIZE];

    printf("Enter the number of rows and columns: ");
    scanf("%d %d", &rows, &columns);

    if (rows > MAX_SIZE || columns > MAX_SIZE) 
    {
        printf("Matrix size exceeds the maximum allowed (%d x %d).\n", MAX_SIZE, MAX_SIZE);
        return;
    }

    input_matrix(matrix1, rows, columns);
    input_matrix(matrix2, rows, columns);

    for (int i = 0; i < rows; i++) 
    {
        for (int j = 0; j < columns; j++) 
        {
            diff[i][j] = matrix1[i][j] - matrix2[i][j];
        }
    }

    printf("The difference of the matrices is:\n");
    display_matrix(diff, rows, columns);
}

void multiplication_matrix() 
{
    int rows1, columns1, rows2, columns2;
    int matrix1[MAX_SIZE][MAX_SIZE], matrix2[MAX_SIZE][MAX_SIZE], product[MAX_SIZE][MAX_SIZE] = {0};

    printf("Enter the number of rows and columns for the first matrix: ");
    scanf("%d %d", &rows1, &columns1);

    printf("Enter the number of rows and columns for the second matrix: ");
    scanf("%d %d", &rows2, &columns2);

    if (columns1 != rows2) 
    {
        printf("Matrix multiplication not possible. Number of columns in the first matrix should be equal to the number of rows in the second matrix.\n");
        return;
    }

    input_matrix(matrix1, rows1, columns1);
    input_matrix(matrix2, rows2, columns2);

    for (int i = 0; i < rows1; i++) 
    {
        for (int j = 0; j < columns2; j++) 
        {
            for (int k = 0; k < columns1; k++) 
            {
                product[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }

    printf("The product of the matrices is:\n");
    display_matrix(product, rows1, columns2);
}

void multiple_matrix_multiplication() 
{
    int n;
    printf("Enter the number of matrices to multiply: ");
    scanf("%d", &n);

    if (n < 2) 
    {
        printf("At least two matrices are required for multiplication.\n");
        return;
    }

    int matrices[MAX_SIZE][MAX_SIZE][MAX_SIZE];
    int rows[MAX_SIZE], columns[MAX_SIZE];

    for (int k = 0; k < n; k++) 
    {
        printf("Enter the number of rows and columns for matrix %d: ", k + 1);
        scanf("%d %d", &rows[k], &columns[k]);

        if (rows[k] > MAX_SIZE || columns[k] > MAX_SIZE) 
        {
            printf("Matrix size exceeds the maximum allowed (%d x %d).\n", MAX_SIZE, MAX_SIZE);
            return;
        }

        input_matrix(matrices[k], rows[k], columns[k]);

        if (k > 0 && columns[k - 1] != rows[k]) 
        {
            printf("Matrix multiplication not possible between matrix %d and %d.\n", k, k + 1);
            return;
        }
    }

    int result[MAX_SIZE][MAX_SIZE];
    memcpy(result, matrices[0], sizeof(result));

    for (int k = 1; k < n; k++) 
    {
        int temp[MAX_SIZE][MAX_SIZE] = {0};
        for (int i = 0; i < rows[0]; i++) 
        {
            for (int j = 0; j < columns[k]; j++) 
            {
                for (int l = 0; l < columns[k - 1]; l++) 
                {
                    temp[i][j] += result[i][l] * matrices[k][l][j];
                }
            }
        }
        memcpy(result, temp, sizeof(result));
    }

    printf("The result of multiple matrix multiplication is:\n");
    display_matrix(result, rows[0], columns[n - 1]);
}

void matrix_properties() 
{
    printf("Properties of Matrix Multiplication:\n");
    printf("1. Associative: (AB)C = A(BC)\n");
    printf("2. Distributive: A(B + C) = AB + AC\n");
    printf("3. Not Commutative: AB != BA (in general)\n");
    printf("4. Identity Matrix: AI = IA = A\n");
    printf("5. Zero Matrix: A0 = 0 = 0A\n");
}

void transpose_matrix() 
{
    int rows, columns;
    int matrix[MAX_SIZE][MAX_SIZE], transpose[MAX_SIZE][MAX_SIZE];

    printf("Enter the number of rows and columns: ");
    scanf("%d %d", &rows, &columns);

    if (rows > MAX_SIZE || columns > MAX_SIZE) 
    {
        printf("Matrix size exceeds the maximum allowed (%d x %d).\n", MAX_SIZE, MAX_SIZE);
        return;
    }

    input_matrix(matrix, rows, columns);

    for (int i = 0; i < rows; i++) 
    {
        for (int j = 0; j < columns; j++) 
        {
            transpose[j][i] = matrix[i][j];
        }
    }

    printf("The transpose of the matrix is:\n");
    display_matrix(transpose, columns, rows);
}

void inverse_matrix() 
{
    int rows, columns;
    printf("Enter the number of Rows and Columns of the matrix: ");
    scanf("%d %d", &rows, &columns);

    if (rows != columns) 
    {
        printf("Inverse of a matrix can only be found for a square matrix.\n");
    }

    int n = rows;
    double matrix[MAX_SIZE][MAX_SIZE], temp[MAX_SIZE][MAX_SIZE];

    printf("Enter the elements of the matrix:\n");
    for (int i = 0; i < n; i++) 
    {
        for (int j = 0; j < n; j++) 
        {
            scanf("%lf", &matrix[i][j]);
        }
    }

    // Initialize the identity matrix
    for (int i = 0; i < n; i++) 
    {
        for (int j = 0; j < n; j++) 
        {
            temp[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // Perform Gauss-Jordan Elimination
    for (int i = 0; i < n; i++) 
    {
        double diagElement = matrix[i][i];
        if (diagElement == 0.0) 
        {
            printf("Matrix is singular and cannot be inverted.\n");
        }

        for (int j = 0; j < n; j++) 
        {
            matrix[i][j] /= diagElement;
            temp[i][j] /= diagElement;
        }

        for (int k = 0; k < n; k++) 
        {
            if (k != i) 
            {
                double factor = matrix[k][i];
                for (int j = 0; j < n; j++) 
                {
                    matrix[k][j] -= matrix[i][j] * factor;
                    temp[k][j] -= temp[i][j] * factor;
                }
            }
        }
    }

    // Print the inverse matrix
    printf("The inverse of the matrix is:\n");
    for (int i = 0; i < n; i++) 
    {
        for (int j = 0; j < n; j++) 
        {
            printf("%.2lf ", temp[i][j]);
        }
        printf("\n");
    }
}