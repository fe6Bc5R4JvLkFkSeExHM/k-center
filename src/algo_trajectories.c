/**
The module contains the code about the fully adversary algorithm on trajectories
**/
#include "utils.h"
#include "point.h"
#include "data_trajectories.h"
#include "set.h"
#include "query.h"
#include "algo_trajectories.h"

#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <math.h>

void
trajectories_initialise_level(Trajectory_level * level, unsigned int k,
			      double radius, Trajectory * array,
			      unsigned int nb_points)
{
	level->nb = 0;
	level->k = k;
	level->radius = radius;
	initialise_set_collection(&(level->clusters), k + 1, nb_points,
				  nb_points);
	level->centers =
	    (unsigned int *)malloc_wrapper(sizeof(*(level->centers)) * (k + 1));
	level->true_rad =
	    (double *)malloc_wrapper(sizeof(*(level->true_rad)) * k);
	level->max_trajectories_nb = nb_points;
	level->current_trajectories_nb = 0;
	level->trajectories = array;
}

void trajectories_delete_level(Trajectory_level * level)
{
	free_set_collection(&(level->clusters));
	free(level->centers);
	free(level->true_rad);
	level->centers = NULL;
	level->true_rad = NULL;
	level->max_trajectories_nb = 0;
	level->current_trajectories_nb = 0;
	level->trajectories = NULL;
}

void trajectories_initialise_level_array(Trajectory_level * levels[],
					 unsigned int k,
					 double eps, double d_min, double d_max,
					 unsigned int *nb_instances,
					 Trajectory * array,
					 unsigned int nb_points,
					 unsigned int *helper_array[])
{
	unsigned int i;
	unsigned int tmp;
	*nb_instances = tmp = (unsigned int)(1 + ceil(log(d_max / d_min) /
						      log(1 + eps)));
	*levels = (Trajectory_level *) malloc_wrapper(sizeof(**levels) * tmp);
	*helper_array =
	    (unsigned int *)malloc_wrapper(sizeof(**helper_array) * nb_points);
	trajectories_initialise_level((*levels), k, 0, array, nb_points);
	for (i = 1; i < tmp; i++) {
		trajectories_initialise_level((*levels) + i, k, d_min,
					      array, nb_points);
		d_min = (1 + eps) * d_min;
	}
}

void
trajectories_delete_level_array(Trajectory_level levels[],
				unsigned int nb_instances,
				unsigned int helper_array[])
{
	unsigned int i;
	for (i = 0; i < nb_instances; i++)
		trajectories_delete_level(levels + i);
	free(levels);
	free(helper_array);
}

unsigned int
trajectories_get_index_smallest(Trajectory_level levels[],
				unsigned int nb_instances)
{
	unsigned int i;
	for (i = 0; i < nb_instances; i++)
		if (0 == levels[i].clusters.sets[levels[i].k].card)
			return i;
	return nb_instances;
}

double trajectories_compute_true_radius(Trajectory_level * level)
{
	unsigned int i;
	double max_rad = 0;
	for (i = 0; i < level->nb; i++)
		max_rad = MAX(max_rad, level->true_rad[i]);
	return max_rad;
}

static void trajectories_k_center_add(Trajectory_level * level,
				      unsigned int element)
{
	unsigned int i;
	double tmp;
	for (i = 0; i < level->nb; i++) {
		if (level->radius >=
		    (tmp =
		     trajectories_distance(level->trajectories + element,
					   level->trajectories +
					   level->centers[i]))) {
			add_element_set_collection(&(level->clusters), element,
						   i);
			level->true_rad[i] = MAX(tmp, level->true_rad[i]);
			return;
		}
	}
	/* no insertion possible */
	add_element_set_collection(&(level->clusters), element, level->nb);
	if (level->nb < level->k) {
		level->centers[level->nb] = element;
		level->true_rad[level->nb] = 0;
		level->nb++;
		return;
	}
}

static int
trajectories_is_center(Trajectory_level * level, unsigned int element,
		       unsigned int *cluster_index)
{
	*cluster_index = get_set_index(&(level->clusters), element);
	assert(((unsigned int)-1) != *cluster_index);
	if (level->k > *cluster_index
	    && level->centers[*cluster_index] == element)
		return 1;
	return 0;
}

