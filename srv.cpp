#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "my_function.h"

using namespace std;

const int MAX_SIZE = 1000;
char cli_buff[MAX_SIZE];
char srv_buff[MAX_SIZE];

void bbs_start(int connfd){
    snprintf(cli_buff, sizeof(cli_buff), "********************************\n** Welcome to the BBS server. **\n********************************\n");
    if (write(connfd, cli_buff, strlen(cli_buff)) < 0){
        perror("Write error");
        exit(0);
    }
    return;
}

void get_command(int connfd){
    Read(connfd, srv_buff, sizeof(srv_buff));
    return;
}

int main(int argc, char** argv){
    const int family = AF_INET;
    const int type = SOCK_STREAM;
    const int protocol = 0;
    const int MAX_SIZE = 10000;
    int listenfd, connfd;
    struct sockaddr_in srv_addr;
    int backlog = 20;
    memset(cli_buff, 0, sizeof(cli_buff));
    memset(srv_buff, 0, sizeof(srv_buff));

    listenfd = Socket(family, type, protocol);

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = family;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(13);

    if(bind(listenfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) < 0){
        perror("Bind error");
        exit(0);
    }

    if(listen(listenfd, backlog) < 0){
        cout<<"listen error"<<endl;
        exit(0);
    }

    while(1){
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
        if(connfd < 0){
            cout<<"accept error"<<endl;
            exit(0);
        }

        pid_t pid = -1;
        if((pid = fork()) == 0){
            if (close(listenfd) < 0){
                cout<<"close error"<<endl;
                exit(0);
            }

            bbs_start(connfd);
            while(1){
                get_command(connfd);
                if(srv_buff[0] != 0){
                    printf("%s", srv_buff);
                    memset(srv_buff, 0, sizeof(srv_buff));
                }
            }

            if(close(connfd) < 0){
                cout<<"close error"<<endl;
                exit(0);
            }
            exit(0);
        }

        close(connfd);
    }

    return 0;    
}