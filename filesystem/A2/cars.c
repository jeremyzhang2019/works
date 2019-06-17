#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "traffic.h"

extern struct intersection isection;

/**
 * Populate the car lists by parsing a file where each line has
 * the following structure:
 *
 * <id> <in_direction> <out_direction>
 *
 * Each car is added to the list that corresponds with 
 * its in_direction
 * 
 * Note: this also updates 'inc' on each of the lanes
 */
void parse_schedule(char *file_name) {
    int id;
    struct car *cur_car;
    struct lane *cur_lane;
    enum direction in_dir, out_dir;
    FILE *f = fopen(file_name, "r");

    /* parse file */
    while (fscanf(f, "%d %d %d", &id, (int*)&in_dir, (int*)&out_dir) == 3) {

        /* construct car */
        cur_car = malloc(sizeof(struct car));
        cur_car->id = id;
        cur_car->in_dir = in_dir;
        cur_car->out_dir = out_dir;

        /* append new car to head of corresponding list */
        cur_lane = &isection.lanes[in_dir];
        cur_car->next = cur_lane->in_cars;
        cur_lane->in_cars = cur_car;
        cur_lane->inc++;
    }

    fclose(f);
}

/**
 * TODO: Fill in this function
 *
 * Do all of the work required to prepare the intersection
 * before any cars start coming
 * 
 */
void init_intersection() {
    int i,k;
    //initialize the quadrants' lock within the intersection
    for(i =0;i<4;i++){
        pthread_mutex_init(&(isection.quad[i]),NULL);
        
    }
    for(k=0;k<4;k++){
        //initialize all lanes in the intersection
        pthread_mutex_init(&(isection.lanes[k].lock),NULL);
        pthread_cond_init(&(isection.lanes[k].producer_cv),NULL);
        pthread_cond_init(&(isection.lanes[k].consumer_cv),NULL);
        isection.lanes[k].in_cars = NULL;
        isection.lanes[k].out_cars = NULL;
        isection.lanes[k].inc = 0;
        isection.lanes[k].passed = 0;
        isection.lanes[k].buffer =malloc(sizeof(struct car*)*LANE_LENGTH);
        isection.lanes[k].head = 0;
        isection.lanes[k].tail = 0;
        isection.lanes[k].capacity = LANE_LENGTH;
        isection.lanes[k].in_buf = 0;}
    
}


/**
 * TODO: Fill in this function
 *
 * Populates the corresponding lane with cars as room becomes
 * available. Ensure to notify the cross thread as new cars are
 * added to the lane.
 * 
 */
void *car_arrive(void *arg) {
    struct lane *l = arg;
    /*keep adding cars from in_cars list to the buff*/
    while(l->in_cars != NULL){
        /*case when the buff is full need waiting for consumer to consum*/
        pthread_mutex_lock(&(l->lock));
        while(l->capacity==l->in_buf){
            pthread_cond_wait(&(l->consumer_cv),&(l->lock));
        }
        /*when the buff is not full add a car to the buff update all info*/
        l->tail = ((l->tail)% (l->capacity));
        struct car * a_car = l->in_cars;
        l->buffer[l->tail] =  a_car;
        l->tail = ((l->tail+1)% (l->capacity));
        l->in_cars = l->in_cars->next;
        
        l->in_buf = l->in_buf+1;
        /*signal consumer that there are at least one car in the lane is ready*/
        pthread_cond_signal(&(l->producer_cv));
        pthread_mutex_unlock(&(l->lock));
        
    }
    /*signal consumer that there are somethings to consume*/
    return NULL;
}

/**
 * TODO: Fill in this function
 *
 * Moves cars from a single lane across the intersection. Cars
 * crossing the intersection must abide the rules of the road
 * and cross along the correct path. Ensure to notify the
 * arrival thread as room becomes available in the lane.
 *
 * Note: After crossing the intersection the car should be added
 * to the out_cars list of the lane that corresponds to the car's
 * out_dir. Do not free the cars!
 *
 * 
 * Note: For testing purposes, each car which gets to cross the 
 * intersection should print the following three numbers on a 
 * new line, separated by spaces:
 *  - the car's 'in' direction, 'out' direction, and id.
 * 
 * You may add other print statements, but in the end, please 
 * make sure to clear any prints other than the one specified above, 
 * before submitting your final code. 
 */
void *car_cross(void *arg) {
    struct lane *l = arg;
    struct lane *outl;
    struct car *firstcar;
    int i = 0;
    /*To check that the lane is not empty and there are new cars are coming*/
    while(l->inc>0){
        pthread_mutex_lock(&(l->lock));
        /*case that there is no car in the intersection wait for producer*/
        while(l->in_buf == 0){
             pthread_cond_wait(&(l->producer_cv),&(l->lock));
          }
        firstcar = l->buffer[l->head];
        printf("%d %d %d\n",firstcar->in_dir,firstcar->out_dir,firstcar->id);
        int *path = compute_path(firstcar->in_dir,firstcar->out_dir);
       
        /*lock the path insure that no other cars in the intersection*/
        while(*(path+i)>0 && *(path+i)<5){
            pthread_mutex_lock(&(isection.quad[*(path+i)]));
            i++;
            
        }
        /*add the car to destination lane when it successful passed the intersection*/
        outl = &(isection.lanes[firstcar->out_dir]);
       
        firstcar->next = outl->out_cars;
        outl->out_cars = firstcar;
        outl->passed++;
        l->head++;
        l->in_buf--;
        i=0;
//        printf("%d%d%d",firstcar->in_dir,firstcar->out_dir,firstcar->id);
        while(*(path+i)>0 && *(path+i)<5){
            pthread_mutex_unlock(&(isection.quad[*(path+i)]));
            i++;
        }
        l->inc--;
        /*let producer konw that at least one car been consumed*/
        pthread_cond_signal(&(l->consumer_cv));

        pthread_mutex_unlock(&(l->lock));
    }
    
    return NULL;
}

/**
 * TODO: Fill in this function
 *
 * Given a car's in_dir and out_dir return a sorted 
 * list of the quadrants the car will pass through.
 * 
 */


//moves that not passed Q4
int *compute_path(enum direction in_dir, enum direction out_dir) {
    int i;
    if(in_dir<out_dir){
        int *path = malloc(sizeof(int)*(out_dir-in_dir));
        memset(path,0,sizeof(int)*(out_dir-in_dir));
        for(i = 0;i<(out_dir-in_dir);i++){
            path[i] = (in_dir + i) + 1;
        }
        return path;
    }else{
        //moves that passed Q4
        int *path = malloc(sizeof(int)*((4-in_dir)+out_dir));
        memset(path,0,sizeof(int)*((4-in_dir)+out_dir));
        for(i = 0;i<((4-in_dir)+out_dir);i++){
            path[i] = (((in_dir + i) %4)+1);
        }
        return path;
    }
    return NULL;
    
}
