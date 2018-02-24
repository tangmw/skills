#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/select.h>

typedef struct{
int i_fd ;
char psz_clientpath[100];
struct sockaddr_un addr_server;
}args;

//发送线程，负责消息发送
void *send_thread(args *s_args)
{	char psz_clientpath[100];
	struct sockaddr_un addr_server;
	int i_fd;

	i_fd = s_args->i_fd;
	memcpy(&psz_clientpath, s_args->psz_clientpath, strlen(s_args->psz_clientpath));
	addr_server = s_args->addr_server;

    struct sockaddr_un addr;
    int i_addr_len = 0;
    memset( &addr, 0, sizeof( addr ) );
    addr.sun_family = AF_UNIX;

    printf("create pthread success\n");

    strncpy( addr.sun_path, psz_clientpath, sizeof( addr.sun_path ) - 1 );
    i_addr_len = strlen( addr.sun_path ) + sizeof( addr.sun_family );

    while (1)
    {
        char psz_wbuf[32];
        scanf("%s",psz_wbuf);
        if ( sendto( i_fd, psz_wbuf, strlen( psz_wbuf ) + 1, 0,
            ( struct sockaddr * )&addr_server, i_addr_len ) < 0 )
        {
            perror( "pthread write" );
            return 0;
        }
    }
    return NULL;
}

int main(int argc,char *argv[])
{
    int i_fd = 0;
    struct sockaddr_un addr;
    char psz_clientpath[32];
    strcpy(psz_clientpath,argv[1]);
    char psz_serverpath[32] = "./server_unixsocket_file";
    int i_addr_len = 0;
    char psz_wbuf[32];
    char psz_rbuf[32] = {0};
    int i_readlen = 0;
    int retu;
    pthread_t tid;

    if ( ( i_fd = socket( AF_UNIX, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror("socket");
        return -1;
    }

    memset( &addr, 0, sizeof( addr ) );
    addr.sun_family = AF_UNIX;
    strncpy( addr.sun_path, psz_clientpath, sizeof( addr.sun_path ) - 1 );
    unlink( psz_clientpath );
    i_addr_len = strlen( addr.sun_path ) + sizeof( addr.sun_family );
    if ( bind( i_fd, ( struct sockaddr * )&addr, i_addr_len ) < 0 )
    {
        perror("bind");
        return -1;
    }


    //fill socket adress structure with server's address
    memset( &addr, 0, sizeof( addr ) );
    addr.sun_family = AF_UNIX;
    strncpy( addr.sun_path, psz_serverpath, sizeof( addr.sun_path ) - 1 );

    i_addr_len = strlen( addr.sun_path ) + sizeof( addr.sun_family );

    //创建线程
	args s_args;

	s_args.i_fd = i_fd ;
	memcpy(s_args.psz_clientpath,psz_clientpath,  strlen(psz_clientpath));
	s_args.addr_server = addr;

	retu = pthread_create(&tid, NULL, (void *)send_thread, (void *)&s_args);
    printf("retu = %d\n",retu);
    puts("create pthread success");
    while(1)
    {
        //主线程负责收消息
//        scanf("%s",psz_wbuf);
//        if ( sendto( i_fd, psz_wbuf, strlen( psz_wbuf ) + 1, 0,
//            ( struct sockaddr * )&addr, i_addr_len ) < 0 )
//        {
//            perror( "write" );
//            return -1;
//        }

        if ( ( i_readlen = recvfrom( i_fd, psz_rbuf, sizeof( psz_rbuf ) - 1, 0,
            ( struct sockaddr * )&addr, ( socklen_t * )&i_addr_len ) ) < 0 )
        {
            perror("main thread write");
            return -1;
        }
        psz_rbuf[i_readlen] = '\0';
        printf( "receive msg:%s\n", psz_rbuf );
    }
    unlink( psz_clientpath );
    return -1;
}
