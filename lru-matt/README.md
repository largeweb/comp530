Matthew Gordon
PID: 730400003

Locking Protocol

Ex1:
    pthread_mutex_lock before reference code
    pthread_mutex_unlock after reference code, before return
    pthread_mutex_lock before clean code
    pthread_mutex_unlock after clean code, before return
    pthread_mutex_lock before print code
    pthread_mutex_unlock after clean code, before return

Ex2:
    Same as Ex1 but with a while loop at the beginning of the clean & reference locks
    And conditional signals at the end of the critical signals for the opposite function to ensure deadlocking does not occur
    (ie conditional signal at the end of clean to wakeup any threads potentially waiting in reference)
    I also added a simple locking protocol for shutdown_threads, that sets the done variable & wakes up the other threads that immediately check done

Ex3:
    Using Ex2 Code, adding onto the functions as follows...

    Reference:
        Base case: Grab global lock, create head if doesn't exist, release global lock
        After base case: Grab global lock, grab head lock, read head node next, grab next node lock, set cursor & last AFTER having both of those nodes' locks
            After cursor and last are set, we release the global lock
            Traverse monkey-bar style, releasing last node lock, grabbing next node lock, update cursor & last, repeat
            When we find our node and grab the lock for it, we increment refcount and then release everything
            When we find the key is greater than our cursor (ie node we want doesn't exist):
                We already have the 2 locks for the to-be-added node's left & right neighbor (set as last & current)
                Create new_node, set last->next to new_node, set new_node->next to cursor, release all locks

    Clean(1): (>32 nodes are present)
        Grab global lock, grab head lock, refcount--, read head node next, grab next node lock, set cursor & last AFTER having both of those nodes' locks, repeat
            Repeat monkey-bar style, releasing last node lock, grabbing next node lock, refcount--, update cursor & last, repeat
            After cursor and last are set, we release the global lock
        Deleting Head: Grab global lock, grab head lock, read next node, grab next node lock, if last->refcount==0, set listhead to cursor & free last, read next, grab next lock, update cursor & last, etc...
        Deleting Middle: cursor & last are set, refcount of cursor is decremented, if cursor->refcount==0, grab cursor->next lock, set last->next to cursor->next, free(cursor), set cursor to next
        Deleting Tail: cursor & last are set, cursor->refcount is decremented, if cursor->refcount==0 and cursor->next undefined, set last->next to undefined, free(cursor), release everything
            


Also made a nice bash script for testing ( ./test )
bonus point? :)
