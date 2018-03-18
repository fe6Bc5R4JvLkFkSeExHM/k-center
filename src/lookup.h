#ifndef __HEADER_LOOKUP_STRUCTURE__
#define __HEADER_LOOKUP_STRUCTURE__

/**
 * @struct interval : represent an interval
 *
 * @begin : beginning of the interval
 * @end : end of interval
 */
struct interval {
	unsigned int begin;
	unsigned int end;
};

struct lookup_node;

/**
 * @struct lookup_info : list of clusters and pointer to the leaf node of an element
 *
 * @clusters : the list of clusters of the element
 * @leaf : the leaf node of the element
 */
struct lookup_info {
	struct interval *clusters;
	struct lookup_node *leaf;
};

/**
 * @struct lookup_node : node in the disjoint forest of the lookup structure
 *
 * @lookup_list : list of clusters this node correspond to.
 * @parent : parent node
 * @first_child : first child node
 * @previous : previous child of parent
 * @next : next child of parent
 * @related_element : in leaf, pointer to the lookup_info of @element
 * @element : in leaf : index of related element, in internal node : center.
 * @marked : 1 if @element is center of a cluster, 0 otherwise
 * @lowest : lowest index such this node represents a cluster, 0 for unmarked leaves.
 * @lowest_k : index of the cluster at @lowest level, 0 for unmarked leaves.
 *
 * @remark : the last child of a node must have the same center.
 */
struct lookup_node {
	struct interval *lookup_list;
	struct lookup_node *parent;
	struct lookup_node *first_child;
	struct lookup_node *previous;
	struct lookup_node *next;
	struct lookup_info *related_element;
	unsigned int element;
	char marked;
	unsigned int lowest;
	unsigned int lowest_k;
};

/**
 * @struct leftovers : leftovers cluster of a level
 *
 * @elements : array of all elements in leftovers
 * @nb_elements : number of elements
 * @max_elements : allocated size of @elements
 */
struct leftovers {
	unsigned int *elements;
	unsigned int nb_elements;
	unsigned int max_elements;
};

/**
 * @struct leftovers_ptr : structure that specify the level and position of the elements in the leftovers.
 *
 * @level : level of the element in the leftovers
 * @position : position of the element in the leftovers of level @level
 */
struct leftovers_ptr {
	unsigned int level;
	unsigned int position;
};

/**
 * Initial size of each leftovers level.
 */
#define INIT_LEFTOVERS_SIZE 100

/**
 * @struct lookup_table : structure representing a packed group of levels.
 *
 * @lookup_table : array of @k * @nb_levels pointers to the root node of each cluster
 * @elements : lookup_info for all elements
 * @k : maximum number of cluster per level
 * @nb_level : number of levels
 * @range_elements : specify the range of the index of the elements (from 0 to @range_element -1).
 * @leftovers : array of nb_level + 1 leftovers (highest = element inserted in no level). if an element is in @leftovers[i], it means that it left the leftovers cluster on level i.
 * @leftovers_ptr : leftovers_ptr for all elements
 * @tmp : small array of length @k used by some function
 */
struct lookup_table{
	struct lookup_node **lookup_table;
	struct lookup_info *elements;
	unsigned int k;
	unsigned int nb_level;
	unsigned int range_elements;
	struct leftovers *leftovers;
	struct leftovers_ptr *leftovers_ptr;
	struct interval *tmp;
};

/**
 * @initialise_lookup : initialise @lookup
 *
 * @lookup : the lookup table to initialise
 * @k : the maximum number of cluster per level
 * @nb_level : the number of level in the group
 * @range_elements : range of the elements in the group
 *
 * @remark : @lookup has to be freed using @free_lookup to avoid memory leaks
 */
void initialise_lookup(struct lookup_table * lookup, unsigned int k,
				    unsigned int nb_level,
				    unsigned int range_elements);
