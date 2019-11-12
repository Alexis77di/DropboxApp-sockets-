#include <arpa/inet.h>
#include <sys/wait.h>	     
#include <sys/types.h>	     
#include <sys/socket.h>	    
#include <netinet/in.h>
#include <dirent.h>
#include <cstring>
#include <errno.h>
#include <time.h>	  
#include <netdb.h>	    
#include <fstream>
#include <unistd.h>	       
#include <stdlib.h>	         
#include <ctype.h>	         
#include <signal.h>          
#include <string.h>
#include <pthread.h>
#include <iostream>  
#include "list.h"

using namespace std;

#define GREEN  "\x1B[32m"

char* split_msg[5];      // global
bool flag = true;
int sockfd = 0 , newsockfd;


void signalhandler(int signal){
	flag = false;
    if(signal == SIGINT){
    	// cout << "Closing connection..\n" << endl;
    	// close(sockfd);
   		// close(newsockfd);	  /* Close socket */
        cout << "  SIGNAL --> SIGINT " << endl;
    }
}


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*---------This function helps me to Split a string operation by delimiter ' ' -------------- */

void split(char* message){	

	int i =0;
	char* chars_array = strtok(message, " ");	
	while(chars_array){
		split_msg[i] = chars_array;
        chars_array =strtok(NULL, " ");
        i++;
    }
}


int main(int argc, char *argv[]){

	int portNum,sd,activity;
	//int sockfd = 0 , newsockfd;
	int max_clients = 30;
	int client_socket[30];
	int max_sd;
    int fdmax , i; 
	fd_set master;
	fd_set readfds;
	char buf[3000];
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;


	if(argc > 2){                                // check the number of arguments,must be 2 .
	    cout <<"Too many arguments , please try again ..." << endl;
	    return -1;
	}

	if(argc < 2){
	    cout <<"Too few arguments , Please try again ... " << endl;
	    return -1;
	}

	portNum = atoi(argv[1]);

	list* server_list = new list();

	// initialise all client_socket[] to 0 so not checked 

    for (i = 0; i < max_clients; i++)   
    {   
        client_socket[i] = 0;   
    } 


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
        perror("ERROR opening socket");

    bzero((char *) &server, sizeof(server));

    server.sin_family = AF_INET;  
    server.sin_addr.s_addr = htons(INADDR_ANY);
    server.sin_port = htons(portNum);



// --------- Here is the bind() call --------- //
   
    if (bind(sockfd, serverptr, sizeof(server)) < 0)
        error("bind");

// --------- Listen for connections ----------------------//

    if (listen(sockfd, 10) < 0) 
    	error("listen");

    printf("I'm the Dropbox_Server and I 'm Listening for connections on port %s%d\n",GREEN,portNum);
    //cout << "I'm the Dropbox_Server and I 'm Listening for connections on port " << portNum << endl;
    cout << "                                                                             " << endl;

    clientlen = sizeof(client);

    signal(SIGINT,signalhandler);

    while(flag){

    	FD_ZERO(&readfds);   
     
        // add master socket to set  
        FD_SET(sockfd, &readfds);   
        max_sd = sockfd;

        // add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            // if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            // highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        } 

        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            error("select error");   
        }  

        if (FD_ISSET(sockfd, &readfds)){

		    if ((newsockfd = accept(sockfd, clientptr, &clientlen)) < 0) 
		    	error("accept");

		    // inform user of socket number - used in send and receive commands

		    cout << "New connection , socket fd is " << newsockfd << " , ip is " << inet_ntoa(client.sin_addr) << " and port is " << portNum << endl;
		    cout << "                                                                                                                      " << endl; 


		    char ip[30];
	  		strcpy(ip, (char*)inet_ntoa(client.sin_addr));    //sin_addr to string

		    
		    bzero(buf,3000);

		    if(read(newsockfd, buf, 3000) <0 )       // reading lon_on message
		    	error("read");

		    split(buf);

		    if((strcmp(split_msg[0] , "LOG_ON")==0)){

		    	cout << "I recieved LOG_ON message from client with IP : " << ip << endl;
		    	cout << "                                                      " << endl;

		    	if(!(server_list->find(split_msg[1],split_msg[2]))){           // check if client is already in our list --> else add him

		    		server_list->insert(ip,split_msg[2]);

		    	}
		    	else{
		    		cout << "The client with IP : " << split_msg[1] << " and port : " << split_msg[2] << " is already in our list ..." << endl;
		    		cout << "                                                                                                        " << endl;
		    	}


	    		char buffer[3000];
	    		bzero(buffer,3000);

	    		strcpy(buffer,"USER_ON ");        // create USER_ON Message
				strcat(buffer,ip);
				strcat(buffer," ");
				strcat(buffer,split_msg[2]);

		    	if(write(newsockfd, buffer ,strlen(buffer)) < 0)   // send USER_ON message 
		    		error("write");

		    	cout << "I just sent USER_ON message to the dropbox client ." << endl;
		    	cout << "                                                   " << endl;

		    }

		    bzero(buf,3000);
		   	if(read(newsockfd, buf, 3000) <0 )     // read GET_CLIENTS message
		    	error("read");

		    split(buf);

		    if((strcmp(split_msg[0] , "GET_CLIENTS")==0)){

		    	cout << "I received GET_CLIENTS message from client with IP : " << ip << endl;
		    	cout << "                                                           " << endl;

		    	int numOfClients = server_list->numberOfElemenets();
		    	char numbertostr[20];
		    	sprintf(numbertostr,"%d",numOfClients);
		    	char gt_clnt_buffer[3000];
		    	bzero(gt_clnt_buffer,3000);
		    	strcpy(gt_clnt_buffer,server_list->getClients(gt_clnt_buffer,numbertostr));

		    	if(write(newsockfd, gt_clnt_buffer ,strlen(gt_clnt_buffer)) < 0)      // send the client list
		    		error("write");

		    	cout << "I just sent CLIENT_LIST message to the dropbox client ." << endl;
		    	cout << "                                                       " << endl;
		    }

		 	//add new socket to array of sockets  
            for (int i = 0; i < max_clients; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = newsockfd;                   
                    break;   
                }   
            }  



		}

		 // bzero(buf,3000);
	 	//   	if(read(newsockfd, buf, 3000) <0 )
	   //    	error("read");

	 //    split(buf);


	}

	// if((strcmp(split_msg[0] , "LOG_OFF")==0)){

	//     	cout << "LOG_OFF message ." << endl;
	// }

	
	cout << "                     " << endl;
    cout << "Closing connection.. " << endl;
    close(sockfd);
    close(newsockfd);	  /* Close socket */

}
