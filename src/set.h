/**
This module contains the implementation of a collection of set with O(1) removal operation
**/
#ifndef __HEADER_SET_STRUCTURE
#define __HEADER_SET_STRUCTURE

#define NOT_IN_SET ((unsigned int)-1)

/**
 * @struct  element_pointer : structure specifying the set and the position of the element in its set.
 *
 * @set_index : set of the element, equal NOT_IN_SET if the element is not in any set.
 * @pointer : position of the element in its set
 */
struct element_pointer{
	unsigned int set_index;
	unsigned int pointer;
};

/**
 * @struct set : a data structure encoding a set.
 *
 * @set_index : index of the set.
 * @card : current number of elements in the set.
 * @max_card : maximum number of element that can be stored in the set before any reallocation.
 * @range : range of the elements in the sets (from 0 to range-1).
 * @elements : array of the indices of all elements in the set
 * @elm_ptr : array of struct element_pointer for all elements between 0 and @range
 */
struct set{
	unsigned int index;
	unsigned int card;
	unsigned int max_card;
	unsigned int range;
	unsigned int *elements;
	struct element_pointer *elm_ptr;
};

/**
 * @struct set_collection : Collection of sets
 *
 * @sets : array of sets
 * @nb_sets : number of sets
 */
struct set_collection{
	struct set *sets;
	unsigned int nb_sets;
};

/**
 * @initialise_set : initialise the given set
 *
 * @set : the set to initialise
 * @max_size : the maximum size of the set
 * @range : the range of the elements in the set
 * @set_index : the index of the set
 *
 * @remark : @set must be properly freed with @free_set to avoid memory leaks
 */
void initialise_set(struct set * set, unsigned int max_size, unsigned int range,
		    unsigned int index);

/**
 * @free_set : free @set
 *
 * @set : the set to free
 *
 * @remark : @set must have be properly allocated with @initialise_set
 */
void free_set(struct set * set);

/**
 * @add_element_set : adds @element to @set
 *
 * @set : the set @element must be added to
 * @element : the element to add in @set
 *
 * @warning : require that @element is not in @set
 */
void add_element_set(struct set * set, unsigned int element);

/**
 * @remove_element_set : removes @element from @set
 *
 * @set : the set @element must be removed from
 * @element : the element to remove from @set
 *
 * @warning : requires that @element is indeedd in @set
 */
void remove_element_set(struct set * set, unsigned int element);


/**
 * @initialise_set_collection : initialises @sets
 *
 * @sets : the set collection to initialise.
 * @n : the number of set in the collection to create.
 * @max_size : the maximum size of each set in the collection.
 * @range : the range of the element in the sets of the collection.
 *
 * @remark : @sets must be properly freed with @free_set_collection to avoid memory leaks
 */
void initialise_set_collection(struct set_collection * sets, unsigned int n,
			       unsigned int max_size, unsigned int range);

/**
 * @free_set_collection : free @sets
 *
 * @sets : the set collection to free
 *
 * @remark : @sets must have be properly allocated with @initialise_set_collection
 */

void free_set_collection(struct set_collection * sets);

/**
 * @add_element_set_collection : adds @element to the set of @sets with index @set_index
 *
 * @sets : the collection @element must be added to
 * @element : the element to add in @sets
 * @set_index : the index of the set of @sets element must be added to
 *
 * @warning : require that @element is not in @sets
 */
void add_element_set_collection(struct set_collection * sets, unsigned int element,
				unsigned int set_index);
/**
 * @remove_element_set_collection : removes @element from @sets
 *
 * @sets : the set collection @element must be removed from
 * @element : the element to remove from @sets
 *
 * @warning : require that @element is indeed in @sets
 */
void remove_element_set_collection(struct set_collection * sets, unsigned int element);

/**
 * @remove_all_elements_after_set : remove all elements in @sets that are in a set with index greater or equal to @set_index and put them in @array
 *
 * @sets : the set collections elements must be removed from.
 * @set_index : the minimum set index whose set must be emptied.
 * @array : the array the removed elements will be stored in.
 * @size : the number of elements in @array after removal.
 *
 * @remark : array must be able to contain all removed element
 */
void remove_all_elements_after_set(struct set_collection * sets,
				   unsigned int set_index, unsigned int *array,
				   unsigned int *size);

/**
 * @get_set_index : return the index of the set in @sets that contains @element
 *
 * @sets: the set collection we want to check
 * @element : the element whose set we want to know.
 *
 * @return the index of the set that contains @element
 */
unsigned int get_set_index(struct set_collection * sets, unsigned int element);

/**
 * @has_element_set_collection : check if @element is present in one of the sets in @sets
 *
 * @sets : the set collection we are looking for @element in.
 * @element : the element we are looking for.
 *
 * @return 1 is element is present, 0 otherwise.
 */
int has_element_set_collection(struct set_collection * sets, unsigned int element);

#endif
