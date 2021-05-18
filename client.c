/* Spencer McAvey's Chat Client */

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

char messageOut[1024]; /* message outbound from client */
char messageIn[1024]; /* message inbound to client */

int socketfd = 0;

void *receiveMessages(void *arg);

int main(int argc, char **argv)
{
	pthread_t thread_id; /* thread reference id */
	int connfd = 0;
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
	pthread_create(&thread_id, NULL, receiveMessages, (void*)&socketfd);
	do
	{
		fgets(messageOut, sizeof(messageOut), stdin); /* get user input from keyboard */
		messageOut[strcspn(messageOut, "\n")] = 0; /* strip trailing new line */
		/* stay in here and parse messageOut for commands that the server accepts and have a bunch of conditions to check those commands and execute them while valid and have helper methods for handling those...when a message is sent, call a receiveMessage method to get the reply...also think about how the user will get messages at any time from other clients */
	}
	while(strcmp(messageOut, "!quit") != 0);
}
void *receiveMessages(void *arg)
{
	while(1)
	{
		read(socketfd, messageIn, sizeof(messageIn));
		printf("%s", messageIn);
	}
}
