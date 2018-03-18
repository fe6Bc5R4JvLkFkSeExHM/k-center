/**
This file contains a program to create a queryfile from an datafile in a sliding window setting.
 **/

#define _ISOC99_SOURCE

#include "utils.h"

#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	unsigned int out_buffer[BUFSIZ];
	char buffer[BUFSIZ];
	unsigned int duration, current, out_current, oldest;
	int out;
	FILE *in, *out_readable;
	unsigned int *array =
	    (unsigned int *)calloc_wrapper(100000000, sizeof(unsigned int)),
	    w_max = 0;
	if (argc != 5) {
		fprintf(stderr, "%s in out out_read duration\n", argv[0]);
		return EXIT_FAILURE;
	}
	in = fopen_wrapper(argv[1], "r");
	out = open_wrapper(argv[2], O_WRONLY | O_CREAT | O_TRUNC);
	out_readable = fopen_wrapper(argv[3], "w");
	if (strtoui_wrapper(argv[4], &duration)) {
		fprintf(stderr, "Position duration required\n");
		return EXIT_FAILURE;
	}
	out_current = oldest = current = 0;
	while (fgets(buffer, BUFSIZ, in)) {
		if (strtoui_wrapper(strtok(buffer, "\t \n"), array + current)) {
			fprintf(stderr, "Invalid input file\n");
			return EXIT_FAILURE;
		}
		w_max = max(current - oldest, w_max);
		while (oldest < current
		       && array[oldest] + duration < array[current]) {
			if (out_current == BUFSIZ) {
				write_wrapper(out, out_buffer, BUFSIZ,
					      sizeof(unsigned int));
				out_current = 0;
			}
			out_buffer[out_current] = oldest;
			fprintf(out_readable, "%d\n", oldest);
			out_current++;
			oldest++;
		}
		if (out_current == BUFSIZ) {
			write_wrapper(out, out_buffer, BUFSIZ,
				      sizeof(unsigned int));
			out_current = 0;
		}
		out_buffer[out_current] = current;
		fprintf(out_readable, "%d\n", current);
		out_current++;
		current++;
	}
	if (out_current != 0)
		write_wrapper(out, out_buffer, out_current,
			      sizeof(unsigned int));
	printf("%d\n", w_max);
	close(out);
	fclose(out_readable);
	fclose(in);
	return 0;
}
