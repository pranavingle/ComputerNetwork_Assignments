#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 5003
#define BUF_SIZE 128

typedef struct {
    int seq;
    char data[BUF_SIZE];
} Packet;

typedef struct {
    int ack;
} Ack;

int main(void)
{
    int sock;
    int n, ws, bits;
    int i, base, next_seq;

    double tt, tp, a, tao, total;

    struct sockaddr_in receiver_addr;
    socklen_t addr_len = sizeof(receiver_addr);

    Packet packet;
    Ack ack;

    printf("============================================================\n");
    printf("           SLIDING WINDOW SENDER - NOISELESS\n");
    printf("============================================================\n");

    /* ---------------------------------------------------------
       INPUT
       --------------------------------------------------------- */

    printf("Enter Tt (ms): ");
    scanf("%lf", &tt);

    printf("Enter Tp (ms): ");
    scanf("%lf", &tp);

    printf("Enter Number of Packets: ");
    scanf("%d", &n);

    if (tt <= 0 || tp < 0 || n <= 0)
    {
        printf("Invalid input.\n");
        return 1;
    }

    /* ---------------------------------------------------------
       CALCULATE NUMBER OF SEQUENCE BITS
       --------------------------------------------------------- */

    bits = 0;

    while ((1 << bits) < n)
        bits++;

    if (bits == 0)
        bits = 1;

    int seq_space = 1 << bits;

    /* Maximum window size for Go-Back-N */
    ws = seq_space - 1;

    /*
     * We don't need a window larger than the number
     * of packets to be transmitted.
     */
    if (ws > n)
        ws = n;

    /* ---------------------------------------------------------
       CALCULATIONS
       --------------------------------------------------------- */

    a = tp / tt;

    tao = tt + 2.0 * tp;

    total = n * (tt + 2.0 * tp);

    printf("\n============================================================\n");
    printf("                    CALCULATIONS\n");
    printf("============================================================\n");

    printf("a = Tp/Tt\n");
    printf("a = %.2f / %.2f = %.4f\n", tp, tt, a);

    printf("\n2^m >= N\n");
    printf("2^m >= %d\n", n);
    printf("m = %d bits\n", bits);

    printf("\nSequence Number Space = 2^m\n");
    printf("                     = %d\n", seq_space);

    printf("\nMaximum Window Size = 2^m - 1\n");
    printf("                    = %d\n", seq_space - 1);

    printf("\nEffective Window Size = %d\n", ws);

    printf("\nTAO = Tt + 2Tp\n");
    printf("    = %.2f + 2(%.2f)\n", tt, tp);
    printf("    = %.2f ms\n", tao);

    printf("\nTheoretical Total Time = N(Tt + 2Tp)\n");
    printf("                      = %d(%.2f)\n", n, tao);
    printf("                      = %.2f ms\n", total);

    printf("============================================================\n");

    /* ---------------------------------------------------------
       WINDOW DEMONSTRATION
       --------------------------------------------------------- */

    printf("\nWINDOW MOVEMENT\n");
    printf("------------------------------------------------------------\n");

    base = 0;

    while (base < n)
    {
        printf("Window: [ ");

        for (i = base;
             i < base + ws && i < n;
             i++)
        {
            printf("%d ", i % seq_space);
        }

        printf("]\n");

        base++;
    }

    printf("------------------------------------------------------------\n");

    /* ---------------------------------------------------------
       CREATE UDP SOCKET
       --------------------------------------------------------- */

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    /* Receiver address */
    memset(&receiver_addr, 0, sizeof(receiver_addr));

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(PORT);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &receiver_addr.sin_addr
    );

    /* ---------------------------------------------------------
       SLIDING WINDOW TRANSMISSION
       --------------------------------------------------------- */

    printf("\n============================================================\n");
    printf("                    TRANSMISSION\n");
    printf("============================================================\n");

    base = 0;
    next_seq = 0;

    while (base < n)
    {
        /*
         * -----------------------------------------------------
         * SEND ALL PACKETS THAT FIT IN THE CURRENT WINDOW
         * -----------------------------------------------------
         */

        while (next_seq < base + ws && next_seq < n)
        {
            packet.seq = next_seq % seq_space;

            snprintf(
                packet.data,
                sizeof(packet.data),
                "SW-PACKET-%d",
                next_seq
            );

            printf("Sending Packet %d (Seq=%d)\n",
                   next_seq,
                   packet.seq);

            sendto(
                sock,
                &packet,
                sizeof(packet),
                0,
                (struct sockaddr *)&receiver_addr,
                sizeof(receiver_addr)
            );

            next_seq++;
        }

        /*
         * -----------------------------------------------------
         * RECEIVE ACKS
         * -----------------------------------------------------
         */

        while (base < next_seq)
        {
            /*
             * Since this is a noiseless channel,
             * ACKs are guaranteed to arrive.
             */

            int bytes = recvfrom(
                sock,
                &ack,
                sizeof(ack),
                0,
                (struct sockaddr *)&receiver_addr,
                &addr_len
            );

            if (bytes < 0)
            {
                perror("recvfrom");
                close(sock);
                return 1;
            }

            printf("ACK %d received.\n", ack.ack);

            base++;
        }

        printf("Window moved. Base = %d\n\n", base);
    }

    /* ---------------------------------------------------------
       SUMMARY
       --------------------------------------------------------- */

    printf("============================================================\n");
    printf("                       SUMMARY\n");
    printf("============================================================\n");

    printf("Number of Packets       : %d\n", n);
    printf("Sequence Number Bits    : %d\n", bits);
    printf("Sequence Number Space   : %d\n", seq_space);
    printf("Window Size             : %d\n", ws);
    printf("a = Tp/Tt               : %.4f\n", a);
    printf("TAO                     : %.2f ms\n", tao);
    printf("Theoretical Total Time  : %.2f ms\n", total);

    printf("============================================================\n");

    close(sock);

    return 0;
}