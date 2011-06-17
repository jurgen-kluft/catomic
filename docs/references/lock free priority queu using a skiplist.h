#ifndef INCLUDE_GUARD_pq_lock_free_H
#define INCLUDE_GUARD_pq_lock_free_H

/** @file pq_lock_free.h 
 *
 * This is an implementation of a concurrent priority queue as defined by 
 * Håkon Sundell and Philippas Tsaigas in the paper 
 * "Fast and Lock Free Concurrent Priority Queues for Multi-Thread Systems ".
 *
 * Unfortunately there is a bug somewhere, which I have not succeeded in
 * finding.  Other talented people have also taken a look at this, but the bug
 * has not been found.  If you do find it, please let me know!
 *
 * $Id: pq_lock_free.h 351 2008-08-28 00:24:40Z alt $ 
 *
 * Anders Thøgersen, 2008
 *
 * NOTES :
 *
 *   - randomLevel() could be made more efficient I think.
 *
 *   - How do we do block free memory management of the node.level array?
 *     
 *     * The current solution is to fix the size of the memory allocated for a node,
 *       potentially wasting a lot of space.
 *
 *     * See that we compile with correct memory alignment
 *      
 *     * We should probably use __sync_fetch_and_add, etc. in GCC 4.3 and
 *       onwards instead of atomic opts
 */

//#if defined(__GNUC__)
  //#define AO_USE_PENTIUM4_INSTRS
// TODO : portability
//#elif defined __windows__
//#define AO_ASSUME_WINDOWS98
//#endif

#include <stdlib.h>  
#include <string.h>  
#include <atomic_ops.h>

#if defined(PQ_TEST) || defined(DOT)
using namespace std;

#define DEBUG_REFCNTS
#define MAX_REFCNT 20000000
#define PAUSE usleep(2000)

volatile int free_nodes_reclaimed;
volatile int free_nodes_requested;

struct node;

vector<node*> nodes_released;
vector<node*> nodes_reclaimed;

// This is for getting a look at the value of the highest refcount
volatile AO_t HIGHEST_REFCNT;

#else
#define NDEBUG // remove asserts
#endif

#define AO_REQUIRE_CAS

// TODO : better random numbers
#define RAND rand

// Atomic ops README: <URL:/usr/share/doc/libatomic-ops-dev/README.txt>

/** @def CAS(addr, old, new) : if addr and old_val are equal replace addr with new_val.
 *
 * @return Non-zero if successful
 */
#define CAS(addr, old_val, new_val) AO_compare_and_swap((AO_t*)(addr), (AO_t)(old_val), (AO_t)(new_val))

/**
 * FAA : Atomically fetch and add a value.  Used for the refcounts.
 *
 * @return Old value of *ptr
 */
#define FAA(ptr, value) AO_fetch_and_add((AO_t*)(ptr), (AO_t)(value))

/**
 * RF : Read value so that it is in correspondance to earlier and later mem ops.
 */
#define RF(ptr)		AO_load_full(ptr)


/* Marking the last bit in a pointer */
#define IS_MARKED(ptr)    (((AO_t)(ptr)) & (AO_t)0x1)  /* reading last bit */
#define GET_MARKED(ptr)   (((AO_t)(ptr)) | (AO_t)0x1)  /* set the marked bit */
//#define GET_UNMARKED(ptr) (((AO_t)(ptr)) ^ (AO_t)0x1)  [> remove the marked bit <]
#define GET_UNMARKED(ptr) (((AO_t)(ptr)) & (AO_t)~0x1)  /* remove the marked bit - also if it is not set */

#define NODE_1 (node*)0x1

#define BACK_OFF_BUFFER_SZ 0xff + 1

#define BACK_OFF(var) AO_t backoff = var;                     \
                      do { -- backoff; } while (backoff > 0); \
                      FAA(&back_off_buffer_i, 1);             \
                      var *= 2; var += RF( &back_off_buffer[back_off_buffer_i & 0xff ] )
                      

