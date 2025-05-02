#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 /* The maximum length command */

int main(void)
{
    char *args[MAX_LINE/2 + 1];         // command-line arguments
    char input[MAX_LINE];               // user input
    char last_command[MAX_LINE] = "";   // history buffer for most recent command
    int should_run = 1;                 // flag to determine when to exit program

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        // Read input from user
        if (fgets(input, MAX_LINE, stdin) == NULL) {
            break;
        }
        // Remove trailing newline character if it exists
        input[strcspn(input, "\n")] = '\0';

        // Check if command is "!!"
        if (strcmp(input, "!!") == 0) {
            if (strlen(last_command) == 0) {
                printf("No commands in history.\n");
                continue;
            }
            else {
                // Echo command on user screen
                printf("%s\n", last_command);
                strcpy(input, last_command);
            }
        }
        else { // If not "!!", update history with current command
            if (strlen(input) > 0) {
                strcpy(last_command, input);
            }
        }

        // Parse input into tokens and store them in args array
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL && i < MAX_LINE/2) {
            args[i] = token;
            i++;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;     // last elemen must be NULL for execvp()


        // If command is "exit", terminate shell
        if (args[0] != NULL && strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        // If last argument is "&", run in background
        int runInBackground = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            runInBackground = 1;
            args[i - 1] = NULL;     // Remove "&" from args
        }

        // Input/Output Redirection
        int redirect_out = 0;
        int redirect_in = 0;
        int redir_index = -1;
        char *redir_file = NULL;
        // Loop through the tokens to check for ">" or "<"
        for (int j = 0; j < i; j++) {
            if (strcmp(args[j], ">") == 0) {
                redirect_out = 1;
                redir_index = j;
                redir_file = args[j+1]; // filename should follow '>'
                break;
            }
            else if (strcmp(args[j], "<") == 0) {
                redirect_in = 1;
                redir_index = j;
                redir_file = args[j+1]; // filename should follow '<'
                break;
            }
        }

        // Fork child process
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }
        else if (pid == 0) {     // in child process
            /* Upon output redirection request */
            if (redirect_out) {
                int fd = open(redir_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open for output failed");
                    exit(1);
                }
                // Duplicate the file descriptor to STDOUT_FILENO so that
                // writes to standard output are sent to the file
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("duplicate for output failed");
                    exit(1);
                }
                close(fd);
                // Remove the redirection operator and filename from args
                args[redir_index] = NULL;
            }
            /* Upon input redirection request */
            if (redirect_in) {
                int fd = open(redir_file, O_RDONLY);
                if (fd < 0) {
                    perror("open for input failed");
                    exit(1);
                }
                // Duplicate the file descriptor to STDIN_FILENO so that
                // reads come from the file
                if (dup2(fd, STDIN_FILENO) < 0) {
                    perror("duplicate for input failed");
                    exit(1);
                }
                close(fd);
                args[redir_index] = NULL;
            }

            // Execute command
            if (execvp(args[0], args) == -1) {
                perror("execvp failed");
            }
            exit(1);
        }
        else {              // in parent process
            // Wait for child to finish
            if (!runInBackground) {
                wait(NULL);
            }
        }
    }
    return 0;
}