#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define         my_printf(args...)          printf("%s=%s=%d:", __FILE__, __func__, __LINE__); printf(args);

static int server_port = 8088;

typedef struct IPC_INFO_T
{
        char name[20];
        int port;
        char ipaddr[32];
}IPC_INFO;

int main(int argc, char *argv[])
{
        struct sockaddr_in server_addr, client_addr;
        int socket_fd;
        char recv_buf[1024] = {0};
        char send_buf[] = "server:\ti get the msg\n";
        char ipc_name[20] = {0};
        fd_set readfd;
        int send_len, addrlen = sizeof(struct sockaddr);
        struct IPC_INFO_T ipc_info;

        bzero(&server_addr, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
                my_printf("error at socket\n");
                return -1;
        }
        
        if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0)
        {
                my_printf("error at bind\n");
                return -1;
        }

        while(1)
        {
                FD_ZERO(&readfd);
                FD_SET(socket_fd, &readfd); 
                select(socket_fd+1, &readfd, NULL, NULL, NULL);              
                if(FD_ISSET(socket_fd, &readfd))
                {
                        recvfrom(socket_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&client_addr, &addrlen);
                        if (!strncmp("where is", recv_buf, strlen("where is")))          /* pc request */
                        {
                                sscanf(recv_buf, "where is %s", ipc_name);
                                if (!strcmp(ipc_name, ipc_info.name))
                                {
                                        /* format 20+32+8 */
                                        memcpy(send_buf, ipc_info.name, 20);
                                        memcpy(send_buf+20, ipc_info.ipaddr, 32);
                                        memcpy(send_buf+52, &ipc_info.port, 4);
                                        sendto(socket_fd, send_buf, 60, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in));
                                }
                        }
                        else
                        {
                                sscanf(recv_buf, "my name is %s", ipc_info.name);                        /* ipc request */
                                strcpy(ipc_info.ipaddr, inet_ntoa(client_addr.sin_addr));  
                                ipc_info.port = ntohs(client_addr.sin_port);
                                printf("ipc name:%s, ip:%s, port:%d\n", ipc_info.name, ipc_info.ipaddr, ipc_info.port);         /* server response */
                        }
                }
                
                usleep(10*1000);
        }

        return 0;
}
