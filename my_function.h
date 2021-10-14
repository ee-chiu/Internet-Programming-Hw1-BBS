#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int Socket(int family, int type, int protocol){
    int sockfd = socket(family, type, protocol);
    if(sockfd < 0){
        perror("Socket error");
        exit(0);
    }

    return sockfd;
} 

int Read(int socket, void* buffer, unsigned int size){
    int r = read(socket, buffer, size);
    if(r < 0){
        perror("Read error");
        exit(0);
    }

    return r;
}


