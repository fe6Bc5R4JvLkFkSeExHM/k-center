/**
The module contains the code concerning the fully adversary algorithm on GPS point
**/
#ifndef __KCENTER_FULLY_ADV_HEADER__
#define __KCENTER_FULLY_ADV_HEADER__

#include "point.h"
#include "data_fully_adv.h"
#include "set.h"
#include "query.h"

#include <stdint.h>

typedef struct {
	unsigned int nb;	/* Number of cluster */
	unsigned int k;		/* Maximum number of cluster allowed */
	double radius;		/* maximum cluster radius of current level */
	unsigned int *centers;	/* index of center of each cluster */
	double *true_rad;	/* exact radius of each cluster */
	struct set_collection clusters;	/* content of all clusters */
	unsigned int nb_points;	/* total number of points in array */
	void *array;	/* pointer to all points */
} Fully_adv_cluster;

void fully_adv_initialise_level(Fully_adv_cluster * level, unsigned int k,
				double radius, void * array,
				unsigned int nb_points,
				unsigned int cluster_size);

void fully_adv_delete_level(Fully_adv_cluster * clusters);

void fully_adv_initialise_level_array(Fully_adv_cluster * levels[],
				      unsigned int k, double eps, double d_min,
				      double d_max, unsigned int *nb_instances,
				      void *points,
				      unsigned int nb_points,
				      unsigned int cluster_size,
				      unsigned int *helper_array[]);

void fully_adv_delete_level_array(Fully_adv_cluster levels[],
				  unsigned int nb_instances,
				  unsigned int helper_array[]);

double fully_adv_compute_true_radius(Fully_adv_cluster * level);

void fully_adv_k_center_add(Fully_adv_cluster * level, unsigned int index);

void fully_adv_k_center_delete(Fully_adv_cluster * level,
			       unsigned int element_index,
			       unsigned int helper_array[]);

void fully_adv_k_center_run(Fully_adv_cluster levels[],
			    unsigned int nb_instances, struct query_provider * queries,
			    unsigned int *helper_array);
#endif
