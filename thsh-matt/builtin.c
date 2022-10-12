// PID: 730400003
/* COMP 530: Tar Heel SHell
 *
 * This file implements a table of builtin commands.
 */
#include "thsh.h"
#include <stdlib.h>

struct builtin {
  const char * cmd;
  int (*func)(char *args[MAX_ARGS], int stdin, int stdout);
};


static char old_path[MAX_INPUT];
static char cur_path[MAX_INPUT];

/* This function needs to be called once at start-up to initialize
 * the current path.  This should populate cur_path.
 *
 * Returns zero on success, -errno on failure.
 */
int init_cwd(void) {

  // Lab 1: Your code here

  return 0;
}

/* Handle a cd command.  */
// int handle_cd(char *args[MAX_INPUT], int stdin, int stdout) {
int handle_cd(char *args[MAX_ARGS], int stdin, int stdout) {

  // Note that you need to handle special arguments, including:
  // "-" switch to the last directory
  // "." switch to the current directory.  This should change the
  //     behavior of a subsequent "cd -"
  // ".." go up one directory
  //
  // Hint: chdir can handle "." and "..", but saving
  //       these results may not be the desired outcome...

  // XXX: Be sure to write test cases for ., .., and weirdness
  // XXX: Test for errors in the output if a cd fails

  // Lab 1: Your code here
  // set return value
  int retval = 0;

  // make copy of old path for '-' case (last path)
  char old_path_copy[MAX_INPUT];
  strcpy(old_path_copy, old_path);

  // if arg is NULL, return and do nothing
  if(!args[1]) {
        // printf("no argument given, not doing anything\n");
        // printf("old path is: %s\n", old_path);
        // printf("cur path is: %s\n", cur_path);
        return retval;
  }
  // set old path to current working dir if not set by default, append a slash
  // if(old_path[0] == '\0') {
  getcwd(old_path, MAX_INPUT);
  strcat(old_path, "/");
  // }
  // else {
  //     // set old path to cur path for next run
  //     strcpy(old_path, cur_path);
  // }
  // if the arg starts with a slash go to root instead of appending to oldpath, if dash, then set to old path
  // by default, just append the arg to cur path
  switch(args[1][0]) {
      case '/':
          strcpy(cur_path, args[1]);
          // printf("typed /\n");
          break;
      case '-':
          strcpy(cur_path, old_path_copy);
          // printf("typed -\n");
          break;
      case '.':
          if(args[1][1] == '.') {
              strcpy(cur_path, old_path_copy);
              // printf("typed ..\n");
              // printf("first arg: %s\n", &args[1][0]);
              retval = chdir(&args[1][0]);
              return retval;
          } else {
              strcpy(cur_path, old_path_copy);
              // printf("typed .\n");
              retval = chdir(".");
              return retval;
          }
          break;
      default:
          // printf("typed %s\n", args[1]);
          strcpy(cur_path, old_path);
          strcat(cur_path, args[1]);
          // append a slash if last char of arg is not one
          if (args[1][strlen(args[1])-1] != '/')
              strcat(cur_path, "/");
  }
  // change dir
  retval = chdir(cur_path);
  // getcwd(cur_path, MAX_INPUT);
  // printf("old path is: %s\n", old_path);
  // printf("cur path is: %s\n", cur_path);

  return retval;
}

/* Handle an exit command. */
int handle_exit(char *args[MAX_ARGS], int stdin, int stdout) {
  exit(0);
  return 0; // Does not actually return
}


static struct builtin builtins[] = {{"cd", handle_cd},
				    {"exit", handle_exit},
				    {NULL, NULL}};

/* This function checks if the command (args[0]) is a built-in.
 * If so, call the appropriate handler, and return 1.
 * If not, return 0.
 *
 * stdin and stdout are the file handles for standard in and standard out,
 * respectively. These may or may not be used by individual builtin commands.
 *
 * Places the return value of the command in *retval.
 *
 * stdin and stdout should not be closed by this command.
 *
 * In the case of "exit", this function will not return.
 */
