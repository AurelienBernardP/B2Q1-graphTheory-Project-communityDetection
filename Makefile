CC=gcc
LD=gcc
CFLAGS=--std=c99 --pedantic -Wall -W -Wmissing-prototypes -Wextra
LDFLAGS=--std=c99 --pedantic -Wall -W -Wmissing-prototypes -Wextra
# files 

EXEC=graph
MODULES_GRAPH=optimisation.c aggregation.c
OBJECTS_GRAPH=optimisation.o aggregation.o

#rules

all: $(EXEC)

graph: $(OBJECTS_GRAPH)
	$(LD) -o $@ $^ $(LDFLAGS)

optimisation.o: optimisation.c optimisation.h
	$(CC) -c optimisation.c -o optimisation.o $(CFLAGS)

aggregation.o: aggregation.c aggregation.h
	$(CC) -c aggregation.c -o aggregation.o $(CFLAGS)


clean:
	rm -f *.o
