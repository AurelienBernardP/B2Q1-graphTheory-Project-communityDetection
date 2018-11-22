#include <stdio.h>
#include "Graphes.h"

static void initGraph(Graph *g);
static int addCommunity(Graph *g);
static int addEdge(Graph *g, long unsigned int a, long unsigned int b, double weight);

static void initGraph(Graph *g){
	g->nbNode=0;
	g->nbEdge=0;
	g->weightTot=0.0;
    g->community=NULL;
}

static int addCommunity(Graph *g){
	Node *node;
	Community* community;
	community=(Community *) malloc(sizeof(Community));
	if (community == NULL){
		printf("Erreur! Memoire insuffisante pour creer une community\n");
		return -1;
	}
	node=(Node *) malloc(sizeof(Node));
	if (node == NULL){
		printf("Erreur! Memoire insuffisante pour creer un sommet\n");
		return -1;
	}
	
	
	g->nbNode++;
	community.label = g->nbNode;
	community.weightInt = 0.0;
	node->myCom = g->nbNode;
	node->neighbours=NULL;
	node->next=NULL;
	community->member=node;
	community->next=g->com->next;
	g->community = community;
	return 0;
}

static int addEdge(Graph *g, long unsigned int a, long unsigned int b, double weight){
	Community *pCom, *pCom2;
	Edge *pEdge;
	pCom=g->community;

	/* on parcourt les community jusqu'a trouver a */
	while (pCom != NULL && pCom.label != a)
		pCom=pCom->next;
	if (pCom == NULL){
		printf("Erreur! Creation d'un arc dont l'origine n'existe pas\n");
		return -1;
	}
	/* on parcourt les sommets pour trouver b */
	pCom2=g->community;
	while (pCom2 != NULL && pCom2.label != b) //Peut etre probleme is pCom == NULL car pCom.label n'existe pas et donc peut pas y avoir accès
		pCom2=pCom2->next;
	if (pnode2 == NULL){
		printf("Erreur! Creation d'un arc dont l'extremite n'existe pas\n");
		return -2;
	}
	pEdge=(Edge *) malloc(sizeof(Edge));
	if (pEdge == NULL){
		printf("Erreur! Memoire insuffisante pour creer un arc\n");
		return -3;
	}
	
	pEdge.weight = weight;
	pEdge->dest = pCom2->member;
	pEdge->next = pCom->member->neighbours;
	pCom->member->neighbours = pEdge;
	pCom.weightInt += weight;
	g->nbEdge++;
	g->weightTot += weight;
	return 0;
}


void deleteGraph(Graph *g){
	Community *pCom,*tmpCom;
	Node* pNode, *tmpNode;
	Edge *pEdge,*tmpEdge;
	pCom=g->community;
	while (pCom != NULL){
	    pNode=pCom->member;
	    while(pNode != NULL){
		    pEdge=pNode->neighbours;
		    while(pEdge != NULL){
			    tmpEdge=pEdge;
			    pEdge=pEdge->next;
			    free(tmpEdge);
			}
		    tmpNode = pNode;
		    pNode = pNode->next;
		    free(tmpNode);
		}
		tmpCom=pCom;
		pCom=pCom->next;
		free(tmpCom);
	}
	initGraph(g);
}

void afficherGraph(Graph *g){
	Community *pCom;
	Node *pNode;
	Edge *pEdge;
	if(g->community == NULL){
		printf("Graphe vide\n");
		return
	}
	printf("Nombre Sommets: %lu\n Nombre Arc: %lu\n Poinds total=%d\n", g->nbNode, g->nbEdge, g->weightTot);
	pCom=g->community;
	while(pCom != NULL){
		printf("\n");
		printf("Communauté label: %lu\n Poids intérieur: %lf\n", pCom.label, pCom.weightInt);
		pNode=pCom->member
		while(pNode != NULL){
		    if (pNode->neighbours == NULL)
			    printf(" -> ce sommet n'a aucun arc sortant\n ");
		    else{
		    	pEdge=pNode->neighbours;
		    	while(pEdge != NULL){
		    		printf(" -> arc de %lu vers %lu avec l'info. %lf \n", pNode.myCom,pEdge->dest.myCom, pEdge->weight);
			    	pEdge=pEdge->suivant;
			    }
		    }
		    printf("\n");
		    pNode=pNode->next;
		}
	pCom=pCom->next;
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
				weight=0;
				createEdge=1;
				//Lis le nombre/chiffre pour savoir le poid entre l'arc
				// %node et %nbEdge
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
				deleteGraph(g);
				return -1; /* pas le bon nombre de champs */
			}
		}
    }
	fclose(fp);
	return 0;
}

/*int supprimerArc(Graph *g, int a, int b){
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
*/

/* la liste d'adjacence est non vide, on la parcourt pour voir si b s'y trouve
if(pEdge->dest > b){
	pEdge=(Edge *) malloc(sizeof(Edge));
	if (pEdge == NULL){
		printf("Erreur! Memoire insuffisante pour creer un sommet\n");
		return -3;
	}else{
		pEdge->suivant=pnode->adj;
		pnode->adj=pEdge;
	}
}else{
	while (pEdge != NULL){
		if (pEdge->dest == b){
				pEdge->info=info;
				break;
		} /* l'arc existe, update info
		if (pEdge->dest > b){
			pEdge=NULL;
			break;
		} /* on depasse b sans le trouver
		precedent=pEdge;
		pEdge=pEdge->suivant;
	}
	if (pEdge == NULL){ /* l'arc n'existe pas, il faut le creer
		pEdge=(Edge *) malloc(sizeof(Edge));
		if (pEdge == NULL){
			printf("Erreur! Memoire insuffisante pour creer un sommet\n");
			return -3;
		}else
			if (precedent->suivant==NULL){ /* element ajouter a la fin
				precedent->suivant=pEdge;
				pEdge->suivant=NULL;
			}
			else{ /* element ajouter "au milieu" pour garder ordre
				pEdge->suivant=precedent->suivant;
				precedent->suivant=pEdge;
			}
	}*/







	/*int supprimerSommet(Graph *g, int a){
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

				/* il faut aussi supprimer les arcs ayant le sommet a supprimer comme extremite
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
	*/