#define INITIAL_BACKOFF 2


/* The fixed maximum level of a node. Could memory allocation be made dynamic somehow */
#define MAX_LEVEL 20 // Leaves room for ~ a million elements

/**
 * The nodes that make up elements in the priority queue.
 */
struct node {
	/* memory management stuff */
	AO_t refcnt;               // Combined refcount and claim bit.  Claim bit is set if the node is free
	volatile node *next_free;  // When in the free_list this pointer points to the next free node

	/* Actual priority queue data */
	AO_t key;           // priority
	int level;            // The level of this node
	int valid_level;      // TODO : Max. level of this node?
	AO_t *value;        // This is the value that recieves the priority of key.  This means it is the actual data.
	volatile node** next; // array of next pointers to the next nodes, size is next[level]
	node*  prev;          // This is for speeding up execution of HelpDelete.

	// Constructor for preallocating memory in the free_list
	// at least next_free should be NULL so we know when we have no more nodes
	// in the free_list.
	node() : refcnt(0), next_free(NULL), key(0), level(0), valid_level(0), value(NULL), next(NULL), prev(NULL) { 
        valid_level = MAX_LEVEL;
		next = new volatile node*[ MAX_LEVEL ];
#ifdef PQ_TEST
        memset(next, 0, MAX_LEVEL * sizeof(node*));
#endif
	}

	~node() {
		delete[] next;
	}
};

/**
 * Lock free memory management as described by Valois and corrected by Michael and Scott.
 *
 * We use the next_free field to maintain a singly linked list of free nodes.
 *
 * TODO : It would be nice to make this dynamic.
 *
 */
class pq_memory_management {

#ifdef PQ_TEST
	friend class pq_mem_test;
	AO_t allocs_and_frees;
#endif

private:

	volatile node *free_list; 

	AO_t prealloc;

public:

	/**
	 * pre allocate some nodes.
	 *
	 * TODO : Extend this so we can dynamically add or remove allocated nodes.
	 * TODO : This is not concurrent
	 */
	pq_memory_management(AO_t preallocated = 100 ) {

#ifdef PQ_TEST
    allocs_and_frees = 0;
#endif
	  volatile node *p;
	  p = free_list = new node();

	  prealloc = preallocated;
	  //cerr << "allocating space for " << preallocated << " elements " << endl;
	  for (AO_t i=0; i<preallocated; i++) {
          p->next_free = new node();
		  p->refcnt = 1; // Set claim bit
		  p = p->next_free;
#ifdef PQ_TEST
		  allocs_and_frees ++;
#endif
	  }
      p->refcnt = 1; // Set claim bit
	}

	/**
	 * Destructor to free allocated nodes.
	 */
	~pq_memory_management() {
		volatile node *p = free_list;
		if (p == NULL)
			return;
		while (p->next_free != NULL) {
			volatile node *old = p;
			p = p->next_free;
			delete old;
#ifdef PQ_TEST
		allocs_and_frees --;
#endif
		}
	}

	/**
	 * Unset the lowest bit.
	 */
	void CLEAR_LOWEST_BIT(AO_t *ptr) {
		AO_t vold, vnew;

		assert( ptr != NULL );
        assert( (*ptr & 0x1) != 0 );

		do {
			vold = *ptr;
            //vnew = vold - 1;
            vnew = vold & (~0x1); // XXX : Done with binary ops
		} while ( ! CAS(ptr, vold, vnew) );
	}

