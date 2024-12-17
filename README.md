# OS Shell Assignment

## Version 1
### Features:
1. **External Commands:** Supports executing commands such as `ls`, `pwd`, `whoami`, `date`, `echo`, etc.
2. **Absolute and Relative Paths:** Programs can be executed using their full paths.  
   Example: `/bin/ls` or `./my_program`
3. **Basic Error Feedback:** The shell displays an error message if a command is not found or if `execvp` fails.

### Limitations:
1. **No Built-in Commands:** Built-in commands are limited to `exit`.
2. **No Command History:** Previously typed commands cannot be recalled.
3. **No I/O Redirection:** Does not support input/output redirection (`ls > output.txt` or `cat < input.txt`).
4. **No Piping:** Commands with pipes (e.g., `ls | grep txt`) are not supported.
5. **No Background Execution:** Commands cannot be run in the background using `&`.
6. **No Signal Handling:** Does not handle signals like `Ctrl+C`.

### Bugs:
- None found.


## Version 2
### Features:
1. **External Commands:** Executes commands such as `ls`, `pwd`, `whoami`, `date`, `echo`, and more.
2. **Absolute and Relative Paths:** Supports execution via full paths.  
   Example: `/bin/ls` or `./my_program`
3. **Basic Error Feedback:** Displays an error message if a command isn't found.
4. **I/O Redirection:** Supports redirection using `<` and `>`.  
   Example: `ls > output.txt`, `cat < input.txt`
5. **Piping:** Supports single piping, allowing command output to be used as input for another command.  
   Example: `ls | grep txt`

### Limitations:
1. **No Built-in Commands:** Only `exit` is supported; other commands are not available.
2. **No Command History:** Cannot recall previously typed commands.
3. **Single Piping Only:** Does not support complex pipelines.
4. **No Background Execution:** Cannot run commands in the background.
5. **No Signal Handling:** Signals are not managed.
6. **No Complex Redirection Combinations:** Advanced redirection patterns are not supported.

### Bugs:
- None identified.


## Version 3
### Features:
1. **External Commands:** Executes commands like `ls`, `pwd`, `whoami`, `date`, etc.
2. **Absolute and Relative Paths:** Programs can be executed via full paths.  
   Example: `/bin/ls` or `./my_program`
3. **Basic Error Feedback:** Displays errors if commands aren't found.
4. **I/O Redirection:** Supports `<` and `>` for input/output redirection.
5. **Single Piping:** Allows output of one command to be piped to another.
6. **Background Execution:** Introduces background execution with `&`, displaying job number and PID.
   Example: `find / -name f1.txt &`
7. **Basic Signal Handling:** Manages signals like `Ctrl+C` for interrupting foreground jobs.

### Limitations:
1. **No Command History:** Commands cannot be recalled once typed.
2. **Single Piping Only:** Does not support multiple pipes.
3. **Limited Signal Handling:** Only manages basic signals.
4. **No Built-in Commands:** Only `exit` is supported.
5. **No Complex Redirection Combinations:** Advanced redirection is unsupported.

### Bugs:
- None identified.


## Version 4
### Features:
1. **External Commands:** Supports commands like `ls`, `pwd`, `whoami`, etc.
2. **Absolute and Relative Paths:** Run programs by specifying full paths.
3. **Basic Error Feedback:** Displays errors if a command fails.
4. **I/O Redirection:** Supports `<` and `>` for input/output.
5. **Single Piping:** Allows single `|` pipes.
6. **Command History:** Maintains a history of the last 10 commands.
7. **Repeat Previous Commands:** Use `!number` to repeat commands.
8. **Arrow Key Navigation:** Navigate command history with the up/down arrow keys.

### Limitations:
1. **No Advanced Built-in Commands:** Only `exit` and `history` are available.
2. **Limited Command History:** Stores only the last 10 commands.
3. **Single Piping Only:** Does not support complex pipelines.
4. **No Background Execution:** Cannot run commands in the background.
5. **Basic Signal Handling:** Limited support for handling signals.


## Version 5
### Features:
1. **External Commands:** Supports external commands with absolute/relative paths.
2. **I/O Redirection:** Supports `<` and `>` symbols for redirection.
3. **Piping:** Allows a single `|` between commands.
4. **Command History with Recall:** Maintains a history of the last 10 commands, with navigation using the arrow keys.
5. **Background Execution:** Supports background execution with `&`.
6. **Signal Handling and Process Control:** Manages signals and allows process interruption.
7. **Built-in Commands:** 
   - `cd [directory]`: Change directory.
   - `exit`: Exit the shell.
   - `jobs`: List active background processes.
   - `kill [PID]`: Terminate background processes.
   - `help`: List available commands.

### Limitations:
1. **No Complex Pipelines:** Supports only a single pipe between two commands.
2. **Limited Redirection:** Complex redirection is unsupported.
3. **Partial Signal Handling:** Basic signal management only.
4. **Background Process Limitations:** Limited to listing and killing background processes.
5. **Limited Built-in Command Set:** Only basic commands available.


## Version 6
### Features:
1. **External Command Execution:** Runs external commands and supports paths.
2. **Error Handling:** Displays errors for unknown commands or failed executions.
3. **I/O Redirection:** Supports `<` and `>` for input/output redirection.
4. **Piping:** Allows single piping between commands.
5. **Built-in Commands:**
   - `cd`: Change directories.
   - `exit`: Exit the shell.
   - `jobs`: List background jobs.
   - `kill`: Terminate a background job.
   - `help`: List built-in commands.
   - `list_variables`: List user-defined variables.
6. **Background Execution:** Allows commands to run in the background.
7. **Command History:** Stores the last 10 commands, with support for recalling them.
8. **Variable Management:** Supports user-defined variables with `$` retrieval.
9. **Command Navigation:** Navigate history with up/down arrows.

### Limitations:
1. **Single Piping Only:** Only supports one `|` in commands.
2. **Limited Redirection Combinations:** Cannot combine complex patterns.
3. **Limited Signal Handling:** Basic signal management only.
4. **Basic Background Process Management:** Only lists background processes started in the session.
5. **Simple Variables:** Variables are basic key-value pairs.

