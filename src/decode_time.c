/**
This utility transforms a timelog file in a readable format
**/

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	struct timeval buffer[BUFSIZ];
	int nb_element, i;
	FILE *in, *out;
	if (argc < 3)
		printf("%s in out\n", argv[0]);
	if (NULL == (in = fopen(argv[1], "rb"))) {
		fprintf(stderr, "Invalid file %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	if (NULL == (out = fopen(argv[2], "w"))) {
		fprintf(stderr, "Invalid file %s\n", argv[2]);
		return EXIT_FAILURE;
	}
	while ((nb_element = fread(buffer, sizeof(*buffer), BUFSIZ, in))) {
		for (i = 0; i < nb_element; i++) {
			fprintf(out, "%lu %lu\n", buffer[i].tv_sec,
				buffer[i].tv_usec);
			printf("%lu %lu\n", buffer[i].tv_sec,
			       buffer[i].tv_usec);
		}
		if (feof(in)) {
			fclose(in);
			fclose(out);
			break;
		}
	}
	return 0;
}
