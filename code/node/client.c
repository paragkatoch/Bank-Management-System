#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>

#include "../include/config.h"        // for MSG_DELIM
#include "../include/communication.h" // for send_message(), receive_message()

#define PORT 8080
#define IP "127.0.0.1"

void client_run(int sock_fd)
{
    serverfd = sock_fd;
    struct pollfd fds[2];
    fds[0].fd = serverfd; // server socket
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO; // keyboard input
    fds[1].events = POLLIN;

    char *recv_buf = NULL;
    char send_buf[1024];

    printf("🟢 Connected to server! Type messages (or 'exit' to quit)\n");

    while (1)
    {
        int ret = poll(fds, 2, -1); // wait for input
        if (ret < 0)
        {
            perror("poll");
            break;
        }

        // Message from Server
        if (fds[0].revents & (POLLIN | POLLHUP | POLLERR))
        {
            // if server closed the connection
            if (fds[0].revents & (POLLHUP | POLLERR))
            {
                printf("❌ Server closed connection.\n");
                break;
            }

            if (receive_message(sock_fd, &recv_buf) < 0)
            {
                printf("❌ Server disconnected.\n");
                break;
            }
            printf("%s\n", recv_buf);
            free(recv_buf);
            recv_buf = NULL;
        }

        // Input from Terminal
        if (fds[1].revents & POLLIN)
        {
            if (fgets(send_buf, sizeof(send_buf), stdin) == NULL)
                break; // EOF or Ctrl+D

            send_buf[strcspn(send_buf, "\n")] = '\0'; // remove newline

            if (strcmp(send_buf, "exit") == 0)
                break;

            if (send_message(sock_fd, send_buf) < 0)
            {
                printf("Failed to send message.\n");
                break;
            }
        }
    }

    printf("Disconnected.\n");
    close(sock_fd);
}

int main(void)
{
    int sock_fd;
    struct sockaddr_in server_addr;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock_fd);
        exit(1);
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        exit(1);
    }

    printf("✅ Connected to server %s:%d\n", IP, PORT);
    client_run(sock_fd);

    return 0;
}
