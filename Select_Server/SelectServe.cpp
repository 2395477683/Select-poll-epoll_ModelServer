#include<iostream>
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unistd.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
using namespace std;

int initserver(int port);

int main(int argc,char* argv[]){
    if(argc!=2){
        cout<<"QWQ:参数不够哦！你缺少了端口";
        return -1;
    }

    int listensock=initserver(atoi(argv[1]));
    cout<<"listensock="<<listensock<<endl;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listensock,&readfds);

    int maxfd=listensock;

    while(true){
        struct timeval timeout;
        timeout.tv_sec=10;
        timeout.tv_usec=0;

        fd_set tmpfds=readfds;

        int infds=select(maxfd+1,&tmpfds,NULL,NULL,0);               //

        if(infds<0){
            perror("select() failed");break;
        }

        if(infds==0){
            cout<<"select timeout"<<endl;
            continue;
        }

        for(int i=0;i<=maxfd;++i){
            if(FD_ISSET(i,&tmpfds)==0){
                continue;
            }

            if(i==listensock){
                struct sockaddr_in client;
                socklen_t len=sizeof(client);
                int clientsock=accept(listensock,(struct sockaddr*)&client,&len);
                if(clientsock<0){
                    perror("client accept() failed");
                    continue;
                }

                cout<<"client accept success,clientsock="<<clientsock<<endl;

                FD_SET(clientsock,&readfds);

                if(maxfd<clientsock)maxfd=clientsock;

            }
            else{

                char buffer[1024];
                memset(buffer,0,sizeof(buffer));
                if(recv(i,buffer,sizeof(buffer),0)<=0){
                    cout<<"client(i="<<i<<"),disconnected"<<endl;

                    close(i);
                    
                    FD_CLR(i,&readfds);

                    if(i==maxfd){
                        for(int ii=maxfd;ii>0;ii--){
                            if(FD_ISSET(ii,&readfds)){
                                maxfd=ii;
                                break;
                            }
                        }
                    }


                }
                else{
                    cout<<"recv(i="<<i<<"):"<<buffer<<endl;

                    send(i,buffer,strlen(buffer),0);
                }
            }
        }

    }


}

int initserver(int port){
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0){
        perror("socket() failed");
        return -1;
    }

    int opt=1;unsigned int len=sizeof(opt);                                 //为套接字启用端口地址重用功能，允许该套接字绑定到一个已经在使用中的地址和端口
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);

    struct sockaddr_in servaddr;
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);

    if(bind(sock,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("bind() failed");return -1;
    }
    
    if(listen(sock,5)!=0){
        perror("listen() failed");return -1;
    }

    return sock;
}