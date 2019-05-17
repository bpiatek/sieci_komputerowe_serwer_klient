#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>

#define PORT 20001
#define BACKLOG 5
#define LENGTH 512

void error(const char *msg) {
	perror(msg);
	exit(1);
}

int main() {
	/* Defining Variables */
	int sockfd;
	int nsockfd;
	int activity;
	int max_sd;
	int sd;
	int client_socket[20];
	int max_clients = 20;
	int num;
	int new_socket;
	int addrlen;
	int sin_size;
	struct sockaddr_in addr_local; /* client addr */
	struct sockaddr_in addr_remote; /* server addr */
	char revbuf[LENGTH]; // Receiver buffer

	//initialise all client_socket[] to 0 so not checked
	for (int i = 0; i < max_clients; i++) {
		client_socket[i] = 0;
	}


	/* Get the Socket file descriptor */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
		exit(1);
	} else
		printf("[Server] Obtaining socket descriptor successfully.\n");

	/* Fill the client socket address struct */
	addr_local.sin_family = AF_INET; // Protocol Family
	addr_local.sin_port = htons(PORT); // Port number
	addr_local.sin_addr.s_addr = INADDR_ANY; // AutoFill local address
	bzero(&(addr_local.sin_zero), 8); // Flush the rest of struct

	/* Bind a special Port */
	if (bind(sockfd, (struct sockaddr *) &addr_local, sizeof(struct sockaddr)) == -1) {
		fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
		exit(1);
	} else
		printf("[Server] Binded tcp port %d in addr 127.0.0.1 sucessfully.\n", PORT);


	//specyfy max connections
	if (listen(sockfd, 15) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//set of socket descriptors
	fd_set readfds;

	//accept incoming connection
	addrlen = sizeof(addr_local);

	int a = 0;
	int counter = 0;
	while (a == 0) {
		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(sockfd, &readfds);
		max_sd = sockfd;

		//add child sockets to set
		for (int i = 0; i < max_clients; i++) {
			//socket descriptor
			sd = client_socket[i];

			//if valid socket descriptor then add to read list
			if (sd > 0)
				FD_SET(sd, &readfds);

			//highest file descriptor number, need it for the select function
			if (sd > max_sd)
				max_sd = sd;
		}

		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR)) {
			printf("select error");
		}


		//If something happened on the master socket ,
		//then its an incoming connection
		if (FD_ISSET(sockfd, &readfds)) {
			if ((new_socket = accept(sockfd,
									 (struct sockaddr *) &addr_local, (socklen_t *) &addrlen)) < 0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket,
				   inet_ntoa(addr_local.sin_addr), ntohs
				   (addr_local.sin_port));


			char buff[20];
			sprintf(buff, "from_client_%d", counter);

			char *fr_name = buff;
			FILE *fr = fopen(fr_name, "a");

			counter = counter + 1;

			if (fr == NULL) {
				printf("File %s Cannot be opened file on server.\n", fr_name);
			} else {

				bzero(revbuf, LENGTH);
				int fr_block_sz = 0;
				while ((fr_block_sz = recv(new_socket, revbuf, LENGTH, 0)) > 0) {
					int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);

					if (write_sz < fr_block_sz) {
						error("File write failed on server.\n");
					}
					bzero(revbuf, LENGTH);
					if (fr_block_sz != 512) {
						break;
					}
				}


				//if file empty or path not specified then delete created file
				if(fr_block_sz == 0) {
					remove(buff);
				}

				if(fr_block_sz < 0)
				{
					if (errno == EAGAIN)
					{
						printf("recv() timed out.\n");
					}
					else
					{
						fprintf(stderr, "recv() failed due to errno = %d\n", errno);
						exit(1);
					}
				}

				if(fr_block_sz > 0) {
					printf("Ok file received from client!\n");
				}
				fclose(fr);
			}
			close(new_socket);
			printf("socket %d closed!\n", new_socket);
		}

	}
}