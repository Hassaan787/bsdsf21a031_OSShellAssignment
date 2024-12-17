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
#define PROMPT "PUCITVer5shell:- "
#define HISTORY_SIZE 10
#define MAX_JOBS 10  // Max number of background jobs

typedef struct {
    pid_t pid;
    char command[MAX_LEN];
} Job;

int execute(char* arglist[], int is_background);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);
void add_to_history(char *cmd);
char* get_history_command(int index);
int handle_builtin(char** arglist);
void add_job(pid_t pid, char* cmd);
void list_jobs();
void kill_job(int job_index);
void remove_job(pid_t pid);

// History buffer
char *history[HISTORY_SIZE];
int history_count = 0;
int history_start = 0;

// Background jobs
Job jobs[MAX_JOBS];
int job_count = 0;

int main() {
    char *cmdline;
    char **arglist;
    char *prompt = PROMPT;

    // Initialize jobs array
    for (int i = 0; i < MAX_JOBS; i++) jobs[i].pid = 0;

    // Set up signal handler to prevent zombie processes
    signal(SIGCHLD, SIG_IGN);

    while ((cmdline = read_cmd(prompt, stdin)) != NULL) {
        if (cmdline[0] == '!' && strlen(cmdline) > 1) {
            int hist_index = cmdline[1] == '-' ? history_count - 1 : atoi(&cmdline[1]) - 1;
            if (hist_index < 0 || hist_index >= history_count) {
                printf("No such command in history.\n");
                free(cmdline);
                continue;
            }
            free(cmdline);
            cmdline = strdup(get_history_command(hist_index));
            printf("Repeating command: %s\n", cmdline);
        } else {
            add_to_history(cmdline);
        }

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (handle_builtin(arglist)) {
                free(cmdline);
                for (int j = 0; j < MAXARGS + 1; j++) free(arglist[j]);
                free(arglist);
                continue;
            }

            int is_background = 0;
            int last_arg_index = 0;
            while (arglist[last_arg_index] != NULL) last_arg_index++;
            if (last_arg_index > 0 && strcmp(arglist[last_arg_index - 1], "&") == 0) {
                is_background = 1;
                arglist[last_arg_index - 1] = NULL;
            }

            execute(arglist, is_background);

            for (int j = 0; j < MAXARGS + 1; j++) free(arglist[j]);
            free(arglist);
            free(cmdline);
        }
    }

    for (int i = 0; i < HISTORY_SIZE; i++) if (history[i] != NULL) free(history[i]);
    printf("\nExiting shell...\n");
    return 0;
}

// Executes commands, handling background jobs
int execute(char* arglist[], int is_background) {
    int status;
    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {
        execvp(arglist[0], arglist);
        perror("Command execution failed");
        exit(1);
    } else {
        if (is_background) {
            add_job(pid, arglist[0]);
            printf("[Job %d] %d\n", job_count, pid);
        } else {
            waitpid(pid, &status, 0);
            printf("child exited with status %d\n", status >> 8);
        }
    }
    return 0;
}

// Handles built-in commands
int handle_builtin(char** arglist) {
    if (strcmp(arglist[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL || chdir(arglist[1]) != 0) perror("cd failed");
        return 1;
    } else if (strcmp(arglist[0], "jobs") == 0) {
        list_jobs();
        return 1;
    } else if (strcmp(arglist[0], "kill") == 0) {
        if (arglist[1] != NULL) {
            int job_index = atoi(arglist[1]) - 1;
            kill_job(job_index);
        } else {
            printf("Usage: kill [job_number]\n");
        }
        return 1;
    } else if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("cd [directory] - Change the working directory\n");
        printf("exit - Exit the shell\n");
        printf("jobs - List background jobs\n");
        printf("kill [job_number] - Terminate a background job\n");
        printf("help - Show this help message\n");
        return 1;
    }
    return 0;
}

// Adds a job to the jobs array
void add_job(pid_t pid, char* cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, cmd, MAX_LEN - 1);
        jobs[job_count].command[MAX_LEN - 1] = '\0';
        job_count++;
    } else {
        printf("Job list full, cannot add more background jobs.\n");
    }
}

// Lists all background jobs
void list_jobs() {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] %d %s\n", i + 1, jobs[i].pid, jobs[i].command);
        }
    }
}

// Kills a job by its job index
void kill_job(int job_index) {
    if (job_index >= 0 && job_index < job_count && jobs[job_index].pid != 0) {
        if (kill(jobs[job_index].pid, SIGKILL) == 0) {
            printf("Job [%d] %d terminated\n", job_index + 1, jobs[job_index].pid);
            remove_job(jobs[job_index].pid);
        } else {
            perror("Failed to kill job");
        }
    } else {
        printf("No such job\n");
    }
}

// Removes a job from the jobs array by pid
void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            jobs[i].pid = 0;  // Mark job as removed
            break;
        }
    }
}

// Adds a command to the history
void add_to_history(char *cmd) {
    if (history_count < HISTORY_SIZE) {
        history[history_count] = strdup(cmd);
        history_count++;
    } else {
        free(history[history_start]);
        history[history_start] = strdup(cmd);
        history_start = (history_start + 1) % HISTORY_SIZE;
    }
}

// Gets a command from history by index
char* get_history_command(int index) {
    return history[(history_start + index) % HISTORY_SIZE];
}

// Tokenizes command line input into arguments
char** tokenize(char* cmdline) {
    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int j = 0; j < MAXARGS + 1; j++) arglist[j] = (char*)malloc(sizeof(char) * ARGLEN);

    int argnum = 0;
    char* cp = cmdline;
    char* start;
    int len;

    while (*cp != '\0') {
        while (*cp == ' ' || *cp == '\t') cp++;
        start = cp;
        len = 1;

        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) len++;
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }
    arglist[argnum] = NULL;
    return arglist;
}

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    int c;
    int pos = 0;
    char* cmdline = (char*)malloc(sizeof(char) * MAX_LEN);

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }
    if (c == EOF && pos == 0) return NULL;

    cmdline[pos] = '\0';
    return cmdline;
}