/**
 * @free_lookup : free @lookup
 *
 * @lookup : the lookup_table to free
 *
 * @remark : @lookup must have been previously initialised using @initialise_lookup
 */
void free_lookup(struct lookup_table * lookup);

/**
 * @is_leaf_lookup : check if @node is a leaf
 *
 * @node : the node to check.
 *
 * @return 1 if the given node is a leaf, 0 otherwise.
 */
int is_leaf_lookup(struct lookup_node * node);

/**
 * @has_element_lookup : check if @element has a leaf in @lookup
 *
 * @lookup : the lookup table to check.
 * @element the element to check.
 *
 * @return 1 if @element has a leaf in @lookup, 0 otherwise.
 */
int has_element_lookup(struct lookup_table * lookup, unsigned int element);

/**
 * @is_marked_element_lookup : check if @element is marked
 *
 * @lookup : the lookup to check.
 * @element : the element to check.
 *
 * @return 1 if @element is marked, 0 otherwise.
 */
int is_marked_element_lookup(struct lookup_table * lookup, unsigned int element);

/**
 * @create_leaf_element_lookup : create a leaf for @element in @lookup
 *
 * @lookup : the lookup to create the leaf in
 * @element : the element to create the leaf for
 */
void create_leaf_element_lookup(struct lookup_table * lookup, unsigned int element);

/**
 * @connect_element_lookup : add @element to the cluster @cluster of level @level in @lookup
 *
 * @lookup : the lookup table we want to modify
 * @element : the element to insert.
 * @level : the index of the level we want to add @element in.
 * @cluster : the index of the cluster we want to add @element in.
 */
void connect_element_lookup(struct lookup_table * lookup, unsigned int element,
			    unsigned int level, unsigned int cluster);
/**
 * @remove_element_lookup : remove @element from @lookup and put all elements that need to be reinserted in @array
 *
 * @lookup : the lookup table to remove the element from
 * @element : the element to remove
 * @array : array where the elements that need to be reinserted will be stored
 * @nb_elements : number of elements in @array
 *
 * @return 1 if the element was marked, 0 otherwise
 */
int remove_element_lookup(struct lookup_table * lookup, unsigned int element,
			  unsigned int *array,
			  unsigned int *nb_elements);
/**
 * @get_cluster_lookup : return the index of the cluster of @element on level @level
 *
 * @lookup : lookup in which we look for the cluster
 * @level : index of the level we want the cluster
 * @element : index of the element we want the cluster of
 */
unsigned int get_cluster_lookup(struct lookup_table * lookup, unsigned int level,
				unsigned int element);

/**
 * @get_smallest_valid_level_lookup : return the smallest level in @lookup that has an empty leftovers cluster
 *
 * @lookup : the lookup table we check
 *
 * @return the smallest level in @lookup that has an empty leftovers cluster
 */
unsigned int get_smallest_valid_level_lookup(struct lookup_table * lookup);

/**
 * @get_nb_clusters_lookup : return the number of clusters of @lookup on level @level
 *
 * @lookup : the lookup we want the number of clusters
 * @level : the index of the level of @lookup we want the number of clusters
 *
 * @return the number of clusters of @lookup on level @level
 */
unsigned int get_nb_clusters_lookup(struct lookup_table * lookup,
				    unsigned int level);
/**
 * @compte_cluster_element_lookup : compute the clusters of @element for all levels of @lookup
 *
 * @lookup : the lookup we want to compute the clusters of
 * @element : the index of the element we want to compute the cluster of
 */
void compute_cluster_element_lookup(struct lookup_table * lookup, unsigned int element);

/**
 * @add_highest_leftovers_lookup : add @element to the highest leftovers level of @lookup
 *
 * @lookup : the lookup we will modify the highest leftovers
 * @element : the element to insert in the highest leftovers
 */
void add_highest_leftovers_lookup(struct lookup_table *lookup,unsigned int element);
#endif
