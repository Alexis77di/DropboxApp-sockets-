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
#include <string.h>
#include "list.h"

using namespace std;

#define BLUE  "\x1B[34m"

//---------- global --------------------------------------------//

bool flag=true;
char host[500];

int num_of_files = 0;
char list_buffer[5000];

list* client_list;

char* split_msg[30];     
int number_of_clients = 0;

int err1,err2;

int sockfd;
int sockfd_content_clnt, newsockfd_content_clnt;
socklen_t clilen;
struct sockaddr_in server_client,cli_addr;
struct sockaddr *server_clnt_ptr = (struct sockaddr*)&server_client;

pthread_t * worker_threads;
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;



void signalhandler(int signal){
	flag = false;
    pthread_mutex_destroy(&mtx);
 	close(newsockfd_content_clnt);
 	close(sockfd_content_clnt);
 	close(sockfd);                 /* Close socket and exit */
    delete client_list;

    if(signal == SIGINT){
        cout << "  SIGNAL --> SIGINT " << endl;
        exit(EXIT_SUCCESS);
    }
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


void split(char* message){	                // split messages from dropbox server

	int i =0;
	char* chars_array = strtok(message, " ");	
	while(chars_array){
		split_msg[i] = chars_array;
        chars_array =strtok(NULL, " ");
        i++;
    }
    number_of_clients = i;
}




// --------- Here is a RECURSIVE function to print the content of a given folder --------- // 
		// --> https://stackoverflow.com/questions/8436841/how-to-recursively-list-directories-in-c-on-linux/29402705  //


char* listdir(const char *name, int indent)
{
    DIR *dir;
    struct dirent *entry;
    char path2[1024];

    strcpy(path2,name);

    if (!(dir = opendir(name)))
        exit(1);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            //printf("%*s[%s]\n", indent, "", entry->d_name);
            listdir(path, indent + 2);
        } else {
            strcat(list_buffer,"< ");
            strcat(list_buffer,path2);
            strcat(list_buffer,",");
            strcat(list_buffer,entry->d_name);
            strcat(list_buffer," > ");
            //printf("%*s- %s\n", indent, "", entry->d_name);
        }
        num_of_files++;
    }
    return list_buffer;
    closedir(dir);
}




void * request_manager_fnct(void *arg) {

	int sockfd_cntn_srv, port_cntn_srv, n;
   	struct sockaddr_in serv_addr;
   	struct in_addr server_ip;
    struct hostent *server;
    char buffer_cntn_srv[3000];

	node* ls= (node*) arg;

	char hostbuffer[256]; 
    char *IPbuffer; 
    struct hostent *host_entry; 
    int hostname; 
  
    hostname = gethostname(hostbuffer, sizeof(hostbuffer)); 
  
    host_entry = gethostbyname(hostbuffer);  

    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); 
  


	if(ls!=NULL){ 													// if ls!=Null else exit thread

		if (!inet_aton(client_list->getIp(ls), &server_ip)) {
	        exit(0);
	    }

	    if ((server = gethostbyaddr((const void *)&server_ip, sizeof server_ip, AF_INET)) == NULL) {
	        exit(0);
	    }


	    if( strcmp( client_list->getIp(ls) , IPbuffer) == 0){                                               // condition
	    	cout << "I am the client with IP : "<< IPbuffer << " and I don't want my own files ." << endl;
	    	cout << "                                                                          " << endl;
	    	pthread_exit ( NULL ) ;
	    }

		if (( err1 = pthread_mutex_lock (& mtx ))){ 		// Lock mutex //
			error ("pthread_mutex_lock") ;
		    exit (1) ;
		}
		
		cout << "                                                    " << endl;
		cout << "Thread " << pthread_self () << " Locked the mutex   " << endl;
		cout << "I'm gonna connect with client with Ip : " << client_list->getIp(ls) << " and port : " << client_list->getPort(ls) << " . " << endl;
		cout << "                                                                                                                         " << endl;



		//-------- Here we create the socket ---------- //

	    sockfd_cntn_srv = socket(AF_INET, SOCK_STREAM, 0);
	    if (sockfd_cntn_srv < 0) 
	        error("ERROR opening socket");

		/*--------------------------------------------- */ 


	 	if (server == NULL) {
	        fprintf(stderr,"ERROR, no such host\n");
	        exit(0);
	 	}

		 //-------  here we clear the structure of address  ------ //

	    bzero((char *) &serv_addr, sizeof(serv_addr));

		//------------------------------------------------------ //
	     
	    serv_addr.sin_family = AF_INET;
	    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
	    serv_addr.sin_port = htons(atoi(client_list->getPort(ls)));

		//------- Here we try to connect with the server ---------//
	    
	   
	    if (connect(sockfd_cntn_srv, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	        cout << " No such content server!" << endl;
	        err2 = pthread_mutex_unlock (& mtx );
	        cout << "Thread " << pthread_self ()  <<" Unlocked the mutex  " << endl;
	        cout << "                                                     " << endl;
	        return 0;
	    }

	    bzero(buffer_cntn_srv,3000);
	    strcpy(buffer_cntn_srv,"GET_FILE_LIST");



    	if (write(sockfd_cntn_srv, buffer_cntn_srv, strlen(buffer_cntn_srv))< 0)  // request for GET_FILE_LIST
        	error("ERROR writing to socket");

        cout << "I want the list of files from client with Ip :  " << client_list->getIp(ls) << " . " << endl;
        cout << "                                                                                   " << endl;

        // reading list
        bzero(buffer_cntn_srv,3000);

        if (read(sockfd_cntn_srv,buffer_cntn_srv,2999)< 0)          // ----- waiting for FILE_LIST ----//
    		error("ERROR reading from socket");

    	cout << "The list of file that I received is : " << endl;
    	cout << buffer_cntn_srv << endl;
    	cout << "                                      " << endl;


		sleep(1);

		if ( (err2 = pthread_mutex_unlock (& mtx ))) { 		// Unlock mutex 
			error("pthread_mutex_ unlock ") ; 
			exit (1) ;
		}
	
		cout << "Thread " << pthread_self()  <<" Unlocked the mutex  " << endl;
		cout << "												       "<< endl;
		pthread_exit ( NULL ) ;
	}
	else{
		pthread_exit ( NULL ) ;
	}

}


