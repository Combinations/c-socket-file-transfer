#include <stdio.h>  /* for perror() */
#include <stdlib.h> /* for exit() */
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

void HandleTCPServer(int sock, int isZipped);

void transferFile(char *filename, int sock, int zipped)
{
  FILE *fileToTransfer;
  if (zipped == 0)
  {
    fileToTransfer = fopen("sample_file.zip", "rb"); //open the file
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
  rewind(fileToTransfer); //rewind to the start of the file
  printf("size of file: %ld\n", sizeOfFile);
  printf("transfering file!\n");
  char transferBuffer[1024];
  int i = 0;
  for (i = 0; i < sizeOfFile; i += 1024)
  {
    memset(transferBuffer, 0, sizeof(transferBuffer));
    fread(transferBuffer, 1, sizeof(transferBuffer), fileToTransfer);
    if (send(sock, transferBuffer, sizeof(transferBuffer), 0) != sizeof(transferBuffer))
      printf("error sent too many bytes.... !\n");
  }
  fclose(fileToTransfer);
  //    close(sock);
  printf("File transfer completed!");
  return;
}
