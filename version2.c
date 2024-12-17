#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LEN 512          // Maximum length of command line input
#define MAXARGS 10           // Maximum number of arguments
#define ARGLEN 30            // Maximum length of each argument
#define PROMPT "PUCITVer2shell:- " // Shell prompt

int execute(char* arglist[]);     // Function prototype to execute commands
char** tokenize(char* cmdline);   // Function prototype to tokenize command line input
char* read_cmd(char*, FILE*);     // Function prototype to read command line input

int main() {
    char *cmdline;               // Variable to hold the command line input
    char** arglist;              // Array of argument strings
    char* prompt = PROMPT;       // Shell prompt string
    
    // Main loop to continuously read commands until the user exits
    while((cmdline = read_cmd(prompt, stdin)) != NULL) {
        // Check if the user typed "exit" to terminate the shell
        if (strcmp(cmdline, "exit") == 0) {
            free(cmdline);       // Free allocated memory for the command line
            break;               // Exit the while loop
        }

        if((arglist = tokenize(cmdline)) != NULL) {
            execute(arglist); // Execute the command
            // Free allocated memory for arguments
            for(int j = 0; j < MAXARGS + 1; j++)
                free(arglist[j]);
            free(arglist); // Free the argument list
            free(cmdline); // Free the command line string
        }
    }
    printf("\nExiting shell...\n"); // Print exit message
    return 0; // Exit the program
}

int execute(char* arglist[]) {
    int status;                  // Variable to hold the exit status of child processes
    int redirect_in = -1, redirect_out = -1; // File descriptors for input and output redirection
    char *infile = NULL, *outfile = NULL;    // Input and output file names
    int pipe_index = -1;                     // Index for pipes

    // Check for I/O redirection and pipes
    for (int i = 0; arglist[i] != NULL; i++) {
        // Check for input redirection
        if (strcmp(arglist[i], "<") == 0) {
            infile = arglist[i + 1];     // Get the input file name
            arglist[i] = NULL;           // Remove "<" from the arguments
            redirect_in = i;             // Mark the redirect index
        }
        // Check for output redirection
        else if (strcmp(arglist[i], ">") == 0) {
            outfile = arglist[i + 1];    // Get the output file name
            arglist[i] = NULL;           // Remove ">" from the arguments
            redirect_out = i;            // Mark the redirect index
        }
        // Check for pipes
        else if (strcmp(arglist[i], "|") == 0) {
            pipe_index = i;              // Mark the pipe index
            arglist[i] = NULL;           // Split commands at "|"
            break;                       // Stop checking further
        }
    }

    // If a pipe is detected, handle piping
    if (pipe_index != -1) {
        int pipe_fd[2]; // Array to hold pipe file descriptors
        if (pipe(pipe_fd) == -1) { // Create the pipe
            perror("Pipe failed");
            return 1; // Return error
        }

        pid_t pid1 = fork(); // Fork the first child process
        if (pid1 == -1) {
            perror("Fork failed");
            return 1; // Return error
        }

        if (pid1 == 0) { // In the first child process
            close(pipe_fd[0]);                     // Close unused read end
            dup2(pipe_fd[1], STDOUT_FILENO);       // Redirect stdout to pipe write end
            close(pipe_fd[1]);                     // Close the write end
            execvp(arglist[0], arglist);           // Execute the first command
            perror("Command execution failed");    // Error handling
            exit(1);                               // Exit if execution fails
        }

        pid_t pid2 = fork(); // Fork the second child process
        if (pid2 == -1) {
            perror("Fork failed");
            return 1; // Return error
        }

        if (pid2 == 0) { // In the second child process
            close(pipe_fd[1]);                     // Close unused write end
            dup2(pipe_fd[0], STDIN_FILENO);        // Redirect stdin to pipe read end
            close(pipe_fd[0]);                     // Close the read end
            execvp(arglist[pipe_index + 1], &arglist[pipe_index + 1]); // Execute the second command
            perror("Command execution failed");    // Error handling
            exit(1);                               // Exit if execution fails
        }

        // Parent process
        close(pipe_fd[0]); // Close read end of the pipe
        close(pipe_fd[1]); // Close write end of the pipe
        waitpid(pid1, &status, 0); // Wait for the first child to finish
        waitpid(pid2, &status, 0); // Wait for the second child to finish

    } else {
        // Handle input/output redirection without piping
        pid_t pid = fork(); // Create a new process
        if (pid == -1) {
            perror("Fork failed");
            exit(1); // Exit on fork failure
        }

        if (pid == 0) { // Child process
            // Handle input redirection
            if (redirect_in != -1) {
                int fd_in = open(infile, O_RDONLY); // Open input file
                if (fd_in == -1) {
                    perror("Input file open failed"); // Error handling
                    exit(1); // Exit if open fails
                }
                dup2(fd_in, STDIN_FILENO); // Redirect stdin to input file
                close(fd_in); // Close the input file descriptor
            }

            // Handle output redirection
            if (redirect_out != -1) {
                int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open output file
                if (fd_out == -1) {
                    perror("Output file open failed"); // Error handling
                    exit(1); // Exit if open fails
                }
                dup2(fd_out, STDOUT_FILENO); // Redirect stdout to output file
                close(fd_out); // Close the output file descriptor
            }

            execvp(arglist[0], arglist); // Execute the command
            perror("Command execution failed"); // Error handling
            exit(1); // Exit if execution fails
        } else { // Parent process
            waitpid(pid, &status, 0); // Wait for the child process to finish
            printf("child exited with status %d\n", status >> 8); // Print exit status
        }
    }
    return 0; // Return success
}

char** tokenize(char* cmdline) {
    // Allocate memory for the argument list
    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int j = 0; j < MAXARGS + 1; j++) {
        arglist[j] = (char*)malloc(sizeof(char) * ARGLEN); // Allocate memory for each argument
        bzero(arglist[j], ARGLEN); // Clear the memory
    }
    
    if (cmdline[0] == '\0') // If the user entered nothing
        return NULL; // Return NULL for empty input

    int argnum = 0; // Number of arguments parsed
    char* cp = cmdline; // Pointer to traverse the command line
    char* start; // Pointer to mark the start of an argument
    int len; // Length of the current argument

    // Tokenize the command line input
    while (*cp != '\0') {
        while (*cp == ' ' || *cp == '\t') // Skip spaces
            cp++; // Move to the next character
        start = cp; // Start of the argument
        len = 1; // Initialize length to 1

        // Find the end of the argument
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t'))
            len++; // Increment length until space or end

        strncpy(arglist[argnum], start, len); // Copy the argument into the list
        arglist[argnum][len] = '\0'; // Null-terminate the argument
        argnum++; // Increment argument count
    }
    arglist[argnum] = NULL; // Null-terminate the argument list
    return arglist; // Return the list of arguments
}

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt); // Display the prompt
    int c; // Variable to hold input character
    int pos = 0; // Position of the character in cmdline
    char* cmdline = (char*)malloc(sizeof(char) * MAX_LEN); // Allocate memory for cmdline

    while ((c = getc(fp)) != EOF) { // Read each character from input
        if (c == '\n') // If newline, end the input
            break;
        cmdline[pos++] = c; // Add character to cmdline
    }

    if (c == EOF && pos == 0) // If EOF with no input
        return NULL; // Return NULL to indicate EOF

    cmdline[pos] = '\0'; // Null-terminate the command line
    return cmdline; // Return the command line string
}