static void
__trajectories_update_non_center(Trajectory_level * level,
				 unsigned int element,
				 unsigned int cluster_index)
{
	double tmp;
	if (cluster_index == level->k) {
		remove_element_set(level->clusters.sets + cluster_index,
				   element);
		trajectories_k_center_add(level, element);
		return;
	}
	tmp =
	    trajectories_distance(level->trajectories + element,
				  level->trajectories +
				  level->centers[cluster_index]);
	if (tmp > level->radius) {
		remove_element_set(level->clusters.sets + cluster_index,
				   element);
		trajectories_k_center_add(level, element);
	}
}

static unsigned int
__trajectories_reverse_update_non_center(Trajectory_level * level,
					 unsigned int element,
					 unsigned int cluster_index)
{
	double tmp =
	    trajectories_distance(level->trajectories +
				  level->centers[cluster_index],
				  level->trajectories + element);
	if (tmp > level->radius) {
		remove_element_set(level->clusters.sets + cluster_index,
				   element);
		trajectories_k_center_add(level, element);
		return 0;
	}
	return 1;
}

static void
__trajectories_update_center_restart(Trajectory_level * level,
				     unsigned int element,
				     unsigned int cluster_index,
				     unsigned int *helper_array)
{
	unsigned int i, size;
	remove_element_set(level->clusters.sets + cluster_index, element);
	level->nb = cluster_index;
	remove_all_elements_after_set(&(level->clusters), cluster_index,
				      helper_array, &size);
	shuffle_array(helper_array, size);
	for (i = 0; i < size; i++)
		trajectories_k_center_add(level, helper_array[i]);
	trajectories_k_center_add(level, element);
}

static unsigned int
__trajectories_check_legit_center(Trajectory_level * level,
				  unsigned int center_index,
				  unsigned int cluster_index)
{
	unsigned int i;
	double tmp;
	for (i = 0; i < cluster_index; i++) {
		tmp =
		    trajectories_distance(level->trajectories + center_index,
					  level->trajectories +
					  level->centers[i]);
		if (level->radius >= tmp)
			return 0;
	}
	i++;
	for (; i < level->nb; i++) {
		tmp =
		    trajectories_distance(level->trajectories + center_index,
					  level->trajectories +
					  level->centers[i]);
		if (level->radius >= tmp)
			return 0;
	}
	return 1;
}

static void
__trajectories_iterate_reverse_center_trash(Trajectory_level * level,
					    unsigned int center_index,
					    unsigned int cluster_index)
{
	unsigned int i, element;
	struct set *set = level->clusters.sets + level->k;
	double tmp;
	for (i = 0; i < set->card;) {
		element = set->elements[i];
		tmp =
		    trajectories_distance(level->trajectories + center_index,
					  level->trajectories + element);
		if (tmp <= level->radius) {
			remove_element_set(level->clusters.sets +
					   level->k, element);
			add_element_set_collection(&(level->clusters),
						   element, cluster_index);
			level->true_rad[cluster_index] =
			    MAX(tmp, level->true_rad[cluster_index]);
		} else
			i++;
	}
}

static void
__trajectories_iterate_reverse_center(Trajectory_level * level,
				      unsigned int center_index,
				      unsigned int cluster_index)
{
	unsigned int i, old;
	struct set *set = level->clusters.sets + cluster_index;
	old = (unsigned int)-1;
	for (i = 0; i < set->card;) {
		assert(old != set->elements[i]);
		old = set->elements[i];
		if (old != center_index) {
			i += __trajectories_reverse_update_non_center(level,
								      set->
								      elements
								      [i],
								      cluster_index);
		} else
			i++;
	}
}

static void
__trajectories_update_center(Trajectory_level * level, unsigned int index,
			     unsigned int cluster_index,
			     unsigned int helper_array[])
{
	if (__trajectories_check_legit_center(level, index, cluster_index)) {
		__trajectories_iterate_reverse_center(level, index,
						      cluster_index);
		__trajectories_iterate_reverse_center_trash(level, index,
							    cluster_index);
	} else
		__trajectories_update_center_restart(level, index,
						     cluster_index,
						     helper_array);
}

