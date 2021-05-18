/* Spencer McAvey's Chat Client */

#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ERROR -1
#define OK 0
#define SERVER_PORT 5555

char messageOut[024]; /* message outbound from client */
char messageIn[1024]; /* message inbound to client */

int socketfd = 0;

void receiveHelpMessage();

int main(int argc, char **argv)
{
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
	receiveHelpMessage();
	do
	{
		fgets(messageOut, sizeof(messageOut), stdin); /* get user input from keyboard */
		messageOut[strcspn(messageOut, "\n")] = 0; /* strip trailing new line */
/*		list of commands to implement:
			see who else is online
			set your username
			quit
			send someone a message
*/
	}
	while(strcmp(messageOut, "!quit") != 0);
}
void receiveHelpMessage()
{
	read(socketfd, messageIn, sizeof(messageIn));
	printf("%s", messageIn);
}
