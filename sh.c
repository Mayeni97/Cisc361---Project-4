#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"
#include <glob.h>

extern char **environ;

int sh(int argc, char **argv, char **envp) {
    char *prompt = calloc(PROMPTMAX, sizeof(char));
    char *commandline = calloc(MAX_CANON, sizeof(char));
    char *command, *pwd, *owd;
    char **args = calloc(MAXARGS, sizeof(char*));
    int go = 1, status;
    struct passwd *password_entry;
    char *homedir;
    struct pathelement *pathlist;

    // Initialize $0 and $?
    setenv("?", "0", 1);  // Initialize $? to 0
    setenv("0", argv[0], 1);  // Set $0 to the command used to run the shell

    int uid = getuid();
    password_entry = getpwuid(uid);  /* Get passwd info */
    homedir = password_entry->pw_dir; /* Home directory */

    /* Get the current working directory */
    if ((pwd = getcwd(NULL, PATH_MAX+1)) == NULL) {
        perror("getcwd");
        exit(2);
    }
    owd = strdup(pwd);
    prompt[0] = ' '; 
    prompt[1] = '\0';

    /* Put PATH into a linked list */
    pathlist = get_path();

    while (go) {
        /* Display the prompt */
        if (getcwd(pwd, PATH_MAX) == NULL) {
            perror("getcwd");
            continue;
        }
        printf("%s [%s]> ", prompt, pwd);
        fflush(stdout);

        /* Get command line */
        if (fgets(commandline, MAX_CANON, stdin) == NULL) {
            clearerr(stdin);
            continue;
        }

        /* Remove newline from command line */
        commandline[strcspn(commandline, "\n")] = '\0';

        /* Execute the command */
        int last_exit_code = sh_execute_command(commandline, envp);

        /* Update $? with the last exit code */
        char exit_code_str[10];
        sprintf(exit_code_str, "%d", last_exit_code);
        setenv("?", exit_code_str, 1);
    }

    /* Clean up */
    free(prompt);
    free(commandline);
    free(args);
    free(pwd);
    free(owd);
    return 0;
}

int sh_execute_command(char *cmd, char **envp) {
    // Parse command and arguments
    char *args[MAXARGS];
    char *command = strtok(cmd, " ");
    int i = 0;

    while (command != NULL && i < MAXARGS - 1) {
        args[i++] = substitute_env_vars(command);  // Handle environment variable substitution
        command = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Handle empty command
    if (args[0] == NULL) {
        return 0;
    }

    // Built-in command: exit
    if (strcmp(args[0], "exit") == 0) {
        if (args[1] != NULL) {
            exit(atoi(args[1]));  // Exit with the provided status code
        }
        exit(0);
    }

    // Built-in command: cd
    if (strcmp(args[0], "cd") == 0) {
        char *homedir = getenv("HOME");
        if (args[1] == NULL) {
            chdir(homedir);  // Change to home directory if no path is provided
        } else {
            if (chdir(args[1]) != 0) {
                perror("chdir");
            }
        }
        return 0;
    }

    // Built-in command: pwd
    if (strcmp(args[0], "pwd") == 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
        return 0;
    }

    // Built-in command: addacc
    if (strcmp(args[0], "addacc") == 0) {
        addacc(args[1]);
        return 0;
    }

    // Built-in command: setenv
    if (strcmp(args[0], "setenv") == 0) {
        if (args[1] && args[2]) {
            custom_setenv(args[1], args[2]);  // Call custom setenv function
        } else {
            fprintf(stderr, "Usage: setenv VAR VALUE\n");
        }
        return 0;
    }

    // Built-in command: printenv
    if (strcmp(args[0], "printenv") == 0) {
        if (args[1]) {
            custom_printenv(args[1]);  // Call custom printenv function
        } else {
            custom_printenv(NULL);  // Print all environment variables
        }
        return 0;
    }

    // Built-in command: unset
    if (strcmp(args[0], "unset") == 0) {
        if (args[1]) {
            unsetenv(args[1]);  // Unset environment variable
        } else {
            fprintf(stderr, "Usage: unset VAR\n");
        }
        return 0;
    }

    // Conditional Execution: ?
    if (args[0][0] == '?') {
        int last_exit_code = atoi(getenv("?"));
        if (last_exit_code == 0) {
            return sh_execute_command(cmd + 1, envp);  // Skip '?' and execute the command
        }
        return 0;  // Do nothing if $? is non-zero
    }

    // NOECHO environment variable check
    if (getenv("NOECHO") == NULL || strlen(getenv("NOECHO")) == 0) {
        printf("Executing: %s\n", cmd);  // Only print if NOECHO is not set
    }

    // External command execution
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(args[0], args);
        perror("execvp");  // If execvp fails, print error
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);  // Wait for the child to complete
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);  // Return the exit status
        } else {
            return -1;  // Return error code if the process did not exit normally
        }
    } else {
        perror("fork");
        return -1;
    }
}

void addacc(char* arg) {
    char* acc_val_str = getenv("ACC");
    int acc_value = (acc_val_str != NULL) ? atoi(acc_val_str) : 0;  // Default to 0 if ACC doesn't exist

    if (arg != NULL) {
        acc_value += atoi(arg);  // Add the provided value
    } else {
        acc_value += 1;  // Increment by 1 if no argument provided
    }

    char new_acc_val[10];
    sprintf(new_acc_val, "%d", acc_value);
    setenv("ACC", new_acc_val, 1);  // Update the ACC environment variable
}

char* substitute_env_vars(char* arg) {
    if (arg[0] == '$') {
        char* var_value = getenv(arg + 1);  // Skip the '$' to get env variable name
        if (var_value != NULL) {
            return strdup(var_value);  // Return the environment variable's value
        } else {
            return strdup("");  // Return empty string if variable doesn't exist
        }
    }
    return arg;  // Return the argument unchanged if no substitution needed
}

void list(char *dir) {
    /* List files in the directory using opendir() and readdir() */
    DIR *dp;
    struct dirent *entry;

    if ((dp = opendir(dir)) == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dp);
}

/* Custom function for printenv */
void custom_printenv(const char *var) {
    if (var == NULL) {
        for (char **env = environ; *env; ++env) {
            printf("%s\n", *env);
        }
    } else {
        char *value = getenv(var);
        if (value) {
            printf("%s=%s\n", var, value);  // Remove extra quotes here
        } else {
            printf("%s is not set\n", var);
        }
    }
}

/* Custom function for setenv */
void custom_setenv(const char *var, const char *value) {
    if (strcmp(var, "ACC") == 0) {
        int acc_value = atoi(value);
        char acc_str[20];
        snprintf(acc_str, sizeof(acc_str), "%d", acc_value);
        setenv("ACC", acc_str, 1);
    } else {
        setenv(var, value, 1);
    }
}

/* Usage examples for custom_printenv and custom_setenv */
int custom_printenv_main(int argc, char **argv) {
    if (argc > 1) {
        custom_printenv(argv[1]);
    } else {
        custom_printenv(NULL);
    }
    return 0;
}

int custom_setenv_main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: setenv VAR VALUE\n");
        return -1;
    }
    custom_setenv(argv[1], argv[2]);
    return 0;
}
