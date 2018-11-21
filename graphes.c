#include <stdio.h>
#include "Graphs.h"

void initGraph(Graph *g)
{
	g->nbNode=0;
	g->nbEdge=0;
	g->weightTot=0.0;
	g->start=NULL;
   g->community=NULL;
}

int addNode(Graph *g){
	Node *node;
	//g->maxS++;
	node=(Node *) malloc(sizeof(Node));
	if (node == NULL){
		printf("Erreur! Memoire insuffisante pour creer un sommet\n");
		return -1;
	}
	node->prevModularity = 0;
	//node->community = NULL;
	node->neighbours=NULL;
	node->next=NULL;
	if (g->nbNode == 0)
		g->start=node;
	else{
		node->next = g->start;
		g->start = node;
	}
	g->nbNode++;
	return 0;
}

int addEdge(Graph *g, long unsigned int a, long unsigned int b, double weight){
	Node *pnode, *pnode2;
	Edge *pEdge, *prev;
	pnode=g->start;

	/* on parcourt les sommets jusqu'a trouver a */
	while (pnode != NULL && g->nbNode-a>0){
		psommet=psommet->suivant;
		a++;

	}
	if (pnode == NULL){
		printf("Erreur! Creation d'un arc dont l'origine n'existe pas\n");
		return -1;
	}
	pEdge=pnode->afj;

			/* on parcourt les sommets pour trouver b */
			pnode2=g->premierSommet;
			while (pnode2 != NULL)
				{
					if (pnode2->label == b) break;
					pnode2=pnode2->suivant;
				}
			if (pnode2 == NULL)
				{
					printf("Erreur! Creation d'un arc dont l'extremite n'existe pas\n");
					return -2;
				}
			else /* on a trouver a et b */
				{
					if (pEdge == NULL) /* la liste d'adjacence est vide */
						{
							pEdge=(Edge *) malloc(sizeof(Edge));
							if (pEdge == NULL)
								{
									printf("Erreur! Memoire insuffisante pour creer un sommet\n");
									return -3;
								}
							else
								{
									pnode->adj=pEdge; /* premier element de la liste d'adjacence */
									pEdge->suivant=NULL;
								}
						}
					else /* la liste d'adjacence est non vide, on la parcourt pour voir si b s'y trouve */
						{
							if (pEdge->dest > b)
								{
									pEdge=(Edge *) malloc(sizeof(Edge));
									if (pEdge == NULL)
										{
											printf("Erreur! Memoire insuffisante pour creer un sommet\n");
											return -3;
										}
									else
										{
											pEdge->suivant=pnode->adj;
											pnode->adj=pEdge;
										}
								}
							else
								{
									while (pEdge != NULL)
										{
											if (pEdge->dest == b) {pEdge->info=info; break;} /* l'arc existe, update info */
											if (pEdge->dest > b) {pEdge=NULL; break;} /* on depasse b sans le trouver */
											precedent=pEdge;
											pEdge=pEdge->suivant;
										}
									if (pEdge == NULL) /* l'arc n'existe pas, il faut le creer */
										{
											pEdge=(Edge *) malloc(sizeof(Edge));
											if (pEdge == NULL)
												{
													printf("Erreur! Memoire insuffisante pour creer un sommet\n");
													return -3;
												}
											else
												if (precedent->suivant==NULL) /* element ajouter a la fin */
													{
														precedent->suivant=pEdge;
														pEdge->suivant=NULL;
													}
												else /* element ajouter "au milieu" pour garder ordre */
													{
														pEdge->suivant=precedent->suivant;
														precedent->suivant=pEdge;
													}
										}
								}
						}
					pEdge->dest=b;
					pEdge->info=info;
					g->nbA++;
				}
			return 0;
		}
}


int supprimerSommet(Graph *g, int a)
{
	Node *pnode, *precedent;
	Edge *pEdge, *suivant, *precedent_adj;
	int flag_premier_sommet, flag_premier_arc;
	if (g->premierSommet == NULL)
		{
			printf("Erreur! Graph vide, suppression impossible\n");
			return -1;
		}
	else
		{
			pnode=g->premierSommet;
			flag_premier_sommet=1;
			while (pnode != NULL)
				{
					if (pnode->label == a) break;
					else
						{
							flag_premier_sommet=0;
							precedent=pnode;
							pnode=pnode->suivant;
						}
				}
			if (pnode == NULL)
				{
					printf("Erreur! Le sommet a supprimer n'existe pas\n");
					return -1;
				}
			else
				{
					if (pnode->suivant == NULL) g->dernierSommet=precedent;

					if (flag_premier_sommet == 1) g->premierSommet=pnode->suivant;
					else precedent->suivant=pnode->suivant;
					pEdge=pnode->adj;
					free(pnode);
					g->nbS--;
					while (pEdge != NULL)
						{
							suivant=pEdge->suivant;
							free(pEdge);
							g->nbA--;
							pEdge=suivant;
						}
				}

			/* il faut aussi supprimer les arcs ayant le sommet a supprimer comme extremite */
			pnode=g->premierSommet;
			while (pnode != NULL)
				{
					pEdge=pnode->adj;
					flag_premier_arc=1;
					while (pEdge !=NULL)
						{
							if (pEdge->dest == a) break;
							else
								{
									flag_premier_arc=0;
									precedent_adj=pEdge;
									pEdge=pEdge->suivant;
								}
						}
					if (pEdge != NULL)
						{
							if (flag_premier_arc == 1) pnode->adj=pEdge->suivant;
							else precedent_adj->suivant=pEdge->suivant;
							free(pEdge);
							g->nbA--;
						}
					pnode=pnode->suivant;
				}
			return 0;
		}
}

