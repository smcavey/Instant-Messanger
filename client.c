/* Spencer McAvey's Chat Client */
/* To compile: gcc client.c -o client -pthread */
/* To run: ./client */

#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define ERROR -1
#define OK 0
#define SERVER_PORT 5555

void *receiveMessages (void *socketfd);

int main(int argc, char **argv)
{
	pthread_t thread_id; /* thread reference id */
	char messageOut[1024]; /* outbound message from client */
	int socketfd = 0;
	struct sockaddr_in server_address; /* server ip */
	struct sockaddr_in client_address; /* client ip */
	
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd < 0)
	{
		perror("socket creation failed");
	}
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT);

	if(connect(socketfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
	{
		perror("connection with server failed");
		exit(0);
	}
	else
	{
		printf("connected to server...\n");
	}
	pthread_create(&thread_id, NULL, receiveMessages, (void *)&socketfd);
	while(1)
	{
		char messageOut[1024]; /* client command line */
		fgets(messageOut, sizeof(messageOut), stdin);
		messageOut[strcspn(messageOut, "\n")] = 0; /* remove new line */
		if((send(socketfd, messageOut, 1024, 0)) < 0)
		{
			perror("unable to send message");
		}
		if(strcmp(messageOut, "!quit") == 0)
		{
			pthread_detach(pthread_self());
			exit(0);
		}
	}
}
void *receiveMessages(void *socketfd)
{
	int socket = *((int *) socketfd);
	while(1)
	{
		char message[1024];
		int messageSize = recv(socket, message, 1024, 0);
		message[messageSize] = '\0';
		printf("%s\n", message);
		
	}
}
