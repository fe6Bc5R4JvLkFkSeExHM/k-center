/**
The module contains wrappers for some standard C librairy function and some other utility function
**/

#ifndef __HEADER_UTILS__
#define __HEADER_UTILS__

#define _GNU_SOURCE

#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
/**
 * min of two numbers
 */
#define MIN(a,b) ((a)<=(b)?a:b)
/**
 * max of two numbers
 */
#define MAX(a,b) ((a)>=(b)?a:b)
/**
 * Square of a number
 */
#define SQUARE(a) ((a)*(a))
/**
 * Absolute value of a number
 */ 
#define ABS(a) ((a)<0?-(a):(a))

/**
 * Macro used by realloc to increase size of arrays
 */
#define RESIZE_FACTOR(a) (((a)*3)/2)

/**
 * Remove unused warning on unused variable
 */
#ifdef __GNUC__
#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_ ## x
#endif

/**
 * Remove unused warning on unused function
 */
#ifdef __GNUC__
#define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif

/**
 * An enum for some error return value.
 *
 * @NO_ERROR : success
 * @INVAL_ERROR : the given arguments are invalid
 * @IO_ERROR : error on some IO operation
 * @FILE_FORMAT_ERROR : the file has an invalid format
 * @ONLY_BAD_LEVELS_ERROR : all levels of the k-center algorithm are invalid.
 */
typedef enum {
	NO_ERROR,
	INVAL_ERROR,
	IO_ERROR,
	FILE_FORMAT_ERROR,
	ONLY_BAD_LEVELS_ERROR
} Error_enum;

/**
 * @fopen_wrapper : Wrapper of fopen that tests NULL pointer.
 *
 * @path : the path of the file to open
 * @mode the mode to open the file
 *
 * @return the asked file opened with the correct mode.
 */
FILE *fopen_wrapper(char *path, char *mode);

/**
 * @malloc_wrapper : wrapper for malloc that tests NULL pointer.
 *
 * @size : the size in byte to allocate
 *
 * @return a pointer to the allocated memory
 */
void *malloc_wrapper(size_t size);

/**
 * @calloc_wrapper : wrapper for calloc that tests NULL pointer.
 *
 * @nmemb : the number of elements.
 * @size : the size of each element in byte.
 *
 * @return a pointer to @nmemb elements of size @size with bytes initialised to 0.
 */
void *calloc_wrapper(size_t nmemb, size_t size);
/**
 * @realloc_wrapper : wrapper for realloc that tests NULL pointer. It uses the RESIZE_FACTOR macro to get the increased size.
 *
 * @pointer : a pointer to memory already allocated by malloc or realloc
 * @nmemb : the current number of element in @pointer
 * @nmemb : the current size of each element in @pointer
 *
 * @return a pointer to the memory zone with increased size
 */
void *realloc_wrapper(void *pointer, size_t * nmemb, size_t size);

/**
 *  @open_wrapper : Wrapper for open that checks for errors
 *
 * @path : the path of the file to open
 * @flag : the flag required to open the file
 *
 * @return a file descriptor for the asked file
 */
int open_wrapper(char *path, int flags);

/**
 * @close_wrapper : Wrapper for close that checks for errors.
 *
 * @fd : the file descriptor to close.
 */
void close_wrapper(int fd);

/**
 * @read_wrapper: Wrapper for read that checks for errors.
 *
 * @fd : the file descriptor that must be read
 * @buffer : a buffer to store what has been read from the file
 * @nmemb : the number of element to read
 * @size : the size of each element to read
 *
 * @return the total size of what has been read
 */
ssize_t read_wrapper(int fd, void *buffer, size_t nmemb, size_t size);

/**
 * @write_wrapper : Wrapper for write that checks for errors.
 *
 * @fd : the file descriptor that msut be written into
 * @buffer : a buffer with elements to write
 * @nmemb : the number of element to write
 * @size : the number of each element to write
 *
 * @return the total size of what has been written
 */
ssize_t write_wrapper(int fd, void *buffer, size_t nmemb, size_t size);

