#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>

#define PORT 1234
#define BACKLOG 5
#define MAXDATASIZE 1000
void process_cli(FILE* fp,int connfd, struct sockaddr_in client);
void* function(void* arg);
struct ARG{
	int connfd;
	struct sockaddr_in client;
};

int main() 
{
	int listenfd, connfd;
	pthread_t tid;
	struct ARG *arg;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t len;

	if ((listenfd=socket(AF_INET, SOCK_STREAM, 0))==-1)					//create socket (return to listenfd)
	{
		perror("Creating socket failed.");
		exit(1);
	}             

	int opt = SO_REUSEADDR;									//set reuse address
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&server, sizeof(server));								//bzero
	server.sin_family = AF_INET;								//fill in server socketaddress information
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr*)&server, sizeof(server))==-1)			//bind
	{
		perror("Bind() error");
		exit(1);
	}

	if (listen(listenfd, BACKLOG) == -1) 							//listen
	{
		perror("listen() error.\n");
		exit(1);
	}

	len = sizeof(client);

	while(1){
		if((connfd=accept(listenfd,(struct sockaddr *)&client,&len))==-1)		//accept endless until client num reach "BACKLOG" (return to connfd)
		{
			perror("accept() error\n");
			exit(1);
		}	

		arg = (struct ARG *)malloc(sizeof(struct ARG));					
		arg->connfd = connfd;
		memcpy((void*)&arg->client,&client,sizeof(client));
		if(pthread_create(&tid,NULL,function,(void*)arg))				//create new thread , following with "arg" which including the information of the client
		{
			perror("Pthread_create() error");
			exit(1);		
		}
	}
	close(listenfd);
}


void process_cli(FILE* fp,int connfd,struct sockaddr_in client)
{
	int num;
	char recvbuf[MAXDATASIZE], sendbuf[MAXDATASIZE], cli_name[MAXDATASIZE];
	printf("You got a connection from %s.",inet_ntoa(client.sin_addr));
	num=recv(connfd,cli_name,MAXDATASIZE,0);						//recive the "send" from client for his name and return num for infomation
 	if(num==0)
	{
		close(connfd);
		printf("Client disconnected.\n");
		return;	
	}
	cli_name[num-1]='\0';
	printf("Client's name is %s.\n",cli_name);
	while(num=recv(connfd,recvbuf,MAXDATASIZE,0))						//recive the "send" from client endless
	{
		recvbuf[num]='\0';
		printf("Received client(%s) message: %s",cli_name,recvbuf);

		printf("Send infomation to client:");						//send message to client
		if(fgets(sendbuf,MAXDATASIZE,fp)==NULL)
		{
			printf("\nExit.\n");
			return;
		}	
		send(connfd,sendbuf,strlen(sendbuf),0);
	}
	close(connfd);
}



void* function(void* arg)
{
	struct ARG *info;
	info = (struct ARG *) arg;
	process_cli(stdin,info->connfd,info->client);
	free(arg);
	pthread_exit(NULL);
}
