#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
    int i_fd = 0;
    struct sockaddr_in addr;
    char psz_serverpath[32] = "./server_unixsocket_file";
    int i_addr_len = sizeof(struct sockaddr_in);
    char psz_wbuf[32];
    char psz_rbuf[32] = {0};
    int i_readlen = 0;

    if ( ( i_fd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror("socket");
        return -1;
    }

    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(50000);

    if ( bind( i_fd, ( struct sockaddr * )&addr, i_addr_len ) < 0 )
    {
        perror("bind");
        return -1;
    }


    //fill socket adress structure with server's address
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = (inet_addr(argv[1]));
    addr.sin_port = htons(40000);

    while(1)
    {
        scanf("%s",psz_wbuf);
        if ( sendto( i_fd, psz_wbuf, strlen( psz_wbuf ) + 1, 0,
            ( struct sockaddr * )&addr, i_addr_len ) < 0 )
        {
            perror( "write" );
            return -1;
        }

        if ( ( i_readlen = recvfrom( i_fd, psz_rbuf, sizeof( psz_rbuf ) - 1, 0,
            ( struct sockaddr * )&addr, ( socklen_t * )&i_addr_len ) ) < 0 )
        {
            perror("write");
            return -1;
        }
        psz_rbuf[i_readlen] = '\0';
        printf( "receive msg:%s\n", psz_rbuf );
    }
    return -1;
}
