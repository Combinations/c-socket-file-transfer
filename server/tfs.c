#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <errno.h>

#define MAXPENDING 5 /* Maximum outstanding connection requests */

void DieWithError(char *errorMessage);                              /* Error handling function */
void HandleTCPClient(int clntSocket, int isZipped, int sizeOfFile); /* TCP client handling function */
void transferFile(char *filename, int sock, int isZipped);

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned short echoServPort;     /* Server port */
    unsigned int echoServIP;         /* Server IP */
    unsigned int clntLen;            /* Length of client address data structure */
    int fileSizeNet;
    int fileSize;
    int check = 0;

    if (argc != 3) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServIP = atoi(argv[1]);   /* First arg: server IP */
    echoServPort = atoi(argv[2]); /* Second arg:  local port */

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(echoServIP); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the address */
    if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);

    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *)&echoClntAddr,
                           &clntLen)) < 0)
        DieWithError("accept() failed");

    /* clntSock is connected to a client! */

    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

    /* recieve the size of file */
    check = recv(clntSock, &fileSizeNet, 4, 0);
    fileSize = ntohl(fileSizeNet);

    HandleTCPClient(clntSock, 0, fileSize);
    printf("unzipping\n");
    system("unzip transzip.zip");
    printf("transfering unzipped file..\n");

    transferFile("sample_file.txt", clntSock, 1);
    close(clntSock);
}

void transferFile(char *filename, int sock, int zipped)
{
    FILE *fileToTransfer;
    if (zipped == 0)
    {
        fileToTransfer = fopen("sample_file.zip", "rb"); //open the file
    }
    else
    {
        fileToTransfer = fopen("sample_file.txt", "r");
    }
    if (fileToTransfer == NULL)
    {
        printf("Failed to open file with error %d \n", errno);
        exit(0);
    }
    fseek(fileToTransfer, 0, SEEK_END); //find the size of the file
    long int sizeOfFile = ftell(fileToTransfer);

    int temp = htonl((uint32_t)sizeOfFile);
    write(sock, &temp, sizeof(temp)); //send file size
    rewind(fileToTransfer);           //rewind to the start of the file

    rewind(fileToTransfer); //rewind to the start of the file
    printf("size of unzipped file: %ld\n", sizeOfFile);
    char transferBuffer[1024];
    int i = 0;
    for (i = 0; i < sizeOfFile; i += 1024)
    {
        memset(transferBuffer, 0, sizeof(transferBuffer));
        fread(transferBuffer, sizeof(char), sizeof(transferBuffer), fileToTransfer);
        if (send(sock, transferBuffer, sizeof(transferBuffer), 0) != sizeof(transferBuffer))
            printf("error sent too many bytes.... !\n");
    }

    fread(transferBuffer, sizeof(char), sizeof(transferBuffer), fileToTransfer);
    if (send(sock, transferBuffer, sizeof(transferBuffer), 0) != sizeof(transferBuffer))
        printf("error sent too many bytes.... !\n");
    fclose(fileToTransfer);
    printf("File transfer of unzipped file completed!\n");
}
