#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

void doprocessing (int sock,int port);

int main( int argc, char *argv[] ) {
   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int n, pid;
   FILE * output;
   output = fopen("node.txt","w");
   fclose(output);  


   if (argc < 2) {
      fprintf(stderr,"usage %s port\n", argv[0]);
      exit(0);
   }
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = atoi(argv[1]);
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
   
   /* Now start listening for the clients, here
      * process will go in sleep mode and will wait
      * for the incoming connection
   */
   
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
  printf("Server started, now listening....\n");
   
   while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }
            
      /* Create child process */
      pid = fork();
		
      if (pid < 0) {
         perror("ERROR on fork");
         exit(1);
      }
      
      if (pid == 0) {
         /* This is the client process */
         close(sockfd);
    
    //processing-------------------------------------------------     
   int n,flag=0,port=ntohs(cli_addr.sin_port)+200,sock=newsockfd;
   char buffer[256];
   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   if(strcmp(buffer,"REQUEST : node")==0)
      flag=1;
   else if(strcmp(buffer,"REQUEST : client")==0)
      flag=2;

   
   printf("Received Message - %s\n",buffer);
   
   if(flag==1){
	      printf("Received port from client on which it will listen - %d\n",port);
	 /* Store the IP address and port number of the clients------------------------------------------------- */
         char clntName[INET_ADDRSTRLEN];
		if(inet_ntop(AF_INET,&cli_addr.sin_addr.s_addr,clntName,sizeof(clntName))!=NULL){
			output = fopen("node.txt","a+");  
			fprintf(output,"%s%c%d\n",clntName,' ',port);  
			fclose(output);
		} else {
			printf("Unable to get address\n"); 
		} 
	   
	char* resp="RESPONSE : Node: 1,";
	char buffer[50];
	sprintf(buffer,"%s %d",resp, port);
	printf("sending the following message - %s\n",buffer);
	n = write(sock,buffer,strlen(buffer));
	if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   }//if node
   
   else if(flag==2){
	char* resp="RESPONSE : client: 1";
	n = write(sock,resp,strlen(resp));
	
	n = read(sock,buffer,255);
	if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   printf("Received Message from client - %s\n",buffer);

   if(strcmp(buffer,"REQUEST : peer info")==0){
	//read the file that stored the peer info
	FILE *f = fopen("node.txt", "rb");
	fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);//send the pointer to beginning of the file

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    printf("Server has the following info:\n%s",string);
    
    //send the info to client
	n = write(sock,string,strlen(string));
	if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
	}//n
	}//if peero info
   }// if client
   
   else printf("ERROR : Unknown REQEST message, no action taken\n");
         
        // doprocessing(newsockfd,ntohs(cli_addr.sin_port)+200);
         exit(0);
      }
      else {
         close(newsockfd);
      }
		
   } /* end of while */
}

void doprocessing (int sock,int port) {
   int n,flag=0;
   char buffer[256];
   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   if(strcmp(buffer,"REQUEST : node")==0)
      flag=1;
   else if(strcmp(buffer,"REQUEST : client")==0)
      flag=2;

   
   printf("Received Message from client - %s\n",buffer);
   printf("Received port from client on which it will listen - %d\n",port);
   if(flag==1){
	char* resp="RESPONSE : Node: 1,";
	char buffer[50];
	sprintf(buffer,"%s %d",resp, port);
	printf("sending the following message - %s\n",buffer);
	n = write(sock,buffer,strlen(buffer));
	if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   }
   
   else if(flag==2){
	char* resp="RESPONSE : client: 1";
	n = write(sock,resp,strlen(resp));
	
	n = read(sock,buffer,255);
	if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   printf("Received Message from client - %s\n",buffer);

   if(strcmp(buffer,"REQUEST : peer info")==0){
	//read the file that stored the peer info
	FILE *f = fopen("node.txt", "rb");
	fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);//send the pointer to beginning of the file

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    printf("Server has the following info:\n%s",string);
    
    //send the info to client
	n = write(sock,string,strlen(string));
	if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
	}//n
	}//if peero info
   }// if client
   
   else printf("ERROR : Unknown REQEST message, no action taken\n");
   	
}
