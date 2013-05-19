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
static int native_port = 8099;
static struct sockaddr_in server_addr, native_addr , client_addr;

void heart_beat(void *arg)
{
        int socked_fd = *(int *)arg;
        char  recv_buf[1024] = {0};
        char  send_buf[1024] = {0};
        int ret;

        sprintf(send_buf, "my name is %s", IPC_NAME);

        while(1)
        {     
                printf("%s\n", send_buf);
                ret = sendto(socked_fd, send_buf, strlen(send_buf)+1, MSG_NOSIGNAL, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                if (ret < 0)
                        printf("end msg error\n");

                sleep(1);
        }
}

void server(void *arg)
{
        int socket_fd = *(int *)arg;
        char  recv_buf[1024] = {0};
        char  send_buf[1024] = {"my name \n"};        
        int ret, addrlen = sizeof(struct sockaddr);
        fd_set readfd;
        
        FD_ZERO(&readfd);
        FD_SET(socket_fd, &readfd);

        if (bind(socket_fd, (struct sockaddr *)&native_addr, sizeof(struct sockaddr_in)) < 0)
        {
                my_printf("error at bind\n");
                exit(-1);
        }

        while(1)
        {

                select(socket_fd+1, &readfd, NULL, NULL, 0);
                if(FD_ISSET(socket_fd, &readfd))
                {
                        recvfrom(socket_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&client_addr, &addrlen);
                        if (!strcmp("hello ipcam!", recv_buf))
                        {
                                sprintf(send_buf, "hello pc!");
                                sendto(socket_fd, send_buf, strlen(send_buf)+1, MSG_NOSIGNAL, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                        }
                }

                sleep(1);
        }
}

int main(int argc, char *argv[])
{
        int socket_fd;
//        struct sockaddr_in server_addr, native_addr;
        pthread_t heartbeat_id, server_id;
        
        if (argc == 3)
        {
                strncmp(server_ip, argv[1], strlen(argv[1]));
                server_port = atoi(argv[2]);
        }

        bzero(&server_addr, sizeof(struct sockaddr_in));
        bzero(&native_addr, sizeof(struct sockaddr_in));
        bzero(&client_addr, sizeof(struct sockaddr_in));        

        server_addr.sin_port = htons(server_port);
        server_addr.sin_addr.s_addr = inet_addr(server_ip);
        server_addr.sin_family = AF_INET;
        native_addr.sin_port = htons(native_port);
        native_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        native_addr.sin_family = AF_INET;

        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd < 0)
        {
                my_printf("error at socket\n");
                return -1;
        }

        pthread_create(&heartbeat_id, NULL, heart_beat, &socket_fd);
        pthread_create(&server_id, NULL, server, &socket_fd);

        pthread_join(heartbeat_id);
        pthread_join(server_id);

        return 0;
}

