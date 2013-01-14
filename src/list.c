// Luke Mitchell

// Definition for linked list ADS

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

// returns the element of the list, l, containing the address specified
// returns NULL if not found
node* list_search (list* l, uint32_t addr)
{
	// begin at the start of the list
	list_reset (l);

	// empty list? return not found
	if (l->current == NULL)
		return NULL;
	
	// iterate through each list item, comparing strings
	do
	{
		if (l->current->addr == addr)
			return  l->current; // found, return the node
	} while (list_advance (l) != -1);
 
	// return 'not found'
	return NULL;
}

// returns 1 if the specified list is empty
// returns 0 if list isn't empty
int list_is_empty (list* l)
{
	return (l->start == NULL) ? 1 : 0;
}

// moves the current pointer to the start of the specified list
void list_reset (list* l)
{
	l->current = l->start;
}

// inserts an element at the front of the list
// the new element will contain the instruction and its address
// returns -1 on failure
// returns 0 on success
int list_add_node (list* l, uint32_t addr, uint8_t data)
{
	node* n = calloc (1, sizeof(node));

	if (n == NULL)
		return -1; 

        n->addr = addr;
	n->data = data;

	// empty list?
	if (list_is_empty (l))
	{
		// set the current and end pointers
		// to the new (and only) node
		l->current = n;
		l->end = n;
	}
	else
	{
		// no, merely set the next pointer
		// as the node was inserted at the front
		// of the list
		n->next = l->start;
	}

	// set the start pointer to the new node
	l->start = n;
	
	// and increment the list size counter
	l->size++;

	// return success
	return 0;
}

// inserts an element at the rear of the list
// see list_add_node for details of workings
// returns -1 on failure
// returns 0 on success
int list_add_node_rear (list* l, uint32_t addr, uint8_t data)
{
	node* n = calloc (1, sizeof(node));

        if (n == NULL)
                return -1;

	// copy the data
	n->addr = addr;
        n->data = data;

	// empty list?
	if (list_is_empty (l))
	{
		l->start = n;
                l->current = n;
        }
	else
	{
                l->end->next = n;
	}

        l->end = n;
	l->size++;
	
	return 0;
}
 	
// removes a specified node, n, from the specified list, l.
// returns -1 if n isn't found in l
// returns 0 if removal is successful
int list_remove_node (list* l, node* n)
{
	node *k, *j;

	k = l->start;
	j = n->next;

	// search list for preceeding node, k, to n
	while (k->next != n && k != l->end)
                k = k->next;

	// check we found the right node
	if (k->next != n)
		return -1;

	// update next pointer from k
	k->next = j;

	// check for special circumstances
	if (l->start == n)
		l->start = k;
	if (l->current == n)
		l->current = k;
	if (l->end == n)
		l->end = k;

	// free memory from n
	free (n);

	l->size--;

	return 0;
}

// advances the current pointer to the next item in the list
// if the current pointer points to the end of the list returns -1
// returns 0 on successful advance
int list_advance (list* l)
{
	if (l->current == l->end)
		return -1;
	
	l->current = l->current->next;
	return 0;
}

// retreats the current pointer to the previous item in the list
// returns -1 if the current pointer is the start
// returns 0 on successful retreat
int list_retreat (list* l)
{
	if (l->current == l->start)
		return -1;

	node* n = l->start;

	// find the preceeding list item
	while (n->next != l->current && n != l->end)
		n = n->next;

	// ensure we found it
	if (n->next != l->current)
		return -1;

	l->current = n;

	return 0;
}

// creates an empty list
// allocate memory and initialises struct fields
// returns a pointer to the list, or NULL if there's
// insufficient memory
list* list_create (void)
{
	// allocate memory for list
	list* l = malloc (sizeof(list));
	
	// if malloc failed, return null (value of l)
	if (l)
	{
		l->size = 0;
		l->start = NULL;
		l->end = NULL;
		l->current = NULL;
	}
	
	return l;
}

// destroys the specified list and frees all memory it occupied
void list_destroy (list* l)
{
	// free each item in the list
	list_reset (l);
	
	// remove (and free) each node individually
	do {
		list_remove_node (l, l->current);
	} while (list_advance(l) != -1);

	// free the list structure itself
	free (l);
}
