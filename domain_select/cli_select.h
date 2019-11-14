#include <arpa/inet.h>          /* inet_pton */
#include <errno.h>
#include <netinet/in.h>         /* socket */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>             /* bzeros strlen */
#include <unistd.h>             /* write */

extern void str_cli(FILE * fp, int sockfd);