	/**
	 * Get a pointer to a piece of memory that we may use.  
	 */
	node *NEW() {
		while (true) {
			node *p = READ_NODE(&free_list);
			if (p == NULL) {
				std::cerr << "TODO : Out of memory error. Had " << prealloc << " nodes pre allocated " << std::endl;
                std::cerr.flush();
                usleep(200);
                return NULL;
			}

			assert( p != NULL );
			assert( p != (node*)0x1 );
			
			if ( CAS(&free_list, p, p->next_free) ) {
				CLEAR_LOWEST_BIT(&p->refcnt);
#if defined (DEBUG_REFCNTS)
                if ( p->refcnt > MAX_REFCNT ) {
                    cout << "REFCNT 1"<< endl;
                }
#endif
#if defined(PQ_TEST)
                free_nodes_requested ++;
#endif
				return p;
			} else
				RELEASE(p);
		}
	}


#if defined(PQ_TEST)
    // This could be useful to see that there are infact some free nodes
    bool nodes_left() {
        node *p = READ_NODE(&free_list);
        bool result = true;
        if (p == NULL)
            result = false;
        RELEASE(p);
        return result;
    }

    // Return a count of how many free nodes there are
    // not concurrent
    AO_t free_nodes() {
        node *p, *q;
        AO_t count = 0;

        p = (node*) free_list;
        while (p != NULL) {
            q = p;
            p = (node*)p->next_free;
            if ( ! IS_MARKED(q) )
                count ++;
#if defined(PQ_TEST)
            else
				cout << " node in the free list is marked. " << endl;
#endif
        }
        return count;
    }
#endif

	/**
	 * Used for reclaiming nodes when the refcount reaches zero.
	 */
	AO_t DECREMENT_AND_TEST_AND_SET(AO_t *ptr) {
		AO_t vold, vnew;

		assert( ptr != NULL );


        // XXX 
        // A refcount of 1
        if (ptr == (AO_t*)0x1) {
            //cout << "DATAS " << ptr <<  endl;
            //PAUSE;
            return 0;
        }
#if defined (DEBUG_REFCNTS)
        if ( *ptr > MAX_REFCNT ) {
            cout << "REFCNT 2"<< endl;
        }
#endif
        //if ( *ptr & 0x1 == 0) return 0;
        //if ( *ptr == 0 ) return 0;

		do {
			vold = *ptr;
#if defined( PQ_TEST )
	        if (vold == 1) {
	        	cout << " Problem! " << endl;
	        }
#endif
			vnew = vold - 2;
			if (vnew == 0)
				vnew = 0x1;
		} while ( ! CAS(ptr, vold, vnew) );
#if defined(PQ_TEST)
		if (vnew > MAX_REFCNT) {
        	cout << " Problem 2! " << endl;
		}
#endif
		return (vold - vnew) & 0x1;
	}

	/**
	 * Reclaim a pointer to some memory that is not beeing used anymore and put
	 * it into the list of free memory.
	 */
	void RECLAIM(volatile node *p) {
		volatile node *q;
#if defined( PQ_TEST )
		nodes_reclaimed.push_back((node *) p);
#endif
		do {
			q = free_list;
			p->next_free = q;
		} while ( ! CAS(&free_list, q, p) );
#if defined( PA_TEST )
		free_nodes_reclaimed ++;
#endif
	}

	/**
	 * Decrement the refcount and put the node back into the free list. 
	 */
	void RELEASE(volatile node *p) {
		if (p == NULL || p == NODE_1) 
            return;
#if defined (DEBUG_REFCNTS)
        if ( p->refcnt > MAX_REFCNT ) {
            cout << "REFCNT 6"<< endl;
        }
#endif
		if ( DECREMENT_AND_TEST_AND_SET((AO_t *) &p->refcnt) == 0) {
            return;
        }
        //cout << "Release " << p << " Level = " << p->level;

#if defined (DEBUG_REFCNTS)
        if ( p->refcnt > MAX_REFCNT ) {
            cout << "REFCNT 3"<< endl;
        }
#endif

        // XXX : Here 0x1's are released
		for (int i = 0; i< p->level; i++) {
			RELEASE( p->next[i]);
        }

		RECLAIM(p);
	}

