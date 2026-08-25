#include "stopwait_common.h"

int main()
{
    int sock;
    struct sockaddr_in server, client;
    socklen_t clientLen = sizeof(client);

    char buffer[BUFFER_SIZE];

    double Tp;

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Server address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind socket
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(sock);
        exit(1);
    }

    printf("=====================================\n");
    printf("       STOP-AND-WAIT RECEIVER\n");
    printf("=====================================\n");

    printf("Enter Propagation Time (Tp) in ms: ");
    scanf("%lf", &Tp);

    printf("\nReceiver is waiting for packets...\n\n");

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        int bytes = recvfrom(
            sock,
            buffer,
            BUFFER_SIZE - 1,
            0,
            (struct sockaddr *)&client,
            &clientLen
        );

        if (bytes < 0)
        {
            perror("Receive failed");
            break;
        }

        buffer[bytes] = '\0';

        // Check for termination message
        if (strcmp(buffer, "END") == 0)
        {
            printf("All packets received.\n");
            break;
        }

        int packetNo = atoi(buffer);

        printf("Received Packet %d\n", packetNo);

        /*
         * Simulate propagation delay:
         * Sender -> Receiver = Tp
         */
        usleep((useconds_t)(Tp * 1000));

        printf("Packet %d reached receiver after %.2f ms\n",
               packetNo, Tp);

        // Create ACK
        char ack[BUFFER_SIZE];

        sprintf(ack, "%d", packetNo);

        // Send ACK
        sendto(
            sock,
            ack,
            strlen(ack),
            0,
            (struct sockaddr *)&client,
            clientLen
        );

        printf("ACK %d sent\n\n", packetNo);
    }

    close(sock);

    return 0;
}