static void
trajectories_k_center_update(Trajectory_level * level, unsigned int index,
			     unsigned int helper_array[])
{
	unsigned int cluster_index;
	if (trajectories_is_center(level, index, &cluster_index))
		__trajectories_update_center(level, index, cluster_index,
					     helper_array);
	else
		__trajectories_update_non_center(level, index, cluster_index);
}

Error_enum trajectories_write_log(Trajectory_level levels[],
				  unsigned int nb_instances,
				  unsigned int nb_points, struct query * query)
{
	if (has_log()) {
		char key = query->type == ADD ? 'a' : 'u';
		unsigned int result =
		    trajectories_get_index_smallest(levels, nb_instances);
		if (result == nb_instances) {
			fprintf(stderr,
				"Error, no valid level found with bound given\n");
			return ONLY_BAD_LEVELS_ERROR;
		}
		if (!has_long_log())
			fprintf(get_log_file(), "%c %u %u c%u %lf %d\n", key,
				query->data_index, nb_points, result,
				levels[result].radius, levels[result].nb);
		else
			fprintf(get_log_file(), "%c %u %u c%u %lf %lf %d\n",
				key, query->data_index, nb_points, result,
				levels[result].radius,
				trajectories_compute_true_radius(levels
								 + result),
				levels[result].nb);
	}
	return NO_ERROR;
}

void
trajectories_apply_one_query(Trajectory_level levels[],
			     unsigned int nb_instances, struct query * query,
			     unsigned int helper_array[])
{
	unsigned int i;
	static unsigned int nb_points = 0;
	if (add_point_trajectory(levels[0].trajectories + query->data_index)) {
		query->type = UPDATE;
		for (i = 0; i < nb_instances; i++)
			trajectories_k_center_update(levels + i,
						     query->data_index,
						     helper_array);
	} else {
		nb_points++;
		for (i = 0; i < nb_instances; i++)
			trajectories_k_center_add(levels + i,
						  query->data_index);
	}
	trajectories_write_log(levels, nb_instances, nb_points, query);
}

void
trajectories_k_center_run(Trajectory_level levels[],
			  unsigned int nb_instances, struct query_provider * queries,
			  unsigned int helper_array[])
{
	struct query query;
	while (get_next_query_trajectories(queries, &query)) {
		trajectories_apply_one_query(levels, nb_instances,
					     &query, helper_array);
	}
}

struct query master_query;
Trajectory_level *master_levels = NULL;
unsigned int master_nb_instances = 0;

pthread_mutex_t mutex_done = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_done = PTHREAD_COND_INITIALIZER;
unsigned int shared_done = 0;

pthread_mutex_t mutex_to_do = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_to_do = PTHREAD_COND_INITIALIZER;
unsigned int shared_to_do = 0;

int end_program = 0;

pthread_t *threads = NULL;

void feed_workers(struct query * query, unsigned int *nb_points)
{
	if (pthread_mutex_lock(&mutex_to_do)) {
		perror("Problem with mutex in master\n");
		exit(EXIT_FAILURE);
	}
	if (add_point_trajectory
	    (master_levels[0].trajectories + query->data_index)) {
		master_query.type = UPDATE;
		master_query.data_index = query->data_index;
	} else {
		(*nb_points)++;
		master_query.type = ADD;
		master_query.data_index = query->data_index;
	}
	shared_to_do = master_nb_instances;
	pthread_cond_broadcast(&cond_to_do);
	pthread_mutex_unlock(&mutex_to_do);
}

void wait_workers(void)
{
	if (pthread_mutex_lock(&mutex_done)) {
		perror("Problem with mutex in master\n");
		exit(EXIT_FAILURE);
	}
	while (shared_done != master_nb_instances)
		pthread_cond_wait(&cond_done, &mutex_done);
	shared_done = 0;
	pthread_mutex_unlock(&mutex_done);
}

