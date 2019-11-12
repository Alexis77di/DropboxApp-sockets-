#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include "queue.h"


queue_list::queue_list() 
        {
            front = NULL;
            rear = NULL;
            
         
        }  


void queue_list::insert(char* item)

{
	node *tmp;
    tmp = new (struct node);
    strcpy(tmp->info , item);
    tmp->link = NULL;
    if (front == NULL)
        front = tmp;
    else
        rear->link = tmp;
    rear = tmp;
  
}


void queue_list::del()

{
	node *tmp;
    if (front == NULL)
        cout<<"Queue Underflow"<<endl;
    else
    {       
        tmp = front;
        cout<<"Element Deleted from queue : "<<tmp->info<<endl;
        front = front->link;
        
        free(tmp);
    }

}

int queue_list::q_is_empty(){
	//node *ptr;
    //ptr = front;
    if (front == NULL){
        
        return 0;
    }
    else
    {
        return 1;
    }
}

char* queue_list::get_element()
{
	node *ptr;
    ptr = front;
    if (front == NULL){
        cout<<"Queue is empty"<<endl;
        return NULL;
    }
    else
    {
        return ptr->info;
    }

}

void queue_list::display()

{       
    node *ptr;
    ptr = front;
    if (front == NULL)
        cout<<"Queue is empty"<<endl;
    else
    {
        cout<<"Queue elements :"<<endl;
        while (ptr != NULL)
        {
            cout<<ptr->info<<" " << endl;
            ptr = ptr->link;
        }
        cout<<endl;

    }

}