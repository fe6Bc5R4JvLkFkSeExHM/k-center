/**
This module contains the implementation of a collection of set with O(1) removal operation
**/
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "set.h"
#include "utils.h"

void initialise_set(struct set * set, unsigned int max_size, unsigned int range,
		    unsigned int set_index)
{
	assert(range >= max_size);
	set->elm_ptr = malloc_wrapper(sizeof(*(set->elm_ptr)) * range);
	memset(set->elm_ptr, -1, sizeof(*(set->elm_ptr)) * range);
	set->elements = malloc_wrapper(sizeof(unsigned int) * max_size);
	set->max_card = max_size;
	set->range = range;
	set->card = 0;
	set->index = set_index;
}

void free_set(struct set * set)
{
	free(set->elements);
	set->elements = NULL;
	free(set->elm_ptr);
	set->elm_ptr = NULL;
}

static void initialise_set_n_common(struct set * sets, unsigned int n, unsigned int max_size,
			     unsigned int range)
{
	unsigned int i;
	initialise_set(sets, max_size, range, 0);
	for (i = 1; i < n; i++) {
		sets[i].elm_ptr = sets[0].elm_ptr;
		sets[i].elements = malloc_wrapper(sizeof(unsigned int) * max_size);
		sets[i].max_card = max_size;
		sets[i].range = range;
		sets[i].card = 0;
		sets[i].index = i;
	}
}

static void free_set_n_common(struct set * sets, unsigned int n)
{
	free_set(sets);
	for (; 0 < n;) {
		n--;
		free(sets[n].elements);
		sets[n].elements = NULL;
	}
}

void
initialise_set_collection(struct set_collection * sets, unsigned int n,
			  unsigned int max_size, unsigned int range)
{
	sets->sets = (struct set *) malloc_wrapper(sizeof(*(sets->sets)) * n);
	sets->nb_sets = n;
	initialise_set_n_common(sets->sets, n, max_size, range);
}

void free_set_collection(struct set_collection * sets)
{
	free_set_n_common(sets->sets, sets->nb_sets);
	free(sets->sets);
	sets->sets = NULL;
	sets->nb_sets = 0;
}

void add_element_set(struct set * set, unsigned int element)
{
	assert(element < set->range);
	assert((unsigned)-1 == set->elm_ptr[element].set_index);
	assert(set->card < set->max_card);
	set->elements[set->card] = element;
	set->elm_ptr[element].set_index = set->index;
	set->elm_ptr[element].pointer = set->card;
	set->card++;
}

void remove_element_set(struct set * set, unsigned int element)
{
	unsigned int position;
	assert(set->set_index == set->elm_ptr[element].set_index);
	assert(0 < set->card);
	position = set->elm_ptr[element].pointer;
	set->card--;
	set->elm_ptr[element].set_index = NOT_IN_SET;
	set->elm_ptr[element].pointer = NOT_IN_SET;
	set->elements[position] = set->elements[set->card];
	set->elm_ptr[set->elements[position]].pointer = position;
}



void
add_element_set_collection(struct set_collection * sets, unsigned int element,
			   unsigned int set_index)
{
	assert(set_index < sets->nb_sets);
	add_element_set(sets->sets + set_index, element);
}

void remove_element_set_collection(struct set_collection * sets, unsigned int element)
{
	assert(NOT_IN_SET != sets->sets[0].elm_ptr[element].set_index);
	remove_element_set(sets->sets +
			   sets->sets[0].elm_ptr[element].set_index, element);
}


unsigned int get_set_index(struct set_collection * sets, unsigned int element)
{
	return sets->sets[0].elm_ptr[element].set_index;
}

void
remove_all_elements_after_set(struct set_collection * sets, unsigned int set_index,
			      unsigned int *array, unsigned int *size)
{
	*size = 0;
	for (; set_index < sets->nb_sets; set_index++) {
		unsigned int iter_set;
		for (iter_set = 0; iter_set < sets->sets[set_index].card;
		     iter_set++, (*size)++) {
			array[*size] = sets->sets[set_index].elements[iter_set];
			sets->sets[set_index].elm_ptr[array[*size]].pointer =
			    NOT_IN_SET;
			sets->sets[set_index].elm_ptr[array[*size]].set_index =
			    NOT_IN_SET;
		}
		sets->sets[set_index].card = 0;
	}

}

int has_element_set_collection(struct set_collection * sets, unsigned int element)
{
	return NOT_IN_SET != sets->sets[0].elm_ptr[element].set_index;
}
