#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sieve(int) __attribute__((noreturn));

void sieve(int read_fd) 
{
    int first, current;
    int new_pipe[2];

    if (read(read_fd, &first, sizeof(int)) == 0) 
    {
        close(read_fd);
        exit(0);
    }

    printf("prime %d\n", first);

    if (pipe(new_pipe) < 0) {
        printf("Error: Pipe creation failed\n");
        close(read_fd);
        exit(1);
    }

    int child_pid = fork();
    if (child_pid < 0) 
    {
        printf("Error: Fork failed\n");
        close(read_fd);
        close(new_pipe[0]);
        close(new_pipe[1]);
        exit(1);
    } 
    else if (child_pid == 0) 
    { 
        // Tiến trình con
        close(new_pipe[1]);
        close(read_fd);
        sieve(new_pipe[0]);
        close(new_pipe[0]);
        exit(0);
    } 
    else
    {
        // Tiến trình cha
        close(new_pipe[0]);

        while (read(read_fd, &current, sizeof(int))) {
            if (current % first != 0) {
                write(new_pipe[1], &current, sizeof(int));
            }
        }
        close(new_pipe[1]);
        close(read_fd);
        wait(0);
    }

    exit(0);
}

int main() {
    int initial_pipe[2];

    if (pipe(initial_pipe) < 0) 
    {
        printf("Error: Pipe creation failed\n");
        exit(1);
    }

    int root_pid = fork();
    if (root_pid < 0) 
    {
        printf("Error: Fork failed\n");
        close(initial_pipe[0]);
        close(initial_pipe[1]);
        exit(1);
    } 
    else if (root_pid == 0) 
    {
        close(initial_pipe[1]);
        sieve(initial_pipe[0]);
        close(initial_pipe[0]);
        exit(0);
    } 
    else {
        close(initial_pipe[0]);

        for (int num = 2; num <= 280; num++) {

            write(initial_pipe[1], &num, sizeof(int));
        }
        close(initial_pipe[1]);
        wait(0);
    }
    exit(0);
}
