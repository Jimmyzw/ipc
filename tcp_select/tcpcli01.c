/* Use standard echo server; baseline measurements for nonblocking version */
#include <arpa/inet.h>          /* inet_pton */
#include <errno.h>
#include <netinet/in.h>         /* socket */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>             /* bzeros strlen */
#include <unistd.h>             /* write */
#include "common.h"
#include "cli_select.h"

int main(int argc, char **argv)
{
    int     sockfd, ret;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        printf("usage: tcpcli <IPaddress>");
        exit(0);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("socket error\n");
        return sockfd;
    } else {
        printf("socket success, sockfd = %d\n", sockfd);
    }

    bzero(&servaddr, sizeof(servaddr));     /* 是用 bzero 合适，还是 memset */
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    ret = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    if (ret <= 0) {
        printf("inet_pton error for %s\n", (char *) argv[1]);
    } else {
        printf("inet_pton success, servaddr.sin_addr=%s\n", (char *) argv[1]);
    }

    ret = connect(sockfd, (SA *) & servaddr, sizeof(servaddr));
    if (ret < 0) {
        printf("connect error\n");
    } else {
        printf("connect success, ret = %d\n", ret);
    }

    str_cli(stdin, sockfd);     /* do it all */

    exit(0);
}
