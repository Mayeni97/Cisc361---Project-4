#include "sh.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/* Signal handler */
void sig_handler(int signal); 

/* Main function */
int main(int argc, char **argv, char **envp) {
    // Set up signal handlers
    signal(SIGINT, sig_handler);
    signal(SIGTSTP, sig_handler);
    signal(SIGTERM, sig_handler);

    // If a file is passed as an argument, open and execute commands from it
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }

        char line[1024];
        int last_exit_code = 0;
        while (fgets(line, sizeof(line), file)) {
            last_exit_code = sh_execute_command(line, envp);  // Handle command execution
        }

        fclose(file);
        return last_exit_code;
    }

    // If no file is provided, run the shell interactively
    return sh(argc, argv, envp);
}

/* Signal handler implementation */
void sig_handler(int asignal) {
    // Ignore signals
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
}
