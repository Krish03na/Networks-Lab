#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int startserver(char * port);


int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   char buffer[256];
   
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }
	
   portno = atoi(argv[2]);
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
   
   /* Now ask for a message from the user, this message
      * will be read by server
   */
	
   printf("Connecting to the relay server.Sending Request message\n");
   char* req="REQUEST : node";
   
   /* Send message to the server */
   n = write(sockfd, req, strlen(req));
   
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   
   /* Now read server response */
   bzero(buffer,256);
   n = read(sockfd, buffer, 255);
   
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
	
   printf("%s\n",buffer);
   
   //start server if node accepted by relay
   if(buffer[17]=='1'){
	   printf("RESPONSE : Node accepted\nSUCESSFULLY connected\n");
	   //close the connection gracefully
	   printf("Closing the connection gracefully...\n");
	   n=shutdown(sockfd,0);
	    if (n < 0) {
		perror("ERROR closing the connection");
		exit(1);
		}
	   //start the server
	   printf("port no. of the client for listening is : %s\n",&buffer[20]);
	   startserver(&buffer[20]);
   }
   else printf("Node not accepted by the relay server, try again..\n");
   
   return 0;
}

//change this
int startserver(char * port)
{
     int sockfd, newsockfd,portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(port); /* +1 because getting segmentation fault otherwise*/
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     printf("Server running on peer node, listening.....\n");
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     close(sockfd);
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Got a message from peer, Here is the message: %s\n",buffer);
     
     //check if the message is a request for file
     char a[]="REQUEST : FILE : ";
     int i,flag=0;
     for(i=0;i<strlen(a);i++){
		if(a[i]==buffer[i]) flag=1; 
		else {
			flag=0;
			break;
			}
	}
     if(flag){
     printf("received request for the file : %s\n",&buffer[strlen(a)]);
     FILE * file=fopen(&buffer[strlen(a)],"r");
     if(file==NULL){
		 printf("requested file NOT Found\n");
	 char response[]="File NOT FOUND";
     n = write(newsockfd,response,strlen(response));
     if (n < 0) error("ERROR writing to socket");
		}//if file not found
	else {
	  printf("Found the requested file \n");
	 char response[]="File FOUND";
     n = write(newsockfd,response,strlen(response));
     if (n < 0) error("ERROR writing to socket");
	
     //send the file
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);//send the pointer to beginning of the file

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, file);
    fclose(file);
    printf("file has the following content:\n%s",string);
    
    //send the content to client
	n = write(newsockfd,string,strlen(string));
	if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
	}//n
		}//if file is found
	}//if file request
	else {
		printf("received request is not of file name,closing connection......\n");
		close(newsockfd);
	}//if not file request
//inculde the option to keep the server alive even after the transfer? 
     return 0; 
}
