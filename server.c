/* Spencer McAvey's Chat Server */

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define ERROR -1
#define OK 0
#define SERVER_PORT 5555 /* server port */

typedef struct
{
	struct sockaddr_in address; /* client address */
	int connfd; /* connection file descriptor */
	char *username; /* client username */
} client_t;

int main(int argc, char **argv)
{
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
	printf("past socket bind");
	/* listen for connections on socket */
	if(listen(listenfd, 10) < 0)
	{
		perror("socket listening failed");
		return ERROR; 
	}
	printf("past socket listen");
	/* accept clients */
	while(1)
	{
		socklen_t client_length = sizeof(client_address);
		connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_length);


		/* add client */
		client_t *client = (client_t *)malloc(sizeof(client_t));
		client->address = client_address;
		client->connfd = connfd;
		client->username = NULL;
	}
	return 0;
}