	/**
	 * Dereference the pointer and increase the refcount for the node. If the node
	 * is marked we return NULL.
	 *
	 * This is implemented similarly to SafeRead in the above paper, except that
	 * we also check if a node is marked for deletion.
	 */
	node* READ_NODE(volatile node **p) {
        while (true) {

            volatile node *q = *p;
            if (q == NULL || IS_MARKED(q))
                return NULL;
            // XXX 1
            FAA(&q->refcnt, 2);
#if defined(PQ_TEST) && defined(DEBUG_REFCNTS)
            //cout << "I'm counting " << q->refcnt << endl; 
            AO_t n1ref;
            AO_t ref;
            do {
                if ( IS_MARKED(q) )
                    break;

                n1ref = RF((AO_t*)&q->refcnt);
                ref = RF(&HIGHEST_REFCNT);
                if (ref >= n1ref)
                    break;
            } while (CAS(&HIGHEST_REFCNT, ref, n1ref)); 
#endif
            if (q == *p)
                return (node*)q;
            else
                RELEASE(q);
        }
	}

	/**
	 * Increase refcount for the node.
	 *
	 * @TODO : This method is not mentioned in the paper, so this might not be
	 * exactly what was intended.
	 *
	 * @note  We increment in steps of two to keep the claim bit untouched.  if this bit
	 * is set then we can release nodes this one points to and reclaim this node.
	 */
    node* COPY_NODE(volatile node *n1) {

        //assert( n1->refcnt != NULL );

        // TODO : I found a bug : a segfault occurs when the head is heavily
        // referenced and the amount of elements is larger than the max depth
        // allows. 
        //if (n1->refcnt > 100) {
        //cerr << " Large refcount for " << (node*) n1 << endl;
        //}
        FAA(&n1->refcnt, 2);
#if defined(PQ_TEST) && defined(DEBUG_REFCNTS)
        if ( n1->refcnt > MAX_REFCNT ) {
            cout << "REFCNT 4"<< endl;
        }
        AO_t n1ref;
        AO_t ref;
        do {
            if ( IS_MARKED(n1) ) {
                break;
            }
            n1ref = RF((AO_t*)&n1->refcnt);
            ref = RF(&HIGHEST_REFCNT);

            if (ref >= n1ref)
                break;

        }    while (CAS(&HIGHEST_REFCNT, ref, n1ref)); 
#if defined (DEBUG_REFCNTS)
        if ( n1->refcnt > MAX_REFCNT ) {
            cout << "REFCNT 5"<< endl;
        }
#endif
#endif
        return (node*)n1;
    }
};


class pq_lock_free {
private:

	volatile AO_t _size;
	int MaxLevel;
	pq_memory_management mem;

    // start values for back off
    AO_t back_off_buffer[BACK_OFF_BUFFER_SZ];
    AO_t back_off_buffer_i;

	/**
	 * Maximum node level we can create.
	 */
	volatile node *head;
	volatile node *tail;

	/**
	 * ReadNext traverses to the next node of n1 while helping with deletion.
	 */
	node *ReadNext(node **n1, int level) {

		assert( level < MAX_LEVEL );


		if ( IS_MARKED((*n1)->value) )
			*n1 = HelpDelete(*n1, level);

		node *n2 = mem.READ_NODE( &((*n1)->next[level]) ); 

        // XXX 1
		while (n2 == NULL) {
			*n1 = HelpDelete(*n1, level);
			n2 = mem.READ_NODE( &((*n1)->next[level]) );
		}
		return n2;
	}

	/**
	 * Traverse through the next pointers at the current level until finding a node
	 * with a key that is equal or higher than *n1->key.
	 */
	node *ScanKey(node **n1, int level, AO_t key) {

		node *n2 = ReadNext(n1, level);
		while (n2->key < key) {
			mem.RELEASE(*n1);
			*n1 = n2;
			n2  = ReadNext(n1, level);
		}
		return n2;
	}

