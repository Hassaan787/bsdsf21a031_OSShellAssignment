#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "PUCITVer4shell:- "
#define HISTORY_SIZE 10

int execute(char* arglist[]);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);
void add_to_history(char *cmd);
char* get_history_command(int index);

// History buffer
char *history[HISTORY_SIZE];
int history_count = 0;  // Total commands issued
int history_start = 0;  // Start index for the circular buffer

int main() {
   char *cmdline;
   char **arglist;
   char *prompt = PROMPT;

   // Set up signal handler to prevent zombie processes
   signal(SIGCHLD, SIG_IGN);

   while ((cmdline = read_cmd(prompt, stdin)) != NULL) {
      // Check if the user wants to repeat a previous command
      if (cmdline[0] == '!' && strlen(cmdline) > 1) {
         int hist_index;
         
         if (cmdline[1] == '-') {
            hist_index = history_count - 1;  // !-1 means last command
         } else {
            // Parse the command number after '!'
            hist_index = atoi(&cmdline[1]) - 1;  // Convert to zero-based index
         }

         // Validate the index
         if (hist_index < 0 || hist_index >= history_count) {
            printf("No such command in history.\n");
            free(cmdline);
            continue;
         }

         // Retrieve the command from history
         free(cmdline);  // Free current cmdline memory
         cmdline = strdup(get_history_command(hist_index));
         if (!cmdline) {
            perror("Failed to retrieve command from history");
            continue;
         }
         printf("Repeating command: %s\n", cmdline);
      } else {
         // Add the command to history if it's a new command
         add_to_history(cmdline);
      }

      if (strcmp(cmdline, "exit") == 0) {
         free(cmdline);
         break;
      }

      if ((arglist = tokenize(cmdline)) != NULL) {
         execute(arglist);

         // Free allocated memory for arguments
         for (int j = 0; j < MAXARGS + 1; j++)
            free(arglist[j]);
         free(arglist);
         free(cmdline);
      }
   }

   // Free memory in history buffer before exiting
   for (int i = 0; i < HISTORY_SIZE; i++) {
      if (history[i] != NULL) free(history[i]);
   }

   printf("\nExiting shell...\n");
   return 0;
}

// Adds a command to the history buffer
void add_to_history(char *cmd) {
   if (history_count < HISTORY_SIZE) {
      // Add to the end of the history if there's space
      history[history_count] = strdup(cmd);
      history_count++;
   } else {
      // Overwrite the oldest command in circular fashion
      free(history[history_start]);
      history[history_start] = strdup(cmd);
      history_start = (history_start + 1) % HISTORY_SIZE;
   }
}

// Retrieves a command from history by index
char* get_history_command(int index) {
   if (history_count <= HISTORY_SIZE) {
      return history[index];
   } else {
      // Calculate actual position in circular buffer
      return history[(history_start + index) % HISTORY_SIZE];
   }
}

int execute(char* arglist[]) {
   int status;
   int redirect_in = -1, redirect_out = -1;
   char *infile = NULL, *outfile = NULL;
   int pipe_index = -1;
   int is_background = 0;

   // Check if the command should run in the background
   int last_arg_index = 0;
   while (arglist[last_arg_index] != NULL) last_arg_index++;
   if (last_arg_index > 0 && strcmp(arglist[last_arg_index - 1], "&") == 0) {
       is_background = 1;
       arglist[last_arg_index - 1] = NULL;
   }

   // Check for I/O redirection and pipes
   for (int i = 0; arglist[i] != NULL; i++) {
       if (strcmp(arglist[i], "<") == 0) {
           infile = arglist[i + 1];
           arglist[i] = NULL;
           redirect_in = i;
       }
       else if (strcmp(arglist[i], ">") == 0) {
           outfile = arglist[i + 1];
           arglist[i] = NULL;
           redirect_out = i;
       }
       else if (strcmp(arglist[i], "|") == 0) {
           pipe_index = i;
           arglist[i] = NULL;
           break;
       }
   }

   if (pipe_index != -1) {
       int pipe_fd[2];
       if (pipe(pipe_fd) == -1) {
           perror("Pipe failed");
           return 1;
       }

       pid_t pid1 = fork();
       if (pid1 == -1) {
           perror("Fork failed");
           return 1;
       }

       if (pid1 == 0) {
           close(pipe_fd[0]);
           dup2(pipe_fd[1], STDOUT_FILENO);
           close(pipe_fd[1]);
           execvp(arglist[0], arglist);
           perror("Command execution failed");
           exit(1);
       }

       pid_t pid2 = fork();
       if (pid2 == -1) {
           perror("Fork failed");
           return 1;
       }

       if (pid2 == 0) {
           close(pipe_fd[1]);
           dup2(pipe_fd[0], STDIN_FILENO);
           close(pipe_fd[0]);
           execvp(arglist[pipe_index + 1], &arglist[pipe_index + 1]);
           perror("Command execution failed");
           exit(1);
       }

       close(pipe_fd[0]);
       close(pipe_fd[1]);
       waitpid(pid1, &status, 0);
       waitpid(pid2, &status, 0);

   } else {
       pid_t pid = fork();
       if (pid == -1) {
           perror("Fork failed");
           exit(1);
       }

       if (pid == 0) {
           if (redirect_in != -1) {
               int fd_in = open(infile, O_RDONLY);
               if (fd_in == -1) {
                   perror("Input file open failed");
                   exit(1);
               }
               dup2(fd_in, STDIN_FILENO);
               close(fd_in);
           }

           if (redirect_out != -1) {
               int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
               if (fd_out == -1) {
                   perror("Output file open failed");
                   exit(1);
               }
               dup2(fd_out, STDOUT_FILENO);
               close(fd_out);
           }

           execvp(arglist[0], arglist);
           perror("Command execution failed");
           exit(1);
       } else {
           if (is_background) {
               printf("[1] %d\n", pid);
           } else {
               waitpid(pid, &status, 0);
               printf("child exited with status %d\n", status >> 8);
           }
       }
   }
   return 0;
}

// Tokenizes the command line input into arguments
char** tokenize(char* cmdline){
   char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
   for (int j = 0; j < MAXARGS + 1; j++) {
       arglist[j] = (char*)malloc(sizeof(char) * ARGLEN);
       bzero(arglist[j], ARGLEN);
   }
   
   if (cmdline[0] == '\0') 
       return NULL;

   int argnum = 0;
   char* cp = cmdline;
   char* start;
   int len;

   while (*cp != '\0') {
       while (*cp == ' ' || *cp == '\t')
           cp++;
       start = cp;
       len = 1;

       while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t'))
           len++;

       strncpy(arglist[argnum], start, len);
       arglist[argnum][len] = '\0';
       argnum++;
   }
   arglist[argnum] = NULL;
   return arglist;
}

char* read_cmd(char* prompt, FILE* fp){
   printf("%s", prompt);
   int c;
   int pos = 0;
   char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);

   while ((c = getc(fp)) != EOF) {
       if (c == '\n')
           break;
       cmdline[pos++] = c;
   }

   if (c == EOF && pos == 0)
       return NULL;

   cmdline[pos] = '\0';
   return cmdline;
}
