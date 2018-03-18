/**
The module contains the code about the sliding window algorithm on GPS point
**/

#include "utils.h"
#include "point.h"
#include "data_sliding.h"
#include "algo_sliding.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

void
sliding_initialise_level(Sliding_level * level, unsigned int k,
			 double radius, void * array,
			 unsigned int nb_points)
{
	unsigned int i;
	level->attr_nb = 0;
	level->k = k;
	level->radius = radius;
	level->elements = malloc_wrapper(sizeof(*level->elements) * nb_points);
	level->attr = malloc_wrapper(sizeof(*level->attr) * (k + 1));
	level->first_attr = 0;
	level->repr =
	    (unsigned int *)malloc_wrapper(sizeof(unsigned int) * (k + 1));
	level->orphans =
	    (unsigned int *)malloc_wrapper(sizeof(unsigned int) * (k + 2));
	level->parents =
	    (unsigned int *)malloc_wrapper(sizeof(unsigned int) * (k + 2));
	for (i = 0; i < k + 2; i++) {
		level->orphans[i] = (unsigned int)-1;
		level->parents[i] = (unsigned int)-1;
	}
	level->centers =
	    (unsigned int *)malloc_wrapper(sizeof(unsigned int) * (k + 1));
	level->cluster_nb = 0;
	level->sp_points =
	    (unsigned int *)malloc_wrapper(sizeof(unsigned int) * (2 * k + 3));
	level->first_point = 0;
	level->last_point = 0;
	level->nb_points = nb_points;
	level->array = array;
}

void sliding_delete_level(Sliding_level * level)
{
	free(level->elements);
	level->elements = NULL;
	free(level->attr);
	level->attr = NULL;
	free(level->repr);
	level->repr = NULL;
	free(level->orphans);
	level->orphans = NULL;
	free(level->parents);
	level->parents = NULL;
	free(level->centers);
	level->centers = NULL;
	free(level->sp_points);
	level->sp_points = NULL;
}

static void remove_expired_orphans(Sliding_level * level,
				   unsigned int first_point)
{
	unsigned int i;
	for (i = 0; i < level->k + 2; i++) {
		if ((unsigned int)-1 != level->orphans[i]
		    && level->orphans[i] < first_point) {
			level->orphans[i] = (unsigned int)-1;
			level->parents[i] = (unsigned int)-1;
		}
	}
}

static void create_orphan_simple(Sliding_level * level,
				 unsigned int parent, unsigned int orphan)
{
	unsigned int i;
	if (parent != orphan) {
		for (i = 0; i < level->k + 2; i++)
			if ((unsigned int)-1 == level->orphans[i]) {
				level->orphans[i] = orphan;
				level->parents[i] = parent;
				return;
			}
		/* should never happen, if it does, there's a bug */
		assert(0);
	}
}

static void create_orphan_complex(Sliding_level * level,
				  unsigned int parent, unsigned int orphan)
{
	unsigned int i;
	if (parent != orphan) {
		for (i = 0; i < level->k + 2; i++)
			if ((unsigned int)-1 == level->orphans[i]) {
				level->orphans[i] = orphan;
				level->parents[i] = parent;
				return;
			}
		remove_expired_orphans(level, level->attr[level->first_attr]);
		for (i = 0; i < level->k + 2; i++)
			if ((unsigned int)-1 == level->orphans[i]) {
				level->orphans[i] = orphan;
				level->parents[i] = parent;
				return;
			}
		/* should never happen, if it does, there's a bug */
		assert(0);
	}
}

static void remove_expired_attraction(Sliding_level * level)
{
	unsigned int orphan, parent;
	while (level->attr_nb
	       && level->attr[level->first_attr] < level->first_point) {
		orphan = level->repr[level->first_attr];
		parent = level->attr[level->first_attr];
		level->attr[level->first_attr] = (unsigned int)-1;
		level->repr[level->first_attr] = (unsigned int)-1;
		level->first_attr = (level->first_attr + 1) % (level->k + 1);
		(level->attr_nb)--;
		if (orphan >= level->first_point)
			create_orphan_simple(level, parent, orphan);
	}
}

static void remove_expired_points(Sliding_level * level, unsigned int exp_date)
{
	remove_expired_orphans(level, exp_date);
	remove_expired_attraction(level);
}

static void add_cluster(Sliding_level * level, unsigned int element)
{
	if (level->attr_nb > level->k) {
		unsigned int orphan = level->repr[level->first_attr];
		unsigned int parent = level->attr[level->first_attr];
		level->first_attr = (level->first_attr + 1) % (level->k + 1);
		(level->attr_nb)--;
		create_orphan_complex(level, parent, orphan);
	}
	if (level->attr_nb > level->k - 1) {
		remove_expired_orphans(level, level->attr[level->first_attr]);
	}
	level->elements[element] = element;
	level->attr[(level->first_attr + level->attr_nb) %
		    (level->k + 1)] = element;
	level->repr[(level->first_attr + level->attr_nb) %
		    (level->k + 1)] = element;
	level->attr_nb++;
	assert(level->attr_nb <= level->k + 1);
}

static int
__compute_centers(Sliding_level * level, unsigned int element,
		  unsigned int elm_index)
{
	unsigned int i;
	double tmp;
	for (i = 0; i < level->cluster_nb; i++) {
		tmp = sliding_distance(level->array+element,
				     level->array+level->centers[i]);
		if (level->radius >= tmp) {
			level->sp_points[elm_index] = level->centers[i];
			return 0;
		}
	}
	if (level->cluster_nb == level->k)
		return 1;
	level->centers[level->cluster_nb] = element;
	level->cluster_nb++;
	level->sp_points[elm_index] = element;
	return 0;
}

