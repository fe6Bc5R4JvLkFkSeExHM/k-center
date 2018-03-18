/**
This module defines the data structure used for trajectories and related function
**/

#include "utils.h"
#include "point.h"
#include "data_trajectories.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static double hausdorff_distance(Trajectory * a, Trajectory * b)
{
	double cmax = 0, cmin, tmp;
	unsigned int i, j;
	for (i = 0; i < a->current; i++) {
		cmin = euclidean_distance(a->points + i, b->points);
		for (j = 0; j < b->current; j++) {
			tmp = euclidean_distance(a->points + i, b->points + j);
			if (tmp < cmax)
				break;
			if (tmp < cmin)
				cmin = tmp;
		}
		if (cmin > cmax)
			cmax = cmin;
	}
	return cmax;
}

double trajectories_distance(Trajectory * a, Trajectory * b)
{
	return MAX(hausdorff_distance(a, b), hausdorff_distance(b, a));
}

Error_enum trajectories_read_first_line(FILE * f, char *buffer,
					unsigned int *nb_elements,
					unsigned int *nb_points)
{
	char *tmp;
	if (!fgets(buffer, LIMIT_CHARACTER_LINE, f)) {
		fprintf(stderr, "Empty file !\n");
		return FILE_FORMAT_ERROR;
	}
	tmp = strtok(buffer, " \t\n");
	if (!tmp || strtoui_wrapper(tmp, nb_elements)) {
		fprintf(stderr, "Wrong first line\n");
		return FILE_FORMAT_ERROR;
	}
	tmp = strtok(NULL, " \t\n");
	if (!tmp || strtoui_wrapper(tmp, nb_points)) {
		fprintf(stderr, "Wrong first line\n");
		return FILE_FORMAT_ERROR;
	}
	return NO_ERROR;
}

Error_enum read_point_trajectory(Geo_point * point)
{
	char *coordinate;
	coordinate = strtok(NULL, " \t\n,");
	if (strtod_wrapper(coordinate, &(point->longitude)))
		return FILE_FORMAT_ERROR;
	coordinate = strtok(NULL, " \t\n,");
	if (strtod_wrapper(coordinate, &(point->latitude)))
		return FILE_FORMAT_ERROR;
	return NO_ERROR;
}

Error_enum read_trajectory(Trajectory * trajectory, char *buffer,
			   Geo_point * points, unsigned int *nb_points)
{
	unsigned int iter_points, max_length;
	char *tmp;
	tmp = strtok(buffer, "\t \n");
	if (!tmp)
		return FILE_FORMAT_ERROR;
	tmp = strtok(NULL, " \t\n");
	if (!tmp || strtoui_wrapper(tmp, &max_length))
		return FILE_FORMAT_ERROR;
	trajectory->current = 0;
	trajectory->points = points + *nb_points;
	for (iter_points = 0; iter_points < max_length; iter_points++) {
		if (read_point_trajectory(points + *nb_points))
			return FILE_FORMAT_ERROR;
		(*nb_points)++;
	}
	trajectory->max_length = max_length;
	return NO_ERROR;
}

Error_enum
trajectories_import_points(Trajectory ** trajectories,
			   unsigned int *nb_elements, char *path)
{
	char *buffer;
	unsigned int current_point, current_trajectory, nb_points;
	FILE *f = fopen_wrapper(path, "r");
	Geo_point *points;
	int line = 2;
	buffer = (char *)malloc_wrapper(sizeof(char) * LIMIT_CHARACTER_LINE);
	buffer[LIMIT_CHARACTER_LINE - 2] = '\n';
	if (trajectories_read_first_line(f, buffer, nb_elements, &nb_points))
		return FILE_FORMAT_ERROR;
	points = (Geo_point *) malloc_wrapper(nb_points * sizeof(*points));
	*trajectories =
	    (Trajectory *) malloc_wrapper(*nb_elements *
					  sizeof(**trajectories));
	current_point = 0;
	current_trajectory = 0;
	while (fgets(buffer, LIMIT_CHARACTER_LINE, f)) {
		if (buffer[LIMIT_CHARACTER_LINE - 2] != '\n') {
			fprintf(stderr,
				"Line longer than %d characters found\n",
				LIMIT_CHARACTER_LINE);
			return FILE_FORMAT_ERROR;
		}
		if (read_trajectory
		    (*trajectories + current_trajectory, buffer, points,
		     &current_point)) {
			fprintf(stderr, "Wrong point format line %d\n", line);
			return FILE_FORMAT_ERROR;
		}
		current_trajectory++;
		line++;
	}
	fclose(f);
	free(buffer);
	return NO_ERROR;
}

unsigned int add_point_trajectory(Trajectory * t)
{
	return (t->current)++;
}

void trajectories_delete_points(Trajectory * array)
{
	free(array[0].points);
	free(array);
}
