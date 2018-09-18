#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define RCVBUFSIZE 32 /* Size of receive buffer */

void DieWithError(char *errorMessage); /* Error handling function */
char *findFile(char *);
void transferFile(char *filename, int sock, int isZipped);
void HandleTCPServer(int clntSocket, int isZipped, int fileSize);

int main(int argc, char *argv[])
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *zippedFileName;            /* String to send to echo server */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int zippedFileLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() and total bytes read */
    char *zippedFile;
    unsigned int servLen;
    int servSock;
    int fileSizeNet;
    int fileSize;
    int check;
    char *basePath = "proj2";
    struct timeval start, end;

    //    start = clock();
    gettimeofday(&start, NULL);
    double start_time = (start.tv_sec) * 1000 + (start.tv_usec) / 1000;

    if ((argc < 3) || (argc > 4)) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>] <Zipped File>\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];         /* First arg: server IP address (dotted quad) */
    zippedFileName = argv[3]; /* Second arg: string to echo */

    if (argc == 4)
        echoServPort = atoi(argv[2]); /* Use given port, if any */
    else
        echoServPort = 7; /* 7 is the well-known port for the echo service */

    zippedFile = findFile(zippedFileName);
    if (zippedFile)
    {
        printf("file found!\n");
    }

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */

    /* Establish the connection to the server */
    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");

    char *fullPath = malloc(strlen(zippedFileName) + strlen(basePath) + 2);
    if (fullPath == NULL)
    { /* deal with error and exit */
    }
    sprintf(fullPath, "%s/%s", basePath, zippedFileName);

    transferFile(fullPath, sock, 0);

    free(fullPath);

    check = recv(sock, &fileSizeNet, 4, 0); /*recieve file size of unzipped file */
    fileSize = ntohl(fileSizeNet);

    HandleTCPServer(sock, 1, fileSize);
    close(sock);
    //    stop = clock();
    //   double totalTime = (double) (stop - start) / CLOCKS_PER_SEC;
    gettimeofday(&end, NULL);
    double end_time = (end.tv_sec) * 1000 + (end.tv_usec) / 1000;
    double totalTime = end_time - start_time;
    printf("total time: %f ms\n", totalTime);
    exit(0);
}

void transferFile(char *fullPath, int sock, int zipped)
{
    FILE *fileToTransfer;
    if (zipped == 0)
    {
        fileToTransfer = fopen(fullPath, "rb"); //open the file
    }
    else
    {
        fileToTransfer = fopen("sample_file.txt", "rb");
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

    printf("size of file: %ld\n", sizeOfFile);
    printf("transfering file!\n");
    char transferBuffer[1024];
    long int i = 0;
    for (i = 0; i < sizeOfFile; i += 1024)
    {
        memset(transferBuffer, 0, sizeof(transferBuffer));
        fread(transferBuffer, 1, sizeof(transferBuffer), fileToTransfer);
        if (send(sock, transferBuffer, sizeof(transferBuffer), 0) != sizeof(transferBuffer))
            printf("error sent too many bytes.... !\n");
    }
    fread(transferBuffer, 1, sizeof(transferBuffer), fileToTransfer);
    if (send(sock, transferBuffer, sizeof(transferBuffer), 0) != sizeof(transferBuffer))
        printf("error sent too many bytes.... !\n");
    fclose(fileToTransfer);
    printf("zip file transfer completed!\n");
}

char *findFile(char *filename)
{
    DIR *dp;
    struct dirent *entry;
    char *file = NULL;
    char *dir = ".";
    if ((dp = opendir(dir)) == NULL)
    { //directory does not exist
        fprintf(stderr, "directory does not exist: %s\n", dir);
        return filename;
    }
    chdir(dir);
    while ((entry = readdir(dp)) != NULL)
    {
        if (strcmp(filename, entry->d_name) == 0)
        {
            file = entry->d_name;
            chdir("..");
            closedir(dp);
            return file; //return the file if we find it
        }
    }
    chdir(".."); //no file found
    closedir(dp);
    printf("zip file not found... exiting!\n");
    exit(0);
}
