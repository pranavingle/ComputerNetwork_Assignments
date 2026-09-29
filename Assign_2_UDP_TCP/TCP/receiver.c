#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    printf("TCP Receiver\n");
    printf("----------------------\n");

    // 2. Configure server address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // 3. Bind socket to port
    if (bind(server_fd,
             (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    // 4. Listen for connection
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Listening on TCP port %d...\n", PORT);

    // 5. Accept sender connection
    client_fd = accept(server_fd,
                       (struct sockaddr *)&client,
                       &client_len);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(1);
    }

    printf("Sender connected!\n");

    // 6. Receive message
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_received = recv(client_fd,
                              buffer,
                              BUFFER_SIZE - 1,
                              0);

    if (bytes_received < 0)
    {
        perror("Receive failed");
    }
    else
    {
        buffer[bytes_received] = '\0';

        printf("Received from Sender: %s\n", buffer);

        // 7. Send acknowledgement
        char ack[] = "Message received successfully";

        send(client_fd,
             ack,
             strlen(ack),
             0);

        printf("Acknowledgement sent.\n");
    }

    // 8. Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}