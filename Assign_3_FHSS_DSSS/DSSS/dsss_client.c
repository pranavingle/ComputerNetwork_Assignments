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
    char bits[101];

    printf("DSSS SENDER\n");
    printf("===========\n\n");

    /* Take input */
    printf("Enter binary data: ");
    scanf("%100s", bits);

    /* Validate input */
    for(int i = 0; bits[i] != '\0'; i++)
    {
        if(bits[i] != '0' && bits[i] != '1')
        {
            printf("Invalid input! Enter only 0 and 1.\n");
            return 1;
        }
    }

    printf("\nPN Code: %s\n", pnCode);
    printf("Original Data: %s\n\n", bits);

    /* Create UDP socket */
    int sock = socket(
        AF_INET,
        SOCK_DGRAM,
        0
    );

    if(sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    /* Receiver address */
    struct sockaddr_in server;

    memset(
        &server,
        0,
        sizeof(server)
    );

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if(inet_pton(
        AF_INET,
        "127.0.0.1",
        &server.sin_addr
    ) <= 0)
    {
        perror("Invalid IP address");
        close(sock);
        return 1;
    }

    printf("----- DSSS SPREADING -----\n\n");

    /* Process every original bit */
    for(int i = 0; bits[i] != '\0'; i++)
    {
        Packet p;

        memset(
            &p,
            0,
            sizeof(p)
        );

        p.seqNo = i;

        char repeatedBit[5];
        char spreadData[5];

        /*
         * Original bit:
         *
         * 1 -> 1111
         * 0 -> 0000
         */
        for(int j = 0; j < PN_LENGTH; j++)
        {
            repeatedBit[j] = bits[i];
        }

        repeatedBit[PN_LENGTH] = '\0';

        /* XOR with PN code */
        xorBits(
            repeatedBit,
            pnCode,
            spreadData
        );

        strcpy(
            p.data,
            spreadData
        );

        /* Send packet */
        if(sendto(
            sock,
            &p,
            sizeof(p),
            0,
            (struct sockaddr *)&server,
            sizeof(server)
        ) < 0)
        {
            perror("sendto failed");
            close(sock);
            return 1;
        }

        printf(
            "Packet %d | Bit = %c | "
            "Repeated = %s | "
            "PN = %s | "
            "Spread = %s\n",
            i + 1,
            bits[i],
            repeatedBit,
            pnCode,
            p.data
        );

        sleep(1);
    }

    /* Send END packet */
    Packet end;

    memset(
        &end,
        0,
        sizeof(end)
    );

    end.seqNo = END;

    if(sendto(
        sock,
        &end,
        sizeof(end),
        0,
        (struct sockaddr *)&server,
        sizeof(server)
    ) < 0)
    {
        perror("END packet send failed");
        close(sock);
        return 1;
    }

    printf("\nAll packets sent successfully.\n");

    close(sock);

    return 0;
}
