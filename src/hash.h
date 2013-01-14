// Luke Mitchell lm0466
// Principles of programming CW2
// Hash table/Linked list assignment

#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include "list.h"

// define the intial size for a hashtable
#define HASHTABLE_INITIAL_SIZE 2

// define the percentage utilisation threshold to be reached before resizing the table
#define HASHTABLE_UTILISATION_THRESHOLD 80

// data structure representing a  hashtable
typedef struct {
	int size;	// the size of the table
	int threshold;	// the threshold (%) at which the table size will be expanded
	int in_use;	// number of entries with data
	list** table;	
} hashtable;

// add/remove nodes
int 		hashtable_add_node 		(hashtable*, uint32_t, uint8_t);
int		hashtable_remove_node		(hashtable*, uint32_t);

// search table
node*           hashtable_search       		(hashtable*, uint32_t);
node**          hashtable_get_values            (hashtable*, int*);

// ctor and dtor
hashtable*      hashtable_create                (void);
void 		hashtable_destroy		(hashtable*);

#endif
