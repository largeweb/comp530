// PID: 730400003
/* COMP 530: Tar Heel SHell
 *
 * This module implements command parsing, following the grammar
 * in the assignment handout.
 */

#include "thsh.h"

/* This function returns one line from input_fd
 *
 * buf is populated with the contents, including the newline, and
 *      including a null terminator.  Must point to a buffer
 *      allocated by the caller, and may not be NULL.
 *
 * size is the size of *buf
 *
 * Return value: the length of the string (not counting the null terminator)
 *               zero indicates the end of the input file.
 *               a negative value indicates an error (e.g., -errno)
 */
int read_one_line(int input_fd, char *buf, size_t size) {
  int count, rv;
  // pointer to next place in cmd to store a character
  char *cursor;
  // the last character that was written into cmd
  char last_char;

  assert (buf);

  /*
   * We want to continue reading characters until:
   *   - read() fails (rv will become 0) OR
   *   - count == MAX_INPUT-1 (we have no buffer space left) OR
   *   - last_char was '\n'
   * so we continue the loop while:
   *   rv is nonzero AND count < MAX_INPUT - 1 AND last_char != '\n'
   *
   * On every iteration, we:
   *   - increment cursor to advance to the next char in the cmd buffer
   *   - increment count to indicate we've stored another char in cmd
   *     (this is done during the loop termination check with "++count")
   *
   * To make the termination check work as intended, the loop starts by:
   *   - setting rv = 1 (so it's initially nonzero)
   *   - setting count = 0 (we've read no characters yet)
   *   - setting cursor = cmd (cursor at start of cmd buffer)
   *   - setting last_char = 1 (so it's initially not '\n')
   *
   * In summary:
   *   - START:
   *      set rv = 1, count = 0, cursor = cmd, last_char = 1
   *   - CONTINUE WHILE:
   *      rv (is nonzero) && count < MAX_INPUT - 1 && last_char != '\n'
   *   - UPDATE PER ITERATION:
   *      increment count and cursor
   */
  for (rv = 1, count = 0, cursor = buf, last_char = 1;
       rv && (last_char != '\n') && (++count < (size - 1)); cursor++) {

    // read one character
    // file descriptor 0 -> reading from stdin
    // writing this one character to cursor (current place in cmd buffer)
    rv = read(input_fd, cursor, 1);
    last_char = *cursor;
  }
  // null terminate cmd buffer (so that it will print correctly)
  *cursor = '\0';

  // Deal with an error from the read call
  if (!rv) {
    count = -errno;
  }

  return count;
}


/* Parse one line of input.
 *
 * This function should populate a two-dimensional array of commands
 * and tokens.  The array itself should be pre-allocated by the
 * caller.
 *
 * The first level of the array is each stage in a pipeline, at most MAX_PIPELINE long.
 * The second level of the array is each argument to a given command, at most MAX_ARGS entries.
 * In each command buffer, the entry after the last valid entry should be NULL.
 * After the last valid pipeline buffer, there should be one command entry with just a NULL.
 *
 * For instacne, a simple command like "cd" should parse as:
 *  commands[0] = ["cd", '\0']
 *  commands[1] = ['\0']
 *
 * The first "special" character to consider is the vertical bar, or "pipe" ('|').
 * This splits a single line into multiple sub-commands that form the pipeline.
 * We will implement pipelines in lab 1, but for now, just use this character to delimit
 * commands.
 *
 * For instance, the command: "ls | grep foo\n" should be broken into:
 *
 * commands[0] = ["ls", '\0']
 * commands[1] = ["grep", "foo", '\0']
 * commands[2] = ['\0']
 *
 * Hint: Make sure to remove the newline at the end
 *
 * Hint: Make sure the implementation is robust to extra whitespace, like: "grep      foo"
 *       should still be parsed as:
 *
 * commands[0] = ["grep", "foo", '\0']
 * commands[1] = ['\0']
 *
 * This function should ignore anything after the '#' character, as
 * this is a comment.
 *
 * Finally, the command should identify file redirection characters ('<' and '>').
 * The string right after these tokens should be returned using the special output
 * parameters "infile" and "outfile".  You can assume there is at most one '<' and one
 * '>' character in the entire inbuf.
 *
 * For example, in input: "ls > out.txt", the return should be:
 *   commands[0] = ["ls", '\0']
 *   commands[1] = ['\0']
 *   outfile = "out.txt"
 *
 * Hint: Be sure your implementation is robust to arbitrary (or no) space before or after
 *       the '<' or '>' characters.  For instance, "ls>out.txt" and "ls      >      out.txt"
 *       are both syntactically valid, and should parse identically to "ls > out.txt".
 *       Similarly, "ls|grep foo" is also syntactically valid.
 *
 * You do not need to handle redirection of other handles (e.g., "foo 2>&1 out.txt").
 *
 * inbuf: a NULL-terminated buffer of input.
 *        This buffer may be changed by the function
 *        (e.g., changing some characters to \0).
 *
 * length: the length of the string in inbuf.  Should be
 *         less than the size of inbuf.
 *
 * commands: a two-dimensional array of character pointers, allocated by the caller, which
 *           this function populates.
 *
 *
 * return value: Number of entries populated in commands (1+, not counting the NULL),
 *               or -errno on failure.
 *
 *               In the case of a line with no actual commands (e.g.,
 *               a line with just comments), return 0.
 */
