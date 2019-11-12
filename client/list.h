#ifndef LIST_H
#define LIST_H

struct node {
    char* ip;
    char* port;
    node *next;

};


class list{

	private:

    	node *head;

	public:

    	list();
    	~list();

    	void insert(char* client_ip,char* client_port);
        bool isempty();
    	bool find(char* client_ip,char* client_port);
    	void print();
    	int numberOfElemenets();
    	char* getClients(char* buffer,char* num_of_clients);
        node* getHead();
        node* getNext(node* current);
        char* getIp(node* current);
        char* getPort(node* current);

 
};


#endif