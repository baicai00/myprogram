#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        return 0;
    }

    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == s)
    {
        printf("create socket failed. err=%u\n", errno);
        return -1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0)
    {
        printf("inet_pton error for %s\n", argv[1]);
        return -1;
    }

    if (connect(s, (struct sockaddr*)&servaddr, sizeof(struct sockaddr)) < 0)
    {
        printf("connect error\n");
        return 0;
    }

    const int MAXLINE = 1024;
    char recvline[MAXLINE+1];
    int n = 0;
    while((n = read(s, recvline, MAXLINE)) > 0)
    {
        recvline[n] = 0;
        //printf("%s\n", recvline);
        if (fputs(recvline, stdout) == EOF)
        {
            printf("fputs error\n");
        }
    }

    if (n < 0)
    {
        printf("read error\n");
    }
    
    return 0;
}