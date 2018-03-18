/**
Main module
**/

#include "utils.h"
#include "point.h"
#include "data_sliding.h"
#include "data_fully_adv.h"
#include "data_packed.h"
#include "data_trajectories.h"
#include "query.h"
#include "algo_sliding.h"
#include "algo_fully_adv.h"
#include "algo_packed.h"
#include "algo_trajectories.h"

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <string.h>

char *prog_name = NULL;

typedef enum {
	SLIDING_K_CENTER, FULLY_ADV_K_CENTER, PACKED_K_CENTER,
	TRAJECTORIES_K_CENTER,
	LAST_ALGO_TYPE
} Algo_type;

struct program_args {
	char *points_path;	/* path to points file */
	char *queries_path;	/* path for query file */
	unsigned int k;		/* the famous k */
	double epsilon;		/* approximation required */
	char log_file[120];	/* path of log_file */
	int long_log;		/* type of log (display true radius or not */
	unsigned int window_length;	/* size of sliding window */
	double d_min;		/* lower bound given */
	double d_max;		/* upper_bound given */
	Algo_type algo;		/* algo type asked */
	int parallel;		/* multithread asked by user */
	unsigned int nb_thread;	/* nb of thread asked by user */
	unsigned int cluster_size;	/* limit of cluster size specified by user */
};

void help(void)
{
	fprintf(stderr,
		"Sliding window: %s -s [-l log_file] k eps window_size d_min d_max data_file\n",
		prog_name);
	fprintf(stderr,
		"Fully adversary: %s -m [-l log_file] k eps d_min d_max data_file query_file\n",
		prog_name);
	fprintf(stderr,
		"Packed Fully adversary: %s -o [-l log_file] k eps d_min d_max data_file query_file\n",
		prog_name);
	fprintf(stderr,
		"Fully adversary Trajectories: %s -p [-l log_file -n nb_threads] k eps d_min d_max data_file query_file\n",
		prog_name);

}

double n_log_n(double n)
{
	return n * log(n);
}

void init_prog_args(struct program_args *prog_args)
{
	prog_args->cluster_size = 0;
	prog_args->algo = LAST_ALGO_TYPE;
	prog_args->epsilon = -1;
	prog_args->long_log = 0;
	prog_args->window_length = 0;
	prog_args->parallel = 0;
	prog_args->log_file[0] = '\0';
}

