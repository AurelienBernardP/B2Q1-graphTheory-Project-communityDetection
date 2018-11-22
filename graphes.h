#if defined(GRAPHES_H)
#else
#define GRAPHES_H

#define MAX 10000

typedef struct community Community;
typedef struct edge Edge;
typedef struct node Node;
typedef struct graph Graph;


struct edge {
   double weight;
   Node *dest;
   Edge *next;
};

struct node {
   long unsigned int myCom;
   Edge *neighbours;
   Node *next;
};

struct community{
   long unsigned int label;
   double weightInt;
   Node *member;
   Community *next;
};

struct graph{
	long unsigned int nbNode;
	long unsigned int nbEdge;
	double weightTot;
	Community *community;
};

void deleteGraph(Graph *);

void showGraph(Graph *);

int readFile(char *filename, Graph *);

#endif
