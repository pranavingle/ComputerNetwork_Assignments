#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    printf("TCP Sender\n");
    printf("----------------------\n");

    // 1. Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // 2. Configure receiver address
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Receiver is running on same computer
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 3. Connect to receiver
    if (connect(sock,
                (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("Connection failed");
        close(sock);
        exit(1);
    }

    printf("Connected to Receiver.\n");

    // 4. Take message from user
    printf("Enter message: ");
    fgets(message, BUFFER_SIZE, stdin);

    // Remove newline
    message[strcspn(message, "\n")] = '\0';

    // 5. Send message
    send(sock,
         message,
         strlen(message),
         0);

    printf("Message sent successfully.\n");

    // 6. Receive acknowledgement
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_received = recv(sock,
                              buffer,
                              BUFFER_SIZE - 1,
                              0);

    if (bytes_received < 0)
    {
        perror("Acknowledgement receive failed");
    }
    else
    {
        buffer[bytes_received] = '\0';

        printf("Receiver: %s\n", buffer);
    }

    // 7. Close socket
    close(sock);

    return 0;
}