int handle_builtin(char *args[MAX_ARGS], int stdin, int stdout, int *retval) {
  int rv = 0;
  // Lab 0: Your Code Here
  // Comment this line once implemented.  This just suppresses
  // the unused variable warning from the compiler.

  // make token of first arg, delimit by space string
  char* token = strtok(args[0], " ");
  // compare token with cd string, set retval & rv accordingly
  if(!strcmp(token, "cd")) {
        rv = handle_cd(args, stdin, stdout);
        *retval = rv;
  }
  // compare token with goheels string, display ascii
  if(!strcmp(token, "goheels")) {
    printf("\n /&&&&&&&&&\n");
    printf(" &&&*******&&&&&&&&&(\n");
    printf(" &&&********&&******&&&&&&&&\n");
    printf(" &&&********/&/*****&******&&&&&&\n");
    printf(" &&********&&&&&&&&&&****/&*****&&\n");
    printf(" &&&*****&&&(*******&&&&&&&*****&&&&&\n");
    printf(" &&&&&&*****************&&&&&&*****&&\n");
    printf(" &&&**********************&&&***&&\n");
    printf(" &&*************************#&&&\n");
    printf(" &&&****************************&&&\n");
    printf(" &&&*****************************&&&\n");
    printf(" &&&****************************/&&\n");
    printf(" &&&&#*************************&&\n");
    printf(" &&&&**********************&&\n");
    printf(" &&&********************&&\n");
    printf(" &&&******************&&&\n");
    printf(" ,&&******************&&@\n");
    printf(" &&******************&&\n");
    printf(" *&&*****************&&&\n");
    printf(" &&&*****************&&@\n");
    printf(" ,&&*****************/&&\n");
    printf(" &&&******************&&&\n");
    printf(" .&&&***@@@@@@@@@&******&&\n");
    printf(" &&&**@@@@@@@@@@@@@@****&&&\n");
    printf(" &&&*&@@@@@@@@@@@@@@@@***&&\n");
    printf(" &&**@@@@@@@@@@@@@@@@@**&&&\n");
    printf(" &&**@@@@@@@@@@@@@@@@@*&&&\n");
    printf(" @#&&**@@@@@@@@@@@@@@(*@&&\n");
    printf(" &&&***&@@@@@@@@***&&&\n");
    printf(" &&&&&&/****&&&&&&\n");
    printf(" ,&&&&&(\n");
    printf(" \n");
    printf(" \");\n");
        rv = 0;
        *retval = rv;
  }
  // compare token with exit string, handle exit
  if(!strcmp(token, "exit")) {
        // rv = handle_cd();
        handle_exit(args, stdin, stdout);
  }
  (void) builtins;
  return rv;
}

/* This function initially prints a default prompt of:
 * thsh>
 *
 * In Lab 1, Exercise 3, you will add the current working
 * directory to the prompt.  As in, if you are in "/home/foo"
 * the prompt should be:
 * [/home/foo] thsh>
 *
 * Returns the number of bytes written
 */
int print_prompt(void) {
  int ret = 0;
  // Print the prompt
  // file descriptor 1 -> writing to stdout
  // print the whole prompt string (write number of
  // bytes/chars equal to the length of prompt)
  //

  // Lab 1: Your code here

  const char *prompt = "thsh> ";
  char *dir_prompt = malloc(MAX_INPUT);
  dir_prompt[0] = '[';
  getcwd(dir_prompt+1, MAX_INPUT);
  strcat(dir_prompt,"] ");
  strcat(dir_prompt,prompt);
  // char *final_prompt = malloc(MAX_INPUT);
  // final_prompt[0] = '[';
  // final_prompt = strcat("[", dir_prompt);
  // final_prompt = strcat(final_prompt, "] ");
  // final_prompt = strcat(final_prompt, prompt);
  // cur_prompt = strcat("[test] ",prompt);
  // if cur path is not set, set it and append slash
  // if(cur_path[0] == '\0') {
  //     getcwd(cur_path, MAX_INPUT);
  //     strcat(cur_path, "/");
  // }
  // printf("[%s] ", cur_prompt);
  // strcat();
  // printf("%s",strcat(cur_prompt, prompt));
  // printf("%s",prompt);

  ret = write(1, dir_prompt, strlen(dir_prompt));
  // ret = write(1, prompt, strlen(prompt));
  return ret;
}
