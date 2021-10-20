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
#include <queue>
#include <pthread.h>
#include "my_function.h"
 
using namespace std;

const int MAX_SIZE = 10000;
char cli_buff[MAX_SIZE];
char srv_buff[MAX_SIZE];
map<string, string> user2password;
bool isLogin = false;
string user = "";
map< string, map< string, queue<string> > > user2other_user_message;

int char2int(const char * c){
    int num = 0;
    int i = 0;
    while(c[i] != 0){
        num = num * 10 + (c[i] - '0');
        i++;
    }
    return num;
}

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
        if(command[i] == '"') {
            i++;
            break;
        }
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

    while(command[i] != 0 && command[i] != '"'){
        tmp += command[i];
        i++;
    }
    
    if(!tmp.empty())
        para.push_back(tmp);
    return para;
}

void reg(int connfd,const vector<string> &para){
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

void login(int connfd,const vector<string> &para){
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

void send(int connfd, const vector<string> &para){
    if(para.size() != 3){
        write2cli(connfd, "Usage: send <username> <message>\n");
        return;
    }

    if(!isLogin){
        write2cli(connfd, "Please login first.\n");
        return;
    }
    
    auto itr = user2password.find(para[1]);
    if(itr == user2password.end()){
        write2cli(connfd, "User not existed.\n");
        return;
    }

    user2other_user_message[para[1]][user].push(para[2]+"\n");
    return;
}

void list_message(int connfd){
    if(!isLogin){
        write2cli(connfd, "Please login first.\n");
        return;
    }

    map<string, queue<string>> message_box = user2other_user_message[user];
    if(message_box.empty()){
        write2cli(connfd, "Your message box is empty.\n");
        return;
    }

    bool has_message = false;
    for(auto itr = message_box.begin(); itr != message_box.end(); itr++){
        if(itr->second.size() == 0) continue;
        snprintf(cli_buff, sizeof(cli_buff), "%zu message from %s.\n", itr->second.size(), itr->first.c_str());
        Write(connfd, cli_buff, strlen(cli_buff));
        has_message = true;
    }

    if(!has_message) write2cli(connfd, "Your message box is empty.\n");

    return;
}

void receive(int connfd, const vector<string> &para){
    if(para.size() != 2){
        write2cli(connfd, "Usage: receive <username>\n");
        return;
    }

    if(!isLogin){
        write2cli(connfd, "Please login first.\n");
        return;
    }
    
    auto itr = user2password.find(para[1]);
    if(itr == user2password.end()){
        write2cli(connfd, "User not existed.\n");
        return;
    }

    map<string, queue<string>> message_box = user2other_user_message[user];
    auto itr2 = message_box.find(para[1]);
    
    if(itr2 == message_box.end()) return;
    if(message_box[para[1]].empty()) return;
    
    string message = message_box[para[1]].front();
    user2other_user_message[user][para[1]].pop();

    write2cli(connfd, message.c_str());
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
                isLogin = false;
                user.clear();                
                break;
            }

            vector<string> para = split(command);
            if(para[0] == "register") reg(connfd, para);
            else if(para[0] == "login") login(connfd, para);
            else if(para[0] == "whoami") who(connfd);
            else if(para[0] == "logout") logout(connfd);
            else if(para[0] == "list-user") list_user(connfd);
            else if(para[0] == "send") send(connfd, para);
            else if(para[0] == "list-msg") list_message(connfd);
            else if(para[0] == "receive") receive(connfd, para);
            memset(srv_buff, 0, sizeof(srv_buff));
        }
    }

    Close(connfd);
    return;
}

void *bbs_main2(void* arg){
    int connfd = *((int* )arg);
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
                isLogin = false;
                user.clear();                
                break;
            }

            vector<string> para = split(command);
            if(para[0] == "register") reg(connfd, para);
            else if(para[0] == "login") login(connfd, para);
            else if(para[0] == "whoami") who(connfd);
            else if(para[0] == "logout") logout(connfd);
            else if(para[0] == "list-user") list_user(connfd);
            else if(para[0] == "send") send(connfd, para);
            else if(para[0] == "list-msg") list_message(connfd);
            else if(para[0] == "receive") receive(connfd, para);
            memset(srv_buff, 0, sizeof(srv_buff));
        }
    }

    return NULL;
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

    if(argc != 2){
        printf("Usage: ./srv <Port Number>\n");
        exit(0);
    }
    listenfd = Socket(family, type, protocol);

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = family;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int num = char2int(argv[1]);
    srv_addr.sin_port = htons(num);

    Bind(listenfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));

    Listen(listenfd, backlog);

    while(1){
        connfd = Accept(listenfd);

        pthread_t child;
        pthread_create(&child, NULL, &bbs_main2, (void*) &connfd);
    }

    return 0;    
}