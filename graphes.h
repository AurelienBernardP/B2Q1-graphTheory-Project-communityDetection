#if defined(GRAPHES_H)
#else
#define GRAPHES_H

//Max == 10000 caracteres par ligne
#define MAX 10000

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
   Community *myCom;
   size_t label;
   double weightMember;
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

/*
Alloue et initialise le graphe

*/
Graph *initGraph();

//Supprime le graphe en entier
void deleteGraph(Graph *);

/*
Ajout au debut de la liste $community$ une nouvelle Communaute qui possede
un unique membre.

input: g->un graphe initialise
return : 0 si succes
		 -1 si memoire insuffisante pour allocation
		 -2 si precondition(s) non-respectee(s)
*/
int addCommunity(Graph *g);

/*
Ajout d'un arc de $start$ vers $end$ et de poids $weight$.

input: g : un graphe deja initialise
	  start: la communaute d'origine de l'arc (0 < start < g->nbCommunity+1)
	  end: la communaute de destination de l'arc (0 < end < g->nbCommunity+1)
	  weight : le poids de cet arc ( weight != 0)

return: 0 si succes
		-1 si la communaute d'origine n'existe pas
		-2 si la communaute de destination n'existe pas
		-3 si erreur allocation
		-4 si precondition(s) non respectee(s)
*/
int addEdge(Graph *g, size_t a, size_t b, double weight);


#endif
