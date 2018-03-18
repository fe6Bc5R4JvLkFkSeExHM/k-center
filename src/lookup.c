#include "utils.h"
#include "lookup.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

/**
 * @allocate_lookup_node : allocate a new lookup node for @element
 *
 * @element : the element of the new node
 * @k : length of the lookup list of the node
 *
 * @return a new allocated node with all but the element and lookup_list fields initialised to 0
 * @remark : the node must be freed with @free_lookup_node to avoid memory leak
 */
static struct lookup_node *allocate_lookup_node(unsigned int element,
						   unsigned int k)
{
	struct lookup_node *tmp;
	tmp = calloc_wrapper(1,sizeof(*tmp));
	tmp->element = element;
	tmp->lookup_list = calloc_wrapper(k, sizeof(*tmp->lookup_list));
	return tmp;
}
/**
 * @reset_lookup_node : reset @node to its state after being allocated.
 *
 * @node : the node to reset
 * @k : the length of the lookup_list
 */
static void reset_lookup_node(struct lookup_node * node, unsigned int k)
{
	unsigned int tmp_elem=node->element;
	struct interval *tmp_list=node->lookup_list;
	memset(node->lookup_list, 0, sizeof(*node->lookup_list) * k);
	memset(node,0,sizeof(*node));
	node->lookup_list=tmp_list;
	node->element=tmp_elem;
}

/**
 * @free_lookup_node : free @node
 *
 * @node : the node to free
 *
 * @remark : @node must have been previously allocated using @allocate_lookup_node.
 */
static void free_lookup_node(struct lookup_node * node)
{
	free(node->lookup_list);
	node->parent = node->next= node->previous= node->first_child=NULL;
	free(node);
}

int is_leaf_lookup(struct lookup_node * node)
{
	return NULL == node->first_child;
}

static void allocate_leftovers_lookup(struct lookup_table * lookup)
{
	lookup->leftovers =
	    calloc_wrapper(lookup->nb_level + 1, sizeof(*lookup->leftovers));
	lookup->leftovers_ptr =
	    calloc_wrapper(lookup->range_elements,
			   sizeof(*lookup->leftovers_ptr));
}

static void free_leftovers_lookup(struct lookup_table * lookup)
{
	unsigned int i;
	for (i = 0; i <= lookup->nb_level; i++) {
		if (NULL != lookup->leftovers[i].elements) {
			free(lookup->leftovers[i].elements);
			lookup->leftovers[i].elements = NULL;
			lookup->leftovers[i].nb_elements = 0;
			lookup->leftovers[i].max_elements = 0;
		}
	}
	free(lookup->leftovers);
	free(lookup->leftovers_ptr);
}

void initialise_lookup(struct lookup_table * lookup, unsigned int k,
				    unsigned int nb_level,
				    unsigned int range_elements)
{
	lookup->lookup_table =
		calloc_wrapper( k * nb_level, sizeof(*lookup->lookup_table));
	lookup->tmp = malloc_wrapper(sizeof(*lookup->tmp) * k);
	lookup->k = k;
	lookup->nb_level = nb_level;
	lookup->range_elements = range_elements;
	lookup->elements =
		calloc_wrapper(lookup->range_elements,sizeof(*lookup->elements));
	allocate_leftovers_lookup(lookup);
}

/**
 * @remove_lookup_ptr : remove all links pointing to @node from the lookup_table of lookup.
 *
 * @lookup : the lookup links must be remove from
 * @node : the node links must be removed from
 */
static void remove_lookup_ptr(struct lookup_table * lookup, struct lookup_node * node)
{
	struct interval *lookup_list = node->lookup_list;
	unsigned int i, j;
	for (i = 0; i < lookup->k; i++) {
		for (j = lookup_list[i].begin; j < lookup_list[i].end; j++) {
			lookup->lookup_table[j * lookup->k + i] = NULL;
		}
	}
}

/**
 * @unlink_lookup_child : remove @child from its parent
 *
 * @child : the node to remove
 */
static void unlink_lookup_child(struct lookup_node *child)
{
	if (child->parent) {
		if(!child->previous){
			child->parent->first_child=child->next;
			child->next->previous = NULL;
		} else {
			child->previous->next=child->next;
			child->next->previous =child-> previous;
		}
		if(child->next)
			child->next->previous=child->previous;
	}
}