int __parse_options(int argc, char *argv[], struct program_args *prog_args)
{
	Error_enum tmp;
	int opt;
	while ((opt = getopt(argc, argv, "hvl:tsmpn:bc:u:o")) != -1) {
		switch (opt) {
		case 'u':
			enable_time_log(optarg);
			break;
		case 't':
			prog_args->long_log = 1;
			break;
		case 'h':
			help();
			return 1;
		case 'l':
			strcpy(prog_args->log_file, optarg);
			break;
		case 's':
			prog_args->algo = SLIDING_K_CENTER;
			break;
		case 'm':
			prog_args->algo = FULLY_ADV_K_CENTER;
			break;
		case 'o':
			prog_args->algo = PACKED_K_CENTER;
			break;
		case 'p':
			prog_args->algo = TRAJECTORIES_K_CENTER;
			break;
		case 'n':
			prog_args->parallel = 1;
			tmp = strtoui_wrapper(optarg, &prog_args->nb_thread);
			if (tmp || 0 == prog_args->nb_thread) {
				fprintf(stderr,
					"Positive number of thread required for -n option");
				exit(EXIT_FAILURE);
			}
			break;
		case 'c':
			tmp = strtoui_wrapper(optarg, &prog_args->cluster_size);
			if (tmp || 0 == prog_args->cluster_size) {
				fprintf(stderr,
					"Positive cluster size required for -c option");
				exit(EXIT_FAILURE);
			}
			break;
		default:
			fprintf(stderr, "unknown option\n");
			help();
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

int parse_options(int argc, char *argv[], struct program_args *prog_args)
{
	char *err_ptr = NULL;
	int next_arg = 0;
	prog_name = argv[0];
	init_prog_args(prog_args);
	if (__parse_options(argc, argv, prog_args))
		return 1;
	if (argc - optind != 6) {
		help();
		exit(EXIT_FAILURE);
	}
	if (strtoui_wrapper(argv[optind], &prog_args->k) || 0 == prog_args->k) {
		fprintf(stderr, "positive k required\n");
		help();
		exit(EXIT_FAILURE);
	}
	next_arg++;
	if (strtod_wrapper(argv[optind + next_arg], &prog_args->epsilon)
	    || 0 >= prog_args->epsilon) {
		fprintf(stderr, "positive eps required\n");
		help();
		exit(EXIT_FAILURE);
	}
	next_arg++;
	if (SLIDING_K_CENTER == prog_args->algo) {
		if (strtoui_wrapper
		    (argv[optind + next_arg], &(prog_args->window_length))
		    || 0 == prog_args->window_length) {
			fprintf(stderr, "positive window required\n");
			help();
			exit(EXIT_FAILURE);
		}
		next_arg++;
	}
	prog_args->d_min = strtod(argv[optind + next_arg], &err_ptr);
	if (prog_args->d_min <= 0) {
		fprintf(stderr, "positive d_min required\n");
		help();
		exit(EXIT_FAILURE);
	}
	next_arg++;
	prog_args->d_max = strtod(argv[optind + next_arg], &err_ptr);
	if (prog_args->d_max < prog_args->d_min) {
		fprintf(stderr, "d_max should be greater than d_min !\n");
		help();
		exit(EXIT_FAILURE);
	}
	next_arg++;
	prog_args->points_path = argv[optind + next_arg];
	next_arg++;
	if (prog_args->algo == FULLY_ADV_K_CENTER
	    || prog_args->algo == TRAJECTORIES_K_CENTER
	    || prog_args->algo == PACKED_K_CENTER) {
		prog_args->queries_path = argv[optind + next_arg];
	}
	printf("k: %d eps: %lf d_min: %lf d_max: %lf\n", prog_args->k,
	       prog_args->epsilon, prog_args->d_min, prog_args->d_max);
	return 0;
}

void sliding_k_center(struct program_args *prog_args)
{
	Sliding_level *levels;
	void *array;
	unsigned int size, nb_instances;
	sliding_import_points(&array, &size, prog_args->points_path,
			      prog_args->window_length);
	printf("import ended!\n");
	sliding_initialise_levels_array(&levels, prog_args->k,
					prog_args->epsilon, prog_args->d_min,
					prog_args->d_max, &nb_instances, array,
					size);
	sliding_k_center_run(levels, nb_instances);
	free(array);
	sliding_delete_levels_array(levels, nb_instances);
}

void fully_adv_k_center(struct program_args *prog_args)
{
	Fully_adv_cluster *clusters_array;
	void *array;
	struct query_provider queries;
	unsigned int size, nb_instances;
	unsigned int *helper_array;
	fully_adv_import_points(&array, &size, prog_args->points_path);
	printf("import ended!\n");
	initialise_query_provider(&queries, prog_args->queries_path);
	if (0 == prog_args->cluster_size)
		prog_args->cluster_size = size;
	fully_adv_initialise_level_array(&clusters_array, prog_args->k,
					 prog_args->epsilon, prog_args->d_min,
					 prog_args->d_max, &nb_instances, array,
					 size, prog_args->cluster_size,
					 &helper_array);
	fully_adv_k_center_run(clusters_array, nb_instances, &queries,
			       helper_array);
	free(array);
	fully_adv_delete_level_array(clusters_array, nb_instances,
				     helper_array);
	free_query_provider(&queries);
}

void packed_k_center(struct program_args *prog_args)
{
	Packed_level *levels;
	void *array;
	struct query_provider queries;
	unsigned int size, nb_instances;
	packed_import_points(&array, &size, prog_args->points_path);
	printf("import ended!\n");
	initialise_query_provider(&queries, prog_args->queries_path);
	packed_initialise_levels_array(&levels, prog_args->k,
				       prog_args->epsilon, prog_args->d_min,
				       prog_args->d_max, &nb_instances, array,
				       size);
	packed_k_center_run(levels, nb_instances, &queries);
	free(array);
	packed_free_levels_array(levels, nb_instances);
	free_query_provider(&queries);
}

void trajectories_k_center(struct program_args *prog_args)
{
	Trajectory_level *clusters_array;
	Trajectory *array;
	struct query_provider queries;
	unsigned int size, *helper_array, nb_instances;
	trajectories_import_points(&array, &size, prog_args->points_path);
	printf("import ended!\n");
	initialise_query_provider(&queries, prog_args->queries_path);
	trajectories_initialise_level_array(&clusters_array, prog_args->k,
					    prog_args->epsilon,
					    prog_args->d_min, prog_args->d_max,
					    &nb_instances, array, size,
					    &helper_array);
	trajectories_k_center_run(clusters_array, nb_instances, &queries,
				  helper_array);
	trajectories_delete_level_array(clusters_array, nb_instances,
					helper_array);
	free_query_provider(&queries);
	trajectories_delete_points(array);
}

void trajectories_parallel_k_center(struct program_args *prog_args)
{
	Trajectory *array;
	struct query_provider queries;
	unsigned int size;
	trajectories_import_points(&array, &size, prog_args->points_path);
	printf("import ended!\n");
	initialise_query_provider(&queries, prog_args->queries_path);
	trajectories_parallel_initialise_level_array(prog_args->k,
						     prog_args->epsilon,
						     prog_args->d_min,
						     prog_args->d_max,
						     array, size,
						     prog_args->nb_thread);
	trajectories_parallel_k_center_run(&queries);
	trajectories_parallel_delete_level_array();
	free_query_provider(&queries);
	trajectories_delete_points(array);
}

int main(int argc, char *argv[])
{
	struct program_args prog_args;
	srand48(time(NULL));
	srand((unsigned int)time(NULL));
	if (parse_options(argc, argv, &prog_args))
		return 0;
	if (prog_args.long_log)
		enable_long_log(prog_args.log_file);
	else
		enable_log(prog_args.log_file);
	switch (prog_args.algo) {
	case SLIDING_K_CENTER:
		printf("Sliding window algorithm chosen\n");
		sliding_k_center(&prog_args);
		break;
	case FULLY_ADV_K_CENTER:
		printf("Fully adversary algorithm chosen\n");
		fully_adv_k_center(&prog_args);
		break;
	case PACKED_K_CENTER:
		printf("Packed fully adversary algorithm chosen\n");
		packed_k_center(&prog_args);
		break;
	case TRAJECTORIES_K_CENTER:
		printf("Trajectories fully adversary algorithm chosen\n");
		if (prog_args.parallel)
			trajectories_parallel_k_center(&prog_args);
		else
			trajectories_k_center(&prog_args);
		break;
	default:
		fprintf(stderr, "Unknow algorithm\n");
		return EXIT_FAILURE;
	}
	disable_log();
	if (has_time_log())
		disable_time_log();
	return EXIT_SUCCESS;
}
