#include <assert.h>
#include <string.h> 
#include "sim.h"
#include "pagetable.h"

// The top-level page table (also known as the 'page directory')
pgdir_entry_t pgdir[PTRS_PER_PGDIR]; 

// Counters for various events.
// Your code must increment these when the related events occur.
int hit_count = 0;
int miss_count = 0;
int ref_count = 0;
int evict_clean_count = 0;
int evict_dirty_count = 0;

/*
 * Allocates a frame to be used for the virtual page represented by p.
 * If all frames are in use, calls the replacement algorithm's evict_fcn to
 * select a victim frame.  Writes victim to swap if needed, and updates 
 * pagetable entry for victim to indicate that virtual page is no longer in
 * (simulated) physical memory.
 *
 * Counters for evictions should be updated appropriately in this function.
 */
int allocate_frame(pgtbl_entry_t *p) {
	int i;
	int frame = -1;
	for(i = 0; i < memsize; i++) {
        coremap[i].usage = 0;
		if(!coremap[i].in_use) {
			frame = i;
			break;
		}
	}
	if(frame == -1) { // Didn't find a free page.
		// Call replacement algorithm's evict function to select victim
		frame = evict_fcn();
        // All frames were in use, so victim frame must hold some page
		// Write victim page to swap, if needed, and update pagetable
		// IMPLEMENTATION NEEDED
        
        //find the vitim in the coremap
        pgtbl_entry_t *curr_frame = coremap[frame].pte;
        
        if((curr_frame->frame & PG_DIRTY) == 0){
            //since the frame is clean
            evict_clean_count++;
        }else{
             //since the frame is dirtey
              evict_dirty_count++;
             //found victim and swap it out of the table
             curr_frame->swap_off = swap_pageout(((curr_frame->frame)>>PAGE_SHIFT),curr_frame->swap_off);
            
            //check it has successfully swaped
            //assert((curr_frame->swap_off & INVALID_SWAP) == 0);
        
            //mask the frame number onswap  and not dirty
            curr_frame->frame = curr_frame->frame | PG_ONSWAP;
            curr_frame->frame = curr_frame->frame & ~PG_DIRTY;
            
        }
        //reset the frame to invalid(since its out of memo)
        curr_frame->frame = curr_frame->frame & ~PG_VALID;
        

	}
    
	// Record information for virtual page that will now be stored in frame
	coremap[frame].in_use = 1;
	coremap[frame].pte = p;

	return frame;
}

/*
 * Initializes the top-level pagetable.
 * This function is called once at the start of the simulation.
 * For the simulation, there is a single "process" whose reference trace is 
 * being simulated, so there is just one top-level page table (page directory).
 * To keep things simple, we use a global array of 'page directory entries'.
 *
 * In a real OS, each process would have its own page directory, which would
 * need to be allocated and initialized as part of process creation.
 */
void init_pagetable() {
	int i;
	// Set all entries in top-level pagetable to 0, which ensures valid
	// bits are all 0 initially.
	for (i=0; i < PTRS_PER_PGDIR; i++) {
		pgdir[i].pde = 0;
	}
}

// For simulation, we get second-level pagetables from ordinary memory
pgdir_entry_t init_second_level() {
	int i;
	pgdir_entry_t new_entry;
	pgtbl_entry_t *pgtbl;

	// Allocating aligned memory ensures the low bits in the pointer must
	// be zero, so we can use them to store our status bits, like PG_VALID
	if (posix_memalign((void **)&pgtbl, PAGE_SIZE, 
			   PTRS_PER_PGTBL*sizeof(pgtbl_entry_t)) != 0) {
		perror("Failed to allocate aligned memory for page table");
		exit(1);
	}

	// Initialize all entries in second-level pagetable
	for (i=0; i < PTRS_PER_PGTBL; i++) {
		pgtbl[i].frame = 0; // sets all bits, including valid, to zero
		pgtbl[i].swap_off = INVALID_SWAP;
	}

	// Mark the new page directory entry as valid
	new_entry.pde = (uintptr_t)pgtbl | PG_VALID;

	return new_entry;
}

/* 
 * Initializes the content of a (simulated) physical memory frame when it 
 * is first allocated for some virtual address.  Just like in a real OS,
 * we fill the frame with zero's to prevent leaking information across
 * pages. 
 * 
 * In our simulation, we also store the the virtual address itself in the 
 * page frame to help with error checking.
 *
 */
void init_frame(int frame, addr_t vaddr) {
	// Calculate pointer to start of frame in (simulated) physical memory
	char *mem_ptr = &physmem[frame*SIMPAGESIZE];
	// Calculate pointer to location in page where we keep the vaddr
        addr_t *vaddr_ptr = (addr_t *)(mem_ptr + sizeof(int));
	
	memset(mem_ptr, 0, SIMPAGESIZE); // zero-fill the frame
	*vaddr_ptr = vaddr;             // record the vaddr for error checking

	return;
}

/*
 * Locate the physical frame number for the given vaddr using the page table.
 *
 * If the entry is invalid and not on swap, then this is the first reference 
 * to the page and a (simulated) physical frame should be allocated and 
 * initialized (using init_frame).  
 *
 * If the entry is invalid and on swap, then a (simulated) physical frame
 * should be allocated and filled by reading the page data from swap.
 *
 * Counters for hit, miss and reference events should be incremented in
 * this function.
 */
