#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "PUCITshell:- "

int execute(char* arglist[]);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);

int main() {
    char *cmdline;
    char **arglist;
    char *prompt = PROMPT;

    while ((cmdline = read_cmd(prompt, stdin)) != NULL) {
        if ((arglist = tokenize(cmdline)) != NULL) {
            execute(arglist);

            // Free dynamically allocated memory for each argument in arglist
            for (int j = 0; j < MAXARGS; j++) {
                if (arglist[j] != NULL) free(arglist[j]);
            }
            free(arglist); // Free the array of pointers
        }
        free(cmdline); // Free the command line buffer
    }
    printf("\nExiting shell...\n");
    return 0;
}

int execute(char* arglist[]) {
    int status;
    pid_t cpid = fork();
    
    switch (cpid) {
        case -1:
            perror("fork failed");
            exit(1);
        case 0: // Child process
            execvp(arglist[0], arglist);
            perror("Command not found...");
            exit(1);
        default: // Parent process
            waitpid(cpid, &status, 0);
            return 0;
    }
}

char** tokenize(char* cmdline) {
    char **arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    int argnum = 0;
    
    char *token = strtok(cmdline, " \t\n"); // Tokenize based on spaces, tabs, newlines
    while (token != NULL && argnum < MAXARGS) {
        arglist[argnum] = (char*)malloc(strlen(token) + 1);
        strcpy(arglist[argnum], token);
        argnum++;
        token = strtok(NULL, " \t\n");
    }
    arglist[argnum] = NULL; // NULL-terminate the list of arguments

    // Return NULL if no arguments were added (e.g., empty input or whitespace-only)
    if (argnum == 0) {
        free(arglist);
        return NULL;
    }
    
    return arglist;
}

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char *cmdline = (char*)malloc(MAX_LEN);
    int pos = 0;
    int c;

    while ((c = getc(fp)) != EOF && c != '\n') {
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) { // Handle Ctrl+D (EOF) at the start
        free(cmdline);
        return NULL;
    }

    cmdline[pos] = '\0'; // Null-terminate the command string
    return cmdline;
}
