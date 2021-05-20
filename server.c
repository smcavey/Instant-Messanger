/* Spencer McAvey's Chat Server */
/* To compile: gcc server.c -o server -pthread */
/* To run: ./server */

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

#define ERROR -1
#define OK 0
#define SERVER_PORT 5555 /* server port */
#define MAX_NUM_CLIENTS 10

int numUsersConnected = 0;

typedef struct
{
	struct sockaddr_in address; /* client address */
	int connfd; /* connection file descriptor */
	char *username; /* client username */
	int userID; /* user ID */
} client_t;

client_t *clients[MAX_NUM_CLIENTS];

void *clientInterface(void *arg); /* handles communication with the client */
void clientHelpMenu(int connfd); /* provides client with helpful informaiton */

int main(int argc, char **argv)
{
	pthread_t thread_id; /* thread reference id */
	int userID = 0;
	int listenfd = 0; /* listening descriptor */
	int connfd = 0; /* connecton descriptor */
	struct sockaddr_in server_address; /* server ip */
	struct sockaddr_in client_address; /* client ip */

	/* socket settings */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT);
	
	/* bind to associate socket with local ip and port */
	if(bind(listenfd, (struct sockaddr*)&server_address, sizeof(server_address)) <0)
	{
		perror("socket binding failed");
		return ERROR;
	}
	/* listen for connections on socket */
	if(listen(listenfd, 10) < 0)
	{
		perror("socket listening failed");
		return ERROR; 
	}
	printf("Server online...\n");
	/* accept clients */
	while(1)
	{
		int i = 0;
		socklen_t client_length = sizeof(client_address);
		connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_length);

		/* add client */
		client_t *client = (client_t *)malloc(sizeof(client_t));
		client->address = client_address;
		client->connfd = connfd;
		client->username = NULL;
		client->userID = userID++;

		numUsersConnected++;

		printf("User %d connected\n", client->userID);
/*		for(int i = 0; i < numClientsConnected; i++)
		{
			if(!clients[i])
			{
*/
				clients[i] = client;
/*
				break;
			}
		}
*/
		pthread_create(&thread_id, NULL, clientInterface, (void*)client);
		i++;
	}
	return 0;
}

void *clientInterface(void *arg)
{
	client_t *client = (client_t*)arg;
	clientHelpMenu(client->connfd);
	while(1)
	{
		char messageIn[1024];
		char *firstArg;
		int messageSize = recv(client->connfd, messageIn, 1024, 0);
		messageIn[messageSize] = '\0';
		printf("%d: %s\n", client->userID, messageIn);
		firstArg = strtok(messageIn, " ");
		if(strcmp(firstArg, "!quit") == 0)
		{
			numUsersConnected--;
			break;
		}
		else if(strcmp(firstArg, "!name") == 0)
		{
			char *name = strtok(NULL, " ");
			client->username = name;
		}
		else if(strcmp(firstArg, "!help") == 0)
		{
			clientHelpMenu(client->connfd);
		}
		else if(strcmp(firstArg, "!msg") == 0)
		{
			char *recipient = strtok(NULL, " ");
			char *message = strtok(NULL, "");
			for(int i = 0; i < numUsersConnected; i++)
			{
				if(strcmp(recipient, client[i].username) == 0)
				{
					send(client[i].connfd, message, 1024, 0);
					printf("message sent");
				}
			}
		}
		else if(strcmp(firstArg, "!who") == 0)
		{
			for(int i = 0; i < 5; i++)
			{
				char *user = client[i].username;
				send(client->connfd, user, 1024, 0);
			}
		}
	}
	close(client->connfd);
	free(client);
	pthread_detach(pthread_self());
	return NULL;
}
void clientHelpMenu(int connfd)
{
	char *helpMessage = "Welcome! Begin by choosing a username. Type \"!name\"\
	followed by your name to choose a name. Type \"!help\" for help. Type \
	\"!quit\" to quit.";
	send(connfd, helpMessage, 1024, 0);
}
