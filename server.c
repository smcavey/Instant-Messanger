/* Spencer McAvey's Chat Server */
/* To compile: gcc server.c -o server -pthread */
/* To run: ./server */

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
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
	char username[32]; /* client username */
	int userID; /* user ID */
} client_t;

client_t *clients[MAX_NUM_CLIENTS];

void *clientInterface(void *arg); /* handles communication with the client */
void clientHelpMenu(int connfd); /* provides client with helpful informaiton */
void setUsername(int connfd, int userID); /* gives user chat access once they have a unique username */
void userJoinedMessage(int userID); /* sends message to everyone that a client joined */
void userLeftMessage(int userID); /* sends message to everyone that a client left */

pthread_mutex_t clients_list = PTHREAD_MUTEX_INITIALIZER;

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
	int i = 0;
	while(1)
	{
		socklen_t client_length = sizeof(client_address);
		connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_length);

		/* add client */
		client_t *client = (client_t *)malloc(sizeof(client_t));
		client->address = client_address;
		client->connfd = connfd;
		strcpy(client->username, "temp");
		client->userID = userID;

		printf("User %d connected\n", client->userID);
		pthread_mutex_lock(&clients_list);
		clients[i] = client;
		printf("clients[i]->userID: %d clients[i]->connfd: %d\n", clients[i]->userID, clients[i]->connfd);
		pthread_mutex_unlock(&clients_list);
		printf("ID: %d CONNFD: %d\n", clients[i]->userID, clients[i]->connfd);
		i++;
		userID++;
		numUsersConnected++;
		pthread_create(&thread_id, NULL, clientInterface, (void*)client);
	}
	return 0;
}

void *clientInterface(void *arg)
{
	client_t *client = (client_t*)arg;
	clientHelpMenu(client->connfd);
	setUsername(client->connfd, client->userID);
	userJoinedMessage(client->userID);
	while(1)
	{
		char messageIn[1024];
		int size;
		char *firstArg = NULL;
		char *secondArg = NULL;
		char *thirdArg = NULL;
		char *serverMessage = NULL;
		char *user = NULL;
		int i = 0;
		size = recv(client->connfd, messageIn, 1024, 0);
		messageIn[size] = '\0';
		while(messageIn[i] != '\0') /* strip newline */
		{
			if(messageIn[i] == '\n')
			{
				messageIn[i] = '\0';
			}
			i++;
		}
		printf("%d: %s\n", client->userID, messageIn);
		firstArg = strtok(messageIn, " ");
		if(strcmp(firstArg, "!quit") == 0)
		{
			userLeftMessage(client->userID);
			numUsersConnected--;
			break;
		}
		else if(strcmp(firstArg, "!help") == 0)
		{
			clientHelpMenu(client->connfd);
		}
		else if(strcmp(firstArg, "!msg") == 0)
		{
			char messageOut[1024];
			strcat(messageOut, client->username);
			strcat(messageOut, ": ");
			secondArg = strtok(NULL, " ");
			thirdArg = strtok(NULL, "");
			strcat(messageOut, thirdArg);
			pthread_mutex_lock(&clients_list);
			for(int i = 0; i < numUsersConnected; i++)
			{
				user = clients[i]->username;
				if(strcmp(secondArg, user) == 0)
				{
					send(clients[i]->connfd, messageOut, 1024, 0);
					printf("message sent\n");
				}
			}
			pthread_mutex_unlock(&clients_list);
			continue;
		}
		else if(strcmp(firstArg, "!who") == 0)
		{
			pthread_mutex_lock(&clients_list);
			for(int i = 0; i < numUsersConnected; i++)
			{
				if(i != client->userID)
				send(client->connfd, clients[i]->username, 1024, 0);
			}
			pthread_mutex_unlock(&clients_list);
			continue;
		}
		else
		{
			serverMessage = "unknown command";
			send(client->connfd, serverMessage, 1024, 0);	
		}
	}
	close(client->connfd);
	free(client);
	pthread_detach(pthread_self());
	return NULL;
}
void clientHelpMenu(int connfd)
{
	char *helpMessage = "Welcome! Begin by choosing a username. Type \"!name\" to select a username. Type \"!help\" for to repeat this help menu. Try \"who\" to see a list of users online. Every user must have a unique username. To send a message type \"!msg\" followed by the user you would like to message, then followed by your message like this: \"!msg Bob Hey Bob!\". To quit type \"!quit\"";
	send(connfd, helpMessage, 1024, 0);
}
void setUsername(int connfd, int userID)
{
	pthread_mutex_lock(&clients_list);
	char *messageOut = "Input a unique username before you can begin chatting";
	int i = 0;
	send(connfd, messageOut, 1024, 0);	
	char messageIn[1024] = "";
	recv(connfd, messageIn, 1024, 0);
	while(1)
	{
		if(strcmp(clients[i]->username, messageIn) ==0)
		{
			send(connfd, messageOut, 1024, 0);
			recv(connfd, messageIn, 1024, 0);
			i = 0;
			continue;
		}
		if(i == numUsersConnected -1)
		{
			break;
		}
		i++;
	}
	printf("messageIn: %s userID: %d\n", messageIn, userID);
	strcpy(clients[userID]->username, messageIn);
	pthread_mutex_unlock(&clients_list);
}
void userJoinedMessage(int userID)
{
	pthread_mutex_lock(&clients_list);
	char messageOut[1024];
	strcpy(messageOut, clients[userID]->username);
	strcat(messageOut, " has joined!");
	for(int i = 0; i < numUsersConnected; i++)
	{
		send(clients[i]->connfd, messageOut, 1024, 0);
	}
	pthread_mutex_unlock(&clients_list);
}
void userLeftMessage(int userID)
{
	pthread_mutex_lock(&clients_list);
	char messageOut[1024];
	strcpy(messageOut, clients[userID]->username);
	strcat(messageOut, " has left.");
	for(int i = 0; i < numUsersConnected; i++)
	{
		send(clients[i]->connfd, messageOut, 1024, 0);
	}
	pthread_mutex_unlock(&clients_list);
}