int main(int argc, char *argv[]){

	int portNum,workerThreads,bufferSize,serverPort,err;

	//int sockfd;

	char dirname[400];
	char input_line[3000];

	struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
	struct hostent *rem;
	char hostname[3000];
	char *serverIP;

	char hostbuffer[3000];                  // to get my ip
	struct hostent *host_entry; 
	char *myIP;
	int myip_host;

	if(argc > 7){                                // check the number of arguments,must be 2 .
	    cout <<"Too many arguments , please try again ...\n"<< endl;
	    return -1;
	}

	if(argc < 7){
	    cout <<"Too few arguments , please try again ...\n" << endl;
	    return -1;
	}


// ------------ get my own ip  ---------------------------------------------//	

    myip_host = gethostname(hostbuffer, sizeof(hostbuffer)); 
  
    host_entry = gethostbyname(hostbuffer);  

    myIP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    printf(" %s",BLUE);
    cout << "I am the client with hostname : " << hostbuffer << " and with IP : " << myIP << " and I just connected to this app ... " << endl;
    cout << "                                                                                                                       " << endl; 


// ----------------- parsing arguments -------------------------------------//

	strcpy(dirname,argv[1]);
	portNum = atoi(argv[2]);
	workerThreads = atoi(argv[3]);
	bufferSize = atoi(argv[4]);
	serverPort = atoi(argv[5]);
	strcpy(hostname,argv[6]);

// -------------- Find server address ----------------------------------//

    if ((rem = gethostbyname(hostname)) == NULL) {	
        error("gethostbyname"); 

    }

    serverIP = inet_ntoa(*((struct in_addr*)rem->h_addr_list[0]));


// ---------------- Create socket -----------------------------------//

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    	error("socket");


    bzero((char *) &server, sizeof(server));

    server.sin_family = AF_INET;       						  // ------- Internet domain -------//
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(serverPort);         			// --------- Server port ----------//


// -----------------Initiate connection ---------------------------//

    if (connect(sockfd, serverptr, sizeof(server)) < 0)
	   	error("connect");



	char buf[3000];                      // ------------- creating LOG_ON message -----------------//
	bzero(buf,3000);
	strcpy(buf,"LOG_ON ");
	strcat(buf,myIP);
	strcat(buf," ");
	strcat(buf,argv[2]);

	if (write(sockfd, buf, strlen(buf)) < 0)           // --------- wrtite LOG_ON message to buffer ------//
		error("write");

	cout << "I just sent this message : " << buf << "  to dropbox server . " << endl;
	cout << "                                                              " << endl;

	bzero(buf,3000);

	if (read(sockfd,buf,3000) < 0)                   // -------------- read  USER_ON message -------------//
		error("read");

	cout << "I received this message from dropbox server : " << buf << endl;
	cout << "                                                     " << endl;  

	bzero(buf,3000);
	strcpy(buf,"GET_CLIENTS");

	if (write(sockfd, buf, strlen(buf)) < 0)                    // ----- write GET_CLIENTS message to buffer -------//
		error("write");

	cout << "I just sent this message : " << buf << "  to dropbox server . " << endl;
	cout << "                                                              " << endl;

	bzero(buf,3000);

	if (read(sockfd,buf,3000) < 0)
		error("read");

	cout << "I received this message from dropbox server : " << buf << endl;
	cout << "                                                     " << endl;




	split(buf);    				// ---------------- split CLIENTS_LIST message -------------//


	client_list = new list();


	for(int i = 2; i<number_of_clients; i++){                        // ----- insert the online clients to my list -------- //
		client_list->insert(split_msg[i],split_msg[i+1]);
		i++;
	}

	client_list->print();                     // ------- printing my list -----------//



	signal(SIGINT,signalhandler);
   	

// ---------------- initialization --------------------------------------------//

	pthread_mutex_init(&mtx, 0);              
	pthread_cond_init(&cond_nonempty, 0);
	pthread_cond_init(&cond_nonfull, 0);

	// ---- Malloc first ----- //

     if (( worker_threads = static_cast<pthread_t*>(malloc ( workerThreads * sizeof ( pthread_t ))) ) == NULL ) {
		perror ( " malloc " ) ;
		exit (1) ;
	 }


   	if(client_list->numberOfElemenets() == 1){             // ------ first client works as a server for multiple clients 

   			char buffer_client[3000];

		   	cout << "                                                                      " << endl;
		    cout << "I'm the client (server) 1 and I'm waiting for requests to this port " << portNum << endl;
		   	cout << "                                                                      " << endl;


		    //---------- Here we create a socket to communicate with Initiator --------//

		    sockfd_content_clnt =  socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd_content_clnt < 0) 
		        error("ERROR opening socket");


		   	//-------  here we clear the structure of address  ------ //
		     bzero((char *) &server_client, sizeof(server_client));


		    //-------------- setup the host_addr structure for use in bind call ----------- //
		    
		    server_client.sin_family = AF_INET;  
		    server_client.sin_addr.s_addr = htons(INADDR_ANY);
		    server_client.sin_port = htons(portNum);


		    // ------------------------ Here is the bind() call ------------------------- //
		   
    		if (bind(sockfd_content_clnt, server_clnt_ptr, sizeof(server_client)) < 0)
        		error("bind");



			listen(sockfd_content_clnt,5);


			while(flag) {	

		    	//--------------------- Here is the accept() call ---------------------//
		    	// -------- The accept() returns a new socket file descriptor for the accepted connection ---------- //

		     	clilen = sizeof(cli_addr);
		     	newsockfd_content_clnt = accept(sockfd_content_clnt, (struct sockaddr *) &cli_addr, &clilen);
		     
		     	if (newsockfd_content_clnt< 0) 
		        	error("ERROR on accept");

		        bzero(buffer_client,3000);

    			if (read(newsockfd_content_clnt,buffer_client,2999)< 0)          // ----- waiting for GET_FILE_LIST ----//
    				error("ERROR reading from socket");			

		    	cout << "I am the first client and I received this message : " << buffer_client << endl;

		    	// I'm gonna send the list of files

		    	char list_of_files[5000];
			    char* list_of_files2;
			    char numbertostr[5];
			    bzero(list_buffer,5000);
			    bzero(list_of_files,5000);

			    strcpy(list_of_files,"FILE_LIST ");

			    list_of_files2 = listdir(dirname, 0);
			    sprintf(numbertostr,"%d",num_of_files);
			    strcat(list_of_files,numbertostr);
			    strcat(list_of_files," ");
			    strcat(list_of_files,list_of_files2);
			    cout << "I responded to LIST_OF_FILES message and I'm gonna send this type of message : " << endl;
			    cout << list_of_files << endl;
			    cout << "                                                                               " << endl;


			    if (write(newsockfd_content_clnt, list_of_files, strlen(list_of_files))< 0)  // FILE_LIST
        			error("ERROR writing to socket");



		    	//break;
			}
   	}
   	else{
   			while(flag){

   				// ---------- ------------------Create threads ----------------------------------------------- //
	 			node* head_ls = client_list->getHead();

	 			for (int k =0; k < workerThreads; k++){

   					sleep(1);


	 				if ( (err = pthread_create ( worker_threads +k , NULL , request_manager_fnct , (void*)head_ls))) {

						perror (" pthread_create ") ;
						exit (1) ;
					}

					if(head_ls != NULL){                             // next client
						head_ls = client_list->getNext(head_ls);
					}

	 			}

	 			// ------------------------------------ Join ---------------------------------------------- //

				for ( int i =0 ; i < workerThreads ; i ++) {

					if ( (err = pthread_join (*( worker_threads + i ) , NULL ) )) {

					// ------ Wait for thread termination ------ //
						perror (" pthread_join ") ;
						exit (1) ;
					}
				}

				break;
   			}

   			/* --- Destroying cond_variables --- */
			pthread_cond_destroy(&cond_nonempty);
			pthread_cond_destroy(&cond_nonfull);

			/* --- Destroying mutex --- */
			pthread_mutex_destroy(&mtx);
   			
   	}

   	cout << "                                               " << endl;
	cout << "I sent an ctrl+c message ,so I want to exit ..." << endl;

	bzero(buf,3000);
	strcpy(buf,"LOG_OFF");

	// if (write(sockfd, buf, strlen(buf)) < 0)
	// 	error("write");


    close(sockfd);                 /* Close socket and exit */
    delete client_list;




}