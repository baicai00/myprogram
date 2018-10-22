#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


int main()
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(6666);
    connect(s, (sockaddr*)&addr, sizeof(sockaddr));
    char buf[] = "aaaa";
    write(s, buf, strlen(buf));
    sleep(5);
    return 0;
}