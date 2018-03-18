This repository contains the source code of a program related to an article submited to www 2018.
If you have any question on the algorithm implemented, please refer to said article.

All algorithms are related to the k-center problem.

-- Compilation --

To compile it, use 'make' in the root directory.
It was only tested on ubuntu/debian/fedora. Use it on other systems at your own risk.

-- Command Line arguments -- 

The different algorithm implemented are:
    - the Sliding Window Algorithm, the corresponding option is -s.
    - the Fully Adversary Algorithm, the corresponding option is -m.
    - Fully Adversary Algorithm for trajectories, the corresponding option is -p.

One must specify one of those mendatory option to run the program.

The program takes one of the mendatory option and 5 others mendatory argument:
    - for the sliding window, it requires in this order:
      	  ./k-center -s k epsilon window_length d_min d_max data_file
    - for both version of the fully adversary algorithm, it requires 5:
      	  ./k-center -m/-p k eps d_min d_max data_file query_file

On the fully adversary algorithm for trajectories, multithreading is available, it is possible to activate it using the option -n nb_thread

The option -l file_name create a log file with one line per operation having the following format:
query_type point_of_query nb_point level_of_solution radius_of_level nb_clusters_of_level

-- Data file format -- 

For option -m and -s:
The data_file should have one entry per line and every line should have the following format :
timestamp tab longitude space latitude

The timestamp should be integer that fits in an unsigned int.
The index of point is fixed by their order in the file, ranging from 0 to n-1 where n is the numbe of lines of the data file.
The points should be ordered by timestamp when using option -s.

For option -p:
The first line of the file should correspond to the total number of trajectories follower by the total number of points in it. Those two file should be separated by either a tabulation or a space.

Every following line should correspond to one trajectory. 
The format of one entry is as follows:
timestamp tab n tab longitude_1,latitude_1 tab longitude_2,latitude_2 tab ... tab longitude_n,latitude_n
for a trajectory with n points.

As of now, the euclidean distance is used to compute the distance between 2 points in two different trajectories. It is a projection of Earth on the plane, and as such should not be used on point far appart (and it has problems at poles). 

-- Queryfile format -- 

The queryfile should be a binary file containing a sequence of unsigned int. The endianness and size of elements in the query file should correspond to the endianness and size of the computer it is run on. 

With option -m, the parity of an occurence determines if the corresponding point should be added or removed. If odd, the point is inserted, if even, it is removed.

With option -p, the ith occurence of an index will add the ith point to the corresponding trajectory, up to the lenght of the trajectory. The behavior is undefined if more points are added than the length of the trajectory. All trajectories start empty.

-- Other -- 
When using the -m option in a sliding window setting, the utility sliding_query (make sliding_query on the root directory) can be used to generate a queryfile from a datafile.
This utility can be used by typing:

./sliding_query datafile queryfile readable_format_output_for_query length_of_sliding_window

The datafile format should be the same as the one specified for the -m option.

Beware, points in the datafile should be ordered by timestamp (increasing order). Otherwise, the behavior of the utility is unspecified.
