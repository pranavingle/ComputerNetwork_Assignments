#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    Packet packet;
    Ack ack;

    printf("============================================================\n");
    printf("          SLIDING WINDOW RECEIVER - NOISELESS\n");
    printf("============================================================\n");

    /* Create UDP socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror("socket");
        return 1;
    }

    /* Server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    /* Bind socket */
    if (bind(sock,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {

        perror("bind");
        close(sock);
        return 1;
    }

    printf("Receiver started.\n");
    printf("Listening on port %d...\n\n", PORT);

    while (1)
    {
        /* Receive packet */
        int bytes = recvfrom(
            sock,
            &packet,
            sizeof(packet),
            0,
            (struct sockaddr *)&client_addr,
            &client_len
        );

        if (bytes < 0) {
            perror("recvfrom");
            close(sock);
            return 1;
        }

        printf("Received Packet: Seq=%d, Data=%s\n",
               packet.seq,
               packet.data);

        /*
         * Noiseless channel:
         * Every packet is assumed to be correct.
         * Therefore send ACK immediately.
         */
        ack.ack = packet.seq;

        sendto(
            sock,
            &ack,
            sizeof(ack),
            0,
            (struct sockaddr *)&client_addr,
            client_len
        );

        printf("ACK %d sent.\n\n", ack.ack);
    }

    close(sock);
    return 0;
}