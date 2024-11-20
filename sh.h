
#include "get_path.h"

extern int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);

int sh_execute_command(char *cmd, char **envp);
void addacc(char* arg);
char* substitute_env_vars(char* arg);
void custom_printenv(const char *var);
void custom_setenv(const char *var, const char *value);

#define PROMPTMAX 32
#define MAXARGS 10
