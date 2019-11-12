#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#ifndef QUEUE_H
#define QUEUE_H

using namespace std;


/* ----- Node Declaration ----- */

struct node
{
    char info[500];
    struct node *link;    

};


/* ----- Class Declaration ----- */

class queue_list

{
	public:
		node * front;
		node * rear;
		
        void insert(char*);
        void display();
        void del();
        char* get_element();
        queue_list();
        int q_is_empty();
       
};



#endif /* QUEUE_H */