int parse_line (char *inbuf, size_t length,
		char *commands [MAX_PIPELINE][MAX_ARGS],
		char **infile, char **outfile) {

   // Lab 0: Your code here
   // initialize helper int i & count, set to 0
   int i, j, count;
   i = j = count = 0;

   // check if a comment exists and add null terminator if so
   char* comment_check = strchr(inbuf, '#');
   if(comment_check != NULL) {
       *(comment_check+1) = '\0';
   }

   // bool to check if redirect out char appears
   char* out_red_check = strchr(inbuf, '>');
   char* red_in_ptr;
   char* red_out_ptr;
   (red_in_ptr = NULL);(red_out_ptr = NULL);
   // if there is a redirect out then add null terminator
   if(out_red_check != NULL) {
       *out_red_check = '\0';
       // we may as well set the outfile now that we have the index
       char* index_for_out = ++out_red_check;
       // rid of trailing spaces
       while(index_for_out[0] == 32) {
            index_for_out++;
       }
       // if the last of string is newline or space, replace with null terminator
       if(index_for_out[strlen(index_for_out)-1] == '\n' || index_for_out[strlen(index_for_out)-1] == 32) {
            index_for_out[strlen(index_for_out)-1] = '\0';
       }
       // set any bar or space to null terminator
       char* spaceindex;
       if((spaceindex = strchr(index_for_out, '|')) != NULL) {
            spaceindex[0] = '\0';
           red_out_ptr = ++spaceindex;
       }
       if((spaceindex = strchr(index_for_out, ' ')) != NULL) {
            spaceindex[0] = '\0';
       }
       // set outfile of our index
       *outfile = (index_for_out);
   }
   // same situation for inside redirect, but for infile
   char* in_red_check = strchr(inbuf, '<');
   if(in_red_check != NULL) {
       *in_red_check = '\0';
       char* index_for_in = ++in_red_check;
       while(index_for_in[0] == 32) {
            index_for_in++;
       }
       if(index_for_in[strlen(index_for_in)-1] == '\n' || index_for_in[strlen(index_for_in)-1] == 32) {
            index_for_in[strlen(index_for_in)-1] = '\0';
       }
       // set any bar or space to null terminator
       char* spaceindex;
       if((spaceindex = strchr(index_for_in, '|')) != NULL) {
            spaceindex[0] = '\0';
           red_in_ptr = ++spaceindex;
       }
       if((spaceindex = strchr(index_for_in, ' ')) != NULL) {
            spaceindex[0] = '\0';
       }
       *infile = (index_for_in);
   }
   // save pointer for strtok_r
   char* ptr_save1;
   // initialize full_input as first token in inbuf, delimited by # and \n
   char* full_input = strtok_r(inbuf, "#\n", &ptr_save1);

   char* token = strtok_r(full_input, "|", &ptr_save1);

   // start looping through the command, separating the tokens, iterate by i to specify the first dimension of commands
   for(i = 0; token != NULL; i++) {
        // save pointer for strtok_r
        char* ptr_save2;
        // set j to be the second dimension, going into the commands
        j=0;

        // otherwise, if there is nothing after the bar AND something after the redirect
        if(strchr(token, '|') == NULL && strchr(token, '>') != NULL) {
                // set pointer saves for strtoks
                char* ptr_save3;
                char* ptr_save4;
                // go through all until the delimited redirect less than string
                strtok_r(token, ">", &ptr_save2);
                // set token to next value, delimited by space string
                char* token2 = strtok_r(token, " ", &ptr_save3);
                // go through the post-redirect tokens and add them to commands
                while(token2 != NULL) {
                        commands[i][j++] = token2;
                        count++;
                        // set token to next value, delimited by space string
                        token2 =strtok_r(NULL, " ", &ptr_save3);
                }
                // set token to next value, delimited by space string
                char* token3 = strtok_r(ptr_save1, " ", &ptr_save4);
                if(*token3 == ' ') {
                        char* rest = strchr(token3, ' ');
                        *outfile = rest;
                } else {
                        *outfile = token3;
                }
        }
        else {
                // set token to next value, delimited by space string
                char* token2 = strtok_r(token, " ", &ptr_save2);
                // go through the post-redirect tokens and add them to commands
                while(token2 != NULL) {
                        commands[i][j++] = token2;
                        count++;
                        // set token to next value, delimited by space
                        token2 = strtok_r(NULL, " ", &ptr_save2);
                }
        }
        // set token to next value, delimited by bar |
        token = strtok_r(NULL, "|", &ptr_save1);
        // if(token != NULL && red_exists && (strchr(token, '|') != NULL)) {
        //         printf("got here, token: %s\n", token);
        //         int len_token = strlen(token);
        //         int k = 0;
        //         while(k++ != len_token-1) {
        //             if(token[k] == '<' || token[k] == '>') {
        //                 for(int l=k; l<len_token; l++) {
        //                     token[l] = 32;
        //                 }
        //             }
        //         }
        //     // if(strchr(token, '<') != NULL || strchr(token, '>') != NULL) {
        //     // // if(token == "<" || token == ">") {
        //     //     printf("got here 2\n");
        //     //     // token = strtok_r(NULL, "|", &ptr_save1);
        //     //     // token = strtok_r(NULL, "|", &ptr_save1);
        //     //     // continue;
        //     // }
        // }
   }
   if(red_in_ptr != NULL) {
   // start looping through the command, separating the tokens, iterate by i to specify the first dimension of commands
        char* token = strtok(red_in_ptr, "| ");
       for(; token != NULL; i++) {
            // save pointer for strtok_r
            char* ptr_save2;
            // set j to be the second dimension, going into the commands
            j=0;

            // otherwise, if there is nothing after the bar AND something after the redirect
            if(strchr(token, '|') == NULL && strchr(token, '>') != NULL) {
                    // set pointer saves for strtoks
                    char* ptr_save3;
                    char* ptr_save4;
                    // go through all until the delimited redirect less than string
                    strtok_r(token, ">", &ptr_save2);
                    // set token to next value, delimited by space string
                    char* token2 = strtok_r(token, " ", &ptr_save3);
                    // go through the post-redirect tokens and add them to commands
                    while(token2 != NULL) {
                            commands[i][j++] = token2;
                            count++;
                            // set token to next value, delimited by space string
                            token2 =strtok_r(NULL, " ", &ptr_save3);
                    }
                    // set token to next value, delimited by space string
                    char* token3 = strtok_r(ptr_save1, " ", &ptr_save4);
                    if(*token3 == ' ') {
                            char* rest = strchr(token3, ' ');
                            *outfile = rest;
                    } else {
                            *outfile = token3;
                    }
            }
            else {
                    // set token to next value, delimited by space string
                    char* token2 = strtok_r(token, " ", &ptr_save2);
                    // go through the post-redirect tokens and add them to commands
                    while(token2 != NULL) {
                            commands[i][j++] = token2;
                            count++;
                            // set token to next value, delimited by space
                            token2 = strtok_r(NULL, " ", &ptr_save2);
                    }
            }
            // set token to next value, delimited by bar |
            token = strtok(NULL, "| ");
            // if(token != NULL && red_exists && (strchr(token, '|') != NULL)) {
            //         printf("got here, token: %s\n", token);
            //         int len_token = strlen(token);
            //         int k = 0;
            //         while(k++ != len_token-1) {
            //             if(token[k] == '<' || token[k] == '>') {
            //                 for(int l=k; l<len_token; l++) {
            //                     token[l] = 32;
            //                 }
            //             }
            //         }
            //     // if(strchr(token, '<') != NULL || strchr(token, '>') != NULL) {
            //     // // if(token == "<" || token == ">") {
            //     //     printf("got here 2\n");
            //     //     // token = strtok_r(NULL, "|", &ptr_save1);
            //     //     // token = strtok_r(NULL, "|", &ptr_save1);
            //     //     // continue;
            //     // }
            // }
       }
   }
   if(red_out_ptr != NULL) {
   // start looping through the command, separating the tokens, iterate by i to specify the first dimension of commands
        char* token = strtok(red_out_ptr, "| ");
       for(; token != NULL; i++) {
            // save pointer for strtok_r
            char* ptr_save2;
            // set j to be the second dimension, going into the commands
            j=0;

            // otherwise, if there is nothing after the bar AND something after the redirect
            if(strchr(token, '|') == NULL && strchr(token, '>') != NULL) {
                    // set pointer saves for strtoks
                    char* ptr_save3;
                    char* ptr_save4;
                    // go through all until the delimited redirect less than string
                    strtok_r(token, ">", &ptr_save2);
                    // set token to next value, delimited by space string
                    char* token2 = strtok_r(token, " ", &ptr_save3);
                    // go through the post-redirect tokens and add them to commands
                    while(token2 != NULL) {
                            commands[i][j++] = token2;
                            count++;
                            // set token to next value, delimited by space string
                            token2 =strtok_r(NULL, " ", &ptr_save3);
                    }
                    // set token to next value, delimited by space string
                    char* token3 = strtok_r(ptr_save1, " ", &ptr_save4);
                    if(*token3 == ' ') {
                            char* rest = strchr(token3, ' ');
                            *outfile = rest;
                    } else {
                            *outfile = token3;
                    }
            }
            else {
                    // set token to next value, delimited by space string
                    char* token2 = strtok_r(token, " ", &ptr_save2);
                    // go through the post-redirect tokens and add them to commands
                    while(token2 != NULL) {
                            commands[i][j++] = token2;
                            count++;
                            // set token to next value, delimited by space
                            token2 = strtok_r(NULL, " ", &ptr_save2);
                    }
            }
            // set token to next value, delimited by bar |
            token = strtok(NULL, "| ");
            // if(token != NULL && red_exists && (strchr(token, '|') != NULL)) {
            //         printf("got here, token: %s\n", token);
            //         int len_token = strlen(token);
            //         int k = 0;
            //         while(k++ != len_token-1) {
            //             if(token[k] == '<' || token[k] == '>') {
            //                 for(int l=k; l<len_token; l++) {
            //                     token[l] = 32;
            //                 }
            //             }
            //         }
            //     // if(strchr(token, '<') != NULL || strchr(token, '>') != NULL) {
            //     // // if(token == "<" || token == ">") {
            //     //     printf("got here 2\n");
            //     //     // token = strtok_r(NULL, "|", &ptr_save1);
            //     //     // token = strtok_r(NULL, "|", &ptr_save1);
            //     //     // continue;
            //     // }
            // }
       }
   }



  return count;
}
