/**
The module contains the code about the sliding window algorithm on GPS point
**/
#ifndef __KCENTER_PACKED_HEADER__
#define __KCENTER_PACKED_HEADER__

#include "point.h"
#include "lookup.h"
#include "data_packed.h"

/**
 * The following structure corresponds to a group of levels.
*/
typedef struct {
	struct lookup_table lookup; /* The disjoint tree forest used to store the clusters */
        void *array; /* the list of all points */
	unsigned int nb_points; /* total number of points */
	double *radius; /* An array with the radius of each level in the group */
} Packed_level;

/**
 * Initialise a packed level
 */
void packed_initialise_level(Packed_level * level, unsigned int k,
			     double base_radius, unsigned int nb_level,
			     void * point_array, unsigned int nb_points);
/**
 * Delete a packed level
 */
void packed_free_level(Packed_level * level);

/**
 * Initialise an array of nb_instances packed levels.
 */
void packed_initialise_levels_array(Packed_level * levels[], unsigned int k,
				    double eps, double d_min, double d_max,
				    unsigned int *nb_instances,
				    void * array,
				    unsigned int nb_points);

/**
 * Delete an array of nb_instances packed levels.
 */
void packed_free_levels_array(Packed_level levels[],
				unsigned int nb_instances);

/**
 * Run the packed fully dynamic algorithm on a previously initialised array of packed level using the provided query_provider.
 */
void packed_k_center_run(Packed_level levels[], unsigned int nb_instances,
			 struct query_provider * queries);
#endif
