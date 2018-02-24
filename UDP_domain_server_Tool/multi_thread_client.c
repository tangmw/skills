//����һ���߳̽������ݣ����̷߳������ݵĴ���
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

//�����̣߳�������Ϣ������ʾ
void *recv_thread(void* arg)
{
   int udp_fd = (int)arg;
   struct sockaddr_in addr;
   addr.sin_family = AF_INET;
   addr.sin_port = htons(8000);
   addr.sin_addr.s_addr = htonl(INADDR_ANY);

   socklen_t addrlen = sizeof(addr);
   bzero(&addr, sizeof(addr));
   while (1)
   {
       char buf[256] = "";
       char ipbuf[256] = "";
       recvfrom(udp_fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addrlen);
       printf("\r\033[0:31m[%s]:\033[31m%s\n", inet_ntop(AF_INET, &addr.sin_addr, ipbuf, sizeof(ipbuf)), buf);
       write(1, "chenyu:", 7);
   }
   return NULL;
}


int main() 
{
   char buf[100] = "";
   int udp_fd = 0;
   pthread_t tid;
   struct sockaddr_in addr;
   struct sockaddr_in cliaddr;
    
   //���׽��ֳ�ʼ��
   bzero(&addr, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_port = htons(8000);
   addr.sin_addr.s_addr = htonl(INADDR_ANY);

   bzero(&cliaddr, sizeof(cliaddr));
   cliaddr.sin_family = AF_INET;
   cliaddr.sin_port = htons(8000);
     
   //�����׽ӿ�
   if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
   {
      perror("socket create failed\n");
      exit(EXIT_FAILURE);
   }   
   puts("socket create success");
   //���ö˿�
   if (bind(udp_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
   {
       perror("bind fail\n");
       close(udp_fd);
       exit(EXIT_SUCCESS);
   }
   puts("bind success");
   puts("input:  \"192.168.221.x\" to send msg to body");
//  printf("\033[39m"); //����������
   //�����߳�
   tid = pthread_create(&tid, NULL, recv_thread, (void *)udp_fd);   
   fflush(stdout);
   puts("create pthread success");  
   while (1)
   {
       puts("hello0");
       //���̸߳�����Ϣ
       write(1, "chenyu send", 11); //1��ʾ��־��
       fgets(buf, sizeof(buf), stdin);//�ȴ�����
       puts("hello");
//       buf[strlen(buf) - 1] = '\0';
       if (strncmp(buf, "chen", 4) == 0) 
       {
          puts("go in strncpm"); 
          char ipbuf[INET_ADDRSTRLEN] = "";
          inet_pton(AF_INET, buf + 6, &cliaddr.sin_addr); //���׽��ֵ�ַ�ٸ�ֵ
 	  puts("inet_pton go in");
          printf("connect %s successful!\n", inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuf, sizeof(ipbuf)));       continue;       
       }  
       else if (strncmp(buf, "exit", 4) == 0)
       {
   	  close(udp_fd);
 	  exit(0);
       }
       sendto(udp_fd, buf ,strlen(buf), 0 , (struct sockaddr*)&cliaddr, sizeof(cliaddr));
   }
   return 0;
}