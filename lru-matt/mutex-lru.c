/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

/* @* Matthew Gordon *@
 * @* PID: 730400003 *@
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "lru.h"
#include <assert.h>

/* Define the simple, singly-linked list we are going to use for tracking lru */
struct list_node {
    struct list_node* next;
    int key;
    int refcount;
};

static struct list_node* list_head = NULL;

/* A static mutex */
static pthread_mutex_t mutex;
static int count = 0;
static pthread_cond_t cv_low, cv_high;

static volatile int done = 0;

/* Initialize the mutex. */
int init (int numthreads) {
    /* Your code here */

    // Initialize any global variables or synchronization primitives (Hint: See pthread_mutex_init() and pthread_cond_init(). May not need to do anything. Returns 0 on success, -errno on failure.
    
    // initialize mutex with no attributes
    pthread_mutex_init(&mutex,NULL);
    // mutex now initialized & unlocked

    // initialize conditional variable
    pthread_cond_init(&cv_low, NULL);
    pthread_cond_init(&cv_high, NULL);

    return 0;
}

/* Return 1 on success, 0 on failure.
 * Should set the reference count up by one if found; add if not.*/
int reference (int key) {
    /* Your code here */

    // Search the list, and, if found, increment the key's reference count by one. If not found, add to the list. Returns 1 on success, 0 on failure.
        
    // lock before updating list
    pthread_mutex_lock(&mutex);
        // make thread wait until count is valid
        while(count >= HIGH_WATER_MARK) {
            pthread_cond_wait(&cv_high, &mutex);
            if(done) {
                pthread_mutex_unlock(&mutex);
                return 1;
            }
        }
        // asserting that node count is maintained
        assert(count<HIGH_WATER_MARK);
        // Code copied from Sequential LRU but just inside a critical section
        // printf("thread now executing the reference commands\n");
        int found = 0;
        struct list_node* cursor = list_head;
        struct list_node* last = NULL;
        while(cursor) {
            if (cursor->key < key) {
                last = cursor;
                cursor = cursor->next;
            } else {
                if (cursor->key == key) {
                    cursor->refcount++;
                    found++;
                }
                break;
            }
        }
        if (!found) {
            // Handle 2 cases: the list is empty/we are trying to put this at the
            // front, and we want to insert somewhere in the middle or end of the
            // list.
            struct list_node* new_node = malloc(sizeof(struct list_node));
            if (!new_node) {
                pthread_mutex_unlock(&mutex);
                return 0;
            }
            count++;
            new_node->key = key;
            new_node->refcount = 1;
            new_node->next = cursor;
            if (last == NULL)
                list_head = new_node;
            else
                last->next = new_node;
        }
    
        // gotta make sure we unlock any clean threads to avoid deadlock
        pthread_cond_signal(&cv_low);

    // done updating, so exit the lock
    pthread_mutex_unlock(&mutex);

    return 1;
}

/* Do a pass through all elements, either decrement the reference count,
 * or remove if it hasn't been referenced since last cleaning pass.
 *
 * check_water_mark: If 1, block until there are more elements in the cache
 * than the LOW_WATER_MARK.  This should only be 0 during self-testing or in
 * single-threaded mode.
 */
void clean(int check_water_mark) {
    /* Your code here */
 
    // Iterate through the list, and decrement the reference count. If the reference count drops to zero, delete the element. If check_water_mark is set, block until there are more than LOW_WATER_MARK elements in the list.
    
    // lock before updating list
    pthread_mutex_lock(&mutex);
        // make thread wait until count is valid when clean is 1
        // OR ignore if clean is 0
        while (check_water_mark && count <= LOW_WATER_MARK) {
        // while ((check_water_mark==1 && count <= LOW_WATER_MARK) || (check_water_mark==0)) {
            // no point waiting to clean if there is nothing to clean
            if(count==0) {
                pthread_mutex_unlock(&mutex);
                return;
            }
            pthread_cond_wait(&cv_low, &mutex);
            if(done) {
                pthread_mutex_unlock(&mutex);
                return;
            }
        }
        // asserting that node count is maintained
        if(check_water_mark) assert(count>LOW_WATER_MARK);
        // Code copied from Sequential LRU but just inside a critical section
        struct list_node* cursor = list_head;
        struct list_node* last = NULL;
        while(cursor) {
            cursor->refcount--;
            if (cursor->refcount == 0) {
                struct list_node* tmp = cursor;
                if (last) {
                     last->next = cursor->next;
                } else {
                     list_head = cursor->next;
                }
                tmp = cursor->next;
                free(cursor);
                cursor = tmp;
                count--;
                    // check if count is below HIGH_WATER_MARK, unblock waiting threads if it is
                    // if(count<HIGH_WATER_MARK)
                    //     pthread_cond_signal(&cv_high);
            } else {
                last = cursor;
                cursor = cursor->next;
            }
        }

        // gotta make sure we unlock any reference threads to avoid deadlock
        pthread_cond_signal(&cv_high);

    // done updating, so exit lock
    pthread_mutex_unlock(&mutex);

}

/* Optional shut-down routine to wake up blocked threads.
   May not be required. */
void shutdown_threads (void) {
    /* Your code here */

    // Wake up any blocked threads. May not need to do anything in each variant.
    
    // lock so we can signal all the other threads before anything else happens
    pthread_mutex_lock(&mutex);
        assert(!done);
        // set shutdown variable so waiting threads can exit
        done=1;
        // signal conditionals so threads can wakeup
        pthread_cond_broadcast(&cv_low);
        pthread_cond_broadcast(&cv_high);
    pthread_mutex_unlock(&mutex);

    return;
}

/* Print the contents of the list.  Mostly useful for debugging. */
void print (void) {
    /* Your code here */

    // lock here because we want to print the correct values & change the cursor value several times
    pthread_mutex_lock(&mutex);
        // assert(!done);
        if(done) {
            pthread_mutex_unlock(&mutex);
            return;
        }
        // Code copied from Sequential LRU but just inside a critical section
        printf("=== Starting list print ===\n");
        printf("=== Total count is %d ===\n", count);
        struct list_node* cursor = list_head;
        while(cursor) {
            printf ("Key %d, Ref Count %d\n", cursor->key, cursor->refcount);
            cursor = cursor->next;
        }
        printf("=== Ending list print ===\n");
    // done so exit lock
    pthread_mutex_unlock(&mutex);
}
