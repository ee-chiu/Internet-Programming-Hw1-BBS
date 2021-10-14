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

int Bind(int listenfd, struct sockaddr * ptr_srv_addr, int size){
    int b = bind(listenfd, ptr_srv_addr, size);

    if(b < 0){
        perror("Bind error");
        exit(0);
    }

    return 0;
}

int Listen(int listenfd, int backlog){
    int l = listen(listenfd, backlog);
    
    if(l < 0){
        perror("Listen error");
        exit(0);
    }

    return 0;
}

int Accept(int listenfd){
    int a = accept(listenfd, (struct sockaddr *) NULL, NULL);

    if (a < 0){
        perror("Accept error");
        exit(0);
    }

    return a;
}

int Read(int socket, void* buffer, unsigned int size){
    int r = read(socket, buffer, size);
    if(r < 0){
        perror("Read error");
        exit(0);
    }

    return r;
}


