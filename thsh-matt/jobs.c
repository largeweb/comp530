// PID: 730400003
//
/* COMP 530: Tar Heel SHell
 *
 * This file implements functions related to launching
 * jobs and job control.
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "thsh.h"

#define PATH_DELIMITER ":"
#define MAX_PATH_SIZE 1000

static char ** path_table;

/* Initialize the table of PATH prefixes.
 *
 * Split the result on the parenteses, and
 * remove any trailing '/' characters.
 * The last entry should be a NULL character.
 *
 * For instance, if one's PATH environment variable is:
 *  /bin:/sbin///
 *
 * Then path_table should be:
 *  path_table[0] = "/bin"
 *  path_table[1] = "/sbin"
 *  path_table[2] = '\0'
 *
 * Hint: take a look at getenv().  If you use getenv, do NOT
 *       modify the resulting string directly, but use
 *       malloc() or another function to allocate space and copy.
 *
 * Returns 0 on success, -errno on failure.
 */
int init_path(void) {
  /* Lab 0: Your code here */
  // set path pointer to env path
  char *pathpointer = getenv("PATH");
  // get length of path pointer
  int path_len = strlen(pathpointer);
  // make path string pointer copy
  char *pathstring = (char *) malloc(path_len + 1);
  strcpy(pathstring, pathpointer);
  // initialize helper variable ints i & count, set to 0
  int i, count;
  i = count = 0;
  // loop through pathpointer to count colons
  while(pathpointer[i] != '\0') {
        if(pathpointer[i] == ':') {
            count++;
        }
        i++;
  }
  // allocate space for path table according to number of colons
  path_table = malloc((count+2)*sizeof(*path_table));
  // if there are no colons, the pathstring becomes the only entry in path table
  if(count==0) {
        // don't set path_table if pathstring is null char
        if(pathstring[0] == '\0') {
            return 0;
        }
        path_table[0] = pathstring;
        return 0;
  }
  // set token pointer to first token in pathstring
  char *token = strtok(pathstring, PATH_DELIMITER);
  // loop through the colon delimits and add corresponding tokens to path table
  for(i = 0; i<count+1; i++) {

      // strip trailing slashes
        int end_token;
        end_token = strlen(token) - 1;
        // if the token is null char, continue
        if(token[end_token] == '\0') {
            continue;
        }
        // while we see trailing slashes, set index to null char
        while (end_token > 0 && (token[end_token] == '/' || token[end_token] == '\\')) {
            token[end_token--] = '\0';
        }

        // set path table to current token
        path_table[i] = token;
        token = strtok(NULL, PATH_DELIMITER);
  }
  return 0;
}

/* Debug helper function that just prints
 * the path table out.
 */
void print_path_table() {
  if (path_table == NULL) {
    printf("XXXXXXX Path Table Not Initialized XXXXX\n");
    return;
  }

  printf("===== Begin Path Table =====\n");
  for (int i = 0; path_table[i]; i++) {
    printf("Prefix %2d: [%s]\n", i, path_table[i]);
  }
  printf("===== End Path Table =====\n");
}


static int job_counter = 0;

struct kiddo {
  int pid;
  struct kiddo *next; // Linked list of sibling processes
};

// A job consists of a unique numeric ID and
// one or more processes
struct job {
  int id;
  struct kiddo *kidlets; // Linked list of child processes
  struct job *next; // Linked list of active jobs
};

// A singly linked list of active jobs.
static struct job *jobbies = NULL;

/* Initialize a job structure
 *
 * Returns an integer ID that represents the job.
 */
int create_job(void) {
  struct job *tmp;
  struct job *j = malloc(sizeof(struct job));
  j->id = ++job_counter;
  j->kidlets = NULL;
  j->next = NULL;
  if (jobbies) {
    for (tmp = jobbies; tmp && tmp->next; tmp = tmp->next) ;
    assert(tmp!=j);
    tmp->next = j;
  } else {
    jobbies = j;
  }
  return j->id;
}

/* Helper function to walk the job list and find                                                                    
 * a given job.                                                                                                     
 *                                                                                                                  
 * remove: If true, remove this job from the job list.                                                              
 *                                                                                                                  
 * Returns NULL on failure, a job pointer on success.                                                               
 */
