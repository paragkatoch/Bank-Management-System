#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "config.h"
#include "db/user.h"
#include "init.h"
#include "communication.h"

#define PORT 8080
#define BUF_SIZE 1024

void handle_client(int cl_fd, char *client_address)
{
    clientfd = cl_fd;
    user_login();

    send_message(cl_fd, "\nExiting...");
    sleep(2);

    close(cl_fd);
    printf("Child %s connection closed.\n", client_address);
    exit(0);
}

int main()
{
    init();

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);

    // Ignore zombie children
    signal(SIGCHLD, SIG_IGN);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Concurrent server (fork) listening on port %d...\n", PORT);

    while (1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        char client_address[100]; // or bigger if you want

        snprintf(client_address, sizeof(client_address), "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        printf("New client connected: %s\n", client_address);

        pid_t pid = fork();
        if (pid == 0)
        {
            close(server_fd); // Child does not need listening socket
            handle_client(client_fd, client_address);
        }
        else if (pid > 0)
        {
            close(client_fd); // Parent closes connected socket
        }
        else
        {
            perror("fork");
            close(client_fd);
        }
    }
    return 0;
}
