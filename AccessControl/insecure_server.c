//
//  insecure_server.c
//  AccessControl
//
//  Created by Vicente Santacoloma on 3/10/13.
//  Copyright (c) 2013 Vicente Santacoloma. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "utilities.h"
#include "user_management.h"

#define TRUE 1
#define FALSE 0
#define LISTEN_CONNECTIONS 10
#define BUFFER_SIZE 256
#define USERNAME "Username:"
#define PASSWORD "Password:"
#define ACCESS_GRANTED "Access Granted"
#define ACCESS_DENIED  "Access Denied"

int port_number;

void response_access_control(int newsockfd)
{
  char username [BUFFER_SIZE];
  char password [BUFFER_SIZE];
  int n;
  
  bzero(username, BUFFER_SIZE);
  bzero(password, BUFFER_SIZE);
  
  
  n = write(newsockfd, USERNAME, strlen(USERNAME));
  if (n < 0) {
    error("ERROR writing to socket");
  }
  
  n = read(newsockfd, username, BUFFER_SIZE);
  if (n < 0) {
    error("ERROR reading from socket");
  }
  username[strlen(username) - 1] = 0;
  
  n = write(newsockfd, PASSWORD, strlen(PASSWORD));
  if (n < 0) {
    error("ERROR writing to socket");
  }
  
  n = read(newsockfd, password, BUFFER_SIZE);
  if (n < 0) {
    error("ERROR reading from socket");
  }
  password[strlen(password) - 1] = 0;
  
  int access_control = check_access_control(username, password);
  printf("Access Control: %d", access_control);
  if(access_control) {
    n = write(newsockfd, ACCESS_GRANTED, strlen(ACCESS_GRANTED));
    if (n < 0) {
      error("ERROR writing to socket");
    }
  } else {
    n = write(newsockfd, ACCESS_DENIED, strlen(ACCESS_DENIED));
    if (n < 0) {
      error("ERROR writing to socket");
    }
  }
  
}

void execute()
{
  int sockfd, newsockfd, portno, pid;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
  }
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = port_number;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    error("ERROR on binding");
  }
  listen(sockfd, LISTEN_CONNECTIONS);
  clilen = sizeof(cli_addr);
  
  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
    if (newsockfd < 0) {
      error("ERROR on accept");
    }
    pid = fork();
    if (pid < 0) {
      error("ERROR on fork");
    }
    if (pid == 0)  {
      close(sockfd);
      response_access_control(newsockfd);
      exit(0);
    } else {
      close(newsockfd);
    }
  } /* end of while */
  close(sockfd);

}

int main(int argc, const char * argv[])
{
  
  if (!server_load_parameters(argc, argv, &port_number)) {
    printf("Invalid Input Format\n");
    exit(1);
  }
  
  execute();
  
  return 0;
}