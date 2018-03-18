EXEC=k-center
CFLAGS=-ansi -Wall -Wextra -pthread -Wconversion
OFLAGS=-g -O2 -DNDEBUG
LDFLAGS=-lm -L/usr/lib/x86_64-linux-gnu -pthread
LIBFLAGS=-D_REENTRANT 
CC=gcc
BIN=bin/
SRC=src
HDR=headers
VPATH=$(SRC):$(HDR)


.PHONY: clean mrproper

default: $(EXEC)


$(BIN)main.o: main.c query.h point.h data_sliding.h utils.h algo_sliding.h algo_packed.h algo_fully_adv.h algo_trajectories.h data_packed.h data_fully_adv.h set.h lookup.h

$(BIN)algo_sliding.o: algo_sliding.c point.h utils.h algo_sliding.h data_sliding.h

$(BIN)algo_packed.o: algo_packed.c point.h utils.h algo_packed.h data_packed.h query.h lookup.h

$(BIN)algo_fully_adv.o: algo_fully_adv.c query.h point.h utils.h algo_fully_adv.h data_fully_adv.h set.h

$(BIN)algo_trajectories.o: algo_fully_adv.c query.h point.h utils.h algo_trajectories.h set.h data_trajectories.h

$(BIN)point.o: point.c point.h

$(BIN)data_sliding.o:  data_sliding.c data_sliding.h point.h utils.h

$(BIN)data_packed.o:  data_packed.c data_packed.h point.h utils.h

$(BIN)data_fully_adv.o:  data_fully_adv.c data_fully_adv.h point.h utils.h

$(BIN)data_trajectories.o:  data_trajectories.c data_trajectories.h point.h utils.h

$(BIN)query.o: query.c query.h utils.h point.h set.h lookup.h

$(BIN)set.o: set.c set.h utils.h

$(BIN)lookup.o: lookup.c lookup.h utils.h

$(EXEC): $(BIN)main.o $(BIN)algo_sliding.o $(BIN)algo_packed.o $(BIN)algo_fully_adv.o $(BIN)algo_trajectories.o $(BIN)query.o $(BIN)utils.o $(BIN)point.o $(BIN)data_sliding.o $(BIN)data_fully_adv.o $(BIN)data_trajectories.o $(BIN)data_packed.o $(BIN)set.o $(BIN)lookup.o
	$(CC) -o $@ $^  $(CFLAGS) $(LDFLAGS) $(OFLAGS)

$(BIN)utils.o: utils.c utils.h

$(BIN)sliding_query.o: sliding_query.c utils.h

sliding_query: $(BIN)utils.o $(BIN)sliding_query.o
	$(CC) -o $@ $^  $(CFLAGS) $(LDFLAGS) $(OFLAGS)

clean: 
	rm -f $(BIN)*.o
	rm -f src/*~

mrproper: clean
	rm -f $(EXEC)



$(BIN)%.o : %.c 
	$(CC) -c $< $(CFLAGS) $(OFLAGS) $(LDFLAGS) -o $@
