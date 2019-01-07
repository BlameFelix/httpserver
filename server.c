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
struct response {
	char header[BUF_LEN];
	char content[BUF_LEN];
};
struct response getFile(char *fileName) {
	FILE *f;
	char file[BUF_LEN];
	struct response res;
	char path[60];//, file[BUF_LEN];
	int c;
	strcpy(path, "./var");
	strcat(path, fileName);
	printf("%s\n", fileName);
	//pfad returnen
	if((f = fopen(path, "r"))==NULL) {
		strcpy(res.header,"HTTP/1.0 404 Not Found\r\n");
		strcpy(res.content, "<html><body><b>Kaputt</b><br>ich bin kaputt</body></html>");
		return res;
	}
	int cnt = 0;
	while((c = fgetc(f)) != EOF) {
		file[cnt] = c;
		cnt++;
	}
	strcpy(res.header,"HTTP/1.0 200 OK\r\nContnt-type:text/html\r\n\r\n");
	strcpy(res.content, file);
	fclose(f);
	return res;
	//return file;
};
struct response getResponde(char *msg) {
	/*int count;
	char *parts;
	parts = strtok(msg, " ");
	while(parts != NULcL) {
		parts = strtok(msg, " ");
		printf("%s\n", parts);

	}
		printf("%s\n", parts[0]);
		printf("%s\n", parts[1]);
		printf("%s\n", parts[2]);*/
	struct response res;
	char method[100], version[100], fileName[100];// version[100];
	sscanf(msg, "%s %s %s", method, fileName, version);
	printf("Nachricht: %s %s %s\n", method, fileName, version);
	//getFile(fileName);
	char z[BUF_LEN];
	if(strcmp("GET", method)==0) {
		res = getFile(fileName);
		return res;
		//return fileName;//z;
//"HTTP/1.0 200 OK\r\n\r\n<html><body>Dies ist die eine Fake Seite  des Webservers!</body></html>\r\n";
	}
	return res;
	//return "HTTP/1.0 501 Not Implemented\r\nContent-type: text/html\r\n\r\n<html><body><b>501</b> Operation not supported</body></html>\r\n";
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
				strcpy(msgBuf, request);
			}
			zeile=false;
			printf("%s",request);
		}
		//rename msgBuf
		printf("zeile: %s", msgBuf);
		//rename errRestponde
		//hier array erstellen und übergeben an funktion. evtl dann hier getFIle()	
		struct response r;
		char fileName[BUF_LEN];
		r=getResponde(msgBuf);
		char responde[BUF_LEN];
		//getFile(fileName, (char *) responde);
		strcpy(responde, r.header);
		strcat(responde, r.content);
		//sprintf
		//printing the message out
		//printf("Recived message: %s", msgBuf);

		//sending the response
		if((send(connfd, responde, strlen(responde), 0))==-1) {
			sysErr("Server Fault: send message", -6);
		}

		//clearing the buffer
		memset(&responde,0,BUF_LEN);
		//closing the connection
		close (connfd);
		}
	//closing listensocket
	close (listenfd);
	return 0;
}