void create_leaf_element_lookup(struct lookup_table * lookup, unsigned int element)
{
	struct lookup_node *tmp;
	struct lookup_info *info = lookup->elements + element;
	tmp = allocate_lookup_node(element, lookup->k);
	tmp->related_element = info;
	info->leaf = tmp;
	info->clusters=calloc_wrapper(lookup->k,sizeof(*info->clusters));
}

void remove_leaf_element_lookup(struct lookup_table * lookup, unsigned int element)
{
	struct lookup_node *tmp;
	struct lookup_info *info = lookup->elements + element;
	tmp = info->leaf;
	free(info->clusters);
	free_lookup_node(tmp);
}

void delete_lookup_tree(struct lookup_table * lookup, struct lookup_node * node)
{
	struct lookup_node *tmp;
	if (node->marked)
		remove_lookup_ptr(lookup, node);
	if(node->first_child){
		while(node->first_child){
			tmp=node->first_child;
			node->first_child=tmp->next;
			delete_lookup_tree(lookup, tmp);
		}
		free_lookup_node(node);
	} else {
		remove_leaf_element_lookup(lookup,node->element);
	}
}

void free_lookup(struct lookup_table * lookup)
{
	unsigned int index;
	for (index = lookup->k * lookup->nb_level; index > 0;) {
		index--;
		if (lookup->lookup_table[index]) {
			delete_lookup_tree(lookup, lookup->lookup_table[index]);
		}
	}
	free(lookup->elements);
	free_leftovers_lookup(lookup);
	free(lookup->lookup_table);
	free(lookup->tmp);
}

int has_element_lookup(struct lookup_table * lookup, unsigned int element)
{
	return NULL != lookup->elements[element].leaf;
}

int is_marked_element_lookup(struct lookup_table * lookup, unsigned int element)
{
	return lookup->elements[element].leaf->marked;
}

/**
 * @split_lookup_list : splits @old in two on the @cluster on the level @level. puts the second part in new.
 *
 * @old : the lookup list to split, the second part after the split will be removed
 * @new : the lookup list where the second part of @old will be stored
 * @length :  length of @old
 * @level : the value for @new[@cluster].begin and @old[@cluster].end
 * @cluster : the index old will be split on.
 */
static void split_lookup_list(struct interval *old, struct interval *new,
				     unsigned int length,
				     unsigned int level,
				     unsigned int cluster)
{
	if (cluster + 1 < length)
		memset(new + cluster + 1, 0,
		       (length - cluster - 1) * sizeof(*new));
	if (cluster) {
		memcpy(new, old, cluster * sizeof(*old));
		memset(old, 0, cluster * sizeof(*new));
	}
	new[cluster].end = old[cluster].end;
	new[cluster].begin = level;
	old[cluster].end = level;
}

/**
 * @merge_lookup_list_first : merge @second_part into @first_part.
 *
 * @first_part : the beginning part to merge, after the call, it will contains the merged list.
 * @second_part : the end part to merge, not modified by the function
 * @length : length of @first_part and @second_part
 * @cluster : the index on which @first_part and @second_part will be merged
 *
 * @remark : @first_part and @second_part must be consecutive
 */
static void merge_lookup_list_first(struct interval *first_part,
				     struct interval *second_part,
				unsigned int length,unsigned int cluster)
{
	first_part[cluster].begin = second_part[cluster].begin;
	if (cluster + 1 < length)
		memcpy(first_part + cluster + 1, second_part + cluster + 1,
		       (length - cluster - 1) * sizeof(*first_part));
}

/**
 * @merge_lookup_list_second : merge @first_part into @second_part.
 *
 * @first_part : the beginning part to merge, not modified by the function
 * @second_part : the end part to merge, after the call, it will contains the merged list.* @length : length of @first_part and @second_part
 * @cluster : the index on which @first_part and @second_part will be merged
 *
 * @remark : @first_part and @second_part must be consecutive
 */
static void merge_lookup_list_second(struct interval *first_part,
					     struct interval *second_part,
					unsigned int UNUSED(length),unsigned int cluster)
{
	second_part[cluster].end = first_part[cluster].end;
	if (cluster)
		memcpy(second_part, first_part, cluster * sizeof(*first_part));
}

