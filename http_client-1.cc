#include "minet_socket.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define BUFSIZE 1024

int write_n_bytes(int fd, char * buf, int count);
void error(char *msg);

int main(int argc, char * argv[]) {
    char * server_name = NULL;
    int server_port = 0;
    char * server_path = NULL;

	int n;
    int sock = 0;
    int rc = -1;
    int datalen = 0;
    bool ok = true;
    struct sockaddr_in sa;
    FILE * wheretoprint = stdout;
    struct hostent * site = NULL;
    char * req = NULL;

    char buf[BUFSIZE + 1];
    char * bptr = NULL;
    char * bptr2 = NULL;
    char * endheaders = NULL;
   
    struct timeval timeout;
    fd_set set;

    /*parse args */
    if (argc != 5) {
	fprintf(stderr, "usage: http_client k|u server port path\n");
	exit(-1);
    }

    server_name = argv[2];
    server_port = atoi(argv[3]);
    server_path = argv[4];



    /* initialize minet */
    if (toupper(*(argv[1])) == 'K') { 
	minet_init(MINET_KERNEL);
    } else if (toupper(*(argv[1])) == 'U') { 
	minet_init(MINET_USER);
    } else {
	fprintf(stderr, "First argument must be k or u\n");
	exit(-1);
    }

    /* create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) 
        error("ERROR opening socket");

    // Do DNS lookup
    /* Hint: use gethostbyname() */
	site = gethostbyname(server_name);
    if (site == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", server_name);
        exit(0);
	}
    /* set address */
	bzero((char *) &sa, sizeof(sa));
    sa.sin_family = AF_INET;
    bcopy((char *)site->h_addr, 
	  (char *)&sa.sin_addr.s_addr, site->h_length);
    sa.sin_port = htons(server_port);

    /* connect socket */
    if (connect(sock, (struct sockaddr*) &sa, sizeof(sa)) < 0) 
      error("ERROR connecting");
   
      fd_set read_fds;
   // fd_set write_fds;
    int fdmax = 0;
    int count = 1;
    char* respCode = NULL;
    respCode=(char*)malloc(3);
    fdmax = (fdmax>sock)? fdmax: sock;
   
      /* send request */
        bzero(buf, BUFSIZE);
    	// fgets(buf, BUFSIZE, stdin); // replace with GET request
	char getMethod[] = "GET ";
	char httpVersion[] = " HTTP\/1.0\r\n";
	char endRequest[] = "\r\n";
	strcat(getMethod, server_path);
	strcat(getMethod, httpVersion);
	strcat(getMethod, endRequest);
	//strcpy(req, getMethod);
	if(strlen(getMethod) > BUFSIZE){
	  error("ERROR request greater than buffer");
	  exit(1);
	}
	/* send the message line to the server */
	strcpy(buf, getMethod);
	n = write(sock, buf, strlen(buf));
	if (n < 0) { 
	  error("ERROR writing to socket");
	  exit(1);
	}

	for(;;) {
       /* wait till socket can be read */
       /* Hint: use select(), and ignore timeout for now. */
      
	FD_SET(sock, &read_fds);
	if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
       	   perror("select");
       	   exit(1);
	  }
	if(FD_ISSET(sock, &read_fds))
	{
        	bzero(buf, BUFSIZE);
    		n = recv(sock, buf, BUFSIZE,0);
    		if (n <0) { 
    	  	error("ERROR reading from socket");
	  	break;
		}
		else if(n==0) {
    	  	error("Server connection closed.");
		break;
		}
		if (count > 0){
      /* first read loop -- read headers */
    
      /* examine return code */   
      //Skip "HTTP/1.0"
      //remove the '\0'
    // Normal reply has return code 200

    /* print first part of response */

    /* second read loop -- print out the rest of the response */

	        	strncpy(respCode, &buf[9], 3);
			respCode[3]='\0';
			count = 0;
			//printf("\n%s", respCode);
			if (atoi(respCode)!= 200){
			printf("%s", buf);
			ok = false;
			rc = -1;
			}else{
			rc = 0;
			//printf("\nLine 4 %d", ok);
			}
		}else{
    		printf("%s", buf);
	      }     
	  }	
      }   
    /*close socket and deinitialize */
  
    //if (ok) {
       close(sock);
	return rc;
   /* } else {
       close(sock);
	return rc;
    }*/
 
}

int write_n_bytes(int fd, char * buf, int count) {
    int rc = 0;
    int totalwritten = 0;

    while ((rc = minet_write(fd, buf + totalwritten, count - totalwritten)) > 0) {
	totalwritten += rc;
    }
    
    if (rc < 0) {
	return -1;
    } else {
	return totalwritten;
    }
}

void error(char *msg) {
    perror(msg);
    exit(0);
}