static struct job *find_job(int job_id, bool remove) {
  struct job *tmp, *last = NULL;
  for (tmp = jobbies; tmp; tmp = tmp->next) {
    if (tmp->id == job_id) {
      if (remove) {
        if (last) {
          last->next = tmp->next;
        } else {
          assert (tmp == jobbies);
          jobbies = NULL;
        }
      }
      return tmp;
    }
    last = tmp;
  }
  return NULL;
}

/* Given the command listed in args,
 * try to execute it.
 *
 * If the first argument starts with a '.'
 * or a '/', it is an absolute path and can
 * execute as-is.
 *
 * Otherwise, search each prefix in the path_table
 * in order to find the path to the binary.
 *
 * Then fork a child and pass the path and the additional arguments
 * to execve() in the child.  Wait for exeuction to complete
 * before returning.
 *
 * stdin is a file handle to be used for standard in.
 * stdout is a file handle to be used for standard out.
 *
 * If stdin and stdout are not 0 and 1, respectively, they will be
 * closed in the parent process before this function returns.
 *
 * job_id is the job_id allocated in create_job
 *
 * Returns 0 on success, -errno on failure
 */
int run_command(char *args[MAX_ARGS], int stdin, int stdout, int job_id) {
  /* Lab 1: Your code here */
  int rv = 0;
   // extern bool debug_mode;

// thia where i do fork exec and other type of stuff
   // printf("size of path table is: %d\n", (int)sizeof(path_table));
   // int parent_pid = getpid();
       // printf("testing: %s\n", args[0]);
   int status;
   (void)(status);
   int fork_pid = fork();

   
       // printf("test");
   if (fork_pid == -1)
   {
       printf("error: failed to fork");
   } 
   else if (fork_pid > 0)
   {
       waitpid(fork_pid, &status, 0);
       // (void)(status);
       // rv = status;
   }
   else 
   {
       if((args[0][0] == '.' && args[0][1] == '/') || args[0][0] == '/') {
           // printf("running script...");
           rv = execv(args[0], args);
           printf("RUNNING ./ COMMAND ...");
           return rv;
       }
       // char* buff = malloc(MAX_INPUT);
       // printf("test");
       struct stat buff;
       char* temp_path_to_file;
       char* file_to_find;
       bool found = false;
       int i;
       for(i = 0; i<(int)sizeof(path_table)-1;i++) {
           temp_path_to_file = strdup(path_table[i]);
           strcat(temp_path_to_file, "/");
           file_to_find = strdup(args[0]);
           strcat(temp_path_to_file, file_to_find);
           // printf("trying to find: \n%s\n", temp_path_to_file);
           if(stat(temp_path_to_file, &buff) == 0) {
               found = true;
               if(stdin != 0) {
                    // printf("\nSTDIN NOT 0!\n");
                    dup2(stdin, STDIN_FILENO);
                    close(stdin);
               }
               // if(stdout != 1) {
               //      printf("\nSTDOUT NOT 1!\n");
               //      dup2(stdout, STDOUT_FILENO);
               //      close(stdout);
               // }
               // printf("\nfound!\n");
               // printf("address of arg is: %s\n", args[1]);
               status = execv(temp_path_to_file, args);
               // rv = status;
           }
       }
       if(!found) {
            return -2;
       }
       _exit(EXIT_FAILURE);   // exec never returns
   }


   // printf("this should print twice, parent pid: %d, child pid: %d\n", parent_pid, fork_pid);
  

  // Suppress the compiler warning that find_job is not used in the starer code.
  // You may remove this line if/when you use find_job in your code.
  (void)&find_job;
  return rv;
}

/* Wait for the job to complete and free internal bookkeeping
 *
 * job_id is the job_id allocated in create_job
 *
 * exit_code is the exit code from the last child process, if it executed.
 *           This parameter may be NULL, and is only set if the return
 *           value is zero.  This is the same as the wstatus parameter
 *           to waitpid variants, and can be used with functions such
 *           as WIFEXITED.  If this job includes multiple
 *           processes, the exit code will be the last process.
 *
 * Returns zero on success, -errno on error.
 */
int wait_on_job(int job_id, int *exit_code) {
  int ret = 0;
  return ret;
}