void update_splitted_lookup_table(struct lookup_table * lookup,
					 struct lookup_node * node)
{
	unsigned int i, j;
	for (i = 0; i < lookup->k; i++)
		for (j = node->lookup_list[i].begin;
		     j < node->lookup_list[i].end; j++)
			lookup->lookup_table[j * lookup->k + i] = node;
}

/**
 * @split_lookup_node : split the given node in two, one being the child of the other
 * Split the given node in two. @node will correspond to the child after the call and the parent will be return by the function. @node will keep its children and the new node will obtain the parent and brothers of @node.
 *
 * @lookup : the lookup the split will impact
 * @node : the node to split
 * @level : the level on which to split @node
 * @cluster : the index of the cluster on which to split @node
 *
 * @return the newly created parent of @node
 */
static struct lookup_node *split_lookup_node(struct lookup_table * lookup,
					   struct lookup_node * node,
					   unsigned int level,
					   unsigned int cluster)
{
	struct lookup_node *tmp;
	tmp = allocate_lookup_node(node->element, lookup->k);
	tmp->parent = node->parent;
	if (node->parent) {
		if(!node->previous){
			node->parent->first_child=tmp;
		} else {
			node->previous->next=tmp;
			tmp->previous=node->previous;
			node->previous=NULL;
		}
		if(node->next){
			node->next->previous=tmp;
			tmp->next=node->next;
			node->next=NULL;
		}
	}
	node->parent = tmp;
	tmp->first_child = node;
	tmp->marked = 1;
	tmp->lowest = level;
	tmp->lowest_k=cluster;
	split_lookup_list(node->lookup_list, tmp->lookup_list, lookup->k,
				 level, cluster);
	update_splitted_lookup_table(lookup, tmp);
	return tmp;
}

/**
 * @update_merged_lookup_table : merges the pointer in the lookup table when two node are fused
 *
 * @lookup : the affected lookup table
 * @node : the merged node
 */
void update_merged_lookup_table(struct lookup_table * lookup,
				       struct lookup_node * node)
{
	unsigned int i, j;
	struct lookup_node **tmp;
	for (i = 0; i < lookup->k; i++) {
		for (j = node->lookup_list[i].begin;
		     j < node->lookup_list[i].end; j++) {
			tmp = lookup->lookup_table + j * lookup->k + i;
			if(node != *tmp)
				*tmp = node;
		}
	}
}

/**
 * @merge_unary_lookup_node : merge an unary node with its child. return the child
 *
 * @lookup : the impacted lookup table
 * @node : the node to merge with its unique child
 *
 * @return the child
 */
static struct lookup_node *merge_unary_lookup_node(struct lookup_table * lookup,
						 struct lookup_node * node)
{
	struct lookup_node *child = node->first_child;
	merge_lookup_list_first(node->lookup_list, child->lookup_list,
				lookup->k,node->lowest_k);
	free(child->lookup_list);
	child->lookup_list = node->lookup_list;
	node->lookup_list = NULL;
	child->parent = node->parent;
	if(node->parent){
		if(!node->previous){
			node->parent->first_child=child;
		} else {
			node->previous->next=child;
			child->previous=node->previous;
		}
		if(node->next){
			node->next->previous=child;
			child->next=node->next;
		}
	}
	child->marked=1;
	update_merged_lookup_table(lookup, child);
	free_lookup_node(node);
	return child;
}

/**
 * @graft_lookup_node : adds @child to the children of @parent
 *
 * @parent : the parent
 * @child : the child
 */
static void graft_lookup_node(struct lookup_node * parent,
			      struct lookup_node * child)
{
	child->parent = parent;
	child->next=parent->first_child;
	parent->first_child->previous=child;
	parent->first_child=child;
}

/**
 * @increase_leaftovers_capacity : increases the capacity of the specified leftovers
 *
 * @leftovers : the leftovers whose capacity must be increased
 */
static void increase_leftovers_capacity(struct leftovers *leftovers)
{
	size_t size = leftovers->max_elements;
	if (size == 0) {
		leftovers->elements =
		    malloc_wrapper(sizeof(*leftovers->elements) *
				   INIT_LEFTOVERS_SIZE);
		size = INIT_LEFTOVERS_SIZE;
	} else {
		leftovers->elements =
		    realloc_wrapper(leftovers->elements, &size,
				    sizeof(*leftovers->elements));
	}
	leftovers->max_elements = (unsigned int)size;
}

