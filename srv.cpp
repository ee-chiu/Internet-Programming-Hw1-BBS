#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include "my_function.h"
 
using namespace std;

const int MAX_SIZE = 1000;
char cli_buff[MAX_SIZE];
char srv_buff[MAX_SIZE];
map<string, string> user2password;
bool isLogin = false;
string user = "";

void write2cli(int connfd,const char * message){
    snprintf(cli_buff, sizeof(cli_buff), "%s", message);
    Write(connfd, cli_buff, strlen(cli_buff));
    return;
}

void write2cli2(int connfd, const char * text, const char * user){
    snprintf(cli_buff, sizeof(cli_buff), "%s, %s.\n", text, user);
    Write(connfd, cli_buff, strlen(cli_buff));
    return;
}

void bbs_start(int connfd){
    write2cli(connfd, "********************************\n** Welcome to the BBS server. **\n********************************\n");
    return;
}

void exit_bbs(int connfd){
    write2cli(connfd, "bye\n");
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
            while(command[i] == ' ')
                i++;
            continue;
        }
        tmp += command[i];
        i++;
    }
    para.push_back(tmp);
    return para;
}

void reg(int connfd, vector<string> &para){
    if(para.size() != 3){
        write2cli(connfd, "Usage: register <username> <password>\n");
        return;
    }

    auto itr = user2password.find(para[1]);
    if(itr != user2password.end()){
        write2cli(connfd, "Username is already used.\n");
        return;
    }

    write2cli(connfd, "Register successfully.\n");
    user2password[para[1]] = para[2];
    return;
}

void login(int connfd, vector<string> &para){
    if(para.size() != 3){
        write2cli(connfd, "Usage: login <username> <password>\n");
        return;
    }

    if(isLogin){
        write2cli(connfd, "Please logout first.\n");
        return;
    }

    auto itr = user2password.find(para[1]);
    if(itr == user2password.end()){
        write2cli(connfd, "Login failed.\n");
        return;
    }

    if(user2password[para[1]] != para[2]){
        write2cli(connfd, "Login failed.\n");
        return;
    }

    write2cli2(connfd, "Welcome", para[1].c_str());
    isLogin = true;
    user = para[1];
}

void who(int connfd){
    if(!isLogin){
        write2cli(connfd, "Please login first.\n");
        return;
    }
    snprintf(cli_buff, sizeof(cli_buff), "%s\n", user.c_str());
    Write(connfd, cli_buff, strlen(cli_buff));
    return;
}

void logout(int connfd){
    if(!isLogin){
        write2cli(connfd, "Please login first.\n");
        return;
    }
    isLogin = false;
    write2cli2(connfd, "Bye", user.c_str());
    user.clear();
    return;
}

void list_user(int connfd){
    for(auto it = user2password.begin(); it != user2password.end(); it++){
        snprintf(cli_buff, sizeof(cli_buff), "%s\n", it->first.c_str());
        Write(connfd, cli_buff, strlen(cli_buff));
    }
    return;
}

void bbs_main(int connfd){
    bbs_start(connfd);
    while(1){
        write2cli(connfd, "% ");
        Read(connfd, srv_buff, sizeof(srv_buff));
        if(srv_buff[0] != 0){
            string command(srv_buff);
            command.pop_back();
            if(command == "exit"){
                if(isLogin) {
                    snprintf(cli_buff, sizeof(cli_buff), "Bye, %s.", user.c_str());
                    Write(connfd, cli_buff, strlen(cli_buff));
                }
                break;
            }

            vector<string> para = split(command);
            if(para[0] == "register") reg(connfd, para);
            else if(para[0] == "login") login(connfd, para);
            else if(para[0] == "whoami") who(connfd);
            else if(para[0] == "logout") logout(connfd);
            else if(para[0] == "list-user") list_user(connfd);
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
    srv_addr.sin_port = htons(1234);

    Bind(listenfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));

    Listen(listenfd, backlog);

    while(1){
        connfd = Accept(listenfd);

        bbs_main(connfd);
        /*pid_t pid = -1;
        if((pid = fork()) == 0){
            Close(listenfd);

            bbs_main(connfd);
            
            Close(connfd);
            exit(0);
        }*/

        Close(connfd);
    }

    return 0;    
}