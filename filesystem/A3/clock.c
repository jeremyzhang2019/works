#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

static int head;
/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
    //use loop to check the menmber of coremap's PG_REF has not been set
    while(1){
        //if the member's pg_ref has been set then return it's index
        if((coremap[head].pte->frame & PG_REF)==0){
            //since it not been set the return
            return head;
        }else{
            //other wise set current pg_ref to 0 and increment clock
            coremap[head].pte->frame = coremap[head].pte->frame & ~PG_REF;
            head = (head + 1)%memsize;
        }}
	
	return head;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {

	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
    head = 0;
}
