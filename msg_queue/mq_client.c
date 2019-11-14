#include <sys/types.h>          /* some systems still require this */
#include <sys/stat.h>
#include <stdio.h>              /* for convenience */
#include <stdlib.h>             /* for convenience */
#include <stddef.h>             /* for offsetof */
#include <string.h>             /* for convenience */
#include <unistd.h>             /* for convenience */
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

#define	MAXLINE		8192    /* max text line length */

int main()
{
    mqd_t mqID;
    struct mq_attr mqAttr;
    char sendline[MAXLINE];
    char recvline[MAXLINE];
    ssize_t numRead;
    int prio, i;

//    mq_unlink("/queue.server");
    /* 所创建的POSIX消息队列不会在文件系统中创建真正的路径名。且POSIX的名字只能以一个’/’开头 */
    mqID = mq_open("/queue.server", O_RDWR | O_CREAT, 0666, NULL);
    if (mqID < 0) {
        printf("mq_open error=%s.\n", strerror(errno));
        return -1;
    }

    if (mq_getattr(mqID, &mqAttr) < 0)
    {
        printf("mq_getattr error.\n");
        return -1;
    }
    printf("mq_flags:%d\n", (int) mqAttr.mq_flags);
    printf("mq_maxmsg:%d\n", (int) mqAttr.mq_maxmsg);
    printf("mq_msgsize:%d\n", (int) mqAttr.mq_msgsize);
    printf("mq_curmsgs:%d\n", (int) mqAttr.mq_curmsgs);
    i = 0;
    for (;;) {
        if (fgets(sendline, MAXLINE, stdin) == NULL) {
            printf("fgets error, input is NULL\n");
            break;         /* all done */
        } else {
            i++;
            printf("fgets success, sendline=%s", sendline);
            /* msg_prio 级别，数值越大，优先级越高 */
            if (mq_send(mqID, sendline, mqAttr.mq_msgsize, i) == -1) {
                printf("mq_send failed.\n");
                break;
            } else {
                printf("mq_send success.\n");
            }
        }
    }

    if (mqID > 0) {
        mq_close(mqID);
    }

    return 0;
}