int supprimerArc(Graph *g, int a, int b)
{
	Node *pnode;
	Edge *pEdge, *precedent_adj;
	int flag_premier_arc;
	if (g->premierSommet == NULL)
		{
			printf("Erreur! Graph vide, suppression impossible\n");
			return -1;
		}
	else
		{
			pnode=g->premierSommet;
			while (pnode != NULL)
				{
					if (pnode->label == a) break;
					else pnode=pnode->suivant;
				}
			if (pnode == NULL)
				{
					printf("Erreur! L'extremite de l'arc a supprimer n'existe pas\n");
					return -1;
				}
			else
				{
					pEdge=pnode->adj;
					flag_premier_arc=1;
					while (pEdge !=NULL)
						{
							if (pEdge->dest == b) break;
							else
								{
									flag_premier_arc=0;
									precedent_adj=pEdge;
									pEdge=pEdge->suivant;
								}
						}
					if (pEdge != NULL)
						{
							if (flag_premier_arc == 1) pnode->adj=pEdge->suivant;
							else precedent_adj->suivant=pEdge->suivant;
							free(pEdge);
							g->nbA--;
						}
					else
						{
							printf("Erreur! L'extremite de l'arc a supprimer n'existe pas\n");
							return -1;
						}
				}
			return 0;
		}
}

void supprimerGraph(Graph *g)
{
	Node *pnode,*temps;
	Edge *pEdge,*tempEdge;
	pnode=g->premierSommet;
	while (pnode !=NULL)
		{
			pEdge=pnode->adj;
			while (pEdge !=NULL)
				{
					tempEdge=pEdge;
					pEdge=pEdge->suivant;
					free(tempEdge);
				}
			temps=pnode;
			pnode=pnode->suivant;
			free(temps);
		}
	initGraph(g);
}

void afficherGraph(Graph *g)
{
	Node *pnode;
	Edge *pEdge;
	if (g->premierSommet == NULL) printf("Graph vide\n");
	else
		{
			printf("nbS=%d , nbA=%d, label max.=%d\n", g->nbS, g->nbA, g->maxS);
			pnode=g->premierSommet;
			do
				{
					printf("\n");
					printf("Sommet de label: %d , info: %d\n", pnode->label, pnode->info);
					if (pnode->adj == NULL) printf(" -> ce sommet n'a aucun arc sortant\n ");
					else
						{
							pEdge=pnode->adj;
							do
								{
									printf(" -> arc de %d vers %d avec l'info. %d \n", pnode->label, pEdge->dest, pEdge->info);
									pEdge=pEdge->suivant;
								}
							while (pEdge != NULL);
						}
					printf("\n");
					pnode=pnode->suivant;
				}
			while (pnode != NULL);
		}
}

int lireFichier(char *filename, Graph *g)
{
	FILE *fp;
	char line[MAX+1]; //Max == 10000 caracteres par ligne
	int weight,i,j,nbS1,nbLine,node,nbEdge,createEdge;

	initGraph(g);
	fp=fopen(filename,"r"); /* ouvre un fichier en lecture */
	nbLine=0; /* compte les lines du fichier */
	node=0; /* label du sommet en cours */
	nbS1=0; /* compte les sommets de la 1ere line */


 	while(fgets(line,MAX,fp) != NULL){
		nbLine++; /* compte le nombre de lines du fichier */
		if (line[0] != '\n'){ /* on passe les lines vides */
			i=0;
			//Check le nombre de sommets donné par la premiere ligne
			if (!nbS1){ /* compte les sommets de la 1ere line */
				nbS1=1;
				while (line[i] != '\n'){
						if (line[i] == ',')
							nbS1++;
						i++;
				}
				for (j=1; j<=nbS1; j++)
					addNode(g);
				i=0; /* on relit la 1ere line */
			}
			node++; /* sommet courant & origine des arcs */
			nbEdge=0; /* controle le nombre d'arcs crees */
			while (line[i] != '\n'){
				weight=0; /* va contenir l'extremite */
				createEdge=1;
				//Lis le nombre/chiffre pour savoir le nombre d'arcs
				while (line[i] != ',' && line[i] != '\n'){
					while (line[i]==' ' || line[i]=='\t')
						i++;
					if ((line[i]>'9' || line[i]<'0') && line[i]!='x'){
						printf("Erreur à la line %d !\n",nbline);
						deleteGraph(g);
						return -1; /* pas des chiffres ! */
					}
					if (line[i]=='x')
						createEdge=0;
					weight=10*weight+line[i]-'0';
					i++;
					while (line[i]==' ' || line[i]=='\t')
						i++;
				}
				if (line[i] == ',')
					i++;
				nbEdge++;
				if (nbEdge<=nbS1 && createEdge==1)
					addEdge(g,node,nbEdge,weight); /* line pas trop longue */
			}
			if (nbEdge != nbS1){ /* pas le bon nombre de champs sur line */
				printf("Erreur à la line %d !\n",nbline);
				supprimerGraph(g);
				return -1; /* pas le bon nombre de champs */
			}
		}
    }
	fclose(fp);
	return 0;
}
