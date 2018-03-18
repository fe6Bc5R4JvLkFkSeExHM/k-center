/**
This module contains some fonction related to GPS points
**/
#ifndef __HEADER_DATA
#define __HEADER_DATA

typedef struct {
	double latitude;
	double longitude;
} Geo_point;

typedef struct {
	Geo_point point;
	unsigned int in_date;
	unsigned int exp_date;
} Timestamped_point;

double euclidean_distance(Geo_point * a, Geo_point * b);

double great_circle_distance(Geo_point * a, Geo_point * b);

void translate_coordinates_radian(Geo_point * point);
#endif
