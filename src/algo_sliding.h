/**
The module contains the code about the sliding window algorithm on GPS point
**/
#ifndef __KCENTER_SLIDING_HEADER__
#define __KCENTER_SLIDING_HEADER__

#include "point.h"

#include <stdint.h>

typedef struct {
	unsigned int k;		/* Maximum number of clusters allowed */
	double radius;		/* Cluster radius */
	unsigned int *elements;	/* the ancestor of each element */
	unsigned int attr_nb;	/* Number of attraction points */
	unsigned int *attr;	/* index of all attraction points, looping array */
	unsigned int *repr;	/* index of all representative points */
	unsigned int *orphans;	/* index of all orphans */
	unsigned int *parents;	/* index of the dead parent of each orphan */
	unsigned int first_attr;	/* index of oldest attractor in attr */
	unsigned int *centers;	/* index of all clusters */
	unsigned int cluster_nb;	/* true number of cluster */
	unsigned int *sp_points;	/* true assignment of every attractor and orphan in the clustering */
	unsigned int first_point;	/* oldest point */
	unsigned int last_point;	/* newest point */
	unsigned int nb_points;	/* total number of points in array */
	void *array;	/* pointer to all points */
} Sliding_level;

void sliding_initialise_level(Sliding_level * level, unsigned int k,
			      double radius, void * array,
			      unsigned int nb_points);

void sliding_delete_level(Sliding_level * level);

void sliding_k_center_add(Sliding_level * level, unsigned int element);

void sliding_compute_centers(Sliding_level * level);

unsigned int sliding_find_cluster(Sliding_level * level, unsigned int element);

void sliding_initialise_levels_array(Sliding_level * levels[], unsigned int k,
				     double eps, double d_min, double d_max,
				     unsigned int *nb_instances,
				     void * array,
				     unsigned int nb_points);

void sliding_delete_levels_array(Sliding_level levels[],
				 unsigned int nb_instances);

void sliding_k_center_run(Sliding_level levels[], unsigned int nb_instances);
#endif
