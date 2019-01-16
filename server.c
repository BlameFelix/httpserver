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
#include <signal.h>

#define BUF_LEN 128
struct response {
	char header[BUF_LEN];
	char content[BUF_LEN];
};
struct response error404() {
	struct response res;
	strcpy(res.header,"HTTP/1.0 404 Not Found\r\n\r\n");
	strcpy(res.content, "<html><body><b>404</b><br>Kein File gefunden</body></html>");
	return res;
}
struct response error501() {
	struct response res;
	strcpy(res.header,"HTTP/1.0 501 Not Implemented\r\n\r\n");
	strcpy(res.content, "<html><body><b>501</b><br>Operation not supportet</body></html>");
	return res;
}
struct response getFile(char *fileName) {
	FILE *f;
	char file[BUF_LEN];
	struct response res;
	char path[60];
	int c;
	strcpy(path, "./var");
	strcat(path, fileName);
	printf("%s\n", fileName);
	//pfad returnen
	if((f = fopen(path, "r"))==NULL) {
		return error404();
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
};
struct response getResponde(char *msg) {
	struct response res;
	char method[100], version[100], fileName[100];// version[100];
	sscanf(msg, "%s %s %s", method, fileName, version);
	printf("Nachricht: %s %s %s\n", method, fileName, version);
	if(strcmp(fileName, "/")==0) {
		return error404();
	}
	if(strncmp(fileName, "../", 3)==0) {
		return error404();
	}
	if(strcmp("GET", method)==0) {
		res = getFile(fileName);
		return res;
	}
	return error501();
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
	signal(SIGCHLD, SIG_IGN);
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
		pid_t pid;
		pid = fork();
		if(pid==0) {
			//close(listenfd);
			struct response r;
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
			close(connfd);
			exit(0);
		}
		else {
			close(connfd);
		}
	
	}
	//closing listensocket
	close (listenfd);
	return 0;
}