	node *HelpDelete(node *n1, int level) {
		node *n2, *prev;

		assert( level < MAX_LEVEL );


		/* Set deletion marker on all next pointers if they have not been set */
		for (int i=level; i<n1->level; i++) {
			do {
				n2 = (node*) n1->next[i];

				assert( n1->next[i] != NULL );

			} while ( ! (  IS_MARKED(n2) || CAS(&n1->next[i], n2, GET_MARKED(n2))  ));
		}


#if defined (DEBUG_REFCNTS)
        if ( n1->refcnt > MAX_REFCNT ) {
            cout << "REFCNT 7"<< endl;
        }

        if ( n2->refcnt > MAX_REFCNT ) {
            cout << "REFCNT 8"<<n2<< endl;
        }
#endif

		/* See if the prev node is valid for deletion in this level. Otherwise find it */
		prev = n1->prev;
		if (prev == NULL || level >= prev->valid_level) {
			prev = mem.COPY_NODE(head);
			for (int i=MaxLevel-1; i>= level; i--) {
				n2 = ScanKey(&prev, i, n1->key);
				mem.RELEASE(n2);
			}
		} else mem.COPY_NODE(prev);

		/* Actual deletion of the prev node */
        AO_t back_count = INITIAL_BACKOFF;
		while (true) {
			/* H13 : Synchronize w. DeleteMin */
			if (n1->next[level] == NODE_1)
				break;

			node *last = ScanKey(&prev, level, n1->key);
			mem.RELEASE(last);

			/* H16 : Syncrhronization */
			if (last != n1 || n1->next[level] == NODE_1)
				break;

			if ( CAS(prev->next[level], n1, GET_UNMARKED(n1->next[level])) ) {
				n1->next[level] = NODE_1;
				break;
			}

			/* H20 : Sync */
			if (n1->next[level] == NODE_1)
				break;

            BACK_OFF(back_count);
		}
        // XXX Here the refcount of n1 is sometimes high too
        
#if defined (DEBUG_REFCNTS)
        if ( n1->refcnt > MAX_REFCNT ) {
            cout << "REFCNT 9"<< endl;
        }
#endif
		mem.RELEASE(n1);
		return prev;
	}

	/**
	 * Create a new node.
	 *
	 * TODO : How do we allocate memory for the level?  We need to be able to dynamically
	 *        specify the size of the memory block we need for the next pointers.
	 *
	 * TODO : Make this the constructor of struct node.
	 */
	node *CreateNode(int level, AO_t key, AO_t *value) {
		assert( level < MAX_LEVEL );
		node *n1 = mem.NEW();

		n1->level       = level;
		n1->valid_level = level;
		n1->key         = key;
		n1->value       = value;
        //n1->refcnt      = 1;      // XXX : Very important so we do not run out of nodes!!!
		n1->prev        = NULL;

		for (AO_t i=0; i<MAX_LEVEL; i++) {
			n1->next[i] = tail;
		}
		return n1;
	}

	/**
	 * Return a random level. This should somehow take notice of the
	 * number of elements and the maxdepth.
	 *
	 * This is defined as in the paper <URL:../doc/skiplists.pdf> except that
	 * the index starts at 0
	 *
	 * TODO : This could be more efficient.
	 */
	int randomLevel() {
		int lvl = 0;
		while (RAND() < RAND_MAX/2 && lvl < MaxLevel-1)
			++lvl;
		return lvl;
	}

