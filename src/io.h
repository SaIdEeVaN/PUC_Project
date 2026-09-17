/*
 * Reading matrices and dimensions from stdin, and printing them back.
 *
 * Every read reports its own failure and returns 0, so callers abandon the
 * command with a bare `return`. End of input is sticky: once stdin is
 * exhausted io_input_closed() stays true and the main loop stops.
 */

#ifndef IO_H
#define IO_H

#include "matrix.h"

/* True once stdin has been exhausted. */
int io_input_closed(void);

/* Each returns 1 on success, 0 on EOF or malformed input. */
int io_read_int(int *value);
int io_read_double(double *value);

/* Prompts for a rows/columns pair and validates it against MAX_SIZE. */
int io_read_dimensions(const char *prompt, int *rows, int *columns);

int io_read_matrix(int matrix[][MAX_SIZE], int rows, int columns);
int io_read_matrix_double(double matrix[][MAX_SIZE], int rows, int columns);

void io_print_matrix(const int matrix[][MAX_SIZE], int rows, int columns);
void io_print_matrix_double(const double matrix[][MAX_SIZE], int rows,
                            int columns);

#endif /* IO_H */
