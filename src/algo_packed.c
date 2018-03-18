/**
The module contains the code concerning the packed fully adversary algorithm on GPS point
**/

#include "utils.h"
#include "query.h"
#include "algo_packed.h"
#include "data_packed.h"
#include "lookup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

void
packed_initialise_level(Packed_level * level, unsigned int k,
			double base_radius, unsigned int nb_level,
			void * array, unsigned int nb_points)
{
	unsigned int i;
	initialise_lookup(&(level->lookup), k, nb_level, nb_points);
	level->array = array;
	level->nb_points = nb_points;
	level->radius = malloc(sizeof(*level->radius) * nb_level);
	for (i = 0; i < nb_level; i++) {
		level->radius[i] = base_radius;
		base_radius *= 2;
	}
}

void packed_free_level(Packed_level * level)
{
	free(level->radius);
	free_lookup(&(level->lookup));
}

void packed_initialise_levels_array(Packed_level * levels[], unsigned int k,
				    double eps, double d_min, double d_max,
				    unsigned int *nb_instances,
				    void * array,
				    unsigned int nb_points)
{
	unsigned int nb_level_total =
	    (unsigned int)(1 + ceil(log(d_max / d_min) / log(1 + eps)));
	unsigned int nb_groups =
	    (unsigned int)MAX(1, floor(log(2) / log(1 + eps)));
	unsigned int leftovers = nb_level_total % nb_groups;
	unsigned int level_per_group = nb_level_total / nb_groups;
	unsigned int i, flag;
	*levels = malloc_wrapper(nb_groups * sizeof(**levels));
	for (i = 0; i < nb_groups; i++) {
		flag = i < leftovers ? 1 : 0;
		packed_initialise_level(*levels + i, k, d_min,
					level_per_group + flag, array,
					nb_points);
		d_min = (1 + eps) * d_min;
	}
	*nb_instances = nb_groups;
}

void packed_free_levels_array(Packed_level levels[],
				unsigned int nb_instances)
{
	unsigned int i;
	for (i = 0; i < nb_instances; i++)
		packed_free_level(levels + i);
	free(levels);
}

unsigned int packed_level_get_index_smallest(Packed_level * level)
{
	return get_smallest_valid_level_lookup(&(level->lookup));
}

void packed_get_index_smallest(Packed_level levels[],
			       unsigned int nb_groups,
			       unsigned int *group_index,
			       unsigned int *instance_index)
{
	unsigned int i, tmp;
	*group_index = nb_groups;
	*instance_index = levels[0].lookup.nb_level;
	for (i = 0; i < nb_groups; i++) {
		tmp = packed_level_get_index_smallest(levels + i);
		if (tmp < *instance_index) {
			*group_index = i;
			*instance_index = tmp;
		}
	}
}

static int __packed_k_center_true_add(Packed_level * level, unsigned int element,
				 unsigned int level_index)
{
	unsigned int i;
	double tmp, radius = level->radius[level_index];
	unsigned int center;
	struct lookup_table *lookup = &(level->lookup);
	struct lookup_node **ptr_node;
	i = 0;
	if (is_marked_element_lookup(lookup, element))
		radius = radius / 2;
	ptr_node = lookup->lookup_table + lookup->k * level_index;
	while (i < lookup->k && *ptr_node) {
		center = (*ptr_node)->element;
		tmp = packed_distance(level->array + element,
				      level->array + center);
		if (radius >= tmp) {
			connect_element_lookup(lookup, element, level_index, i);
			return 0;
		}
		i++;
		ptr_node++;
	}
	connect_element_lookup(lookup, element, level_index, i);
	return 1;
}

static void __packed_k_center_add(Packed_level * level, unsigned int element)
{
	unsigned int i;
	for (i = 0;
	     i < level->lookup.nb_level
	     && __packed_k_center_true_add(level, element, i); i++) ;
	if(i == level->lookup.nb_level &&
		!is_marked_element_lookup(&(level->lookup),element))
		add_highest_leftovers_lookup(&(level->lookup),element);
	else
		compute_cluster_element_lookup(&(level->lookup), element);
}

