/*
 * The interactive commands, and the table that names them.
 *
 * Each command wires the I/O layer to the matrix operations: it reads its
 * operands, calls into matrix.h and prints the result or an explanation.
 */

#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum
{
    COMMAND_UNKNOWN = 0, /* no command by that name */
    COMMAND_RAN,         /* ran to completion, keep going */
    COMMAND_EXIT         /* the user asked to quit */
} CommandResult;

/* Look up `name` and run it. */
CommandResult command_run(const char *name);

/* Print the command list. Generated from the same table command_run uses,
   so a new command cannot go undocumented. */
void command_help(void);

#endif /* COMMANDS_H */