/**
 * @add_element_leftovers : add @element to the leftovers of @lookup on level @level
 *
 * @lookup : the lookup_table we want to modify
 * @element : the element to add
 * @level : the level of the leftovers that must be modified
 */ 
static void add_element_leftovers(struct lookup_table * lookup,
					 unsigned int element,
					 unsigned int level)
{
	struct leftovers *leftovers = lookup->leftovers + level;
	if (leftovers->nb_elements == leftovers->max_elements)
		increase_leftovers_capacity(leftovers);
	leftovers->elements[leftovers->nb_elements] = element;
	lookup->leftovers_ptr[element].position = leftovers->nb_elements;
	lookup->leftovers_ptr[element].level = level;
	(leftovers->nb_elements)++;
}

/**
 * @remove_element_leftovers : remove @element from the leftovers of @lookup
 *
 * @lookup : the lookup table to modify
 * @element : the element to remove
 */
static void remove_element_leftovers(struct lookup_table * lookup,
					    unsigned int element)
{
	struct leftovers *leftovers;
	unsigned int position;
	leftovers = lookup->leftovers + lookup->leftovers_ptr[element].level;
	position = lookup->leftovers_ptr[element].position;
	(leftovers->nb_elements)--;
	if (leftovers->nb_elements) {
		unsigned int last_element =
		    leftovers->elements[leftovers->nb_elements];
		lookup->leftovers_ptr[last_element].position = position;
		leftovers->elements[position] = last_element;
	}
}

/**
 * @add_lookup_ptr : add a new lookup_ptr to @node on level @level for cluster @cluster
 *
 * @lookup : the lookup table to modify
 * @node : the node to add a lookup_ptr to
 * @level : the level a new lookup_ptr will be created for
 * @cluster : the cluster index a new lookup_ptr will be created for
 */
static void add_lookup_ptr(struct lookup_table * lookup,
					 struct lookup_node * node,
					 unsigned int level,
					 unsigned int cluster)
{
	lookup->lookup_table[lookup->k * level + cluster] = node;
	if (!node->marked) {
		node->lowest = level;
		node->lowest_k = cluster;
		node->marked = 1;
	}
	if (0 == node->lookup_list[cluster].end)
		node->lookup_list[cluster].begin = level;
	node->lookup_list[cluster].end = level + 1;
}

void connect_element_lookup(struct lookup_table * lookup, unsigned int element,
			    unsigned int level, unsigned int cluster)
{
	struct lookup_node *element_node, *parent;
	if (cluster == lookup->k)
		return;
	element_node = lookup->elements[element].leaf;
	parent = lookup->lookup_table[lookup->k * level + cluster];
	if (parent == NULL) {
		if (!element_node->marked)
			add_element_leftovers(lookup, element,
						     level);
		add_lookup_ptr(lookup, element_node, level,
					     cluster);
	} else {
		if (!element_node->marked)
			add_element_leftovers(lookup, element,
						     level);
		if (is_leaf_lookup(parent) || parent->lowest < level)
			parent =
			    split_lookup_node(lookup, parent, level,
					      cluster);
		graft_lookup_node(parent, element_node);
	}
}

void add_highest_leftovers_lookup(struct lookup_table *lookup,unsigned int element){
	add_element_leftovers(lookup, element,lookup->nb_level);
}

/**
 * remove an unmarked element
 */
static void __remove_unmarked_element_lookup(struct lookup_table * lookup,
					     struct lookup_node * node)
{
	struct lookup_node *parent = node->parent;
	if(parent){
		unlink_lookup_child(node);
		if (parent->first_child->next == NULL)
			merge_unary_lookup_node(lookup, parent);
	}
	remove_element_leftovers(lookup, node->element);
	remove_leaf_element_lookup(lookup, node->element);
}

/**
 * compute the list of clusters the element of the given node is the center of
 *
 * @lookup : the impacted lookup table
 * @node : the node whose element we want to compute the list of
 * @lookup_list : the list where the result is stored
 */
