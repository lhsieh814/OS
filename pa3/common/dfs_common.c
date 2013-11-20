#include "common/dfs_common.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>

/**
 * create a thread and activate it
 * entry_point - the function exeucted by the thread
 * args - argument of the function
 * return the handler of the thread
 */
inline pthread_t * create_thread(void * (*entry_point)(void*), void *args)
{
	//TODO: create the thread and run it
	pthread_t * thread;
	thread = malloc(sizeof(pthread_t));
	pthread_create(&thread, NULL, entry_point, args);

	return thread;
}

/**
 * create a socket and return
 */
int create_tcp_socket()
{
	//TODO:create the socket and return the file descriptor 
	return socket(AF_INET, SOCK_STREAM, 0);
}

/**
 * create the socket and connect it to the destination address
 * return the socket fd
 */
int create_client_tcp_socket(char* address, int port)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
   	int sockfd;
	struct in_addr addr1;
	struct in_addr addr2;

	assert(port >= 0 && port < 65536);
	sockfd = create_tcp_socket();
	if (sockfd == INVALID_SOCKET) {
		printf("ERROR client tcp socket invalid\n");
		return -1;
	}

	//TODO: connect it to the destination port
    server = gethostbyname(address);
    if (server == NULL) {
    	printf("ERROR creating client tcp socket\n");
    	return -1;
    }

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(address);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
       printf("\n Error : Client Connection Failed \n");
       return -1;
    } 

	return sockfd;
}

/**
 * create a socket listening on the certain local port and return
 */
int create_server_tcp_socket(int port)
{
    struct sockaddr_in serv_addr; 

	assert(port >= 0 && port < 65536);
	int sockfd = create_tcp_socket();
	if (sockfd == INVALID_SOCKET) return 1;

	//TODO: listen on local port
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port); 

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    	printf("ERROR create server tcp socket\n");
    	return -1;
    } 

    listen(sockfd,5);

	return sockfd;
}

/**
 * socket - connecting socket
 * data - the buffer containing the data
 * size - the size of buffer, in byte
 */
void send_data(int socket, void* data, int size)
{
	assert(data != NULL);
	assert(size >= 0);
	if (socket == INVALID_SOCKET) return;

	//TODO: send data through socket
	int n = write(socket, data, size);
	if (n < 0) {
		printf("ERROR sending data\n");
	} else {
		printf("Sent data\n");
	}
}

/**
 * receive data via socket
 * socket - the connecting socket
 * data - the buffer to store the data
 * size - the size of buffer in byte
 */
void receive_data(int socket, void* data, int size)
{
	assert(data != NULL);
	assert(size >= 0);
	if (socket == INVALID_SOCKET) return;
	
	//TODO: fetch data via socket
	int n = read(socket, data, size);
	if (n < 0) {
		printf("ERROR receiving data\n");
	} else {
		printf("Received data\n");
	}
}
