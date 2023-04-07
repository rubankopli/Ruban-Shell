#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

// #defines
#define RUBAN_SHELL_RL_BUFFERSIZE 1024
#define RUBAN_SHELL_TOK_BUFFERSIZE 64
#define RUBAN_SHELL_TOK_DELIMITER " \t\r\n\a"

// Declarations for shell built-ins

// Change directory
int ruban_shell_change_directory(char** args);
// Print help info
int ruban_shell_help(char** args);
// Exit the shell
int ruban_shell_exit(char** args);

// List of built-in commands, followed by their corresponding functions
char* builtin_str[]           = {"cd", "help", "exit"};
int (*builtin_func[])(char**) = {&ruban_shell_change_directory, &ruban_shell_help, &ruban_shell_exit};

int ruban_shell_num_builtins()
{
    return (sizeof(builtin_str) / sizeof(char*));
}

// Definitions for built-ins

int ruban_shell_change_directory(char** args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "ruban_shell: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("ruban_shell");
        }
    }
    return 1;
}

int ruban_shell_help(char** args)
{
    int i = 0;
    printf("Ruban Kopli's custom shell, 'ruban_shell'\n");
    printf("Type program names and arguments, then hit enter.\n");
    printf("The following functionalities are built-in to the shell itself:\n");
    for (i = 0; i < ruban_shell_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }
    printf("Use the 'man' command for information on other programs.\n");
    return 1;
}

int ruban_shell_exit(char** args)
{
    return 0;
}

// Helper functions

// Read in a line
char* ruban_shell_read_line()
{
    int   bufferSize       = RUBAN_SHELL_RL_BUFFERSIZE;
    int   position         = 0;
    char* buffer           = malloc(sizeof(char) * bufferSize);
    int   currentCharacter = '\0';

    if (!buffer)
    {
        fprintf(stderr, "ruban_shell: read_line allocation error!\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Read in a character
        currentCharacter = getchar();

        // If we hit EOF replace it with null string terminator and return
        if ((currentCharacter == EOF) || (currentCharacter == '\n'))
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = currentCharacter;
        }
        position++;

        // If we exceed the buffer size, reallocate  memory
        if (position >= bufferSize)
        {
            bufferSize += RUBAN_SHELL_RL_BUFFERSIZE;
            buffer = realloc(buffer, bufferSize);
            if (!buffer)
            {
                fprintf(stderr, "ruban_shell: read_line reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

// Tokenize a line
char** ruban_shell_split_line(char* line)
{
    int    bufferSize = RUBAN_SHELL_TOK_BUFFERSIZE;
    int    position   = 0;
    char** tokens     = malloc(bufferSize * sizeof(char*));
    char*  token      = NULL;

    if (!tokens)
    {
        fprintf(stderr, "ruban_shell: split_line allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, RUBAN_SHELL_TOK_DELIMITER);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufferSize)
        {
            bufferSize += RUBAN_SHELL_TOK_BUFFERSIZE;
            tokens = realloc(tokens, bufferSize * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "ruban_shell: split_line reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, RUBAN_SHELL_TOK_DELIMITER);
    }
    tokens[position] = NULL;
    return tokens;
}

// Launch a process
int ruban_shell_launch(char** args)
{
    pid_t pid    = 1;
    pid_t wpid   = 1;
    int   status = 0;

    pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("ruban_shell");
        }
        // Error forking
        else if (pid < 0)
        {
            perror("ruban_shell");
        }
        // Parent process
        else
        {
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while ((!WIFEXITED(status)) && (!WIFSIGNALED(status)));
        }
    }

    return 1;
}

// Execute arguments
int ruban_shell_execute(char** args)
{
    int i = 0;

    if (args[0] == NULL)
    {
        // An empty command was entered
        return 1;
    }

    for (i = 0; i < ruban_shell_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return ruban_shell_launch(args);
}

// Execution loop
void ruban_shell_loop()
{
    char*  line   = NULL;
    char** args   = NULL;
    int    status = 0;

    do
    {
        printf("► ");
        line   = ruban_shell_read_line();
        args   = ruban_shell_split_line(line);
        status = ruban_shell_execute(args);

        free(line);
        free(args);
    } while (status);
}

// Main function
int main(int argc, char** argv)
{
    // Load config files

    // Run command loop
    ruban_shell_loop();

    // Perform shutdown/cleanup

    return EXIT_SUCCESS;
}