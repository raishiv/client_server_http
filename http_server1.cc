#include "minet_socket.h"
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

#define BUFSIZE 1024
#define FILENAMESIZE 100
#define FILEPATH "/home/agn253/eecs340/minet-netclass-w15/bin/"

int handle_connection(int);
int writenbytes(int,char *,int);
int readnbytes(int,char *,int);

int main(int argc,char *argv[])
{
    int server_port;
    int sock,sock2;
    struct sockaddr_in sa,sa2;
    int rc;
    int yes=1;
    
    /* parse command line args */
    if (argc != 3)
    {
        fprintf(stderr, "usage: http_server1 k|u port\n");
        exit(-1);
    }
    server_port = atoi(argv[2]);
    if (server_port < 1500)
    {
        fprintf(stderr,"INVALID PORT NUMBER: %d; can't be < 1500\n",server_port);
        exit(-1);
    }
    
    /* initialize and make socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        perror("ERROR opening socket");
    
    /* A special error handler for "address already in use" */
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("setsockopt");
        exit(1);
    }
    
    /* set server address*/
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(server_port);
    memset(&(sa.sin_zero),'\0',8);
    
    /* bind listening socket */
    if(bind(sock,(struct sockaddr *)&sa, sizeof(sa)) == -1){
        perror("bind");
        exit(1);
    }
    
    /* start listening */
    if(listen(sock, 10) == -1){
        perror("listen");
        exit(1);
    }
    
    /* connection handling loop */
    while(1)
    {
        /* handle connections */
        int addrlen = sizeof(sa2);
        printf( "we are here in handler\n");
        sock2 = accept(sock,(struct sockaddr*)&sa2,(socklen_t *)&addrlen);
        printf("selectserver: new connection from %s on "
               "socket %d\n", inet_ntoa(sa2.sin_addr), sock2);
        rc = handle_connection(sock2);
        
    }
    
}

int handle_connection(int sock2)
{
    char filename[FILENAMESIZE+1];
    int rc;
    int fd;
    struct stat filestat;
    char buf[BUFSIZE+1];
    char response_buf[BUFSIZE+1];
    char *headers;
    char *endheaders;
    char *bptr;
    char file_name[BUFSIZE];
    char fileContent[BUFSIZE];
    int datalen=0;
    int filename_len=0;
    int headSize = 0;
    int n=0;
    int response=0;
    char *ok_response_f = "HTTP/1.0 200 OK\r\n"\
    "Content-type: text/plain\r\n"\
    "Content-length: %d \r\n\r\n";
    char ok_response[100];
    char *notok_response = "HTTP/1.0 404 FILE NOT FOUND\r\n"\
    "Content-type: text/html\r\n\r\n"\
    "<html><body bgColor=black text=white>\n"\
    "<h2>404 FILE NOT FOUND</h2>\n"
    "</body></html>\n";
    bool ok=true;
    printf( "we are here in handler 1\n");
    /* first read loop -- get request and headers*/
    
   	rc = recv(sock2, buf, BUFSIZE,0);
   if(rc == -1){
      perror("empty message");
      exit(0);
   }
   buf[BUFSIZE+1] = '\0';
    
   for(int i = 0; i < BUFSIZE-3; i++){
      if((buf[i] == '\r') && (buf[i+1] == '\n') && (buf[i+2] == '\r') && (buf[i+3] == '\n')){
         headSize = i+3;
         break;
       }
    }
    printf("Message Recieved : %c", buf[0]);
    for(int j = 1; j < headSize; j++){
        printf("%c", buf[j]);        
    }   
    printf("\n");
//    send(sock2, buf,BUFSIZE ,0);
    
    /* parse request to get file name */
    if((buf[0] == 'G') && (buf[1] == 'E') && (buf[2] == 'T') && (buf[3] == ' ') && (buf[4] == '\/')){
      for(int i = 5; i < headSize; i++){
         if(buf[i]==' '){
            file_name[filename_len] = '\0';
            break;
         }
         file_name[filename_len]= buf[i];
         filename_len++;
         if (filename_len > FILENAMESIZE){
            printf("\nFilename greater than acceptable length.");
            ok=false;
         }
       }
    }
    
    if (stat(file_name,&filestat)==-1){
        perror("stat");
        ok=false;
    }
    
    datalen=filestat.st_size;
    
    if(ok){
        n=sprintf(ok_response,ok_response_f,datalen);
        send(sock2, ok_response, strlen(ok_response) ,0);
       
        std::ifstream fileObj (file_name);
        response=datalen;

        if (fileObj.is_open()){
           while(response>0){
               if (response < BUFSIZE) {
                   fileObj.read(fileContent, response);
                   send(sock2, fileContent, response ,0);
                   response = 0;
                   
               }
               else{
                fileObj.read(fileContent, BUFSIZE-1);
                send(sock2, fileContent, BUFSIZE-1 ,0);
                response=response-BUFSIZE+1;
               }
            }
	       fileObj.close();
        }

    }else{
        send(sock2, notok_response, strlen(notok_response) ,0);
    }  
            /* send response */
    
    /* Assumption: this is a GET request and filename contains no spaces*/
    
    /* try opening the file */
        close(sock2);
        if (ok)
            return 0;
        else
            return -1;
}
        
	int readnbytes(int fd,char *buf,int size)
        {
            int rc = 0;
            int totalread = 0;
            while ((rc = minet_read(fd,buf+totalread,size-totalread)) > 0)
                totalread += rc;
            
            if (rc < 0)
            {
                return -1;
            }
            else
                return totalread;
        }
        
        int writenbytes(int fd,char *str,int size)
        {
            int rc = 0;
            int totalwritten =0;
            while ((rc = minet_write(fd,str+totalwritten,size-totalwritten)) > 0)
                totalwritten += rc;
            
            if (rc < 0)
                return -1;
            else
                return totalwritten;
        }
        
        

