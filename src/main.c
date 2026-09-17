/*
 * Matrix Calculator
 *
 * An interactive calculator for small matrices. Reads a command at a prompt
 * and runs it; see commands.c for the command table.
 */

#include "commands.h"
#include "io.h"

#include <stdio.h>

/* Keep the scanf field width in step with the buffer size. */
#define OPTION_MAX 99
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define OPTION_FORMAT "%" STRINGIFY(OPTION_MAX) "s"

static void print_banner(void)
{
    printf(" __  __       _        _         _____      _            _       _             \n");
    printf("|  \\/  |     | |      (_)       / ____|    | |          | |     | |            \n");
    printf("| \\  / | __ _| |_ _ __ ___  __ | |     __ _| | ___ _   _| | __ _| |_ ___  _ __ \n");
    printf("| |\\/| |/ _` | __| '__| \\ \\/ / | |    / _` | |/ __| | | | |/ _` | __/ _ \\| '__|\n");
    printf("| |  | | (_| | |_| |  | |>  <  | |___| (_| | | (__| |_| | | (_| | || (_) | |   \n");
    printf("|_|  |_|\\__,_|\\__|_|  |_/_/\\_\\  \\_____|\\__,_|_|\\___|\\__,_|_|\\__,_|\\__\\___/|_|   \n");
    printf("                                                                               \n");
}

int main(void)
{
    print_banner();

    printf("Use --help to see the available options.\n");

    while (!io_input_closed())
    {
        char option[OPTION_MAX + 1];

        printf("\nEnter an option: ");
        if (scanf(OPTION_FORMAT, option) != 1)
        {
            break;
        }

        CommandResult result = command_run(option);

        if (result == COMMAND_EXIT)
        {
            break;
        }
        if (result == COMMAND_UNKNOWN)
        {
            printf("Unknown option: %s. Use --help for usage information.\n",
                   option);
        }
    }

    printf("Exiting the program.\n");
    return 0;
}
