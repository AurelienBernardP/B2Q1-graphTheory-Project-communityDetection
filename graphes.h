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
   //De type Community car quand le gain est supérieur on Peut
   //Transferer le membre plus rapidement à la communauté de son voisin
   //(évite de parcourir toutes les communauté jusqu'à trouver la bonne)
   Community *myCom;

   //Evite de parcourir tous ses arcs pour trouver son poids.
   //Il suffit de le calculer au moment où on rajoute un arc à ce membre
   double weightNode;

   Edge *neighbours;

   //Lorsqu'il faut changer le membre de communauté il suffit d'acceder
   //au membre précédent de sa communauté courrante et faire pointer
   //ce membre précédent au suivant
   Node *previous;
   Node *next;
};

struct community{
   long unsigned int label;

   //Utile de garder le weightInt car il permet de supprimer la Communauté
   //s'il n'y a plus de membre. Une alternative à ça ce serait de compter
   //le nombre de membre de la commaunté et permettrait aussi de donner
   //peut etre des infos plus utiles lors de l'inscription de la nouvelle
   //matrice.   Pas pue comparer member à null quand on lui enleve son seul
   //membre. Pourtant je free bien son unique membre mais il ne voit pas
   //com->member == NULL à méditer...
   double weightInt;
   Node *member;
   Community *next;
};

struct graph{
	long unsigned int nbCommunity;
    //NbEdge pas necessaire
	long unsigned int nbEdge;
	double weightTot;
	Community *community;
};

void deleteGraph(Graph *);

void showGraph(Graph *);

int readFile(char *filename, Graph *);

#endif
