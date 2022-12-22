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
        if(pathpointer[i+1] == ':') {
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

