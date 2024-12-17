#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_ARGS 100
#define MAX_JOBS 10
#define MAX_VAR_LEN 50
#define MAX_CMD_LEN 1024

// Struct for background job
typedef struct {
    int pid;
    char command[MAX_CMD_LEN];
} Job;

Job jobs[MAX_JOBS];
int job_count = 0;

// Struct for user-defined variable
typedef struct {
    char name[MAX_VAR_LEN];
    char value[MAX_VAR_LEN];
} Variable;

Variable variables[MAX_ARGS];
int var_count = 0;

void display_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("PUCITshell@%s:- ", cwd);
    } else {
        perror("getcwd error");
    }
}

void add_job(int pid, char* command) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, command, MAX_CMD_LEN);
        job_count++;
    }
}

void remove_job(int pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            break;
        }
    }
}

void list_jobs() {
    printf("Jobs:\n");
    for (int i = 0; i < job_count; i++) {
        printf("[%d] PID: %d Command: %s\n", i + 1, jobs[i].pid, jobs[i].command);
    }
}

void kill_job(int job_num) {
    if (job_num > 0 && job_num <= job_count) {
        int pid = jobs[job_num - 1].pid;
        if (kill(pid, SIGKILL) == 0) {
            printf("Killed job [%d] with PID %d\n", job_num, pid);
            remove_job(pid);
        } else {
            perror("Failed to kill job");
        }
    } else {
        printf("Invalid job number.\n");
    }
}

void set_variable(char* name, char* value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            strncpy(variables[i].value, value, MAX_VAR_LEN);
            return;
        }
    }
    if (var_count < MAX_ARGS) {
        strncpy(variables[var_count].name, name, MAX_VAR_LEN);
        strncpy(variables[var_count].value, value, MAX_VAR_LEN);
        var_count++;
    }
}

char* get_variable(char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    return NULL;
}

void list_variables() {
    printf("Variables:\n");
    for (int i = 0; i < var_count; i++) {
        printf("%s=%s\n", variables[i].name, variables[i].value);
    }
}

void execute_command(char* input) {
    char* args[MAX_ARGS];
    int background = 0;

    // Check for & to set background execution
    if (input[strlen(input) - 1] == '&') {
        background = 1;
        input[strlen(input) - 1] = '\0';
    }

    // Tokenize input
    int arg_count = 0;
    args[arg_count] = strtok(input, " \n");
    while (args[arg_count] != NULL && arg_count < MAX_ARGS - 1) {
        args[++arg_count] = strtok(NULL, " \n");
    }
    args[arg_count] = NULL;

    // Built-in commands
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || chdir(args[1]) != 0) {
            perror("cd error");
        }
        return;
    }
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }
    if (strcmp(args[0], "jobs") == 0) {
        list_jobs();
        return;
    }
    if (strcmp(args[0], "kill") == 0) {
        if (args[1]) {
            int job_num = atoi(args[1]);
            kill_job(job_num);
        } else {
            printf("Usage: kill <job number>\n");
        }
        return;
    }
    if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("cd <dir>, exit, jobs, kill <job number>, help, set <var=value>, list_variables\n");
        return;
    }
    if (strcmp(args[0], "list_variables") == 0) {
        list_variables();
        return;
    }

    // Handle variable assignment
    if (strchr(args[0], '=')) {
        char* name = strtok(args[0], "=");
        char* value = strtok(NULL, "=");
        set_variable(name, value);
        return;
    }

    // Expand variables in arguments
    for (int i = 0; i < arg_count; i++) {
        if (args[i][0] == '$') {
            char* var_name = args[i] + 1; // Skip '$'
            char* value = get_variable(var_name);
            if (value) {
                args[i] = value; // Replace variable with its value
            } else {
                args[i] = ""; // If variable not found, replace with empty string
            }
        }
    }

    // Process external commands
    if (fork() == 0) {
        if (background) {
            setpgid(0, 0);
        }
        execvp(args[0], args);
        perror("Execution failed");
        exit(1);
    }
    if (!background) {
        wait(NULL);
    } else {
        printf("Process running in background with PID %d\n", getpid());
        add_job(getpid(), input);
    }
}

int main() {
    char input[MAX_CMD_LEN];
    while (1) {
        display_prompt();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("fgets error");
            continue; // Handle EOF or error
        }
        if (strlen(input) > 0) {
            execute_command(input);
        }
    }
    return 0;
}
