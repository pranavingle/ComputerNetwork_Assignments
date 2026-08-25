#include "stopwait_common.h"

int main()
{
    int sock;
    struct sockaddr_in server;
    socklen_t serverLen = sizeof(server);

    char buffer[BUFFER_SIZE];

    double Tt, Tp;
    int packets;

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Receiver address
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0)
    {
        perror("Invalid address");
        close(sock);
        exit(1);
    }

    printf("=====================================\n");
    printf("        SIMPLE STOP-AND-WAIT\n");
    printf("             SENDER\n");
    printf("=====================================\n");

    // Input
    printf("Enter Transmission Time (Tt) in ms: ");
    scanf("%lf", &Tt);

    printf("Enter Propagation Time (Tp) in ms: ");
    scanf("%lf", &Tp);

    printf("Enter Number of Packets: ");
    scanf("%d", &packets);

    // Formula
    double timePerPacket = Tt + (2 * Tp);

    double totalTime = packets * timePerPacket;

    double efficiency = Tt / timePerPacket;

    printf("\n=====================================\n");
    printf("             PARAMETERS\n");
    printf("=====================================\n");

    printf("Tt = %.2f ms\n", Tt);
    printf("Tp = %.2f ms\n", Tp);
    printf("Packets = %d\n", packets);

    printf("\nTime for one packet:\n");
    printf("Tt + 2Tp = %.2f + 2(%.2f)\n",
           Tt, Tp);

    printf("          = %.2f ms\n", timePerPacket);

    /*
     * Timeline table
     */
    printf("\n=====================================\n");
    printf("             TIMELINE\n");
    printf("=====================================\n");

    printf("\n");
    printf("%-8s %-12s %-12s %-15s %-15s\n",
           "Packet",
           "Start",
           "Tx End",
           "Receiver",
           "ACK");

    printf("----------------------------------------------------------------\n");

    double currentTime = 0.0;

    for (int i = 1; i <= packets; i++)
    {
        /*
         * Packet transmission:
         *
         * Start -> Start + Tt
         */
        double startTime = currentTime;

        double txEnd = startTime + Tt;

        /*
         * Packet reaches receiver:
         *
         * Tx End + Tp
         */
        double receiverTime = txEnd + Tp;

        /*
         * ACK reaches sender:
         *
         * Receiver Time + Tp
         */
        double ackTime = receiverTime + Tp;

        printf("%-8d %-12.2f %-12.2f %-15.2f %-15.2f\n",
               i,
               startTime,
               txEnd,
               receiverTime,
               ackTime);

        /*
         * Simulate transmission time
         */
        usleep((useconds_t)(Tt * 1000));

        // Send packet number
        sprintf(buffer, "%d", i);

        printf("\nSending Packet %d...\n", i);

        if (sendto(
                sock,
                buffer,
                strlen(buffer),
                0,
                (struct sockaddr *)&server,
                serverLen) < 0)
        {
            perror("Send failed");
            close(sock);
            exit(1);
        }

        /*
         * Wait for ACK
         */
        memset(buffer, 0, BUFFER_SIZE);

        int bytes = recvfrom(
            sock,
            buffer,
            BUFFER_SIZE - 1,
            0,
            (struct sockaddr *)&server,
            &serverLen
        );

        if (bytes < 0)
        {
            perror("ACK receive failed");
            close(sock);
            exit(1);
        }

        buffer[bytes] = '\0';

        printf("ACK %s received\n", buffer);

        /*
         * Simulate ACK propagation:
         *
         * Receiver -> Sender = Tp
         */
        usleep((useconds_t)(Tp * 1000));

        printf("Packet %d completed at %.2f ms\n",
               i, ackTime);

        printf("-------------------------------------\n");

        /*
         * Next packet can only start
         * after ACK of current packet.
         */
        currentTime = ackTime;
    }

    // Tell receiver to stop
    strcpy(buffer, "END");

    sendto(
        sock,
        buffer,
        strlen(buffer),
        0,
        (struct sockaddr *)&server,
        serverLen
    );

    printf("\n=====================================\n");
    printf("          FINAL RESULT\n");
    printf("=====================================\n");

    printf("Time per Packet = %.2f ms\n", timePerPacket);

    printf("Total Time = N(Tt + 2Tp)\n");
    printf("           = %d(%.2f)\n",
           packets, timePerPacket);

    printf("           = %.2f ms\n", totalTime);

    printf("\nEfficiency = Tt / (Tt + 2Tp)\n");
    printf("           = %.2f / %.2f\n",
           Tt, timePerPacket);

    printf("           = %.2f%%\n",
           efficiency * 100);

    printf("=====================================\n");

    close(sock);

    return 0;
}
