#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

int main()
{
    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count;

    while (1)
    {
        printf("Shell> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        command_count = 0;
        char *token = strtok(command, "|");
        while (token != NULL && command_count < MAX_COMMANDS)
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        if (command_count == 0)
            continue;

        int pipefd[command_count - 1][2];

        for (int i = 0; i < command_count; i++)
        {
            if (i < command_count - 1 && pipe(pipefd[i]) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid_t pid = fork();
            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            {

                if (i > 0)
                {
                    dup2(pipefd[i - 1][0], STDIN_FILENO);
                    close(pipefd[i - 1][0]);
                }
                if (i < command_count - 1)
                {
                    dup2(pipefd[i][1], STDOUT_FILENO);
                    close(pipefd[i][1]);
                }

                for (int j = 0; j < command_count - 1; j++)
                {
                    close(pipefd[j][0]);
                    close(pipefd[j][1]);
                }

                char *args[10];
                int arg_count = 0;
                char *arg = strtok(commands[i], " ");
                while (arg != NULL && arg_count < 10)
                {
                    args[arg_count++] = arg;
                    arg = strtok(NULL, " ");
                }
                args[arg_count] = NULL;

                if (execvp(args[0], args) == -1)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
        }

        for (int i = 0; i < command_count - 1; i++)
        {
            close(pipefd[i][0]);
            close(pipefd[i][1]);
        }
        for (int i = 0; i < command_count; i++)
        {
            wait(NULL);
        }
    }
    return 0;
}
