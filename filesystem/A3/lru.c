#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;


/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */


//I have modified the data type frame, frame now has one more attribute
//usage which recorded how many times it has been recently used
//referrence pagetable.h it increament when it been hit
//its a link list then the head of the linked list is the lru frame
typedef struct node{
    int index;
    int usage;
    struct node* next;
}node;

node *head;




int lru_evict() {
    int index = head->index;
    node* tmp1 = head;
    head = head->next;
    coremap[index].usage = 0;
    free(tmp1);
	return index;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
    int i;
    //((struct frame *)(p->frame & PAGE_MASK)) ->usage++;
    for(i=0;i<memsize;i++){
        if(p == coremap[i].pte){
            coremap[i].usage++;
        }
    }
    	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
    int i;
// initialize the head of link list
    head = malloc(sizeof(node *));
    head->index = 0;
    head->usage = coremap[0].usage;
    head->next = NULL;
    
    for(i=1;i<memsize;i++){
        if(coremap[i].in_use){
            //we only deal with frame that been allocated
            node *new = malloc(sizeof(node *));
            new->index = i;
            new->usage = coremap[i].usage;
            new->next = NULL;
            //store head pointer
            node *temp = head;
            
            while(1){
                //update the linked list
                if(new->usage >= head->usage){
                    // case its been used more then head
                    if(head->next == NULL){
                        head->next = new;
                        //store back the value
                        head = temp;
                    }else{
                        head = head->next;
                    }
                }else{
                    //handle case that it been less used then head
                    new->next = head;
                    head = new;
                }
            }
            
        }
        
    }
 return;
}
