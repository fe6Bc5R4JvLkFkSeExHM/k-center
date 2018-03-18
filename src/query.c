/**
Module for query handling
 */
#include "utils.h"
#include "query.h"
#include "set.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

void initialise_query_provider(struct query_provider * queries, char *path)
{
	queries->fd = open_wrapper(path, O_RDONLY | O_CREAT);
	queries->current = queries->nb_query = 0;
}


void free_query_provider(struct query_provider * queries)
{
	close(queries->fd);
}

int
get_next_query_set(struct query_provider * queries, struct query * next_query,
		   struct set_collection * sets)
{
	if (queries->current >= queries->nb_query) {
		queries->current = 0;
		queries->nb_query =
		    read_wrapper(queries->fd, queries->buffer, BUFSIZ,
				 sizeof(next_query->data_index));
		if (!queries->nb_query)
			return 0;
	}
	next_query->data_index = queries->buffer[queries->current];
	next_query->type =
	    (has_element_set_collection(sets, next_query->data_index) ? REMOVE :
	     ADD);
	queries->current++;
	return 1;
}

int get_next_query_lookup(struct query_provider * queries, struct query * next_query,
			  struct lookup_table * lookup)
{
	if (queries->current >= queries->nb_query) {
		queries->current = 0;
		queries->nb_query =
		    read_wrapper(queries->fd, queries->buffer, BUFSIZ,
				 sizeof(next_query->data_index));
		if (!queries->nb_query)
			return 0;
	}
	next_query->data_index = queries->buffer[queries->current];
	next_query->type =
	    (has_element_lookup(lookup, next_query->data_index) ? REMOVE : ADD);
	queries->current++;
	return 1;
}

int get_next_query_trajectories(struct query_provider * queries, struct query * next_query)
{
	if (queries->current >= queries->nb_query) {
		queries->current = 0;
		queries->nb_query =
		    read_wrapper(queries->fd, queries->buffer, BUFSIZ,
				 sizeof(next_query->data_index));
		if (!queries->nb_query)
			return 0;
	}
	next_query->data_index = queries->buffer[queries->current];
	next_query->type = ADD;
	queries->current++;
	return 1;
}

