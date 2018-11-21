#if defined(GRAPHES_H)
#else
#define GRAPHES_H

#define MAX 10000

typedef struct community Community;
typedef struct member Member;
typedef struct edge Edge;
typedef struct node Node;
typedef struct graph Graph;


struct member{
   Node *node;
   Member* next;
}

struct edge {
   double weight;
   Node *node;
   Edge *next;
};

struct node {
   double prevModularity;
   Edge *neighbours;
   Community *community;
   Node *next;
};

struct community{
   unsigned int label;
   double weightInt;
   Member *member;
   Community *next;
}

struct graph{
	long unsigned int nbNode;
	long unsigned int nbEdge;
	double weightTot;
	Node *start;
	Community *community;
};

void initGraph(Graph *);

int addNode(Graph *);

int addEdge(Graph *, long unsigned int a, long unsigned int b, double weight);

int changeCommunity(Graph*);

int deleteNode(Graph *, int label);

int deleteEdge(Graph *, long unsigned int a, long unsigned int b);

void deleteGraph(Graph *);

void showGraph(Graph *);

int readFile(char *filename, Graph *);

#endif
