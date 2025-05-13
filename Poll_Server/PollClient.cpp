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

class ctcpclient                  //TCP 通讯的客户端类
{  
	public :
		int m_clientfd;			  //客户端的socket,-1表示未连接
		string m_ip;              //服务器端的ip地址
		unsigned short m_port;	  //通讯端口
	
	ctcpclient():m_clientfd(-1){};

	//向服务器端发起连接请求
	bool connect(const string &in_ip,const unsigned short in_port){
		//如果已经连接,直接返回false
		if(m_clientfd!=-1){
			cout<<"已经和服务器连接"<<endl;
			return false;
		}

		m_ip=in_ip;					//服务器ip
		m_port=in_port;				//服务器端口

		//1.创建客户端socket
		if((m_clientfd=socket(AF_INET,SOCK_STREAM,0))==-1){
			//如果连接失败,返回false
			cout<<"创建客户端scoket失败"<<endl;
			return false;
		}

		//2.向服务器发起连接请求
		struct sockaddr_in servaddr;  			//用于存放协议族，端口和ip地址的结构体
		memset(&servaddr,0,sizeof(servaddr));
		servaddr.sin_family =AF_INET;			//协议族
		servaddr.sin_port =htons(m_port);		//指定服务器的通信端口

		struct hostent* h;						//用于存放服务器ip地址(大端序)的结构体的指针
		if((h=gethostbyname(m_ip.c_str()))==nullptr)	//把域名/主机名/字符串格式的ip转换为结构体(注意把ip地址转换为c风格的字符串)
		{
			//如果转换失败，关闭客户端socket 返回false
			cout<<"转换ip失败"<<endl;
			close(m_clientfd);
			m_clientfd=-1;
			return false;
		}
		memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);  //复制指定服务器端口的ip(大端序)
		
		//向服务器发送请求连接(注意：库中存在和connect同名的函数用::引用)
		if(::connect(m_clientfd,(struct sockaddr *)&servaddr,sizeof(servaddr))==-1){
			//如果连接失败，关闭客户端socket 返回false
			cout<<"请求连接失败"<<endl;
			close(m_clientfd);
			m_clientfd=-1;
			return false;
		}

		return true;
	}

	//是否成功发送报文
	bool send(const string &buffer){
		//如果没有和服务器端连接，直接返回false
		if(m_clientfd==-1){
			cout<<"没有和服务器端连接"<<endl;
			return false;
		}
		//发送报文失败
		if((::send(m_clientfd,buffer.data(),buffer.size(),0))<=0){
			cout<<"客户端发送报文失败"<<endl;
			return false;
		}
		return true;
	}

	//是否成功接收报文(注意：如果直接操作string对象的内存必须保证：1.不能越界；2.操作后手动设置数据的大小)
	bool recv(string &buffer,const size_t maxlen){
		buffer.clear();					//清空容器
		buffer.resize(maxlen);			//设置容器大小为maxlen
		int readn=::recv(m_clientfd,&buffer[0],buffer.size(),0);	//直接操作buffer内存
		if(readn<=0){
			//-1 连接失败；0 断开连接；
			cout<<"客户端接收报文失败"<<endl;
			buffer.clear();
			return false;
		}
		buffer.resize(readn);			//重置buffer的实际大小

		return true;
	}

    //关闭客户端的socket
    bool closeclient(){
        if(m_clientfd==-1){
            return false;
        }
        ::close(m_clientfd);
        m_clientfd=-1;
        return true;
    }

	~ctcpclient(){closeclient();};
};

int main(int argc,char *argv[]){
	ctcpclient tcpclient;
	
	if(tcpclient.connect(argv[1],atoi(argv[2]))==false){
		cout<<"连接失败"<<endl;
		perror("connect()");
		return -1;
	}
	cout<<"成功连接服务器端"<<endl;
	string buffer;
	for(int i=0;i<10;++i){
		
		cin>>buffer;

		if(tcpclient.send(buffer)==false){
			perror("send");
			break;
		}

		cout<<"发送： "<<buffer<<endl;

		//接收服务器端的回复报文，如果没有回复recv就会一直阻塞
		if(tcpclient.recv(buffer,1024)==false){
			perror("recv");
			break;
		}
		cout<<"接收： "<<buffer<<endl;
		sleep(1);
	}

	close(tcpclient.m_clientfd);
}
