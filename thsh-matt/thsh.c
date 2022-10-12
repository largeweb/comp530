/* COMP 530: Tar Heel SHell */

#include "thsh.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

bool debug_mode;

int main(int argc, char **argv, char **envp) {
  // flag that the program should end
  bool finished = 0;
  int input_fd = 0; // Default to stdin
  int ret = 0;


  // Lab 1:
  // Add support for parsing the -d option from the command line
  // and handling the case where a script is passed as input to your shell

  // Lab 1: Your code here
  
  // set bool debug_mode to true if -d flag exists
  debug_mode = false;
  char* j;
  if(argv[1]) {
      if(strchr(argv[1], '-')) {
          j = strchr(argv[1], '-');
      }
  };
  (void)(j);
  if(j != NULL && ++j[1] == 101) {
        debug_mode = true;
        // fprintf(stderr,"DEBUG MODE ON\n");
  }
  (void)(debug_mode);

  ret = init_cwd();
  if (ret) {
    printf("Error initializing the current working directory: %d\n", ret);
    return ret;
  }

  ret = init_path();
  if (ret) {
    printf("Error initializing the path table: %d\n", ret);
    return ret;
  }

  while (!finished) {
    int length;
    // Buffer to hold input
    char cmd[MAX_INPUT];
    // Get a pointer to cmd that type-checks with char *
    char *buf = &cmd[0];
    char *parsed_commands[MAX_PIPELINE][MAX_ARGS];
    char *infile = NULL;
    char *outfile = NULL;
    int pipeline_steps = 0;

    if (!input_fd) {
      ret = print_prompt();
      if (ret <= 0) {
	// if we printed 0 bytes, this call failed and the program
	// should end -- this will likely never occur.
	finished = true;
	break;
      }
    }

    // Reset memory from the last iteration
    for(int i = 0; i < MAX_PIPELINE; i++) {
      for(int j = 0; j < MAX_ARGS; j++) {
        parsed_commands[i][j] = NULL;
      }
    }

    // Read a line of input
    length = read_one_line(input_fd, buf, MAX_INPUT);
    if (length <= 0) {
      ret = length;
      break;
    }

    // Pass it to the parser
    pipeline_steps = parse_line(buf, length, parsed_commands, &infile, &outfile);
    if (pipeline_steps < 0) {
      printf("Parsing error.  Cannot execute command. %d\n", -pipeline_steps);
      continue;
    }

    // Just echo the command line for now
    // file descriptor 1 -> writing to stdout
    // print the whole cmd string (write number of
    // chars/bytes equal to the length of cmd, or MAX_INPUT,
    // whichever is less)
    //
    // Comment this line once you implement
    // command handling
    // dprintf(1, "%s\n", cmd);

    // In Lab 1, you will need to add code to actually run the commands,
    // add debug printing, and handle redirection and pipelines, as
    // explained in the handout.
    //
    // For now, ret will be set to zero; once you implement command handling,
    // ret should be set to the return from the command.
    ret = 0;




    // ❗❗❗ DO THIS BELOW 
    // ret = handle_builtin();
    // int handle_builtin(char *args[MAX_ARGS], int stdin, int stdout, int *retval) {
    
    // if((!strcmp(parsed_commands[0][0],"cd")) || (!strcmp(parsed_commands[0][0],"exit"))) {
    //     printf("typed cd or exit, also pipeline steps num is: %d\n", pipeline_steps);
    // }

    // loop for each pipeline stage, this is the main loop for processing commands in order
    int i = 0;
    int infd = 0;
    int outfd = 1;
    int pipefd[2];
    while(parsed_commands[i][0] && i < MAX_ARGS) {
        //get job id from function
        int jid = create_job();
        if(debug_mode) {
            fprintf(stderr,"RUNNING: [%s]\n", parsed_commands[i][0]);
            // printf("INPUT TO RUN COMMAND: %s\n", parsed_commands[i]);
        }
        // if it is a builtin, handle it & move on
        if(!strcmp(parsed_commands[i][0], "cd") || !strcmp(parsed_commands[i][0], "exit") || !strcmp(parsed_commands[i][0], "goheels")) {
            char* old_command = strdup(parsed_commands[i][0]);
            handle_builtin(parsed_commands[i++], 0, 1, &ret);
            if(debug_mode) {
                fprintf(stderr,"ENDED: [%s] (ret=%d)\n", old_command,ret);
            }
            continue;;
        }
        // printf("\ncurrently dealing with command: %s\n", parsed_commands[i][0]);
        // ret = run_command(parsed_commands[i], 0, 1, 123);
        // if(i==0 && infile) {
        // if(infile && i != pipeline_steps - 1) {
        
        // if there is an infile, open it and set the file descriptors, set infile back to NULL so it only runs once
        if(infile) {
            // printf("INFILE FOUND AND IS NOT NULL\n");
            infd = open(infile, O_RDONLY);
            dup2(infd, 0);
            close(infd);
            infile = NULL;
        }
       // printf("test runcommand");
        // for the last stage in pipeline, print to output if there is  
        if(i == pipeline_steps - 1) {
            if(outfile) {
                outfd = open(outfile, O_CREAT | O_TRUNC | O_WRONLY);
                fchmod(outfd, 0666);
                if (infd != 0)
                        dup2(infd, STDIN_FILENO);
            } else {
                outfd = STDOUT_FILENO;
                if(infd)
                    dup2(infd, STDIN_FILENO);
            }
            ret = run_command(parsed_commands[i], infd, outfd, jid);
            if(debug_mode) {
                fprintf(stderr,"ENDED: [%s] (ret=%d)\n", parsed_commands[i][0],ret);
            }
            // i++;
            break;
        }
        // if(i == pipeline_steps - 1) {
        // }
       // printf("test afterruncommand");
        if(debug_mode) {
            fprintf(stderr,"ENDED: [%s] (ret=%d)\n", parsed_commands[i][0],ret);
        }
        if(i == pipeline_steps - 1)
            continue;

        pipe(pipefd);
        ret = run_command(parsed_commands[i], pipefd[0], pipefd[1], jid);
        close(pipefd[1]);
        infd = pipefd[0];

        i++;
    }
    
// int handle_cd(char *args[MAX_ARGS], int stdin, int stdout) {
    
    //
    // if(pipeline_steps)
    // test
    //
    // for first pipeline  series, find the command at index 0, run it and add the variables
    // use count from for loop to record pipeline series iteration
    //
    // if there is another pipeline afterwards, pass the stdout as stdin
    
    
    
    
    
    
    

    // Do NOT change this if/printf - it is used by the autograder.
    if (ret) {
      printf("Failed to run command - error %d\n", ret);
    }




    // // Pretty print everything => FROM LAB 1
    // for (int i = 0; parsed_commands[i][0]; i++) {
    //   printf("Pipeline Stage %d: ", i);
    //   for (int j = 0; parsed_commands[i][j]; j++) {
	// printf("[%s] ", parsed_commands[i][j]);
    //   }
    //   printf("\n");
    // }
    // if (infile) {
    //   printf("Input redirection to file [%s]\n", infile);
    // }
    // if (outfile) {
    //   printf("Output redirection to file [%s]\n", outfile);
    // }

    // // If any commands are built-in commands, execute them.
    // // Otherwise, we will handle this in lab 1
    // for (int i = 0; parsed_commands[i][0]; i++) {
    //   int rv;
    //   if (handle_builtin(parsed_commands[i], 0, 1, &rv)) {
	// printf("Command [%s] is a built-in, returned %d.\n", parsed_commands[i][0], rv);
    //   } else {
	// printf("Command [%s] is not a built-in.\n", parsed_commands[i][0]);
    //   }
    // }







  }

  // Only return a non-zero value from main() if the shell itself
  // has a bug.  Do not use this to indicate a failed command.
  return ret;
}
