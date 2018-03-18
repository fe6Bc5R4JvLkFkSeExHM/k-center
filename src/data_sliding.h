/**
The module contains the code about the IO operation related to the sliding window algorithm on GPS point
**/
#ifndef __HEADER_DATA_SLIDING
#define __HEADER_DATA_SLIDING

#include "point.h"

/**
 * @sliding_distance : computes the distance between @a and @b
 *
 * @a : the first element
 * @b : the second element
 *
 * @return the distance between @a and @b
 */
double sliding_distance(void * a, void * b);

/**
 * @sliding_import_points : imports the points in the file @path and stores them in @point_array
 *
 * @point_array : the array where points will be stored. allocated in this function, it has to be freed by the caller
 * @nb_element : the number of point imported
 * @path : the path to the file where points are stored
 * @window_length : length of the sliding window
 *
 * @return NO_ERROR if no error happens and something else otherwise
 */
void sliding_import_points(void ** point_array,
			   unsigned int *nb_element, char *path,
			   unsigned int window_length);

#endif
