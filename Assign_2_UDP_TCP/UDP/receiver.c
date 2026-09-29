#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len = sizeof(client_addr);

    // 1. Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // 2. Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 3. Bind socket to port
    if (bind(sockfd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    printf("UDP Receiver\n");
    printf("----------------------\n");
    printf("Listening on port %d...\n", PORT);

    // 4. Receive message
    int n = recvfrom(sockfd,
                     buffer,
                     BUFFER_SIZE - 1,
                     0,
                     (struct sockaddr *)&client_addr,
                     &client_len);

    if (n < 0)
    {
        perror("Receive failed");
        close(sockfd);
        exit(1);
    }

    buffer[n] = '\0';

    // 5. Display received message
    printf("Received message: %s\n", buffer);

    // 6. Close socket
    close(sockfd);

    return 0;
}