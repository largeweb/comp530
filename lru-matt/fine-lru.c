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
    // Protects this node's contents
    pthread_mutex_t mutex;
};

static struct list_node* list_head = NULL;

/* A static mutex; protects the count and head.
 * XXX: We will have to tolerate some lag in updating the count to avoid
 * deadlock. */
static pthread_mutex_t mutex;
static int count = 0;
static pthread_cond_t cv_low, cv_high;

static volatile int done = 0;

/* Initialize the mutex. */
int init (int numthreads) {
    /* Your code here */

    // initialize mutex with no attributes
    pthread_mutex_init(&mutex,NULL);
    // mutex now initialized & unlocked

    // initialize conditional variable
    pthread_cond_init(&cv_low, NULL);
    pthread_cond_init(&cv_high, NULL);

    /* Temporary code to suppress compiler warnings about unused variables.
     * You should remove the following lines once this file is implemented.
     */
    (void)list_head;
    (void)count;
    /* End temporary code */
    return 0;
}

/* Return 1 on success, 0 on failure.
 * Should set the reference count up by one if found; add if not.*/
int reference (int key) {
    /* Your code here */

    // Reference:
    //     Base case: Grab global lock, create head if doesn't exist, release global lock
    //     After base case: Grab global lock, grab head lock, read head node next, grab next node lock, set cursor & last AFTER having both of those nodes' locks
    //         After cursor and last are set, we release the global lock
    //         Traverse monkey-bar style, releasing last node lock, grabbing next node lock, update cursor & last, repeat
    //         When we find our node and grab the lock for it, we increment refcount and then release everything
    //         When we find the key is greater than our cursor (ie node we want doesn't exist):
    //             We already have the 2 locks for the to-be-added node's left & right neighbor (set as last & current)
    //             Create new_node, set last->next to new_node, set new_node->next to cursor, release all locks

    struct list_node* cursor;
    struct list_node* last;

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
        printf("thread now executing the reference commands\n");
        int found = 0;
    //     Base case: Grab global lock, create head if doesn't exist, release global lock
        if(!list_head) {
            printf("no list head, base case");
            list_head = malloc(sizeof(struct list_node));
            list_head->key=key;
            list_head->refcount=1;
            pthread_mutex_unlock(&mutex);
            return 1;
        } else {
            // // printf("we got head, not base case");
            // // pthread_mutex_lock(&list_head->mutex);
            // // struct list_node* cursor = list_head;
            // // if(cursor->next){
            // //         pthread_mutex_lock(&cursor->next->mutex);
            // // }
            // // struct list_node* last = list_head;
            // // struct list_node* cursor = list_head->next;
            // // pthread_mutex_lock(&cursor->mutex);
    //     After base case: Grab head lock, read head node next, grab next node lock, set cursor & last AFTER having both of those nodes' locks
            struct list_node* cursor = list_head;
            struct list_node* last = NULL;
            _Bool got_head=0;
            while(cursor) {
                pthread_mutex_lock(&cursor->mutex);
    //         Traverse monkey-bar style, releasing last node lock, grabbing next node lock, update cursor & last, repeat
                if (cursor->key < key) {
                    if(last) pthread_mutex_unlock(&last->mutex);
                    last = cursor;
                    if(cursor->next){
                        pthread_mutex_lock(&cursor->next->mutex);
                        cursor = cursor->next;
    //         After cursor and last are set, we release the global lock
                        if(!got_head) {
                            pthread_mutex_unlock(&mutex);
                            got_head=1;
                        }
                    }
                } else {
    //         When we find our node and grab the lock for it, we increment refcount and then release everything
                    if (cursor->key == key) {
                        cursor->refcount++;
                        found++;
                    }
                    break;
                }
            }
        }
        // Stuff I added here
    //         When we find the key is greater than our cursor (ie node we want doesn't exist):
    //             We already have the 2 locks for the to-be-added node's left & right neighbor (set as last & current)
    //             Create new_node, set last->next to new_node, set new_node->next to cursor, release all locks
    //     (not yet implemented)
        if (!found) {
            // Handle 2 cases: the list is empty/we are trying to put this at the
            // front, and we want to insert somewhere in the middle or end of the
            // list.
            printf("FIRST CASE, NOTHING");
            return 0;
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

    // Clean(1): (>32 nodes are present)
    //     Grab global lock, grab head lock, refcount--, read head node next, grab next node lock, set cursor & last AFTER having both of those nodes' locks, repeat
    //         Repeat monkey-bar style, releasing last node lock, grabbing next node lock, refcount--, update cursor & last, repeat
    //         After cursor and last are set, we release the global lock
    //     Deleting Head: Grab global lock, grab head lock, read next node, grab next node lock, if last->refcount==0, set listhead to cursor & free last, read next, grab next lock, update cursor & last, etc...
    //     Deleting Middle: cursor & last are set, refcount of cursor is decremented, if cursor->refcount==0, grab cursor->next lock, set last->next to cursor->next, free(cursor), set cursor to next
    //     Deleting Tail: cursor & last are set, cursor->refcount is decremented, if cursor->refcount==0 and cursor->next undefined, set last->next to undefined, free(cursor), release everything
    
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
    //         After cursor and last are set, we release the global lock
        struct list_node* cursor = list_head;
        struct list_node* last = NULL;
    //     Grab global lock, grab head lock, refcount--, read head node next, grab next node lock, set cursor & last AFTER having both of those nodes' locks, repeat
        while(cursor) {
    //         Repeat monkey-bar style, releasing last node lock, grabbing next node lock, refcount--, update cursor & last, repeat
            pthread_mutex_lock(&cursor->mutex);
            pthread_mutex_lock(&cursor->next->mutex);
            cursor->refcount--;
            if (cursor->refcount == 0) {
                struct list_node* tmp = cursor;
                if (last) {
                     last->next = cursor->next;
                } else {
    //     Deleting Middle: cursor & last are set, refcount of cursor is decremented, if cursor->refcount==0, grab cursor->next lock, set last->next to cursor->next, free(cursor), set cursor to next
    //     Deleting Tail: cursor & last are set, cursor->refcount is decremented, if cursor->refcount==0 and cursor->next undefined, set last->next to undefined, free(cursor), release everything
    //     not fully implemented, i have a headache...
                     list_head = cursor->next;
                     pthread_mutex_unlock(&last->mutex);
                }
    //     Deleting Head: Grab global lock, grab head lock, read next node, grab next node lock, if last->refcount==0, set listhead to cursor & free last, read next, grab next lock, update cursor & last, etc...
    //     not fully implemented, i have a headache...
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
}