/**
 * @log_ceil : compute the ceil of the log of n in base base
 *
 * @n : the double we want the ceil of the log
 * @base : the base we want the log in.
 *
 * @return the ceil of the log of @n in base @base
 */
double log_ceil(double n, double base);

/**
 * @shuffle_array : shuffle the order of all elements in the array
 *
 * @array : the array to shuffle
 * @size : the length of the array
 */
void shuffle_array(unsigned int *array, unsigned int size);

/**
 * @enable_log : enable the logs that will be store in the file path. Will do nothing if given an empty string or a NULL pointer.
 *
 * @path : the path of the file where the logs will be stored.
 */
void enable_log(char *path);


/**
 * @enable_long_log enable the long version of the logs that will be store in the file path.
 *
 * @path : the path of the file where the logs will be stored.
 */
void enable_long_log(char *path);

/**
 * @disable_log : disable the log file.
 */
void disable_log(void);

/**
 * @has_log : check if logs are enabled
 *
 * @return 1 if active, 0 otherwise.
 */
int has_log(void);


/**
 * @has_long_log : check if the long version of the logs is enabled
 *
 * @return 1 if active, 0 otherwise.
 */
int has_long_log(void);

/**
 * @get_log_file : return the logs file
 *
 * @return the logs file
 */
FILE *get_log_file(void);

/**
 * @activate_time_log : active the time log and store them in the file path
 *
 * @path : the path where to store the logs
 */
void enable_time_log(char *path);

/**
 * @has_time_log : Check if time logs are active
 *
 * @return 1 if active, 0 otherwise
 */
int has_time_log(void);

/**
 * @store_time : add the duration between @begin and @end to the time log
 *
 * @begin : a struct timeval at the beginning of what we want to measure
 * @end : a struct timeval at the end of what we want to measure
 *
 * @return 1 if an error happened, 0 otherwise
 */
int store_time(struct timeval *begin, struct timeval *end);

/**
 * @disable_time_log : disable the time logs
 */
int disable_time_log(void);

/**
 * @strtod_wrapper : wrapper for strtod (convert a string into a double)
 *
 * @string : the string to convert.
 * @value : the converted value in case of success.
 *
 * @return @NO_ERORR in case of sucess, @INVAL_ERROR otherwise.
 */
int strtod_wrapper(char *string, double *value);


/**
 * @strtoi_wrapper : wrapper for strtoi (convert a string into an int)
 *
 * @string : the string to convert.
 * @value : the converted value in case of success.
 *
 * @return @NO_ERORR in case of sucess, @INVAL_ERROR otherwise.
 */
int strtoi_wrapper(char *string, int *value);


/**
 * @strtoui_wrapper : wrapper for strtoui (convert a string into an unsigned int)
 *
 * @string : the string to convert.
 * @value : the converted value in case of success.
 *
 * @return @NO_ERORR in case of sucess, @INVAL_ERROR otherwise.
 */
int strtoui_wrapper(char *string, unsigned int *value);


/**
 * @strtol_wrapper : wrapper for strtol (convert a string into a long)
 *
 * @string : the string to convert.
 * @value : the converted value in case of success.
 *
 * @return @NO_ERORR in case of sucess, @INVAL_ERROR otherwise.
 */
int strtol_wrapper(char *string, long *value);

/**
 * @strtoul_wrapper : wrapper for strtoul (convert a string into an unsigned long)
 *
 * @string : the string to convert.
 * @value : the converted value in case of success.
 *
 * @return @NO_ERORR in case of sucess, @INVAL_ERROR otherwise.
 */
int strtoul_wrapper(char *string, unsigned long *value);


/**
 * @strtoull_wrapper : wrapper for strtoull (convert a string into an unsigned long long)
 *
 * @string : the string to convert.
 * @value : the converted value in case of success.
 *
 * @return @NO_ERORR in case of sucess, @INVAL_ERROR otherwise.
 */
int strtoull_wrapper(char *string, unsigned long long *value);

#endif
