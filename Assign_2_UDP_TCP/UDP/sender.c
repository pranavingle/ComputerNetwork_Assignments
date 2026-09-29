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
    char message[BUFFER_SIZE];

    struct sockaddr_in receiver_addr;

    // 1. Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // 2. Configure receiver address
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(PORT);
    receiver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("UDP Sender\n");
    printf("----------------------\n");

    // 3. Take input from user
    printf("Enter message: ");
    fgets(message, BUFFER_SIZE, stdin);

    // Remove newline
    message[strcspn(message, "\n")] = '\0';

    // 4. Send message
    int n = sendto(sockfd,
                   message,
                   strlen(message),
                   0,
                   (struct sockaddr *)&receiver_addr,
                   sizeof(receiver_addr));

    if (n < 0)
    {
        perror("Send failed");
        close(sockfd);
        exit(1);
    }

    printf("Message sent successfully.\n");

    // 5. Close socket
    close(sockfd);

    return 0;
}