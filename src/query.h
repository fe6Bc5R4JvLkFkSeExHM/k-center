/**
Module for query handling
 */

#ifndef __HEADER_QUERY_STRUCTURE__
#define __HEADER_QUERY_STRUCTURE__

#include <stdio.h>

#include "set.h"
#include "lookup.h"

/**
 * @Query_type : types of query
 */
typedef enum {
	ADD, REMOVE, UPDATE, LAST_QUERY_TYPE
}Query_type;

/**
 * @struct query : A query
 *
 * @type : type of query
 * @data_index : index of the element related to the query
 */
struct query{
	Query_type type;
	unsigned int data_index;
};

/**
 * @struct query_provider : A provider of queries
 *
 * @path : path of the query file
 * @fd : file descriptor to the file
 * @buffer : file buffer
 * @current : next query to read
 * @nb_query : total number of query in buffer
 */
struct query_provider{
	char *path;
	int fd;
	unsigned int buffer[BUFSIZ];
	ssize_t current;
	ssize_t nb_query;
};

/**
 * @initiliase_query_provider : initialise @queries using the file pointed by @path
 *
 * @queries : the query provider to initialise
 * @path : the path of the file containing the queries
 */
void initialise_query_provider(struct query_provider * queries, char *path);

/**
 * @free_query_provider : free @queries
 *
 * @queries : the query provider to free
 */
void free_query_provider(struct query_provider * queries);

/**
 * @get_next_query_set : gives the next query from @queries in @next_query and uses @sets for context. return 1 if a query was read, 0 otherwise.
 *
 * @queries : the query provider to get the query from
 * @next_query : where the next query is stored
 * @sets : a set collection used for context
 *
 * @return 1 if sucessfull, 0 otherwise.
 */
int get_next_query_set(struct query_provider * queries, struct query * next_query,
		       struct set_collection * sets);
/**
 * @get_next_query_lookup : gives the next query from @queries in @next_query and uses @lookup for context. return 1 if a query was read, 0 otherwise.
 *
 * @queries : the query provider to get the query from
 * @next_query : where the next query is stored
 * @lookup : a lookup table used for context
 *
 * @return 1 if sucessfull, 0 otherwise.
 */
int get_next_query_lookup(struct query_provider * queries, struct query * next_query,
			  struct lookup_table * lookup);
/**
 * @get_next_query_trajectories : gives the next query from @queries in @next_query for a trajectories context. return 1 if a query was read, 0 otherwise.
 *
 * @queries : the query provider to get the query from
 * @next_query : where the next query is stored
 *
 * @return 1 if sucessfull, 0 otherwise.
 */
int get_next_query_trajectories(struct query_provider * queries, struct query * next_query);

#endif