void packed_k_center_add(Packed_level * level, unsigned int element)
{
	create_leaf_element_lookup(&(level->lookup), element);
	__packed_k_center_add(level,element);
}


void
packed_k_center_delete(Packed_level * level, unsigned int element_index,
		       unsigned int helper_array[])
{
	unsigned int size, i;
	if (remove_element_lookup(&(level->lookup),
				  element_index, helper_array, &size)) {
		shuffle_array(helper_array, size);
		for (i = 0; i < size; i++) {
			__packed_k_center_add(level, helper_array[i]);
		}
	}
}

double packed_compute_radius_cluster(struct lookup_node *node,void *point_array, void *center){
	double radius=0;
	struct lookup_node *tmp;
	if(is_leaf_lookup(node)){
		if( center == point_array + node->element)
			return 0;
		return packed_distance(center,point_array+node->element);
	}
	for(tmp=node->first_child; NULL != tmp; tmp=tmp->next)
		radius=MAX(radius,packed_compute_radius_cluster(tmp,point_array,center));
	return radius;
}

static double __packed_compute_true_radius(struct lookup_table * lookup, unsigned int level_index, void *array){
	double radius=0;
	struct lookup_node **tree;
	unsigned int i;
	for(i = 0, tree = lookup->lookup_table + lookup->k * level_index; i < lookup->k && NULL != *tree; i++, tree++)
		radius = MAX(radius,packed_compute_radius_cluster(*tree, array, array+ (*tree)->element));
	return radius;
}
static double packed_compute_true_radius(Packed_level * level,
					 unsigned int level_index)
{
	return __packed_compute_true_radius(&(level->lookup),level_index,level->array);
}

unsigned int packed_get_number_cluster(Packed_level * level,
				       unsigned int level_index)
{
	return get_nb_clusters_lookup(&(level->lookup), level_index);
}

Error_enum packed_write_log(Packed_level levels[],
			    unsigned int nb_groups,
			    unsigned int nb_points, struct query * query)
{
	unsigned int group_index, instance_index;
	char key = query->type == ADD ? 'a' : 'd';
	if (has_log()) {
		packed_get_index_smallest(levels, nb_groups, &group_index,
					  &instance_index);
		if (group_index == nb_groups) {
			printf
			    ("Error, no feasible radius possible found after inserting %u\n",
			     query->data_index);
			return ONLY_BAD_LEVELS_ERROR;
		}
		if (has_long_log()){
			double true_radius=packed_compute_true_radius(levels + group_index,instance_index);
			fprintf(get_log_file(), "%c %u %u c%u %lf %lf %u\n",
				key, query->data_index, nb_points,
				instance_index * nb_groups + group_index,
				levels[group_index].radius[instance_index],true_radius,
				packed_get_number_cluster(levels + group_index,
							  instance_index));
		} else {
			fprintf(get_log_file(), "%c %u %u c%u %lf %u\n", key,
				query->data_index, nb_points,
				instance_index * nb_groups + group_index,
				levels[group_index].radius[instance_index],
				packed_get_number_cluster(levels + group_index,
							  instance_index));
		}
	}
	return NO_ERROR;
}

Error_enum
packed_apply_one_query(Packed_level levels[], unsigned int nb_groups,
		       struct query * query, unsigned int *helper_array)
{
	unsigned int i;
	static unsigned int nb_points = 0;
	if (query->type == ADD) {
		nb_points++;
		for (i = 0; i < nb_groups; i++)
			packed_k_center_add(levels + i, query->data_index);
	} else {
		nb_points--;
		for (i = 0; i < nb_groups; i++) {
			packed_k_center_delete(levels + i,
					       query->data_index, helper_array);
		}
	}
	return packed_write_log(levels, nb_groups, nb_points, query);
}

void
packed_k_center_run(Packed_level levels[], unsigned int nb_groups,
		    struct query_provider * queries)
{
	struct query query;
	unsigned int *helper_array = malloc(sizeof(*helper_array) * levels->nb_points);
	while (get_next_query_lookup(queries, &query, &(levels[0].lookup))) {
		packed_apply_one_query(levels, nb_groups, &query, helper_array);
	}
}
