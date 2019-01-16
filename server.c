#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_LEN 128

// Something unexpected happened. Report error and terminate.
void sysErr( char *msg, int exitCode ) {
	fprintf( stderr, "%s\n\t%s\n", msg, strerror( errno ) );
	exit( exitCode );
}

// Called with wrong arguments.
void usage( char *argv0 ) {
	printf( "usage : %s portnumber\n", argv0 );
	exit( 0 );
}

int main(int argc, char **argv) {
	//Checking if a port has been supplied
	if(argc<2){
		usage(argv[0]);
	}

	//initializing variables
	struct sockaddr_in server_addr, client_addr;
	char msgBuf[BUF_LEN];
	unsigned int len;

	//creating TCP based listen socket
	int listenfd= 0;
	if((listenfd= socket(AF_INET,SOCK_STREAM, 0)) == -1) {
		sysErr("server fault: create socket", -1);
	}

	//setting up socketadress of the server
	memset( &server_addr, 0, sizeof(server_addr));
	//accepting any IP-address
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//IPv4 based connection
	server_addr.sin_family      = AF_INET;
	//setting port number converted to network byte order
	server_addr.sin_port        = htons( (u_short)atoi( argv[ 1 ] ) );

	//bind socket to a port
	if(bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr))==-1) {
		sysErr("server fault: bind socket", -2);
	}

	//waiting for requests and setting up queue
	if(listen(listenfd,5)==-1) {
		sysErr("Server Fault: listen", -3);
	}

	//endless loop that accepts and handles client requests
	while(true) {
		//accepting next request in the queue and setting up connction socket
		int connfd=0;
		if((connfd= accept(listenfd, (struct sockaddr*)&client_addr, &len))==-1) {
			sysErr("Server Fault: connect", -4);
		}

		//reciving the message of the client and saving the length
		if((recv(connfd, msgBuf, BUF_LEN, 0))==-1) {
			sysErr("Server Fault: recive message", -5);
		}
		//printing the message out
		printf("Recived message: %s sending back....\n", msgBuf);
		//sending the message back
		if((send(connfd, msgBuf, strlen(msgBuf), 0))==-1) {
			sysErr("Server Fault: send message", -6);
		}
		//clearing the buffer
		memset(&msgBuf,0,BUF_LEN);
		//closing the connection
		close (connfd);
		}
	//closing listensocket
	close (listenfd);
	return 0;
}