	/**
	 * Inserting a value with the priority of key.
	 */
	bool insert(AO_t key, AO_t *value) {
		int level = randomLevel();
		node *nnew = CreateNode(level, key, value);
		mem.COPY_NODE(nnew);
		node *n1 = mem.COPY_NODE(head);

		node *n2;   
        node *savedNodes[ MAX_LEVEL ];

		for (int i = MaxLevel - 1; i>0; i--) {
			n2 = ScanKey(&n1, i, key);
			mem.RELEASE(n2);
			if (i<level)
				savedNodes[i] = mem.COPY_NODE(n1);
		}

        AO_t back_count = INITIAL_BACKOFF;
		while (true) {
			n2 = ScanKey(&n1, 0, key);
			AO_t *value2 = n2->value;

			// I12 : Is there a node w. the same priority, then overwrite it.
			if ( ! IS_MARKED(value2) && n2->key == key ) {
				if ( CAS(&n2->value, value2, value) ) {
					for (int i=1; i<level; i++) {
						mem.RELEASE(savedNodes[i]);
					}
					FAA(&_size, 1);
					mem.RELEASE(nnew);
					mem.RELEASE(nnew);
					return true; // true2
				} else continue;
			}

			// I21 : Try to insert new node starting from lowest level
			nnew->next[0] = n2;
			mem.RELEASE(n2);
			if ( CAS(&n1->next[0], n2, nnew) ) {
				mem.RELEASE(n1);
				break;
			}
            BACK_OFF(back_count);
		}

		// I27
		for (int i=1; i<level; i++) {
			nnew->valid_level = i;
			n1 = savedNodes[i];
            back_count = INITIAL_BACKOFF;
			while (true) {
				n2 = ScanKey(&n1, i, key);
				nnew->next[i] = n2;
				mem.RELEASE(n2);
				if ( IS_MARKED(nnew->value) || CAS(&n1->next[i], n2, nnew) ) {
					mem.RELEASE(n1);
					break;
				}
				BACK_OFF(back_count);
			}
		}
		nnew->valid_level = level;
		if ( IS_MARKED(nnew->value) )
			nnew = HelpDelete(nnew, 0);
		FAA(&_size, 1);
		mem.RELEASE(nnew);
		return true;
	}

	/**
	 * Remove and return the node with the highest priority.
	 */
	AO_t* DeleteMin() {
		node *n1, *prev;
		AO_t *value;
        // XXX 1
		prev = mem.COPY_NODE(head);
		// Find first node without the deletion mark set
		while (true) {
			n1 = ReadNext(&prev, 0);
			if (n1 == tail) {
				mem.RELEASE(prev);
				mem.RELEASE(n1);
				return NULL;
			}
retry:
			value = n1->value;
			if ( ! IS_MARKED(value) ) {
				if ( CAS(&n1->value, value, GET_MARKED(value)) ) {
					n1->prev = prev;
					break;
				} else
					goto retry;
			} else if ( IS_MARKED(value) ) {
				n1 = HelpDelete(n1, 0);
			}
			mem.RELEASE(prev);
			prev = n1;
		}
		// D20
		for (int i=0; i<n1->level; i++) {
			volatile node *n2;
			do {
				n2 = n1->next[i];
			} while ( ! (IS_MARKED(n2) || CAS(&n1->next[i], n2, GET_MARKED(n2))) );
		}
		prev = mem.COPY_NODE(head);
		for (int i=n1->level; i>=0; i--) {
			node *last;
            AO_t back_count = INITIAL_BACKOFF;
			while (true) {
				if (n1->next[i] == NODE_1)
					break;
				last = ScanKey(&prev, i, n1->key);
				mem.RELEASE(last);
				if (last != n1 || n1->next[i] == NODE_1)
					break;
				if ( CAS(&prev->next[i], n1, GET_UNMARKED(n1->next[i])) ) {
					n1->next[i] = NODE_1;
					break;
				}
				if (n1->next[i] == NODE_1)
					break;
				
                BACK_OFF(back_count);
			}
		}
		FAA(&_size, -1);
		mem.RELEASE(prev);
		mem.RELEASE(n1);
        // XXX This causes a bad call where the value is 0x1
        mem.RELEASE(n1);
		return value;
	}
                 
public:
	/// TODO
	typedef AO_t  prio_t;
	typedef AO_t* value_t;

