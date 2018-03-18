/**
This module defines the data structure used for trajectories and related function
**/

#ifndef __HEADER_DATA_TRAJECTORIES
#define __HEADER_DATA_TRAJECTORIES

#include "point.h"
#include "utils.h"

typedef struct {
	unsigned int max_length;	/* maximum length of trajectory */
	unsigned int current;	/* current number of point in trajectory */
	Geo_point *points;	/* list of points in trajectory */
} Trajectory;

/**
 * @trajectories_distance : computes the distance between @a and @b
 *
 * @a : the first element
 * @b : the second element
 *
 * @return the distance between @a and @b
 */
double trajectories_distance(Trajectory * a, Trajectory * b);

#define LIMIT_CHARACTER_LINE 10000000

/**
 * @trajectories_import_points: imports the points in the file @path and stores them in @trajectories_array
 *
 * @trajectories_array : the array where points will be stored. allocated in this function, it has to be freed by the caller
 * @nb_element : the number of point imported
 * @path : the path to the file where points are stored
 *
 * @return NO_ERROR if no error happens and something else otherwise
 */
Error_enum trajectories_import_points(Trajectory * trajectories_array[],
				      unsigned int *nb_element, char *path);
/**
Add a point to the trajectory and return its previous size
**/
unsigned int add_point_trajectory(Trajectory * a);

void trajectories_delete_points(Trajectory * trajectories);
#endif
