#include <arpa/inet.h>          /* inet_pton */
#include <errno.h>
#include <netinet/in.h>         /* socket */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>             /* bzeros strlen */
#include <unistd.h>             /* write */
#include "common.h"
#include "cli_select.h"

void str_cli(FILE * fp, int sockfd)
{
    int     n, maxfdp1;
    fd_set  rset;
    char    sendline[MAXLINE], recvline[MAXLINE];

    FD_ZERO(&rset);
    for (;;) {
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);       /* select只是非阻塞IO的使用，cli可以使用select进行输入，server也可以 */
        if (FD_ISSET(fileno(fp), &rset)) {      /* input is readable */
            if (fgets(sendline, MAXLINE, fp) == NULL) {
                printf("fgets error, input is NULL\n");
                return;         /* all done */
            } else {
                printf("fgets success, sendline=%s", sendline);
                write(sockfd, sendline, strlen(sendline));
            }
        }   /* End of If */
        if (FD_ISSET(sockfd, &rset)) {      /* socket is readable */
            if ((n = read(sockfd, recvline, MAXLINE)) == 0) {
                printf("str_cli: server terminated prematurely\n");
                sockfd = -1;
                break;
            } else {
                printf("read success, sockfd=%d recvline=%s\n", sockfd, recvline);
                fputs(recvline, stdout);
            }
        }   /* End of If */
    }   /* End of For */
}
