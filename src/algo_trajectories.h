/**
The module contains the code about the fully adversary algorithm on trajectories
**/
#ifndef __KCENTER_TRAJECTORIES_HEADER__
#define __KCENTER_TRAJECTORIES_HEADER__

#include "point.h"
#include "data_trajectories.h"
#include "set.h"
#include "query.h"

#include <stdint.h>

typedef struct {
	unsigned int nb;
	unsigned int k;
	double radius;
	unsigned int *centers;
	double *true_rad;
	struct set_collection clusters;
	unsigned int max_trajectories_nb;
	unsigned int current_trajectories_nb;
	Trajectory *trajectories;
} Trajectory_level;

void trajectories_initialise_level(Trajectory_level * level, unsigned int k,
				   double radius, Trajectory * array,
				   unsigned int nb_points);
void trajectories_delete_level(Trajectory_level * level);

extern int reset_cnt;

void trajectories_initialise_level_array(Trajectory_level *
					 levels[],
					 unsigned int k,
					 double eps, double d_min, double d_max,
					 unsigned int *nb_instances,
					 Trajectory * array,
					 unsigned int nb_points,
					 unsigned int *helper_array[]);

void trajectories_delete_level_array(Trajectory_level levels[],
				     unsigned int nb_instances,
				     unsigned int *helper_array);

void trajectories_k_center_run(Trajectory_level levels[],
			       unsigned int nb_instances,
			       struct query_provider * queries,
			       unsigned int helper_array[]);

void trajectories_parallel_k_center_run(struct query_provider * queries);

void trajectories_parallel_initialise_level_array(unsigned int k, double eps,
						  double d_min, double d_max,
						  Trajectory * array,
						  unsigned int nb_points,
						  unsigned int nb_threads);

void trajectories_parallel_delete_level_array(void);

#endif
