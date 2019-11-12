#include <iostream>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "list.h"

using namespace std;

//-------------------------  Functions for our List  --------------------------//

list::list(){
	
	head = NULL; 

}

list::~list() {

	while(head) {              
    	node *deleteMe = head;
    	head = head->next;  
        delete [] deleteMe->ip; 
        delete [] deleteMe->port;       
    	delete deleteMe;      
	}
}


void list::insert(char* client_ip,char* client_port){
	node *n = new node();
    n->ip=new char [strlen(client_ip+1)];
    strcpy(n->ip,client_ip);
    n->port=new char [strlen(client_port+1)];
    strcpy(n->port,client_port);           
	n->next = head;       
                        	
	head = n;           
}

char* list::getIp(node* current){
	return current->ip;
}

char* list::getPort(node* current){
	return current->port;
}

bool list::isempty(){
    return head == NULL;
}

bool list::find(char* client_ip,char* client_port){
	node* current = head;
   	while(current != NULL){
        if((strcmp(current->ip ,client_ip)==0) && (strcmp(current->port ,client_port)==0)){ 
            return true;

        }
        current = current->next;
    }
    return false;
}

void list::print() 
{
	node *current;

	current = head;
	cout << "I'm gonna print my list with online clients : " << endl;

	while (current != NULL)
	{	
		cout << "Client Ip = "<< current->ip << " , Client port = " << current->port << endl;
		current = current->next;
	}
}

int list::numberOfElemenets(){
	node *current;
	current = head;
	int n = 0;
	while (current != NULL)
	{	
		n++;
		current = current->next;
	}
	return n;
}

node* list::getHead(){
	return head;
}

node* list::getNext(node* current){
	if(current->next != NULL){
		return current->next;
	}
	else{
		return NULL;
	}
}


char* list::getClients(char* buffer,char* num_of_clients){
	node *current;
	current = head;
	strcpy(buffer,"CLIENTS_LIST ");


	strcat(buffer,num_of_clients);
	int n = 0;
	while (current != NULL)
	{	
		strcat(buffer," ");
		strcat(buffer,current->ip);
		strcat(buffer," ");
		strcat(buffer,current->port);
		n++;
		current = current->next;
	}
	return buffer;
}