#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main(int argc, char *argv[])
{
    int p1[2], p2[2];
    char recv_buf[5];

    pipe(p1);//p1[0]: đầu đọc của cha , p1[1]: đầu ghi của cha
    pipe(p2);//p2[0]: đầu đọc của con, p2[1]: đầu ghi của con
    
    int pid = fork();

    if (pid < 0)
    {
        fprintf(2,"Fork failed\n");
        exit(1);
    }

    if (pid == 0)
    {
        close(p1[1]);
        close(p2[0]);
        read(p1[0], recv_buf, 5);
        printf("%d: received %s\n", getpid(), recv_buf);
        
        write(p2[1], "pong", 5);
        close(p1[0]);
        close(p2[1]);
        exit(0);
    }

    else
    {
        close(p1[0]);
        close(p2[1]);
        write(p1[1], "ping",5);
        
        read(p2[0], &recv_buf,5);
        printf("%d: received %s\n", getpid(), recv_buf);
        close(p1[1]);
        close(p2[0]);
        exit(0);
    }
}