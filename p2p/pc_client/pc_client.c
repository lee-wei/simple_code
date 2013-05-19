#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define         my_printf(args...)          printf("%s=%s=%d:", __FILE__, __func__, __LINE__); printf(args);
#define         IPC_NAME                "jason"

static char server_ip[20] = "127.0.0.1";
static int server_port = 8088;

typedef struct IPC_INFO_T
{
        char name[20];
        int port;
        char ipaddr[32];
}IPC_INFO;

int main(int argc, char *argv[])
{
        struct IPC_INFO_T ipc_info;
        struct sockaddr_in server_addr, ipc_addr;
        char send_buf[1024] = {0};
        char recv_buf[1024] = {0};
        int socket_fd, addrlen = sizeof(struct sockaddr_in);

        strcpy(ipc_info.name, IPC_NAME);

         if (argc == 3)
        {
                strncmp(server_ip, argv[1], strlen(argv[1]));
                server_port = atoi(argv[2]);
        }

        bzero(&server_addr, sizeof(struct sockaddr_in));
        bzero(&ipc_addr, sizeof(struct sockaddr_in));        
        server_addr.sin_port = htons(server_port);
        server_addr.sin_addr.s_addr = inet_addr(server_ip);
        server_addr.sin_family = AF_INET;
        ipc_addr.sin_family = AF_INET;     

        if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
                my_printf("error at socket\n");
                return -1;
        }

        /* send msg to server get ipc info */
        sprintf(send_buf, "where is %s", ipc_info.name);
        sendto(socket_fd, send_buf, strlen(send_buf)+1, MSG_NOSIGNAL, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
        recvfrom(socket_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&server_addr, &addrlen);
        memcpy(ipc_info.name, recv_buf, 20);
        memcpy(ipc_info.ipaddr, recv_buf+20, 32);
        memcpy(&ipc_info.port, recv_buf+52, 4);
        printf("ipc name:%s, ip:%s, port:%d:%d:%d\n", ipc_info.name, ipc_info.ipaddr, ipc_info.port, htons(ipc_info.port), &ipc_info.port);
        
        ipc_addr.sin_addr.s_addr = inet_addr(ipc_info.ipaddr);
        ipc_addr.sin_port = htons(ipc_info.port);

        /* send msg to ipc */
        sprintf(send_buf, "hello ipcam!");
        while(1)
        {
                sendto(socket_fd, send_buf, strlen(send_buf)+1, MSG_NOSIGNAL, (struct sockaddr *)&ipc_addr, sizeof(struct sockaddr));
                recvfrom(socket_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&ipc_addr, &addrlen);
                printf("ipcam msg:\t%s\n", recv_buf);
        }

        return 0;
}

