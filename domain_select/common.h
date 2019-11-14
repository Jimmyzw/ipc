#define	SA	struct sockaddr
#define err_sys printf
#define	INET_ADDRSTRLEN		16      /* "ddd.ddd.ddd.ddd\0" */
#define	LISTENQ		1024    /* 2nd argument to listen() */
#define	MAXLINE		4096    /* max text line length */
#define	SERV_PORT		 9877   /* TCP and UDP client-servers */
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define NOTDEF 1

#define SERVER_PATH "server.socket"
#define CLINET_PATH "client.socket"