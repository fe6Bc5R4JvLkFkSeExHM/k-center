/**
The module contains wrappers for some standard C librairy function and some other utility function
**/
#include "utils.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <assert.h>

FILE *fopen_wrapper(char *path, char *mode)
{
	FILE *f;
	if (NULL == (f = fopen(path, mode))) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	return f;
}

void *malloc_wrapper(size_t size)
{
	void *tmp;
	if (NULL == (tmp = malloc(size))) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	return tmp;
}

void *calloc_wrapper(size_t nmemb, size_t size)
{
	void *tmp;
	if (NULL == (tmp = calloc(nmemb, size))) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}
	return tmp;
}

void *realloc_wrapper(void *pointer, size_t * nmemb, size_t size)
{
	void *tmp;
	size_t new_size = RESIZE_FACTOR(*nmemb);
	if (NULL == (tmp = realloc(pointer, new_size * size))) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	*nmemb = new_size;
	return tmp;
}

int open_wrapper(char *path, int flags)
{
	int fd;
	if (-1 ==
	    (fd =
	     open(path, flags,
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	return fd;
}

void close_wrapper(int fd)
{
	if (-1 == close(fd)) {
		perror("close");
		exit(EXIT_FAILURE);
	}
}

ssize_t read_wrapper(int fd, void *buffer, size_t nmemb, size_t size)
{
	ssize_t tmp = read(fd, buffer, nmemb * size);
	if (-1 == tmp) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	return tmp / ((ssize_t) size);
}

ssize_t write_wrapper(int fd, void *buffer, size_t nmemb, size_t size)
{
	ssize_t tmp = write(fd, buffer, nmemb * size);
	if (-1 == tmp) {
		perror("write");
		exit(EXIT_FAILURE);
	}
	return tmp / ((ssize_t) size);
}

double log_ceil(double n, double base)
{
	return ceil(log(n) / log(base));
}

void shuffle_array(unsigned int *array, unsigned int size)
{
	unsigned int i, pick, tmp;
	if (size) {
		for (i = 0; i < size - 1; i++) {
			pick = i + ((unsigned int)rand()) % (size - i);
			tmp = array[i];
			array[i] = array[pick];
			array[pick] = tmp;
		}
	}
}

static FILE *log_file = NULL;
int long_log = 0;

void enable_log(char *path)
{
	if (path && path[0]) {
		log_file = fopen_wrapper(path, "w");
		long_log = 0;
	}
}

void enable_long_log(char *path)
{
	log_file = fopen_wrapper(path, "w");
	long_log = 1;
}

void disable_log()
{
	if (log_file)
		fclose(log_file);
}

FILE *get_log_file()
{
	return log_file;
}

int has_log()
{
	return log_file != NULL;
}

int has_long_log()
{
	return 0 != long_log;
}

static struct timeval buffer_time[BUFSIZ];
static size_t current_time_index = 0;
static FILE *time_file = NULL;

void enable_time_log(char *path)
{
	time_file = fopen_wrapper(path, "wb");
}

int has_time_log()
{
	return time_file != NULL;
}

int store_time(struct timeval *begin, struct timeval *end)
{
	if (BUFSIZ == current_time_index) {
		fwrite(buffer_time, sizeof(*buffer_time), BUFSIZ, time_file);
		if (ferror(time_file)) {
			fprintf(stderr,
				"An error occured with the time log file.\n");
			return 1;
		}
		current_time_index = 0;
	}
	timersub(end, begin, buffer_time + current_time_index);
	current_time_index++;
	return 0;
}

int disable_time_log()
{
	if (NULL == time_file)
		return 0;
	if (0 < current_time_index) {
		fwrite(buffer_time, sizeof(*buffer_time), current_time_index,
		       time_file);
		if (ferror(time_file)) {
			fprintf(stderr,
				"An error occured with the time log file.\n");
			return 1;
		}
	}
	fclose(time_file);
	time_file = NULL;
	return 0;
}

int strtod_wrapper(char *string, double *value)
{
	char *error;
	if (!string)
		return INVAL_ERROR;
	errno = 0;
	*value = strtod(string, &error);
	if (error[0] != '\0' || errno != 0)
		return INVAL_ERROR;
	return NO_ERROR;
}

int strtol_wrapper(char *string, long *value)
{
	char *error;
	if (!string)
		return INVAL_ERROR;
	errno = 0;
	*value = strtol(string, &error, 10);
	if (error[0] != '\0' || errno != 0)
		return INVAL_ERROR;
	return NO_ERROR;
}

#if INT_MAX != LONG_MAX || INT_MIN != LONG_MIN

static long
strto_subrange(const char *s, char **endptr, int base, long min, long max)
{
	long y = strtol(s, endptr, base);
	if (y > max) {
		errno = ERANGE;
		return max;
	}
	if (y < min) {
		errno = ERANGE;
		return min;
	}
	return y;
}
#endif

static int strtoi(const char *s, char **endptr, int base)
{
#if INT_MAX == LONG_MAX && INT_MIN == LONG_MIN
	return (int)strtol(s, endptr, base);
#else
	return (int)strto_subrange(s, endptr, base, INT_MIN, INT_MAX);
#endif
}

int strtoi_wrapper(char *string, int *value)
{
	char *error;
	if (!string)
		return INVAL_ERROR;
	errno = 0;
	*value = strtoi(string, &error, 10);
	if (error[0] != '\0' || errno != 0)
		return INVAL_ERROR;
	return NO_ERROR;
}

#if INT_MAX != LONG_MAX || INT_MIN != LONG_MIN


static long strto_usubrange(const char *s, char **endptr, int base, long max)
{
	unsigned long y = strtoul(s, endptr, base);
	if (y > max) {
		errno = ERANGE;
		return max;
	}
	return y;
}
#endif
static unsigned int strtoui(const char *s, char **endptr, int base)
{
#if UINT_MAX == ULONG_MAX
	return (unsigned int)strtoul(s, endptr, base);
#else
	return (unsigned int)strto_usubrange(s, endptr, base, UINT_MAX);
#endif
}

int strtoui_wrapper(char *string, unsigned int *value)
{
	char *error;
	if (!string)
		return INVAL_ERROR;
	errno = 0;
	*value = strtoui(string, &error, 10);
	if (error[0] != '\0' || errno != 0)
		return INVAL_ERROR;
	return NO_ERROR;
}

int strtoul_wrapper(char *string, unsigned long *value)
{
	char *error;
	if (!string)
		return INVAL_ERROR;
	errno = 0;
	*value = strtoul(string, &error, 10);
	if (error[0] != '\0' || errno != 0)
		return INVAL_ERROR;
	return NO_ERROR;
}

int strtoull_wrapper(char *string, unsigned long long *value)
{
	char *error;
	if (!string)
		return INVAL_ERROR;
	errno = 0;
	*value = strtoull(string, &error, 10);
	if (error[0] != '\0' || errno != 0)
		return INVAL_ERROR;
	return NO_ERROR;
}
