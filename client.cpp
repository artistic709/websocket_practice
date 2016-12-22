#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

using namespace std;

void error(string msg) {
    cout << msg << endl;
    exit(0);
}

void send(int sockfd, string msg) {
    if(write(sockfd,msg.c_str(), msg.length()) < 0)
        error("Error writing message\n");
}

string receive(int sockfd) {
    char buffer[1024];
    memset(buffer, 0, 1024);
    if(read(sockfd, buffer, 1023) < 0)
        error("Error reading message\n");
    string msg(buffer);
    return msg;
}

int main(int argc, char *argv[]) {
    // check for server
    struct hostent *server;
    string av0(argv[0]);
    if (argc < 3)
        error("usage: " + av0 + " serverHostname serverPort\n");
    int portno = atoi(argv[2]);
    server = gethostbyname(argv[1]);
    if (server == NULL)
        error("host not found\n");

    // setup socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // setup connection
    struct sockaddr_in serv_addr;
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove((char *)&serv_addr.sin_addr.s_addr,(char *)server->h_addr,server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    // wait for action
    string entry;
    bool login = 0;
    entry = receive(sockfd);

    int mode = 0;
    // login
    while(!login){
        mode = 0;
        string uname,balance;
        cout << "Enter 1 for register, 2 for login: ";
        cin >> mode;
        if(mode == 1){
            cout << "Enter Username: ";
            cin >> uname;
            cout << "Enter balance: ";
            cin >> balance;
            send(sockfd, "REGISTER#"+uname);
            send(sockfd, balance);
            entry = receive(sockfd);

            if ( entry[0] == '1' )
                cout << "Success!"<<endl;
            else
                cout<< "Fail, try another username." <<endl;
        }
        else if(mode == 2){
            cout << "Enter Username: ";
            cin >> uname;
            send(sockfd, uname+"#"+argv[2]);
            entry = receive(sockfd);
            
            if (entry[0] == '2')
                cout << "Username not registered!"<<endl;
            else{
                cout << entry << endl;
                login=1;
            }
        }
        else
            error("Wrong input!\n");
    }

    //renew list or quit
    while(login) {
        mode = 0;
        cout << "Enter 1 for renew list, 2 for exit ";
        cin >> mode;
        if (mode == 1){
            send(sockfd, "List");
            entry = receive(sockfd);
            cout << entry << endl;
        }
        else if (mode == 2){
            send(sockfd, "Exit");
            entry = receive(sockfd);
            if(entry == "bye")
                login = 0;
        }
        else
            error("Wrong input!\n");
    }

    close(sockfd);
    return 0;
}
