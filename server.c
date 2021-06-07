/* Spencer McAvey's Chat Server */
/* To compile: gcc server.c -o server -pthread */
/* To run: ./server */
/* Server relays all messages from clients to one another */

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
		perror("socket bind failure");
		return ERROR;
	}
	/* listen for connections on socket */
	if(listen(listenfd, 10) < 0)
	{
		perror("socket listen failure");
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
		client_t *client = (client_t *)malloc(sizeof(client_t)); /* add client to list of client structs and malloc */
		client->address = client_address;
		client->connfd = connfd;
		strcpy(client->username, "temp");
		client->userID = userID;

		printf("User %d connected\n", client->userID);
		pthread_mutex_lock(&clients_list);
		clients[i] = client;
		printf("clients[i]->userID: %d clients[i]->connfd: %d\n", clients[i]->userID, clients[i]->connfd); /* print client info to server */
		pthread_mutex_unlock(&clients_list);
		printf("ID: %d CONNFD: %d\n", clients[i]->userID, clients[i]->connfd); /* print more client info to server */
		i++;
		userID++;
		numUsersConnected++;
		pthread_create(&thread_id, NULL, clientInterface, (void*)client); /* create a thread to receive messages and commands from each unique client */
	}
	return 0;
}

void *clientInterface(void *arg)
{
	client_t *client = (client_t*)arg;
	clientHelpMenu(client->connfd); /* send client helpful message about utilizing the SMcAvey Instant Messenger */
	setUsername(client->connfd, client->userID); /* get a unique username from the client */
	userJoinedMessage(client->userID); /* send a message to every user the a new user has joined */
	while(1)
	{
		char messageIn[1024]; /* message inbound from client containing commands and arguments dependent upon command type */
		int size;
		char *firstArg = NULL; /* the command part of the client's command line being sent to server */
		char *secondArg = NULL; /* argument following first argument if applicable */
		char *thirdArg = NULL; /* argument following second argument if applicable */
		char *errorMessage = NULL; /* error message to be sent to client in the event of certain caught errors */
		char *user = NULL;
		int i = 0;
		size = recv(client->connfd, messageIn, 1024, 0); /* receive message from client and strip newline and append null terminator */
		messageIn[size] = '\0';
		while(messageIn[i] != '\0') /* strip newline */
		{
			if(messageIn[i] == '\n')
			{
				messageIn[i] = '\0';
			}
			i++;
		}
		printf("%d: %s\n", client->userID, messageIn); /* server printout to display client activity */
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
			char messageOut[1024] = ""; /* message to be sent to user */
			strcat(messageOut, client->username); /* formatting message in form of: sender's name: message here */
			strcat(messageOut, ": ");
			secondArg = strtok(NULL, " "); /* recipient */
			thirdArg = strtok(NULL, ""); /* message */
			strcat(messageOut, thirdArg);
			pthread_mutex_lock(&clients_list);
			for(int i = 0; i < numUsersConnected; i++)
			{
				user = clients[i]->username;
				if(strcmp(secondArg, user) == 0)
				{
					send(clients[i]->connfd, messageOut, 1024, 0); /* send message from sender to recipient */
					printf("message sent\n"); /* server printout to confirm successful message traffic */
					break;
				}
				if(i == numUsersConnected - 1)
				{
					char errorMessage[1024] = "Invalid user...";
					send(client->connfd, errorMessage, 1024, 0);
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
		else if(strcmp(firstArg, "!msgall") == 0)
		{
			char messageOut[1024] = ""; /* message to be sent to all users */
			secondArg = strtok(NULL, ""); /* message */
			strcat(messageOut, client->username); /* message formatting */
			strcat(messageOut, ": ");
			strcat(messageOut, secondArg);
			for(int i = 0; i < numUsersConnected; i++)
			{
				if(clients[i]->connfd == client->connfd) /* don't send message to client's self */
				{
					continue;
				}
				else
				{
					send(clients[i]->connfd, messageOut, 1024, 0); /* send message to all but sender */
				}
			}
		}
		else
		{
			errorMessage = "unknown command";
			send(client->connfd, errorMessage, 1024, 0);	
		}
	}
	close(client->connfd); /* close connection */
	free(client); /* free memory */
	pthread_detach(pthread_self()); /* kill thread */
	return NULL;
}
void clientHelpMenu(int connfd)
{
	char *helpMessage = "Welcome to the SMcAvey Instant Messenger! Type \"!help\" for to repeat this help menu. Try \"who\" to see a list of users online. Every user must have a unique username. To send a message type \"!msg\" followed by the user you would like to message, then followed by your message like this: \"!msg Bob Hey Bob!\". To quit type \"!quit\". Before you can begin, you will have to establish your username. That will happen now.";
	send(connfd, helpMessage, 1024, 0);
}
void setUsername(int connfd, int userID)
{
	pthread_mutex_lock(&clients_list);
	char *messageOut = "Input a unique username before you can begin chatting"; /* message to be sent to user */
	int i = 0;
	send(connfd, messageOut, 1024, 0);	
	char messageIn[1024] = ""; /* username to be received from user */
	recv(connfd, messageIn, 1024, 0);
	while(1)
	{
		if(strcmp(clients[i]->username, messageIn) ==0) /* check username against all other usernames for uniqueness */
		{
			send(connfd, messageOut, 1024, 0); /* prompt the user again to enter a unique username */
			recv(connfd, messageIn, 1024, 0); /* receive user's username */
			i = 0; /* reset the loop counter so we can check all users again for uniqueness */
			continue;
		}
		if(i == numUsersConnected -1) /* if we have checked against every other user and the name is therefore unique, break */
		{
			break;
		}
		i++;
	}
	printf("messageIn: %s userID: %d\n", messageIn, userID);
	strcpy(clients[userID]->username, messageIn); /* assign the unique username to this user */
	pthread_mutex_unlock(&clients_list);
}
void userJoinedMessage(int userID)
{
	pthread_mutex_lock(&clients_list);
	char messageOut[1024]; /* message to be sent to user */
	strcpy(messageOut, clients[userID]->username);
	strcat(messageOut, " has joined!");
	for(int i = 0; i < numUsersConnected; i++) /* send a message to all users that a new user has joined */
	{
		send(clients[i]->connfd, messageOut, 1024, 0);
	}
	pthread_mutex_unlock(&clients_list);
}
void userLeftMessage(int userID)
{
	pthread_mutex_lock(&clients_list);
	char messageOut[1024]; /* message to be sent to user */
	strcpy(messageOut, clients[userID]->username);
	strcat(messageOut, " has left.");
	for(int i = 0; i < numUsersConnected; i++)
	{
		send(clients[i]->connfd, messageOut, 1024, 0); /* send a message to all users that a user has left */
	}
	pthread_mutex_unlock(&clients_list);
}
