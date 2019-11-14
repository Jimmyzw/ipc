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

char str[INET_ADDRSTRLEN];

int main(int argc, char **argv)
{
    int i, maxi, maxfd, listenfd, connfd, sockfd, ret, size;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];
    char buf_ret[MAXLINE];
    socklen_t clilen;
    struct sockaddr_un cliaddr, servaddr;

    listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listenfd < 0) {
        printf("socket error\n");
        return listenfd;
    } else {
        printf("socket success, listenfd = %d\n", listenfd);
    }   

    unlink(SERVER_PATH);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, SERVER_PATH);
    size = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);

    ret = bind(listenfd, (SA *) & servaddr, size);
    if (ret < 0) {
        printf("bind error\n");
        return ret;
    } else {
        printf("bind success, ret = %d\n", ret);
    }

    ret = listen(listenfd, LISTENQ);
    if (ret < 0) {
        printf("listen error\n");
        return ret;
    } else {
        printf("listen success, ret = %d\n", ret);
    }

    maxfd = listenfd;           /* initialize */
    maxi = -1;                  /* index into client[] array */
    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;         /* -1 indicates available entry */
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;) {
        rset = allset;          /* structure assignment */
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {        /* new client connection */
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (SA *) & cliaddr, &clilen);
            printf("new client: %d, path %s\n", connfd, cliaddr.sun_path);

            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connfd; /* save descriptor,select一次可能有多个客户端,保存下fd,每个nready保存一个fd */
                    printf("client[%d]=%d\n", i ,connfd);
                    break;
                }
            }
            if (i == FD_SETSIZE) {
                printf("too many clients\n");
            }

            FD_SET(connfd, &allset);    /* add new descriptor to set */
            if (connfd > maxfd) {
                maxfd = connfd; /* for select */
            }
            if (i > maxi) {
                maxi = i;       /* max index in client[] array */
            }

            if (--nready <= 0) {
                continue;       /* no more readable descriptors */
            }
        }

        for (i = 0; i <= maxi; i++) {   /* check all clients for data */
            if ((sockfd = client[i]) < 0) {
                continue;
            }
            if (FD_ISSET(sockfd, &rset)) {  /* ISSET的,即有客户端连接的 */
                if ((n = read(sockfd, buf, MAXLINE)) == 0) {
                    /* 4connection closed by client */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    printf("close success, sockfd=%d\n", sockfd);
                    client[i] = -1;
                } else {
                    printf("buf=%s", buf);
                    (void) memset(buf_ret, 0, sizeof(buf_ret));
                    (void) snprintf(buf_ret, sizeof(buf_ret), "msg from server");
                    printf("buf_ret=%s\n", buf_ret);
                    ret = write(sockfd, buf_ret, sizeof(buf_ret));
                    if (ret != sizeof(buf_ret)) {
                        printf("write failed, ret=%d, len=%d\n", ret, (int) sizeof(buf_ret));
                    }
                }

                if (--nready <= 0)
                    break;      /* no more readable descriptors */
            }
        }   /* End of For */
    }   /* End of For */
}