char *find_physpage(addr_t vaddr, char type) {
	pgtbl_entry_t *p=NULL; // pointer to the full page table entry for vaddr
	unsigned idx = PGDIR_INDEX(vaddr); // get index into page directory

	// IMPLEMENTATION NEEDED
	// Use top-level page directory to get pointer to 2nd-level page table
    
    //if the page table at index idx does not exit then creat a new page
    if(pgdir[idx].pde==0){
        pgdir[idx] = init_second_level();
    }
    // pointer to 2nd level page table
    //since init_second_level return  (uintptr_t)pgtbl which is pgtbl_entry_t* type
    // then pg_pt = (pgtbl_entry_t*)pgdir[idx].pde;
    pgtbl_entry_t *pg_pt = (pgtbl_entry_t *)(pgdir[idx].pde & PAGE_MASK);
    

	// Use vaddr to get index into 2nd-level page table and initialize 'p'
    //get index for 2end-level
    unsigned idx_2nd = (((vaddr) >> PAGE_SHIFT) & PGTBL_MASK);
    p = &pg_pt[idx_2nd];



	// Check if p is valid or not, on swap or not, and handle appropriately
    //check p is valid
    if((p->frame & PG_VALID)==0){
        //page is invalid then the page need allocate it then miss_acount ++
        miss_count++;
        
        if((p->frame & PG_ONSWAP)==0){
            //page is not onswap then create a new frame and alloct it in the page
            int frame_num = allocate_frame(p);
            init_frame(frame_num,vaddr);
            //since it been initilized then store the frame number to the frame
            p->frame = frame_num << PAGE_SHIFT;
            //make it dirty
            p->frame = p->frame | PG_DIRTY;
            
            
            
        }else{
            //page is in swap then bring it back to page -> swapin
            int frame_num = allocate_frame(p);
            swap_pagein(frame_num,p->swap_off);
            p->frame = frame_num << PAGE_SHIFT;
            //since it swap back then reset PG_ONSWAP since ~PG_ONSWAP = ....110111 then and operation set fream to not on swap
            p->frame = p->frame | PG_ONSWAP;
            
            
            
        }
    
    }else{
        //page is valid then hit_count ++ keep using it
        hit_count++;
        //since hit then update it's usage
        //coremap[i].usage ++;
        
    }


	// Make sure that p is marked valid and referenced. Also mark it
    p->frame = p->frame | PG_VALID;
    p->frame = p->frame | PG_REF;
    // dirty if the access type indicates that the page will be written to.
    if(type == 'S' || type == 'M'){
        p->frame = p->frame | PG_DIRTY;
    }
    


	// Call replacement algorithm's ref_fcn for this page
	ref_fcn(p);
    ref_count++;
	// Return pointer into (simulated) physical memory at start of frame
	return  &physmem[(p->frame >> PAGE_SHIFT)*SIMPAGESIZE];
}

void print_pagetbl(pgtbl_entry_t *pgtbl) {
	int i;
	int first_invalid, last_invalid;
	first_invalid = last_invalid = -1;

	for (i=0; i < PTRS_PER_PGTBL; i++) {
		if (!(pgtbl[i].frame & PG_VALID) && 
		    !(pgtbl[i].frame & PG_ONSWAP)) {
			if (first_invalid == -1) {
				first_invalid = i;
			}
			last_invalid = i;
		} else {
			if (first_invalid != -1) {
				printf("\t[%d] - [%d]: INVALID\n",
				       first_invalid, last_invalid);
				first_invalid = last_invalid = -1;
			}
			printf("\t[%d]: ",i);
			if (pgtbl[i].frame & PG_VALID) {
				printf("VALID, ");
				if (pgtbl[i].frame & PG_DIRTY) {
					printf("DIRTY, ");
				}
				printf("in frame %d\n",pgtbl[i].frame >> PAGE_SHIFT);
			} else {
				assert(pgtbl[i].frame & PG_ONSWAP);
				printf("ONSWAP, at offset %lu\n",pgtbl[i].swap_off);
			}			
		}
	}
	if (first_invalid != -1) {
		printf("\t[%d] - [%d]: INVALID\n", first_invalid, last_invalid);
		first_invalid = last_invalid = -1;
	}
}

void print_pagedirectory() {
	int i; // index into pgdir
	int first_invalid,last_invalid;
	first_invalid = last_invalid = -1;

	pgtbl_entry_t *pgtbl;

	for (i=0; i < PTRS_PER_PGDIR; i++) {
		if (!(pgdir[i].pde & PG_VALID)) {
			if (first_invalid == -1) {
				first_invalid = i;
			}
			last_invalid = i;
		} else {
			if (first_invalid != -1) {
				printf("[%d]: INVALID\n  to\n[%d]: INVALID\n", 
				       first_invalid, last_invalid);
				first_invalid = last_invalid = -1;
			}
			pgtbl = (pgtbl_entry_t *)(pgdir[i].pde & PAGE_MASK);
			printf("[%d]: %p\n",i, pgtbl);
			print_pagetbl(pgtbl);
		}
	}
}
