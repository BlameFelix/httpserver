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
#include <signal.h>
#include "get_line.c"

#define BUF_LEN 10000
//struct to return the response
struct response {
        char header[BUF_LEN];
        char content[BUF_LEN];
};

//return 404 Not Found error
struct response error404() {
        struct response res;
        strcpy(res.header,"HTTP/1.0 404 Not Found\r\n\r\n");
        strcpy(res.content, "<html><body><b>404</b><br>Kein File gefunden</body>$
        return res;
}

//return 501 Not implemented error
struct response error501() {
        struct response res;
        strcpy(res.header,"HTTP/1.0 501 Not Implemented\r\n\r\n");
        strcpy(res.content, "<html><body><b>501</b><br>Operation not supportet</$
        return res;
}

//function to get the requested file content
struct response getFile(char *fileName) {
        FILE *f;
        char file[BUF_LEN];
        struct response res;
        char path[60];
        int c, cnt;

        //put the reuqested path together
        strcpy(path, "/var/microwww");
        strcat(path, fileName);

        //if file doesn't exist return 404
        if((f = fopen(path, "r"))==NULL) {
                return error404();
        }
	
	//parse the content of the file in a variable
        cnt = 0;
        while((c = fgetc(f)) != EOF) {
                file[cnt] = c;
                cnt++;
        }

        //copy the needed message in the struct
        strcpy(res.header,"HTTP/1.0 200 OK\r\nContnt-type:text/html\r\n\r\n");
        strcpy(res.content, file);
        fclose(f);
        return res;
};

//method to find out the requested filename
struct response getResponde(char *msg) {
        struct response res;
        char method[100], version[100], fileName[100];

        //fragmenting the line
	sscanf(msg, "%s %s %s", method, fileName, version);

        //if no file is requested, return file not found
        if(strcmp(fileName, "/")==0) {
                return error404();
        }

        //if the path tries to step back to system file not found
        if(strncmp(fileName, "../", 3)==0) {
                return error404();
        }

        //check if we have a implemented method and get the file
        if(strcmp("GET", method)==0) {
                res = getFile(fileName);
                return res;
        }

        //only GET is implementet
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
        //Checking if a port has been supplied
        if(argc<2){
                usage(argv[0]);
	}

        //initializing variables
        struct sockaddr_in server_addr, client_addr;
        struct response r;
        char rcvBuf[BUF_LEN], request[BUF_LEN], response[BUF_LEN];
        unsigned int len;
        int listenfd,connfd, len2;
        pid_t pid;
        bool zeile;

        //avoiding zombies
        signal(SIGCHLD, SIG_IGN);

        //creating TCP based listen socket
        listenfd= 0;
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
        if(bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr))==$
                sysErr("server fault: bind socket", -2);
        }

        //waiting for requests and setting up queue
        if(listen(listenfd,5)==-1) {
                sysErr("Server Fault: listen", -3);
        }
	//endless loop that accepts and handles client requests
        while(true) {
                connfd=0;
                //accepting next request in the queue and setting up connction s$
                if((connfd= accept(listenfd, (struct sockaddr*)&client_addr, &le$
                        sysErr("Server Fault: connect", -4);
                }

                //create child process
                pid = fork();
                if(pid==0) {
                        strcpy(request,"A");
                        len2=1;
                        zeile=true;
                        //request lines until the whole message is done
                        while ((len2>0) && strcmp("\n", request)) {
                                len2 = get_line(connfd, request, BUF_LEN-1);
                                if(zeile) {
                                        strcpy(rcvBuf, request);
				}
                                zeile=false;
                        }

                        close(listenfd);

                        //get the response struct
                        r=getResponde(rcvBuf);

                        //convert response struct into a char
                        strcpy(response, r.header);
                        strcat(response, r.content);

                        //sending the response
                        if((send(connfd, response, strlen(response), 0))==-1) {
                                sysErr("Server Fault: send message", -6);
                        }

                        //clearing the buffer
			            memset(&response,0,BUF_LEN);

                        //closing the connection
                        close(connfd);

                        exit(0);
                }
                //parent can continue
                else if (pid > 0) {
                        close(connfd);
                }
                else {
                        sysErr("server Fault: fork", -8);
                }

        }
        //closing listensocket
        close (listenfd);
        return 0;
}
