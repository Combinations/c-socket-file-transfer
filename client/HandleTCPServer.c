#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>
#include <stdlib.h>

#define RCVBUFSIZE 1024 /* Size of receive buffer */

void DieWithError(char *errorMessage); /* Error handling function */

void HandleTCPServer(int clntSocket, int isZipped, int sizeOFFile)
{

  char rcvBuffer[RCVBUFSIZE]; /* Buffer for echo string */
  int recvMsgSize;            /* Size of received message */
  int recievedSum = 0;

  FILE *zipped_files;
  if (isZipped == 0)
  {
    zipped_files = fopen("transzip.zip", "wb");
  }
  else
  {
    printf("opened new text file for writting!\n");
    zipped_files = fopen("bi.txt", "w");
  }
  if (zipped_files == NULL)
  {
    printf("failed to create file! exiting...");
    exit(0);
  }

  /* Receive chunk from client */

  if ((recvMsgSize = recv(clntSocket, rcvBuffer, RCVBUFSIZE, 0)) < 0)
    DieWithError("recv() failed");

  /* Send received string and receive again until end of transmission */
  for (; recievedSum < sizeOFFile; recievedSum += recvMsgSize)
  {

    if (sizeOFFile < 1024)
    {
      fwrite(rcvBuffer, 1, sizeOFFile, zipped_files);
      break;
    }
    else
    {
      fwrite(rcvBuffer, 1, recvMsgSize, zipped_files);
      memset(rcvBuffer, 0, sizeof(rcvBuffer));
      /* See if there is more data to receive */
      if ((recvMsgSize = recv(clntSocket, rcvBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
      if (sizeOFFile - (recievedSum + recvMsgSize) < 1024)
      {
        printf("this case! size of file %d\n", sizeOFFile);
        printf("recieved sum: %d\n", recievedSum + recvMsgSize);
        printf("remaining: %d\n", sizeOFFile - (recievedSum + recvMsgSize));
        char lastWrite[sizeOFFile - (recievedSum + recvMsgSize)];
        if ((recvMsgSize = recv(clntSocket, lastWrite, sizeOFFile - (recievedSum + recvMsgSize), 0)) < 0)
          DieWithError("recv() failed");
        fwrite(lastWrite, sizeof(char), recvMsgSize, zipped_files);
        break;
      }
    }
  }
  printf("new text file transfer complete\n");
  fclose(zipped_files);
  close(clntSocket);
}
