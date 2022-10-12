#ifndef THSH_H
#define THSH_H

/* Do not change this file */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>

// Assume no input line will be longer than 1023 bytes (+NULL)
#define MAX_INPUT    1024

// Assume a pipeline will never be longer than 31 stages (+NULL)
#define MAX_PIPELINE   32

// Assume any individual command will not have more than 15 arguments (+NULL)
#define MAX_ARGS       16

// Disallow exec*p* variants, lest we spoil the fun
#pragma GCC poison execlp execvp execvpe

// Helper functions

// In parse.c:
int read_one_line(int input_fd, char * buf, size_t size);
int parse_line (char *inbuf, size_t length, char *commands [MAX_PIPELINE][MAX_ARGS],
		char **infile, char **outfile);

// In builtin.c:
int init_cwd(void);
int handle_builtin(char *args[MAX_ARGS], int stdin, int stdout, int *retval);
int print_prompt(void);

// In jobs.c:
int init_path(void);
void print_path_table(void);
int create_job(void);
int run_command(char *args[MAX_ARGS], int stdin, int stdout, int job_id);
int wait_on_job(int job_id, int *exit_code);

#endif // THSH_H
