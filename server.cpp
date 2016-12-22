#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <pthread.h>
#include <vector>
#define numofthread 8

using namespace std;

//userdata
vector<string> UserList;
string Port[1024];
string balance[1024];
bool online[1024];
string IP[1024];
int onlineCount=0;

//threaddata
bool threadbusy[numofthread];
int socketid[numofthread];
string cilIP[numofthread];

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
    cout<<msg<<endl;
    return msg;
}
void reverse(char *s){
    for(int i = 0,j = strlen(s)-1;i<j;i++,j--){
        int c = s[i];
        s[i] = s[j];
        s[j] = c; 
    }
} 

void itoa(int n,char *s){
    int i = 0;
    while(n != 0){
        s[i++] = n%10+'0';
        n = n/10; 
    }
    s[i] = '\0';
    reverse(s);
} 

string getList(int i){
    char count[5];
    itoa(onlineCount,count);
    string list=balance[i]+"\n"+count+"\n";
    for (int i=0;i<UserList.size();++i){
        if(online[i])
            list=list+UserList[i]+"#"+IP[i]+"#"+Port[i]+"\n";
    }
    cout<<"getList "<<list<<endl;
    return list;
    
}
void *serverThread(void *index){
    int n = *((int*)index);
    int x;
    cout<<"this is thread "<<n<<endl;
    while(1){
        for(int j=0;j<10000;++j)
            for(int k=0;k<1000;++k)
                int x=x+1;
        if(threadbusy[n]){

            cout<< "thread"<<n<<"get job"<<endl;
            int newsockfd=socketid[n];
            send(newsockfd, "success!");

            bool login = 0;
            string entry;
            string useraccountname;
            string portnum;
            string userbalance;
            int userindex;
            while(!login){
                entry = receive(newsockfd);
                
                if(entry.substr(0,9)== "REGISTER#"){
                    userbalance=receive(newsockfd);
                    string RName = entry.substr(9);

                    if(find(UserList.begin(), UserList.end(), RName) == UserList.end()){
                        UserList.push_back(RName);
                        balance[UserList.size()-1] = userbalance;
                        send(newsockfd, "100 OK");
                    }
                    else
                        send(newsockfd, "210 FAIL");
                }
                else{
                    int pos = entry.find("#");
                    useraccountname = entry.substr(0,pos);
                    portnum = entry.substr(pos+1);
                    for(int i=0;i<UserList.size();++i){
                        if (UserList[i]==useraccountname){
                            userindex=i;
                            Port[i] = portnum;
                            online[i]=1;
                            IP[i]=cilIP[n];
                            login=1;
                            onlineCount+=1;
                            send(newsockfd,getList(i));
                            break;
                        }
                        if(i==UserList.size()-1)
                            send(newsockfd,"220 AUTH_FAIL");
                    }
                }
            }
            while(login) {
                entry = receive(newsockfd);
                if(entry=="List"){
                    send(newsockfd,getList(userindex));
                }
                else if(entry == "Exit"){
                    login=0;
                    online[userindex]=0;
                    onlineCount-=1;
                    send(newsockfd, "bye");
                }
            }
            close(newsockfd);
        }
        threadbusy[n]=0;
    }
}


int main(int argc, char *argv[]) {

    // check for port
    string av0(argv[0]);
    if (argc < 2)
        error("usage: " + av0 + " serverPort\n");
    int portno = atoi(argv[1]);

    // setup socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // setup server
    struct sockaddr_in serv_addr, cli_addr;
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    // bind to port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    cout << "open" <<endl;

    //create threads
    pthread_t thread[numofthread];
    for(int i=0;i<numofthread;++i){
        int x=0;
         pthread_create(&thread[i],NULL,serverThread,&i);
         for(int j=0;j<10000;++j)
            for(int k=0;k<1000;++k)
                int x=x+1;
    }
    // listen
    cout<<"listening..."<<endl;
    listen(sockfd,5);
    socklen_t clilen = sizeof(cli_addr);
    
    while(1){
        
        // accept connections
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        cout<<"get new connection"<<endl;
        if (newsockfd < 0)
            error("ERROR on accept");

        //allocate to an idle thread
        for(int i=0;;++i){
            if (i==numofthread){
                i=0;
            }
            if(!threadbusy[i]){
                char *Address = inet_ntoa(cli_addr.sin_addr);
                cilIP[i]=Address;
                socketid[i]=newsockfd;
                threadbusy[i]=1;

                cout<<threadbusy[i]<<endl;
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}
