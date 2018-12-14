#if defined(OPTIMISATION_H)
#else
#define OPTIMISATION_H

//The limit number of charaters per line inside the file
//which describes the graph.
#define MAX 10000

//The number of characters to describe the path to the
//files which contains the trace of all previous pass.
#define MAX_NAME 50

typedef struct community Community;
typedef struct edge Edge;
typedef struct member Member;
typedef struct graph Graph;


struct edge {
   double weight;
   Member *dest;
   Edge *next;
};


struct member {
   size_t label;
   double weightMember;
   Community *myCom;
   Edge *neighbours;
   Member *previous;
   Member *next;
};

struct community{
   size_t label;
   double weightTot;
   double weightInt;
   Member *member;
   Community *previous;
   Community *next;
};

struct graph{
	size_t nbCommunity;
	size_t nbEdge;
   size_t nbMembers;
	double weightTot;
	Community *community;
};

/* initGraph
*Allocate and initialise a graph 
*
*[return]
* g: an initialised graph
* NULL: error to allocate memory for a graph
*/
Graph *initGraph();


/* deleteGraph
* Delete all allocated memory for the graph
*
*[input]
* g: a graph
*
*/
void deleteGraph(Graph *g);

/* addCommunity
* Add a community with an unique member at the beginning of the communities of graph $g$
*
* [input]
* g : an initialised graph
*
* [return]
* 0 : a community has been added to g successfully
* -1 : Error at allocating memory for this operation
* -2 : the graph g does not exist
*
*/
int addCommunity(Graph *g);

/* addEdge
* Add an edge from $start$ to $end$ with a weight of $weight$
*
*[input]
* g : an initialised graph which contains the nodes $start$ and $end$
* start : a node of the graph g
* end : a node of the graph g
* weight : the weight of the link between $start$ and $end$
*
*[return]
* 0 : the edge has been added to g successfully
* -1 : the origin of the edge doesn't exist (start is not a node of g)
* -2 : the destination of the edge doesn't exist (end is not a node of g)
* -3 : Error at allocating memory for this operation
* -4 : the graph is empty
*
*/
int addEdge(Graph *g, size_t a, size_t b, double weight);


#endif
