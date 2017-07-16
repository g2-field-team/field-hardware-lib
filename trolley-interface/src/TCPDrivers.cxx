//C includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
//C++ includes
#include <iostream>
#include <string>
#include <strings.h>

#include "TCPDrivers.h"

using namespace std;
using namespace TCPDrivers;

void error(const char *msg)
{
    perror(msg);
}

namespace TCPDrivers{
//Connect TCP server
int ConnectToTCPServer(unsigned int *conversationHandle, unsigned int portNumber, const char* serverHostName, tcpFuncPtr callbackFunction, void *callbackData, unsigned int timeOut)
{
  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  fd_set fdset; 
  struct timeval tv;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    error("ERROR opening socket");
    *conversationHandle=1;
    return kTCP_UnableToEstablishConnection;
  }

  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  server = gethostbyname(serverHostName);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    *conversationHandle=1;
    return kTCP_ServerNotRegistered;
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
      (char *)&serv_addr.sin_addr.s_addr,
      server->h_length);
  serv_addr.sin_port = htons(portNumber);

  int errcode=0;
  errcode  =  connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
  /*if (errcode < 0)
    error("ERROR connecting");
    */

  FD_ZERO(&fdset);
  FD_SET(sockfd, &fdset);
  tv.tv_sec = timeOut/1000;             /* 10 second timeout */
  tv.tv_usec = timeOut%1000;
  int rc = select(sockfd + 1, NULL, &fdset, NULL, &tv) ;
  if (rc== 1)
  {
    int so_error;
    socklen_t len = sizeof so_error;

    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);

    if (so_error == 0) {
      printf("%s:%d is open\n", serverHostName, portNumber);
      *conversationHandle=(unsigned int)(sockfd);
    }else{
      error("ERROR connecting");
      fprintf(stderr,"error number %d\n",so_error);
      *conversationHandle=1;
      return kTCP_FailedToConnect;
    }
  }else if (rc==0){
    printf("Connect time out: %d ms.\n",timeOut);
    *conversationHandle=1;
    return kTCP_TimeOutErr;
  }else{
    *conversationHandle=1;
    return kTCP_FailedToConnect;
  }
  return kTCP_NoError;
}

//Disconnect from TCP server
int DisconnectFromTCPServer (unsigned int conversationHandle)
{
  close(conversationHandle);
  cout << "Disconnected from TCP server."<<endl;
  return kTCP_NoError;
}

//Read from TCP server
int ClientTCPRead (unsigned int conversationHandle, void *dataBuffer, size_t dataSize, unsigned int timeOut)
{
  unsigned int sockfd = conversationHandle;
  struct timeval tv;
  tv.tv_sec = timeOut/1000;             /* 10 second timeout */
  tv.tv_usec = timeOut%1000;

  fd_set read_mask;
  FD_ZERO(&read_mask);
  FD_SET(sockfd, &read_mask);
  int rc = select(sockfd+1, &read_mask, NULL, NULL, &tv) ;
  if (rc== 1)
  {
    int so_error;
    socklen_t len = sizeof so_error;
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error != 0) {
      printf("error number %d\n",so_error);
      return kTCP_FailedToConnect;
    }
  }else if (rc==0){
    printf("Read time out: %d ms.\n",timeOut);
    return kTCP_TimeOutErr;
  }
  int n_read = read(sockfd,dataBuffer,dataSize);
  if (n_read < 0){
    error("ERROR reading from socket");
    return kTCP_ReadFailed;
  }
 // printf("%s\n",buffer);
  return n_read;
}

//Write to TCP server
int ClientTCPWrite (unsigned int conversationHandle, void *dataPointer, int dataSize, unsigned int timeOut)
{
  unsigned int sockfd = conversationHandle;
  struct timeval tv;
  tv.tv_sec = timeOut/1000;             /* 10 second timeout */
  tv.tv_usec = timeOut%1000;

  fd_set write_mask;
  FD_ZERO(&write_mask);
  FD_SET(sockfd, &write_mask);
  int rc = select(sockfd+1, NULL, &write_mask, NULL, &tv) ;
  if (rc== 1)
  {
    int so_error;
    socklen_t len = sizeof so_error;
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error != 0) {
      printf("error number %d\n",so_error);
      return kTCP_FailedToConnect;
    }
  }else if (rc==0){
    printf("Write time out: %d ms.\n",timeOut);
    return kTCP_TimeOutErr;
  }
  int n_written = write(sockfd,dataPointer,dataSize);
  if (n_written < 0){
    error("ERROR writing to socket");
    return kTCP_WriteFailed;
  }
  return n_written;
}

}
