#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc,char *argv[])
{
    int i_listenfd[2] = {0,0}/*, i_clientfd = 0*/;
    struct sockaddr_in servaddr;
    struct sockaddr_un addr_server;
    struct sockaddr_un  addr_client1;
    struct sockaddr_in  addr_client2;

    char psz_path[32] = "./server_unixsocket_file";
    int i_caddr_len = sizeof(struct sockaddr_un);
    int i_saddr_len = 0;
    int i_caddr_len2 = 0;
    char psz_rbuf[32] = {0};
    char psz_wbuf[32] = "i am server.";
    int i_readlen = 0;

    /*define input rule*/
    if(argc < 4 || argv[0] == "?")
    {
	printf("Usage: ./server <domain_addr> <udp_ip> <udp_port> \n");
	exit(0);
    }

    //create a UNIX domain stream socket
    if ( ( i_listenfd[0] = socket( AF_UNIX, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror( "local socket" );
        return -1;
    }

    //create a IPv4 socket
    if ( ( i_listenfd[1] = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror( "ip socket" );
        return -1;
    }
    //in case it already exists
    unlink( psz_path );

    //fill in IPv4 socket address structure
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(40000);

    addr_client2.sin_family = AF_INET;
    addr_client2.sin_addr.s_addr = inet_addr(argv[2]);
    sscanf(argv[3],"%hu" ,&addr_client2.sin_port);
    addr_client2.sin_port = htons(addr_client2.sin_port);

    //fill in domain socket address structure
    memset( &addr_server, 0, sizeof( addr_server ) );
    addr_server.sun_family = AF_UNIX;
    strncpy( addr_server.sun_path, psz_path, sizeof( addr_server.sun_path ) - 1 );

    memset( &addr_client1, 0, sizeof( addr_client1 ) );
    addr_client1.sun_family = AF_UNIX;
    sscanf(argv[1],"%s",addr_client1.sun_path);

    //bind the name to the descriptor
    i_saddr_len = strlen( addr_server.sun_path ) + sizeof( addr_server.sun_family );

    i_caddr_len2 = sizeof(servaddr);
    if ( bind( i_listenfd[0], ( struct sockaddr * )&addr_server, i_saddr_len ) < 0 )
    {
        perror( "bind" );
        return -1;
    }

    if ( bind( i_listenfd[1], ( struct sockaddr * )&servaddr, i_caddr_len2 ) < 0 )
    {
        perror( "bind" );
        return -1;
    }

    int epfd = epoll_create(100);
    struct epoll_event ev[2];
    ev[0].events = EPOLLIN;
    ev[0].data.fd = i_listenfd[0];
    epoll_ctl(epfd,EPOLL_CTL_ADD,i_listenfd[0],&ev[0]);
    ev[1].events = EPOLLIN;
    ev[1].data.fd = i_listenfd[1];
    epoll_ctl(epfd,EPOLL_CTL_ADD,i_listenfd[1],&ev[1]);

    int ret;
    struct epoll_event events[100];
    int guestnum = 0;
    int i;
    while(1)
    {
        ret = epoll_wait(epfd,events,100,-1);
        for(i = 0;i < ret;i ++)
        {
            if((events[i].data.fd == i_listenfd[0]))
            {
                i_readlen = recvfrom( i_listenfd[0], psz_rbuf, sizeof( psz_rbuf ) - 1, 0,
                  ( struct sockaddr * )&addr_client1, ( socklen_t *)&i_caddr_len );
                if ( i_readlen < 0 )
                {
                    perror( "read" );
                    return -1;
                }
                psz_rbuf[i_readlen] = '\0';
                printf( "receive msg:%s\n", psz_rbuf );
                if ( sendto( i_listenfd[1], psz_rbuf, strlen( psz_rbuf ) + 1, 0,
                    ( struct sockaddr * )&addr_client2, i_caddr_len2 ) < 0 )
                    {
                        perror( "write" );
                        return -1;
                    }
		printf("OK,the msg is send to udp client!\n");
            }
            else
            {
                i_readlen = recvfrom( i_listenfd[1], psz_rbuf, sizeof( psz_rbuf ) - 1, 0,
                  ( struct sockaddr * )&addr_client2, ( socklen_t *)&i_caddr_len2 );
                if ( i_readlen < 0 )
                {
                    perror( "read" );
                    return -1;
                }
                psz_rbuf[i_readlen] = '\0';
                printf( "receive msg:%s\n", psz_rbuf );
                if ( sendto( i_listenfd[0], psz_rbuf, strlen( psz_rbuf ) + 1, 0,
                    ( struct sockaddr * )&addr_client1, i_caddr_len ) < 0 )
                    {
                        perror( "write" );
                        return -1;
                    }
		printf("OK, msg is send to domain client!\n");
            }
        }
    }
    unlink( psz_path );
    return 0;
}