    /**
	 * Create a new lock free priority queue.
	 * 
	 * TODO: Is this the right place to call srand?  It may be called several
	 *       times which might disturb the randomness.
	 *
	 * TODO : The GNU extensions drand48_r are reentrant and may be used in a
	 *        multithreaded application to generate the same sequence of random
	 *        numbers. Maybe these should be used.
	 */
	pq_lock_free(prio_t max_key, AO_t prealloc) 
	  : _size(0)
	   ,MaxLevel( MAX_LEVEL )
	   ,mem( prealloc ) 
		{
#if defined(PQ_TEST)
            free_nodes_reclaimed = 0;
            free_nodes_requested = 0;
#endif

        // srand() should have been called at this point!
        for (int i = 0; i < BACK_OFF_BUFFER_SZ; i++) {
            back_off_buffer[i] = rand() & 0x7;
        }
        back_off_buffer_i = 0;

		head = new node();
		tail = new node();

        // Must be set to one
        head->refcnt = 1;
        tail->refcnt = 1;

#if defined(PQ_TEST)
        HIGHEST_REFCNT = 0;
#endif

		//cerr << " head " << (node*) head << endl; 
		//cerr << " tail " << (node*) tail << endl; 

		// TODO : NIL element from skiplist paper.
		// The key must be greater than all other keys.
		tail->key = max_key;
		head->key = 0;

		for (int i=0; i<MAX_LEVEL; i++)
			tail->next[i] = NULL;

		for (int i=0; i<MAX_LEVEL; i++)
			head->next[i] = tail;

		//cerr << " head = " << (int*)head << endl;
		//cerr << " tail = " << (int*)tail << endl;
	}

#if defined(PQ_TEST)
    inline bool nodes_left() { return mem.nodes_left(); }
    inline AO_t free_nodes() { return mem.free_nodes(); }

    void show_max_refcount() {
        cout << "The highest refcount was " << HIGHEST_REFCNT << endl;
    }

    void show_free_nodes() {
        cout << " Free nodes " << free_nodes() << endl;
    }
    void show_free_list_counts() {
    	cout << " Free list requested " << free_nodes_requested << endl
    	     << " reclaimed           " << free_nodes_reclaimed << endl;
    }

    int get_reclaimed() { return free_nodes_reclaimed; }
    int get_requested() { return free_nodes_requested; }

    vector<node*>* get_reclaimed_nodes() { return &nodes_reclaimed; }
    vector<node*>* get_released_nodes() { return &nodes_released; }
#endif


	/**
	 * This is a nice C++ interface to this container.
	 */
	inline AO_t size() const { return _size; }
	inline bool empty() const { return _size == 0; }
	inline void push(AO_t prio, AO_t* key) { insert(prio, key); }
	inline AO_t* pop() { return DeleteMin(); }
    

	// TODO
	//inline node& top() {}

#ifdef DOT  /* Printing nice graphs in dot */
	node *dot_seen[1000];
	node **dot_idx;

	void do_print_node(node *n1, char const *name) {
		if (NULL == n1 || NODE_1 == n1 || IS_MARKED(n1))
			return;
		
		// Avoid duplicates
		for (int i=0; i<1000; i++)
			if (n1 == dot_seen[i])
				return;

		*dot_idx++ = n1;

		cout << '"' << n1 << '"' << " [fixedsize=true, width=1.3, label=\""<< (int*)n1 <<"\"];" << endl;
		for (AO_t i=0; i<MAX_LEVEL; i++) {
			cout << '"' << n1 << '"' << " -> \"" << (int*)(n1->next[i]) << "\" [label=\"\"];" << endl;
			do_print_node((node*)(n1->next[i]), "");
		}
	}
	void dot() {
		dot_idx = &dot_seen[0];
		cout << "digraph dot_thing {" << endl;
		do_print_node((node*)head, "head");
		cout << "}" << endl;
	}
#endif

};

void init() {
}

#endif /* INCLUDE_GUARD_pq_lock_free_H */
/* $Id: pq_lock_free.h 351 2008-08-28 00:24:40Z alt $ */
