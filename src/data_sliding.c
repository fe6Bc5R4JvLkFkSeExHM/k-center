/**
The module contains the code about the IO operation related to the sliding window algorithm on GPS point
**/
#define _POSIX_C_SOURCE 200112L
#include "point.h"
#include "data_sliding.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define INIT_ARRAY_SIZE 100000

Error_enum
sliding_read_point(char *string, Timestamped_point * p, unsigned int window_lenght)
{
	char *tmp = strtok(string, "\t");
	if (!tmp)
		return FILE_FORMAT_ERROR;
	if (strtoui_wrapper(tmp, &(p->in_date)))
		return FILE_FORMAT_ERROR;
	p->exp_date = p->in_date + window_lenght;
	tmp = strtok(NULL, " \t\n");
	if (strtod_wrapper(tmp, &(p->point.longitude)))
		return FILE_FORMAT_ERROR;
	tmp = strtok(NULL, " \t\n");
	if (strtod_wrapper(tmp, &(p->point.latitude)))
		return FILE_FORMAT_ERROR;
	/* translate_coordinates_radian(&(p->point)); */
	return NO_ERROR;
}

static Error_enum
sliding_parse_points_file(Timestamped_point ** point_array,
			  unsigned int *nb_element, char *path,
			  unsigned int window_length)
{
	char buffer[BUFSIZ];
	unsigned line = 1;
	size_t max_array = INIT_ARRAY_SIZE, current = 0;
	FILE *f = fopen_wrapper(path, "r");
	Error_enum tmp;
	*point_array = calloc_wrapper(INIT_ARRAY_SIZE, sizeof(**point_array));
	while (fgets(buffer, BUFSIZ, f)) {
		if (current == max_array)
			*point_array = realloc_wrapper(*point_array, &max_array,
							      sizeof(**point_array));
		if ((tmp = sliding_read_point(buffer, *point_array + current,
					      window_length))) {
			fprintf(stderr,
				"Wrong point format, incident occured line %d\n",
				line);
			return tmp;
		}
		current++;
		line++;
	}
	*nb_element = (unsigned int)current;
	fclose(f);
	return NO_ERROR;
}

void sliding_print_points(void * parray, unsigned int nb_elements)
{
	unsigned int i;
	Timestamped_point *array=parray;
	for (i = 0; i < nb_elements; i++)
		printf("%d %lf %lf\n", i, array[i].point.longitude,
		       array[i].point.latitude);
}

void
sliding_import_points(void ** array, unsigned int *nb_elements,
		      char *path, unsigned int window_length)
{
	sliding_parse_points_file((Timestamped_point **)array, nb_elements, path, window_length);
}

double __sliding_distance(Timestamped_point * a, Timestamped_point * b)
{
	return euclidean_distance(&(a->point), &(b->point));
}

double sliding_distance(void * a, void * b)
{
	return __sliding_distance((Timestamped_point  *)a, (Timestamped_point *)b);
}
