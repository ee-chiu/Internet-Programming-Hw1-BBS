#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <vector>
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

void exit_bbs(int connfd){
    snprintf(cli_buff, sizeof(cli_buff), "bye\n");
    if (write(connfd, cli_buff, strlen(cli_buff)) < 0){
        perror("Write error");
        exit(0);
    }
    return;
}

vector<string> split(string command){
    vector<string> para;
    string tmp;
    int i = 0;
    while(command[i] != 0){
        if(command[i] == ' '){
            para.push_back(tmp);
            tmp.clear();
            i++;
            continue;
        }
        tmp += command[i];
        i++;
    }
    para.push_back(tmp);
    return para;
}

void bbs_main(int connfd){
    bbs_start(connfd);
    while(1){
        Read(connfd, srv_buff, sizeof(srv_buff));
        if(srv_buff[0] != 0){
            string command(srv_buff);
            command.pop_back();
            cout<<command<<endl;
            if(command == "exit"){
                exit_bbs(connfd);
                break;
            }

            vector<string> para = split(command);
            for(string s: para){
                cout<<s<<endl;
            }

            memset(srv_buff, 0, sizeof(srv_buff));
        }
    }
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

    Bind(listenfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));

    Listen(listenfd, backlog);

    while(1){
        connfd = Accept(listenfd);

        pid_t pid = -1;
        if((pid = fork()) == 0){
            Close(listenfd);

            bbs_main(connfd);
            
            Close(connfd);
            exit(0);
        }

        Close(connfd);
        sleep(1);
    }

    return 0;    
}