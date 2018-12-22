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
#include "get_line.c"

#define BUF_LEN 128

char* getResponde(char *msg) {
	int count;
	char parts[BUF_LEN];	
	parts = strtok(msg, " ");
	while(parts != NULL) {
		parts = strtok(msg, " ");
		printf("%s\n", parts);

	}
		printf("%s\n", parts[0]);
		printf("%s\n", parts[1]);
		printf("%s\n", parts[2]);
	if(strncmp("GET", msg, 3)==0) {
		return "HTTP/1.0 200 OK\r\n\r\n<html><body>Dies ist die eine Fake Seite  des Webservers!</body></html>\r\n";
	}	
	return "HTTP/1.0 501 Not Implemented\r\nContent-type: text/html\r\n\r\n<html><body><b>501</b> Operation not supported</body></html>\r\n";
}
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

		/*
		//reciving the message of the client and saving the length
		if((recv(connfd, msgBuf, BUF_LEN, 0))==-1) {
			sysErr("Server Fault: recive message", -5);
		}*/

		char request[BUF_LEN]="A";
		int len=1;
		bool zeile=true;
		while ((len>0) && strcmp("\n", request)) {
			len = get_line(connfd, request, BUF_LEN-1);
			if(zeile) {			
				strcat(msgBuf, request);
			}
			zeile=false;			
			printf("%s",request);
		}
		//rename msgBuf		
		printf("zeile: %s", msgBuf);
		//rename errRestponde
		char *errResponde = getResponde(msgBuf);
		
		//printing the message out
		//printf("Recived message: %s", msgBuf);
		
		//sending the response
		if((send(connfd, errResponde, strlen(errResponde), 0))==-1) {
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
