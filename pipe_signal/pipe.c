#include <sys/types.h>          /* some systems still require this */
#include <sys/stat.h>
#include <stdio.h>              /* for convenience */
#include <stdlib.h>             /* for convenience */
#include <stddef.h>             /* for offsetof */
#include <string.h>             /* for convenience */
#include <unistd.h>             /* for convenience */
#include <signal.h>             /* for SIG_ERR */

#define	MAXLINE	4096            /* max line length */

static void sig_pipe(int);      /* our signal handler */

int main(void)
{
    int n, ret, fd[2];
    pid_t pid;
    char line[MAXLINE];

    if (signal(SIGPIPE, sig_pipe) == SIG_ERR) {
        printf("signal error");
    }
    if (pipe(fd) < 0) {
        printf("pipe error");
    }

    if ((pid = fork()) < 0) {
        printf("fork error");
    } else if (pid > 0) {       /* parent */
        printf("parent pid:%d.\n", getpid());
        close(fd[0]);         /* 关闭读管道 */

        /* 情况1 直接write到fd[1] */
        ret = write(fd[1], "hello world\n", 12);
        printf("write success, ret:%d.\n", ret);

        /* 情况2 从stdin写入fd[1] */
        fgets(line, MAXLINE, stdin);
        n = strlen(line);
        if (write(fd[1], line, n) != n) {
            printf("write error to pipe");
        }
        if (ferror(stdin)) {
            printf("fgets error on stdin");
        }
        exit(0);
    } else {                    /* child */
        printf("child pid:%d.\n", getpid());
        close(fd[0]);
        close(fd[1]);
    }
    exit(0);
}

static void sig_pipe(int signo)
{
    printf("SIGPIPE caught\n");
    exit(1);
}
