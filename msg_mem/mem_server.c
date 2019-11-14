#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define	MAXLINE		128

const char *memname = "/shmem.test";

int main (int argc, char *argv[])
{
    int fd, ret;
    size_t len;                 /* Size of shared memory object */
    char *addr, *read_addr;
    char sendline[MAXLINE];
    struct stat buf;

    shm_unlink(memname);
    fd = shm_open(memname, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1) {
        perror("shm_open");
    }

    if (ftruncate(fd, MAXLINE) == -1) {           /* Resize object to hold string */
        perror("ftruncate");
    }
    printf("Resized to %ld bytes\n", (long) MAXLINE);
    addr = mmap(NULL, MAXLINE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (close(fd) == -1) {                    /* 'fd' is no longer needed */
        perror("close");
    }

    close(fd);
    if (fgets(sendline, MAXLINE, stdin) == NULL) {
        printf("fgets error, input is NULL\n");
    }
    printf("copying %d bytes\n", (int) strlen(sendline));
    memcpy(addr, sendline, strlen(sendline));             /* Copy string to shared memory */
    
    /* 等待读进程读取此部分内存 */
    sleep(20);
    ret = munmap(addr, MAXLINE);
    if (ret != 0) {
        perror("munmap");
    }
    ret = shm_unlink(memname);
    if (ret != 0) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}