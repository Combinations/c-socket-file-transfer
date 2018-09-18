#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>
#include <stdlib.h>

#define RCVBUFSIZE 1024 /* Size of receive buffer */

void DieWithError(char *errorMessage); /* Error handling function */

void HandleTCPClient(int clntSocket, int isZipped, int sizeOFFile)
{
    char rcvBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    int recvMsgSize;            /* Size of received message */
    int recievedSum = 0;

    FILE *zipped_file;
    if (isZipped == 0)
    {
        zipped_file = fopen("transzip.zip", "wb");
        printf("opened new zip for writting...\n");
    }
    else
    {
        zipped_file = fopen("bi.txt", "wb");
    }
    if (zipped_file == NULL)
    {
        printf("failed to create file! exiting...");
        exit(0);
    }

    /* Receive chunk from client */
    if ((recvMsgSize = recv(clntSocket, rcvBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    for (; recievedSum < sizeOFFile; recievedSum += recvMsgSize)
    {
        fwrite(rcvBuffer, 1, recvMsgSize, zipped_file);
        memset(rcvBuffer, 0, sizeof(rcvBuffer));
        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, rcvBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
    }
    printf("finished writting to zip file...\n");
    fclose(zipped_file);
}