static void trajectories_parallel_apply_one_query(struct query * query)
{
	static unsigned int nb_points = 0;
	struct timeval begin, end;
	if (has_time_log())
		gettimeofday(&begin, NULL);
	feed_workers(query, &nb_points);
	wait_workers();
/* wait for workers */
	if (has_time_log()) {
		gettimeofday(&end, NULL);
		store_time(&begin, &end);
	}
	trajectories_write_log(master_levels, master_nb_instances, nb_points,
			       query);
}

void trajectories_parallel_k_center_run(struct query_provider * queries)
{
	struct query query;
	while (get_next_query_trajectories(queries, &query)) {
		trajectories_parallel_apply_one_query(&query);
	}
	pthread_mutex_lock(&mutex_to_do);
	pthread_cond_broadcast(&cond_to_do);
	end_program = 1;
	pthread_mutex_unlock(&mutex_to_do);
}

static void
trajectories_apply_one_query_one_level(Trajectory_level * level,
				       struct query * query,
				       unsigned int helper_array[])
{
	switch (query->type) {
	case ADD:
		trajectories_k_center_add(level, query->data_index);
		break;
	case UPDATE:
		trajectories_k_center_update(level, query->data_index,
					     helper_array);
		break;
	default:
		fprintf(stderr, "Unknown query type %d\n", query->type);
		exit(EXIT_FAILURE);
	}
}

static void *worker_thread(void *args)
{
	unsigned int *helper_array = (unsigned int *)args;
	unsigned int chosen;
	while (1) {
		if (pthread_mutex_lock(&mutex_to_do)) {
			perror("Problem with mutex in worker\n");
			exit(EXIT_FAILURE);
		}
		while (!shared_to_do)
			pthread_cond_wait(&cond_to_do, &mutex_to_do);
		if (end_program) {
			free(helper_array);
			return NULL;
		}
		shared_to_do--;
		chosen = shared_to_do;
		pthread_mutex_unlock(&mutex_to_do);
		trajectories_apply_one_query_one_level(master_levels +
						       chosen, &master_query,
						       helper_array);
		if (pthread_mutex_lock(&mutex_done)) {
			perror("Problem with mutex in worker\n");
			exit(EXIT_FAILURE);
		}
		shared_done++;
		if (master_nb_instances == shared_done)
			pthread_cond_signal(&cond_done);
		pthread_mutex_unlock(&mutex_done);
	}
}

void trajectories_parallel_initialise_level_array(unsigned int k, double eps,
						  double d_min, double d_max,
						  Trajectory * array,
						  unsigned int nb_points,
						  unsigned int nb_threads)
{
	unsigned int i, *tmp, nb_instances;
	nb_instances = (unsigned int)(1 + ceil(log(d_max / d_min) /
					       log(1 + eps)));
	master_levels =
	    (Trajectory_level *) malloc_wrapper(sizeof(Trajectory_level) *
						nb_instances);
	master_nb_instances = nb_instances;
	trajectories_initialise_level(master_levels, k, 0, array, nb_points);
	for (i = 1; i < nb_instances; i++) {
		trajectories_initialise_level(master_levels + i, k,
					      d_min, array, nb_points);
		d_min = (1 + eps) * d_min;
	}
	threads = (pthread_t *) malloc_wrapper(sizeof(pthread_t) * nb_threads);
	for (i = 0; i < nb_threads; i++) {
		tmp = (unsigned int *)malloc_wrapper(sizeof(*tmp) * nb_points);
		if (pthread_create(threads + i, NULL, worker_thread, tmp)) {
			perror("Can not create thread !");
			exit(EXIT_FAILURE);
		}
	}
}

void trajectories_parallel_delete_level_array(void)
{
	unsigned int i;
	for (i = 0; i < master_nb_instances; i++)
		trajectories_delete_level(master_levels + i);
	free(master_levels);
	master_levels = NULL;
	master_nb_instances = 0;
	free(threads);
	threads = NULL;
	pthread_mutex_destroy(&mutex_done);
	pthread_mutex_destroy(&mutex_to_do);
	pthread_cond_destroy(&cond_done);
	pthread_cond_destroy(&cond_to_do);
}