void sliding_compute_centers(Sliding_level * level)
{
	unsigned int i;
	unsigned int index;
	level->cluster_nb = 0;
	if (level->attr_nb > level->k) {
		return;
	}
	for (i = 0, index = level->first_attr; i < level->attr_nb;
	     i++, index = (index + 1) % (level->k + 1)) {
		level->centers[level->cluster_nb] = level->attr[index];
		level->cluster_nb++;
		level->sp_points[index] = level->attr[index];
	}
	for (i = 0; i < level->k + 2; i++) {
		if ((unsigned int)-1 != level->orphans[i])
			if (__compute_centers
			    (level, level->orphans[i], i + level->k + 1)) {
				level->centers[level->cluster_nb] =
				    level->orphans[i];
				level->cluster_nb = level->k + 1;
				return;
			}
	}
}

void sliding_k_center_add(Sliding_level * level, unsigned int element)
{
	unsigned int i, i_min, index, flag = 0;
	double d_min, tmp;
	Timestamped_point *array=array;
	level->last_point = element + 1;
	for (;
	     level->first_point <= element
	     && array[element].in_date >=
	     array[level->first_point].exp_date;
	     (level->first_point)++) ;
	remove_expired_points(level, level->first_point);
	for (i = 0, index = level->first_attr; i < level->attr_nb;
	     i++, index = (index + 1) % (level->k + 1)) {
		tmp =
		    sliding_distance(array + element,
				     array + level->attr[index]);
		if (level->radius >= tmp) {
			if (!flag) {
				flag = 1;
				d_min = tmp;
				i_min = index;
			} else if (d_min > tmp) {
				d_min = tmp;
				i_min = index;
			}
		}
	}
	if (!flag) {
		add_cluster(level, element);
	} else {
		level->elements[element] = level->attr[i_min];
		level->repr[i_min] = element;
	}
}

unsigned int sliding_find_cluster(Sliding_level * level, unsigned int element)
{
	unsigned int parent = level->elements[element], center;
	unsigned int i, j;
	for (i = 0; i < level->cluster_nb; i++) {
		if (parent == level->centers[i])
			return i;
	}
	for (i = 0; i < level->k + 2; i++) {
		if (level->orphans[i] != (unsigned int)-1
		    && level->parents[i] == parent) {
			center = level->sp_points[level->k + 1 + i];
			for (j = 0; j < level->cluster_nb; j++)
				if (center == level->centers[j])
					return j;
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_FAILURE);
}

void sliding_initialise_levels_array(Sliding_level * levels[], unsigned int k,
				     double eps, double d_min, double d_max,
				     unsigned int *nb_instances,
				     void * array,
				     unsigned int nb_points)
{
	unsigned int i;
	unsigned int tmp = (unsigned int)(1 + ceil(log(d_max / d_min) /
						   log(1 + eps)));
	*nb_instances = tmp;
	*levels = (Sliding_level *) malloc_wrapper(sizeof(**levels) * tmp);
	sliding_initialise_level(*levels, k, 0, array, nb_points);
	for (i = 1; i < tmp; i++) {
		sliding_initialise_level((*levels) + i, k, d_min,
					 array, nb_points);
		d_min = (1 + eps) * d_min;
	}
}

void
sliding_delete_levels_array(Sliding_level levels[], unsigned int nb_instances)
{
	unsigned int i;
	for (i = 0; i < nb_instances; i++)
		sliding_delete_level(levels + i);
	free(levels);
}

static unsigned int
sliding_get_index_smallest(Sliding_level levels[], unsigned int nb_instances)
{
	unsigned int i;
	unsigned int upper_limit = levels->k + 1;
	for (i = 0; i < nb_instances; i++)
		if (levels[i].attr_nb < upper_limit
		    && levels[i].cluster_nb < upper_limit)
			return i;
	return nb_instances;
}

double sliding_compute_true_radius(Sliding_level * level)
{
	double true_radius = 0, tmp;
	unsigned int i, index_cluster, index_center;
	for (i = level->first_point; i < level->last_point; i++) {
		index_cluster = sliding_find_cluster(level, i);
		index_center = level->centers[index_cluster];
		tmp = sliding_distance(level->array + i,
				       level->array + index_center);
		if (true_radius < tmp)
			true_radius = tmp;
	}
	return true_radius;
}

int sliding_write_log(Sliding_level levels[], unsigned int nb_instances,
		      unsigned int element)
{
	if (has_log()) {
		unsigned int result =
		    sliding_get_index_smallest(levels, nb_instances);
		if (result == nb_instances) {
			fprintf
			    (stderr,
			     "Error, no feasible radius possible found after inserting %d\n",
			     element);
			return ONLY_BAD_LEVELS_ERROR;
		}
		if (has_long_log())
			fprintf(get_log_file(),
				"a %d %d c%d %lf %lf %d\n",
				levels[result].last_point - 1,
				levels[result].last_point -
				levels[result].first_point,
				result, levels[result].radius,
				sliding_compute_true_radius
				(levels + result), levels[result].cluster_nb);
		else
			fprintf(get_log_file(), "a %d %d c%d %lf %d\n",
				levels[result].last_point - 1,
				levels[result].last_point -
				levels[result].first_point,
				result, levels[result].radius,
				levels[result].cluster_nb);
	}
	return NO_ERROR;
}

void sliding_k_center_run(Sliding_level levels[], unsigned int nb_instances)
{
	unsigned int i, j;
	for (i = 0; i < levels[0].nb_points; i++) {
		for (j = 0; j < nb_instances; j++)
			sliding_k_center_add(levels + j, i);
		for (j = 0; j < nb_instances; j++)
			sliding_compute_centers(levels + j);
		sliding_write_log(levels, nb_instances, i);
	}
}
