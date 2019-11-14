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
    char recvline[MAXLINE];
    ssize_t numRead;
    int prio;

    mq_unlink("/queue.server");
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

    for (;;) {
        numRead = mq_receive(mqID, recvline, mqAttr.mq_msgsize, &prio);
        if (numRead == -1) {
            printf("mq_receive fail\n");
            sleep(1);
            continue;
        }
        printf("Read %ld bytes; priority:%u, recvline:%s", (long) numRead, prio, recvline);
    }
    mq_close(mqID);

    return 0;
}