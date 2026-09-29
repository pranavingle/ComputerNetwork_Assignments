#include "dsss_common.h"

/* XOR two binary strings */
void xorBits(char a[], char b[], char result[])
{
    for(int i = 0; i < PN_LENGTH; i++)
    {
        int bitA = a[i] - '0';
        int bitB = b[i] - '0';

        int xorResult = bitA ^ bitB;

        result[i] = xorResult + '0';
    }

    result[PN_LENGTH] = '\0';
}

int main()
{
    int sock;

    struct sockaddr_in server;
    struct sockaddr_in client;

    socklen_t clientLen = sizeof(client);

    Packet packets[MAX_PACKETS];

    int received[MAX_PACKETS] = {0};

    int count = 0;

    printf("DSSS RECEIVER\n");
    printf("=============\n\n");

    /* Create UDP socket */
    sock = socket(
        AF_INET,
        SOCK_DGRAM,
        0
    );

    if(sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    /* Configure server */
    memset(
        &server,
        0,
        sizeof(server)
    );

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    /* Bind */
    if(bind(
        sock,
        (struct sockaddr *)&server,
        sizeof(server)
    ) < 0)
    {
        perror("Bind failed");
        close(sock);
        return 1;
    }

    printf(
        "DSSS Server listening on UDP port %d...\n",
        PORT
    );

    printf(
        "PN Code: %s\n\n",
        pnCode
    );

    printf("----- RECEIVING PACKETS -----\n\n");

    /* Receive packets */
    while(1)
    {
        Packet p;

        memset(
            &p,
            0,
            sizeof(p)
        );

        ssize_t bytes = recvfrom(
            sock,
            &p,
            sizeof(p),
            0,
            (struct sockaddr *)&client,
            &clientLen
        );

        if(bytes < 0)
        {
            perror("recvfrom failed");
            close(sock);
            return 1;
        }

        /* END packet */
        if(p.seqNo == END)
        {
            printf("\nEND packet received.\n");
            break;
        }

        /* Validate sequence number */
        if(
            p.seqNo < 0 ||
            p.seqNo >= MAX_PACKETS
        )
        {
            printf("Invalid sequence number.\n");
            continue;
        }

        /* Check duplicate */
        if(received[p.seqNo])
        {
            printf(
                "Duplicate Packet %d rejected.\n",
                p.seqNo + 1
            );

            continue;
        }

        /* Store packet */
        packets[p.seqNo] = p;

        received[p.seqNo] = 1;

        if(p.seqNo + 1 > count)
        {
            count = p.seqNo + 1;
        }

        printf(
            "Received Packet %d | "
            "Seq = %d | "
            "Spread Data = %s\n",
            p.seqNo + 1,
            p.seqNo,
            p.data
        );
    }

    /* Despreading */
    char original[MAX_PACKETS + 1] = "";

    printf("\n----- DSSS DESPREADING -----\n\n");

    for(int i = 0; i < count; i++)
    {
        /* Check missing packet */
        if(!received[i])
        {
            printf(
                "Packet %d is missing.\n",
                i + 1
            );

            continue;
        }

        char result[5];

        /*
         * XOR received spread data
         * with the same PN code.
         */
        xorBits(
            packets[i].data,
            pnCode,
            result
        );

        printf(
            "Packet %d | "
            "Received = %s | "
            "After XOR = %s\n",
            i + 1,
            packets[i].data,
            result
        );

        /*
         * 1111 -> original bit 1
         * 0000 -> original bit 0
         */
        if(strcmp(result, "1111") == 0)
        {
            strcat(
                original,
                "1"
            );
        }
        else if(strcmp(result, "0000") == 0)
        {
            strcat(
                original,
                "0"
            );
        }
        else
        {
            printf(
                "Invalid despread result: %s\n",
                result
            );
        }
    }

    printf(
        "\nRecovered Original Data: %s\n",
        original
    );

    close(sock);

    return 0;
}