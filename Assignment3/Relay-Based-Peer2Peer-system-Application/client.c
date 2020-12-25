#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int getFile(int sockfd);
int connectpeer(char * address,int portno,char * filename);

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
   char* req="REQUEST : client";
   
   /* Send message to the server */
   n = fwrite(sockfd, req, strlen(req));
   
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   
   /* Now fread server response */
   bzero(buffer,256);
   n = (sockfd, buffer, 255);
   
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
	
   printf("%s\n",buffer);
   
   //start server if node accepted by relay
   if(buffer[19]=='1'){
	   printf("RESPONSE : client accepted\nSUCESSFULLY connected\nFetcing peer info\n");
	   n = getFile(sockfd);
		
   if (n < 0) {
      perror("ERROR getting the requested file from the peers");
      exit(1);
   }
}
   else printf("Node not accepted by the relay server, try again..\n");
   
   return 0;
}


int getFile(int sockfd){
	//request for active peer information
	char* req="REQUEST : peer info",buffer[256];
	int n;
   
   /* Send message to the server */
   n = fwrite(sockfd, req, strlen(req));
   
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   
   /* Now  server response */
   bzero(buffer,256);
   n = (sockfd, buffer, 255);
   
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   printf("Receive the following response - \n%s\n",buffer);
   printf("gracefully closing the connection with the relay server....\n");
   n=shutdown(sockfd,0);
	if (n < 0) {
	perror("ERROR closing the connection");
	exit(1);
	}
	//store the info in a file
	FILE * peers=fopen("peer.txt","w");
	fprintf(peers,"%s",buffer);
	fclose(peers);
      
   char  file[50];
   printf("Enter the File name : ");
   scanf("%s",file);
   //process the response one peer at a time and try to fetch the file
   char peerName[INET_ADDRSTRLEN];
   int port,flag=0;
   peers=fopen("peer.txt","r");
   while(fscanf(peers,"%s %d",peerName,&port)!=EOF){
	   printf("Connecting to the peer %s:%d...\n",peerName,port);
	   n = connectpeer(peerName,port,file);
	   if(n<0) continue;
	   else {flag=1;break;}//successfult found the file on this node
   }
   fclose(peers);
   if(!flag) printf("File not found on any node!\n");
   
	return 0;
}


int connectpeer(char * address,int portno,char * filename) {
   int sockfd, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   struct in_addr ipv4addr;
   char buffer[256];
        
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        perror("ERROR opening socket");
   
   inet_pton(AF_INET, address, &ipv4addr);
   server = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
   
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
      * will be  by server
   */
	
   printf("Connection to the Peer SUCCESSFUL.\nSending File transfer Request message with the file name....\n");
   char req[50];
   char* buff="REQUEST : FILE :";
	sprintf(req,"%s %s",buff, filename);
   
   /* Send message to the server */
   n = fwrite(sockfd, req, strlen(req));
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   
   /* Now read server response */
   bzero(buffer,256);
   n = freadsockfd, buffer, 255);
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   printf("received the reply :-%s\n",buffer);
   if(strcmp(buffer,"File NOT FOUND")==0){
	   //close the connection gracefully since file not found
	   printf("Closing the connection gracefully since file NOT FOUND on this node...\n");
	   n=shutdown(sockfd,0);
	   if (n < 0) {
		perror("ERROR closing the connection");
		exit(1);
		}
	 }//if file is not found
	 else  if(strcmp(buffer,"File FOUND")==0) {
		printf("FOUND the file...\n");
		 n = freadsockfd, buffer, 255);//read the file content the peer is sending
		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);}
		printf("File has the following content - \n%s",buffer);
		printf("gracefully closing the connection with the peer....\n");
		n=shutdown(sockfd,0);
		if (n < 0) {
			perror("ERROR closing the connection");
			exit(1);
		}//if perror
		
		//save the file on the client too
		FILE * save=fopen("sample1.txt","w");
		fprintf(save,"%s",buffer);
		fclose(save);
		
		 return 0;
		 }//if file found
	else printf("received unknown reply from the node\n");
   //changes to do : allow for larger file transfer with a larger buffer, or file breakdown.
   //assumption : the portname we save in the file, as peer port+200 and use that
   return -1;
}

