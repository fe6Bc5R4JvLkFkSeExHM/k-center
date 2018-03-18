/**
The module contains the code concerning the fully adversary algorithm on GPS point
**/

#include "utils.h"
#include "query.h"
#include "data_fully_adv.h"
#include "algo_fully_adv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

void
fully_adv_initialise_level(Fully_adv_cluster * level, unsigned int k,
			   double radius, void * array,
			   unsigned int nb_points, unsigned int cluster_size)
{
	level->nb = 0;
	level->k = k;
	level->radius = radius;
	initialise_set_collection(&(level->clusters), k + 1, cluster_size,
				  nb_points);
	level->centers =
	    (unsigned int *)malloc_wrapper(sizeof(*(level->centers)) * (k + 1));
	level->true_rad = (double *)malloc_wrapper(sizeof(double) * k);
	level->nb_points = nb_points;
	level->array = array;
}

void fully_adv_delete_level(Fully_adv_cluster * level)
{
	free_set_collection(&(level->clusters));
	free(level->centers);
	free(level->true_rad);
	level->centers = NULL;
	level->nb_points = 0;
	level->array = NULL;
}

void fully_adv_initialise_level_array(Fully_adv_cluster * levels[],
				      unsigned int k, double eps, double d_min,
				      double d_max, unsigned int *nb_instances,
				      void *points,
				      unsigned int nb_points,
				      unsigned int cluster_size,
				      unsigned int *helper_array[])
{
	unsigned int i;
	unsigned int tmp;
	*nb_instances = tmp = (unsigned int)(1 + ceil(log(d_max / d_min) /
						      log(1 + eps)));
	*levels = (Fully_adv_cluster *) malloc_wrapper(sizeof(**levels) * tmp);
	*helper_array =
	    (unsigned int *)malloc_wrapper(sizeof(**helper_array) * nb_points);
	fully_adv_initialise_level((*levels), k, 0, points, nb_points,
				   cluster_size);
	for (i = 1; i < tmp; i++) {
		fully_adv_initialise_level((*levels) + i, k, d_min, points,
					   nb_points, cluster_size);
		d_min = (1 + eps) * d_min;
	}
}

void
fully_adv_delete_level_array(Fully_adv_cluster levels[],
			     unsigned int nb_instances,
			     unsigned int helper_array[])
{
	unsigned int i;
	for (i = 0; i < nb_instances; i++)
		fully_adv_delete_level(levels + i);
	free(levels);
	free(helper_array);
}

unsigned int fully_adv_get_index_smallest(Fully_adv_cluster levels[],
					  unsigned int nb_instances)
{
	unsigned int i;
	for (i = 0; i < nb_instances; i++)
		if (0 == levels[i].clusters.sets[levels[i].k].card)
			return i;
	return nb_instances;
}

void fully_adv_k_center_add(Fully_adv_cluster * level, unsigned int index)
{
	unsigned int i;
	double tmp;
	for (i = 0; i < level->nb; i++) {
		tmp = fully_adv_distance(level->array + index,
					 level->array + level->centers[i]);
		if (level->radius >= tmp) {
			add_element_set_collection(&(level->clusters), index,
						   i);
			level->true_rad[i] = MAX(tmp, level->true_rad[i]);
			return;
		}
	}
	add_element_set_collection(&(level->clusters), index, level->nb);
	if (level->nb < level->k) {
		level->centers[level->nb] = index;
		level->true_rad[level->nb] = 0;
		level->nb++;
	}
}

void
fully_adv_k_center_delete(Fully_adv_cluster * level, unsigned int element_index,
			  unsigned int helper_array[])
{
	unsigned int i, size, cluster_index;
	cluster_index = get_set_index(&(level->clusters), element_index);
	remove_element_set_collection(&(level->clusters), element_index);
	if (cluster_index < level-> k && element_index == level->centers[cluster_index]) {
		level->nb = cluster_index;
		remove_all_elements_after_set(&(level->clusters),
					      cluster_index, helper_array,
					      &size);
		shuffle_array(helper_array, size);
		for (i = 0; i < size; i++)
			fully_adv_k_center_add(level, helper_array[i]);
	}
}

double fully_adv_compute_true_radius(Fully_adv_cluster * level)
{
	unsigned int i;
	double max_rad = 0;
	for (i = 0; i < level->nb; i++)
		max_rad = MAX(max_rad, level->true_rad[i]);
	return max_rad;
}

Error_enum fully_adv_write_log(Fully_adv_cluster levels[],
			       unsigned int nb_instances,
			       unsigned int nb_points, struct query * query)
{
	unsigned int result;
	char key = query->type == ADD ? 'a' : 'd';
	if (has_log()) {
		result = fully_adv_get_index_smallest(levels, nb_instances);
		if (result == nb_instances) {
			printf
			    ("Error, no feasible radius possible found after inserting %u\n",
			     query->data_index);
			return ONLY_BAD_LEVELS_ERROR;
		}
		if (has_long_log())
			fprintf(get_log_file(), "%c %u %u c%u %lf %lf %u\n",
				key, query->data_index, nb_points, result,
				levels[result].radius,
				fully_adv_compute_true_radius(levels +
							      result),
				levels[result].nb);
		else
			fprintf(get_log_file(), "%c %u %u c%u %lf %u\n", key,
				query->data_index, nb_points, result,
				levels[result].radius, levels[result].nb);
	}
	return NO_ERROR;
}

Error_enum
fully_adv_apply_one_query(Fully_adv_cluster levels[], unsigned int nb_instances,
			  struct query * query, unsigned int *helper_array)
{
	unsigned int i;
	static unsigned int nb_points = 0;
	if (query->type == ADD) {
		printf("a %u\n", query->data_index);
		nb_points++;
		for (i = 0; i < nb_instances; i++)
			fully_adv_k_center_add(levels + i, query->data_index);
	} else {
		nb_points--;
		for (i = 0; i < nb_instances; i++)
			fully_adv_k_center_delete(levels + i,
						  query->data_index,
						  helper_array);
	}
	return fully_adv_write_log(levels, nb_instances, nb_points, query);
}

void
fully_adv_k_center_run(Fully_adv_cluster levels[], unsigned int nb_instances,
		       struct query_provider * queries, unsigned int helper_array[])
{
	struct query query;
	while (get_next_query_set(queries, &query, &(levels[0].clusters))) {
		fully_adv_apply_one_query(levels, nb_instances, &query,
					  helper_array);
	}
}
