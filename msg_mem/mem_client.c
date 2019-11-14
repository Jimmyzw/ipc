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

    fd = shm_open(memname, O_RDONLY, 0);
    if (fd == -1) {
        perror("shm_open");
    }

    if (fstat(fd, &buf) == -1) {
        perror("fstat failed");
        exit(EXIT_FAILURE);
    }

    printf("size=%ld, mode=%o\n", buf.st_size, buf.st_mode & 0777);
    read_addr = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (read_addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    printf("the read msg is:%s", read_addr);

    if (close(fd) == -1) {                    /* 'fd' is no longer needed */
        perror("close");
    }
    return 0;
}