static void lookup_list_recluster(struct lookup_table * lookup,
				  struct lookup_node * node,
				  struct interval *lookup_list)
{
	unsigned int i, element = node->element;
	while (node != NULL && element == node->element) {
		merge_lookup_list_second(node->lookup_list, lookup_list,
						lookup->k,node->lowest_k);
		node = node->parent;
	}
	for (i = 0; i < lookup->k && lookup_list[i].begin == lookup_list[i].end;
	     i++) ;
	lookup_list[i].end = lookup->nb_level;
}

/**
 * @extract_lookup_tree_leaves : remove the tree with root @node and store the the elements in all of the leaves in @array.
 *
 * @lookup : the impacted lookup table
 * @node : the root of the tree to remove
 * @array : the array where all leaves elements are stored
 * @nb_elements : the number of elements stored in @array
 *
 * @remark : @array should be able to hold all elements
 */
static void extract_lookup_tree_leaves(struct lookup_table * lookup,
				       struct lookup_node * node,
				       unsigned int array[],
				       unsigned int *nb_elements)
{
	struct lookup_node *tmp;
	if (node->marked)
		remove_lookup_ptr(lookup, node);
	if (is_leaf_lookup(node)) {
		reset_lookup_node(node, lookup->k);
		array[*nb_elements] = node->element;
		(*nb_elements)++;
		remove_element_leftovers(lookup, node->element);
		return;
	}
	while(node->first_child){
		tmp=node->first_child;
		node->first_child=node->first_child->next;
		extract_lookup_tree_leaves(lookup, tmp, array,nb_elements);
	}
	free_lookup_node(node);
}

/**
 * @extract_clusters_level_lookup : remove from @lookup all clusters after @cluster ( @cluster included) on level @level and stores the affected elements in @array after the initial value of @nb_elements. do not change the leftovers cluster of @level.
 *
 * @lookup : the impacted lookup_table
 * @level : the level to remove cluster from
 * @cluster : the smaller cluster index to remove from level @level
 * @array : an array where all affected elements are stored
 * @nb_elements : must be equal to the number of stored element in @array before the call of the function. it is updated accordingly.
 *
 * @remark : @array should be able to hold all elements
 */
static void extract_clusters_level_lookup(struct lookup_table * lookup, unsigned int level,
				     unsigned int cluster, unsigned int array[],
				     unsigned int *nb_elements)
{
	struct lookup_node *node, *parent;
	for (; cluster < lookup->k; cluster++) {
		node = lookup->lookup_table[lookup->k * level + cluster];
		if (NULL != node) {
			parent = node->parent;
			while (parent != NULL
			       && parent->element == node->element) {
				node = parent;
				parent = node->parent;
			}
			if (NULL != parent) {
				unlink_lookup_child(node);
				if (NULL == parent->first_child->next)
					merge_unary_lookup_node(lookup, parent);
			}
			extract_lookup_tree_leaves(lookup, node, array,
						   nb_elements);
		}
	}
}

/**
 * @lookup_extract_cluster_element : removes all clusters in @lookup with center @center and stores them in array
 *
 * @lookup : the impacted lookup table
 * @center : all clusters with @center as their center are extracted
 * @array : where the extracted elements are stored
 * @nb_elements : must be equal to the number of stored element in @array before the call of the function. it is updated accordingly.
 *
 * @remark : @array should be able to hold all elements
 */
static void extract_cluster_element_lookup(struct lookup_table * lookup,
					unsigned int center,
					unsigned int array[],
					unsigned int *nb_elements)
{
	struct lookup_node *node, *parent;
	node = lookup->elements[center].leaf;
	while (node->parent && node->parent->element == center)
		node = node->parent;
	parent = node->parent;
	if (NULL != parent) {
		unlink_lookup_child(node);
		if (NULL == parent->first_child->next )
			merge_unary_lookup_node(lookup, parent);
	}
	extract_lookup_tree_leaves(lookup, node, array, nb_elements);
}

/**
 * @remove_leftovers_level_lookup: extract all elements in the leftovers clusters of level @level and put them in @array
 *
 * @lookup : the impacted lookup table
 * @level : the level whose leftovers clusters must be extracted
 * @array : where the extracted elements are stored
 * @nb_elements : must be equal to the number of stored element in @array before the call of the function. it is updated accordingly.
 *
 * @remark : @array should be able to hold all extracted elements
 */
