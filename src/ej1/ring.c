#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int start, status, pid, n;
	int buffer[1];
	status = EXIT_SUCCESS;

	if (argc != 4)
	{
		printf("Uso: anillo <n> <c> <s> \n");
		exit(0);
	}

	n = atoi(argv[1]);
	buffer[0] = atoi(argv[2]);
	start = atoi(argv[3]);

	if (n <= 0 || start < 0 || start >= n)
	{
		printf("Error: Argumento Invalido. Asegurese que 0 <= s < n.\n");
		status = EXIT_FAILURE;
		exit(status);
	}

	printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);

	int pipes[n][2];
	int parent_pipe[2];

	for (int i = 0; i < n; i++)
	{
		if (pipe(pipes[i]) == -1)
		{
			perror("pipe");
			status = EXIT_FAILURE;
			exit(status);
		}
	}
	if (pipe(parent_pipe) == -1)
	{
		perror("pipe parent");
		status = EXIT_FAILURE;
		exit(status);
	}

	for (int i = 0; i < n; i++)
	{
		pid = fork();

		if (pid == -1)
		{
			perror("fork");
			status = EXIT_FAILURE;
			exit(status);
		}

		if (pid == 0)
		{
			int number;
			for (int j = 0; j < n; j++)
			{
				if (j != i)
					close(pipes[j][0]);
				if (j != (i + 1) % n)
					close(pipes[j][1]);
			}

			close(parent_pipe[0]);

			if (read(pipes[i][0], &number, sizeof(int)) < 0)
			{
				perror("read");
				status = EXIT_FAILURE;
				exit(status);
			}
			close(pipes[i][0]);

			number++;

			if (i == (start + n - 1) % n)
			{
				if (write(parent_pipe[1], &number, sizeof(int)) < 0)
				{
					perror("write to parent");
					status = EXIT_FAILURE;
					exit(status);
				}
			}
			else
			{

				if (write(pipes[(i + 1) % n][1], &number, sizeof(int)) < 0)
				{
					perror("write to next child");
					status = EXIT_FAILURE;
					exit(status);
				}
			}

			close(pipes[(i + 1) % n][1]);
			close(parent_pipe[1]);
			exit(status);
		}
	}

	for (int i = 0; i < n; i++)
	{
		if (i != start)
			close(pipes[i][1]);
		close(pipes[i][0]);
	}

	write(pipes[start][1], &buffer[0], sizeof(int));
	printf("Parent envio %d al hijo %d\n", buffer[0], start);
	close(pipes[start][1]);

	int final_result;
	read(parent_pipe[0], &final_result, sizeof(int));
	printf("Parent recibio final result: %d\n", final_result);
	close(parent_pipe[0]);

	for (int i = 0; i < n; i++)
	{
		wait(NULL);
	}

	printf("Final result despues de los incrementos: %i\n", final_result);
	return 0;
}
