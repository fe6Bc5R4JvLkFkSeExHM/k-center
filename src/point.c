/**
This module contains some fonction related to GPS points
**/
#include "point.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

double euclidean_distance(Geo_point * a, Geo_point * b)
{
	return sqrt(SQUARE(a->latitude - b->latitude) +
		    SQUARE(MIN
			   (ABS(a->longitude - b->longitude),
			    360 - ABS(a->longitude - b->longitude))));
}

double great_circle_distance(Geo_point * a, Geo_point * b)
{
	return acos(sin(a->latitude) * sin(b->latitude) +
		    cos(a->latitude) * cos(b->latitude) * cos(a->longitude -
							      b->longitude));
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void translate_coordinates_radian(Geo_point * point)
{
	point->latitude = point->latitude * M_PI / 180;
	point->longitude = point->longitude * M_PI / 180;
}
