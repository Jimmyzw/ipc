#include <sys/types.h>          /* some systems still require this */
#include <sys/stat.h>
#include <stdio.h>              /* for convenience */
#include <stdlib.h>             /* for convenience */
#include <stddef.h>             /* for offsetof */
#include <string.h>             /* for convenience */
#include <unistd.h>             /* for convenience */
#include <signal.h>             /* for SIG_ERR */

#define	MAXLINE	4096            /* max line length */

int main(void)
{
    int n;
    int fd[2];
    pid_t pid;
    char line[MAXLINE];

    if (pipe(fd) < 0) {
        printf("pipe error");
    }
    if ((pid = fork()) < 0) {
        printf("fork error");
    } else if (pid > 0) {       /* parent */
        close(fd[0]);       /* 关闭读管道fd */
        write(fd[1], "hello world\n", 12);
    } else {                    /* child */
        close(fd[1]);       /* 关闭写管道fd */
        n = read(fd[0], line, MAXLINE);
        write(STDOUT_FILENO, line, n);
    }
    exit(0);
}
