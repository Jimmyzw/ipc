linux进程间支持多种通信机制进行通信、传递消息。常用的包括：管道、共享内存、消息队列、socket。
代码参考自apue、unp、tlpi。代码链接：
https://github.com/Jimmyzw/ipc.git

全部的进程间通信方式如下所示：
![Image.png](https://upload-images.jianshu.io/upload_images/124641-a94a0c1e419e0706.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

#### 管道
管道的含义为数据输入输出不存储过程。管道不做数据储存，只是数据的搬运工，一个端口进入，另一个端口输出，就像水管一样。shell执行命令时，会创建一个单独的新进程执行。命令行使用的“|”也是管道，功能也是进程间通信，即将前一条命令的输出作为后一条命令的输入。与管道有点类似的，在文件系统中写一个文件，另一个进程去读取的方式进行通信。文件系统读写简单，但是IO消耗大，只适用于简单的通信。

查看创建管道的函数pipe。
    int pipe(int pipefd[2]);
通常管道只有一个读进程和一个写进程。其中，一个为读管道，一个为写管道。pipefd[0] refers to the read end of the pipe.  pipefd[1] refers to the write end of the pipe. 

管道是一个半双工通信的机制，读数据会阻塞，直到有数据为止。常见的使用方式是，父进程使用pipe创建了管道的fd，fork出的子进程沿用此fd。父子进程在fork共享同一套的pipefd，于是可以通过pipefd进行通信。代码如下
```
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
```
 实验效果如下所示：
```
# ./pipe_test
# hello world
其中，“hello world”为子进程收到的管道消息，并传到标准输出打印的。
```

正常的管道有两个端口，一端读，一端写。向一个读端口库关闭的fd写数据的时候，系统就会产生 SIGPIPE 信号。
```
SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
```
在网络编程，常见情况是，向一个没有connect的fd写数据会产生 SIGPIPE 信号。向一个已经close的服务端fd写数据也会产生
 SIGPIPE 信号。
>  EPIPE  fd  is connected to a pipe or socket whose reading end is closed.  When this happens the writing process will also receive a SIGPIPE signal.  (Thus, the write return value is seen only if
              the program catches, blocks or ignores this signal.)

 本地测试捕获SIGPIPE的代码如下所示：
```
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
```
 本地测试，在关闭读管道后，直接使用write不会触发SIGPIPE。使用stdin输入的结果进行write会触发SIGPIPE。
 实验效果如下所示：
```
# ./pipe_test
parent pid:18805.
write success, ret:12.
child pid:18806.
abcd
SIGPIPE caught
```
#### 信号
通过注册信号来接收某件事件发生。需要添加信号，以及信号的回调函数。
信号能够承载的信息有限。上述管道通信代码，涉及了SIGPIPE的使用方式。
#### 消息队列
进程可以向队列中添加消息，另一个进程则可以读取队列中的消息。消息队列能够承载信息量较大。
消息队列有两种类型。一个来自SYSTEM V 类型。一个是POSIX 类型。
SYSTEM V来自UNIX。
POSIX为IEEE制定的标准。用POSIX使用更简单些，在移植性上会更好一些。本文也将介绍POSIX的IPC方式。

POSIX消息队列相关函数说明：
mq_open：创建消息队列，消息队列靠名字来进行辨别。
>  mq_open()  creates a new POSIX message queue or opens an existing queue.  The queue is identified by name.  For details of the construction of name,
       see mq_overview(7).

mq_send：向消息队列发送内容
> mq_send, mq_timedsend - send a message to a message queue

mq_receive：接收消息队列里的内容。
> mq_receive, mq_timedreceive - receive a message from a message queue

mq_close：关闭消息队列
> mq_close - close a message queue descriptor

mq_unlink：从系统中删除该消息队列
> mq_unlink()  removes the specified message queue name.  The message queue name is removed immediately.  The queue itself is destroyed once any other
       processes that have the queue open close their descriptors referring to the queue.

消息队列可以理解为一个消息的链表，具有写权限的进程在链表中添加节点，具有读权限的进程从节点中读取节点数据。

使用未命名管道只能实现在父子进程间通信，而使用命名管道则可以实现在进程间通信。
两个写进程，一个读进程的实验效果如下所示：
```
服务端：
$ ./mq_server
mq_flags:0
mq_maxmsg:10
mq_msgsize:8192
mq_curmsgs:0
Read 8192 bytes; priority:1, recvline:send from client1
Read 8192 bytes; priority:1, recvline:send from client2
Read 8192 bytes; priority:2, recvline:send from client1
```
```
客户端1：
$ ./mq_client
mq_flags:0
mq_maxmsg:10
mq_msgsize:8192
mq_curmsgs:0
send from client1
fgets success, sendline=send from client1
mq_send success.
send from client1
fgets success, sendline=send from client1
mq_send success.
客户端2：
$ ./mq_client
mq_flags:0
mq_maxmsg:10
mq_msgsize:8192
mq_curmsgs:0
send from client2
fgets success, sendline=send from client2
mq_send success.
```
需要注意的是，在服务端的代码中，有删除消息队列的操作：
```
    mq_unlink("/queue.server");
```
因此，需要先启动服务端，再启动客户端，不然客户端会找不到消息队列。

将“mq_unlink("/queue.server");”删除消息队列的代码移动到客户端。测试有多个读消息队列进程，一个写消息队列进程。
读消息队列会依次读取消息。即第一个读消息进程读取第1个、第3个消息；第二个读消息进程读取第2、4个消息。效果如下：
```
客户端：
$ ./mq_client
mq_flags:0
mq_maxmsg:10
mq_msgsize:8192
mq_curmsgs:0
send from client
fgets success, sendline=send from client
mq_send success.
send from client
fgets success, sendline=send from client
mq_send success.
send from client
fgets success, sendline=send from client
mq_send success.
服务端1：
$ ./mq_server
mq_flags:0
mq_maxmsg:10
mq_msgsize:8192
mq_curmsgs:0
Read 8192 bytes; priority:1, recvline:send from client
Read 8192 bytes; priority:3, recvline:send from client
服务端2：
./mq_server
mq_flags:0
mq_maxmsg:10
mq_msgsize:8192
mq_curmsgs:0
Read 8192 bytes; priority:2, recvline:send from client
```
#### 共享内存
与消息队列一样的，共享内存、信号量也有SYSTEM V和POSIX两种类型。
Linux共享内存的实现依赖于共享内存文件系统,该文件系统通常装载在/dev/shm,在调用shm_open系统函数的时候,会在/dev/shm/目录下生成mymem文件。多进程共享同一块内存，需要配合信号量等来实现进程的同步、互斥操作。共享内存更容易造成内存改写，较少看到。不太可控，且适用于大量传输传递的场景。

共享内存使用的API接口：
> shm_open, shm_unlink - 开启/关闭 共享内存的fd create/open or unlink POSIX shared memory objects

> mmap将fd映射为一段实在的内存空间。
       mmap()  creates  a  new mapping in the virtual address space of the calling process.  The starting address for the new mapping is specified in addr.
       The length argument specifies the length of the mapping.

> fstat 获取共享内存对象信息 fstat() is identical to stat(), except that the file about which information is to be retrieved is specified by the file descriptor fd.
共享内存的写入方式，使用memcpy后者strncpy等方式写都可以。
共享内存的读取方式，使用write从mmap映射的地址读取。

使用效果如下所示：
```
写内存进程：
$ ./mem_server
Resized to 128 bytes
send buf from server
copying 21 bytes

读内存进程：
 ./mem_client
size=128, mode=664
the read msg is:send buf from server
```
创建了共享内存后，会在/dev/shm创建一个共享内存的文件。文件大小就是 ftruncate 设置的大小。
```
ll /dev/shm/
total 4
drwxrwxrwt  2 root root   60 11月 15 00:28 ./
drwxr-xr-x 19 root root 3900 11月  3 23:38 ../
-rw-rw-r--  1 xx   xx128 11月 15 00:28 shmem.test
```
需要注意，读进程必须在写进程后运行。因为内存映射、fd等是在服务端设置的，不然会提示找不到对应内存块
```
./mem_client
shm_open: No such file or directory
fstat failed: Bad file descriptor
```
#### 信号量
不以传输数据为目的，主要是为了保护共享资源。较少使用，通常都用锁实现。
#### 套接字
网络通信、进程间通信都很常用的通信机制，在rgos系统中很常见。11.x的代码中，不同业务之间的通信，通过加载库，调用对方提供的socket通信接口进行传输消息。
对于多客户端的情况，常使用select进行IO复用，避免等待数据的阻塞。可以参考UNP中的select代码：

客户端的fd，可以获取来自stdin的数据，也可以接收来自网络对端的数据。
select的使用。
> select()机制中提供一fd_set的数据结构，实际上是一long类型的数组，每一个数组元素都能与一打开的文件句柄（不管是socket句柄，还是其他文件或命名管道或设备句柄）建立联系，建立联系的工作由程序员完成，当调用select()时，由内核根据IO状态修改fd_set的内容，由此来通知执行了select()的进程哪一socket或文件发生了可读或可写事件。     -摘自百度百科

fd_set的使用过程如下所示：
1. 原本的 fd_set 是全空的，都为0.从第0位到第1023位。
其中，0 是一个文件描述符，表示标准输入(stdin)1 是一个文件描述符，表示标准输出(stdout)2 是一个文件描述符，表示标准错误(stderr)
![fd_set_原始.jpg](https://upload-images.jianshu.io/upload_images/124641-71dc7c7626f6aa3b.jpg?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
2. 设置了fd_set中两位，一个 stdin，在其中占了一个坑。一个sockfd，占了另一个坑。
![fd_set_占坑.jpg](https://upload-images.jianshu.io/upload_images/124641-478dc2484c5daa9a.jpg?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
3. stdin有数据写入，这个坑的显示就变了，就变为1了。通过 FD_ISSET 进行判断，就知道fd_set中哪个fd有输入数据了。再进行相应的操作。
![fd_set有数据写入.jpg](https://upload-images.jianshu.io/upload_images/124641-d96b81b53f82c0d0.jpg?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
4. 接收数据也是一样道理，select返回不为0，说明 fd_set 发生了改变，通过 FD_ISSET 提取出变化的 fd，即可得到该连接的 fd。再进行相应的读或者写操作

实验效果如下所示：
```
服务端：
# ./tcpservselect01
socket success, listenfd = 3
bind success, ret = 0
listen success, ret = 0
new client: 4, port 49968
client[0]=4
buf=client1 send buf
new client: 5, port 49970
client[1]=5
buf=client2 send buf
```
```
客户端1：
./tcpcli01 127.0.0.1
socket success, sockfd = 3
inet_pton success, servaddr.sin_addr=127.0.0.1
connect success, ret = 0
client1 send buf
fgets success, sendline=client1 send buf

客户端2：
# ./tcpcli01 127.0.0.1
socket success, sockfd = 3
inet_pton success, servaddr.sin_addr=127.0.0.1
connect success, ret = 0
client2 send buf
fgets success, sendline=client2 send buf
```
select函数，能够实现监听多个fd，但只使用select无法监听连接断开的情况。
需要配置read，当read的长度为0的时候，即对端断开。这是因为正常断开连接会发生一个空的数据报文到对端。如果是异常断电引起的连接断开则不会发送这个空数据报文。监听对端断开的代码如下所示：
```
    if ((n = read(sockfd, recvline, MAXLINE)) == 0) {
        printf("str_cli: server terminated prematurely\n");
        sockfd = -1;
        break;
    } else {
        printf("read success, sockfd=%d recvline=%s\n", sockfd, recvline);
        fputs(recvline, stdout);
    }
```
连接和断开的效果如下所示：
```
服务端显示效果：
# ./tcpservselect01
socket success, listenfd = 3
bind success, ret = 0
listen success, ret = 0
new client: 4, port 50050
client[0]=4
close success, sockfd=4
new client: 4, port 50052
client[0]=4
close success, sockfd=4
```
##### domain socket与网络socket
domain socket与网络socket的主要连接方式是一致的，主要区别在于连接的类型和使用的地址不一样。
```
    listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
```
类型从AF_INET变更为AF_UNIX
地址类型从sockaddr_in 变更为 sockaddr_un。
```
    struct sockaddr_un cliaddr, servaddr;
```

最后效果
```
服务端：
# ./tcpservselect01
socket success, listenfd = 3
bind success, ret = 0
listen success, ret = 0
new client: 4, path client.socket
client[0]=4
buf=send from client1
buf_ret=msg from server
close success, sockfd=4
^C
```
```
客户端：
 # ./tcpcli01
socket success, sockfd = 3
connect success, ret = 0
send from client1
fgets success, sendline=send from client1
read success, sockfd=3 recvline=msg from server
```
本地查看即可看到两个socket文件
```
# ll
total 104
drwxrwxrwx 2 root     root       4096 11月 11 23:20 ./
drwxrwxrwx 6 root     root       4096 11月 10 23:08 ../
srwxrwxr-x 1 root     root          0 11月 11 23:20 client.socket=
srwxrwxr-x 1 root     root          0 11月 11 23:20 server.socket=
```![Image.png](https://upload-images.jianshu.io/upload_images/124641-06b398416d836226.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
