// Luke Mitchell
// Linked List
// 22-11-2011
// edited 12/12/2011
// edited 27/12/2011

#ifndef LIST_H
#define LIST_H

/* Includes */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Types */

typedef struct node {
        uint32_t    addr;
	uint8_t   data;
	struct node*    next;
} node;

typedef struct list {
	int	size;
	node* 	start;
	node* 	end;
	node* 	current;
} list;

/* Function prototypes */

node*	list_search		(list*, uint32_t);
int	list_is_empty 		(list*);
void	list_reset		(list*);
int	list_add_node 		(list*, uint32_t, uint8_t);
int	list_add_node_rear 	(list*, uint32_t, uint8_t);
int	list_remove_node	(list*, node*);
int 	list_advance 		(list*);
int	list_retreat		(list*);
list*	list_create		(void);
void	list_destroy		(list*);

#endif