static void remove_leftovers_level_lookup(struct lookup_table * lookup,
					  unsigned int level, unsigned array[],
					  unsigned int *nb_elements)
{
	struct leftovers *tmp;
	for (; level <= lookup->nb_level; level++) {
		tmp = lookup->leftovers + level;
		if( 0 < tmp->nb_elements){
			while (0 < tmp->nb_elements) {
				extract_cluster_element_lookup(lookup,
						    tmp->elements[0],
						    array, nb_elements);
			}
		}
	}
}

/**
 * @__remove_marked_element_lookup : remove the element contained in @node from @lookup and stores all extracted elements in @array
 *
 * @lookup : the impacted lookup table
 * @node : the leaf node of the element to remove
 * @array : where the extracted elements are stored
 * @nb_elements : must be equal to the number of stored element in @array before the call of the function. it is updated accordingly.
 *
 * @remark : @array should be able to hold all extracted elements
 */
static void __remove_marked_element_lookup(struct lookup_table * lookup,
					   struct lookup_node * node,
					   unsigned int array[],
					   unsigned int *nb_elements)
{
	struct interval *lookup_list = lookup->tmp;
	unsigned int i, j, lowest;
	struct lookup_node *parent;
	memcpy(lookup_list, node->lookup_list,
	       sizeof(*lookup_list) * lookup->k);
	parent = node->parent;
	*nb_elements = 0;
	if (NULL != parent && parent->element == node->element)
		lookup_list_recluster(lookup, parent, lookup_list);
	lowest = lookup->nb_level;
	for (i = 0; i < lookup->k; i++) {
		if (lookup_list[i].begin != lookup_list[i].end) {
			lowest = lookup_list[i].begin;
			for (j = lookup_list[i].end; j > lookup_list[i].begin;) {
				j--;
				extract_clusters_level_lookup(lookup, j, i, array,
							 nb_elements);
			}
		}
	}
	remove_leftovers_level_lookup(lookup, lowest, array, nb_elements);
	for (i = 0; i < *nb_elements && array[i] != node->element; i++) ;
	(*nb_elements)--;
	array[i] = array[*nb_elements];
	remove_leaf_element_lookup(lookup, node->element);
}

int remove_element_lookup(struct lookup_table * lookup, unsigned int element,
			  unsigned int *helper_array, unsigned int *nb_elements)
{
	struct lookup_info *info = lookup->elements + element;
	if (!info->leaf->marked) {
		__remove_unmarked_element_lookup(lookup, info->leaf);
		*nb_elements=0;
		return 0;
	}
	__remove_marked_element_lookup(lookup, info->leaf, helper_array,
				       nb_elements);
	return 1;
}

unsigned int get_smallest_valid_level_lookup(struct lookup_table * lookup)
{
	unsigned int res;
	for (res = lookup->nb_level + 1;
	     res > 0 && 0 == lookup->leftovers[res - 1].nb_elements; res--) ;
	return res-1;
}

unsigned int get_nb_clusters_lookup(struct lookup_table * lookup,
				    unsigned int level)
{
	unsigned int i;
	for (i = 0;
	     i < lookup->k
	     && NULL != lookup->lookup_table[level * lookup->k + i];
	     i++) ;
	return i;
}

unsigned int get_cluster_lookup(struct lookup_table * lookup, unsigned int level,
				unsigned int element)
{
	unsigned int i;
	struct lookup_info *tmp = lookup->elements + element;
	for (i = 0; i < lookup->k; i++)
		if (tmp->clusters[i].begin <= level
		    && tmp->clusters[i].end > level)
			return i;
	return lookup->k;
}

void compute_cluster_element_lookup(struct lookup_table * lookup, unsigned int element)
{
	struct lookup_info *tmp = lookup->elements + element;
	struct lookup_node *node;
	memcpy(tmp->clusters, tmp->leaf->lookup_list,
	       sizeof(*tmp->clusters) * lookup->k);
	node = tmp->leaf->parent;
	while (node) {
		merge_lookup_list_second(node->lookup_list,
						tmp->clusters, lookup->k,node->lowest_k);
		node = node->parent;
	}
}
