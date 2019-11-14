/* Use standard echo server; baseline measurements for nonblocking version */
#include <arpa/inet.h>          /* inet_pton */
#include <errno.h>
#include <netinet/in.h>         /* socket */
#include <sys/socket.h>         /* domain socket */
#include <sys/un.h>             /* domain socket */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>             /* bzeros strlen */
#include <unistd.h>             /* write */
#include <stddef.h>             /* offsetof */
#include "common.h"
#include "cli_select.h"

int main(int argc, char **argv)
{
    int sockfd, ret, len;
    struct sockaddr_un servaddr, cliaddr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("socket error\n");
        return sockfd;
    } else {
        printf("socket success, sockfd = %d\n", sockfd);
    }

    memset(&cliaddr, 0, sizeof(cliaddr));  
    cliaddr.sun_family = AF_UNIX;  
    strcpy(cliaddr.sun_path, CLINET_PATH);  
    len = offsetof(struct sockaddr_un, sun_path) + strlen(cliaddr.sun_path);  
    unlink(cliaddr.sun_path);  
    if (bind(sockfd, (struct sockaddr *)&cliaddr, len) < 0) {  
        printf("bind error\n");
        exit(1);  
    }  

    bzero(&servaddr, sizeof(servaddr));     /* 是用 bzero 合适，还是 memset */
    servaddr.sun_family = AF_UNIX;  
    strcpy(servaddr.sun_path, SERVER_PATH);  
    len = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);  

    ret = connect(sockfd, (SA *) & servaddr, len);
    if (ret < 0) {
        printf("connect error\n");
    } else {
        printf("connect success, ret = %d\n", ret);
    }

    str_cli(stdin, sockfd);     /* do it all */

    exit(0);
}
