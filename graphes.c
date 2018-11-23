#include <stdio.h>
#include <stdlib.h>
#include "graphes.h"

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
	community->label = g->nbNode;
	community->weightInt = 0.0;
	node->myCom = g->nbNode;
	node->neighbours=NULL;
	node->weightNode=0.0;
	node->next=NULL;
	node->previous = NULL;
	community->member=node;
	if(g->community == NULL){
	    community->next=NULL;
	}else
	    community->next=g->community;
	g->community = community;
	return 0;
}

//Fonctionne qu'a l'init
static int addEdge(Graph *g, long unsigned int a, long unsigned int b, double weight){
	Community *pCom, *pCom2;
	Edge *pEdge;
	pCom=g->community;

	/* on parcourt les community jusqu'a trouver a */
	//printf("a = %lu, b = %lu, weight=%lf\n",a,b,weight);
	while (pCom != NULL && pCom->label != a){
	    //printf("pcom label:%lu\n",pCom->label);
		pCom=pCom->next;
	}
	if (pCom == NULL){
		printf("Erreur! Creation d'un arc dont l'origine n'existe pas\n");
		return -1;
	}
	/* on parcourt les sommets pour trouver b */
	pCom2=g->community;
	while (pCom2 != NULL && pCom2->label != b) //Peut etre probleme is pCom == NULL car pCom->label n'existe pas et donc peut pas y avoir accès
		pCom2=pCom2->next;
	if (pCom2 == NULL){
		printf("Erreur! Creation d'un arc dont l'extremite n'existe pas\n");
		return -2;
	}
	pEdge=(Edge *) malloc(sizeof(Edge));
	if (pEdge == NULL){
		printf("Erreur! Memoire insuffisante pour creer un arc\n");
		return -3;
	}

	pEdge->weight = weight;
	pEdge->dest = pCom2->member;
	pEdge->next = pCom->member->neighbours;
	pCom->member->neighbours = pEdge;
	pCom->member->weightNode += weight;
	pCom->weightInt += weight;
	g->nbEdge++;
	g->weightTot += weight;
	return 0;
}


//Rajouter weight tot dans sommet node?
//PRENDRE COMPTE DE LA MODIFICATION DE POIDS
//Comment remove le membre de la communauté dans supprimer celui-ci
int changeCommunity(Graph *g, Node* node, Community* community){
	if(node->myCom == community->label)
		return 0;

	Community* pCom, *prevCom;
	Node* pNode;
	unsigned is_first=1;
	unsigned i=0,nbMember=0;

	//Cherche la communauté du membre à déplacer
	prevCom = g->community;
	pCom = g->community;
	while(pCom->label != node->myCom){
		prevCom = pCom;
		pCom=pCom->next;
		is_first=0;
	}
	//Si on remove le champ previous de node alors on doit faire ça
	/*pNode = node->next;
	while(pNode != NULL){
		pNode = pNode->next;
		i++;
	}

	pNode = pCom->member;
	while(pNode != NULL){
		pNode = pNode->next;
		nbMember++;
	}

	pNode = pCom->member;
	while(nbMember-i-1>1){
		pNode = pNode->next;
		nbMember--;
	}

	pNode->next = node->next;*/

	//Si le premier elem ou pas
	if(node->previous == NULL)
		pCom->member = node->next;
	else
		node->previous->next = node->next;
	//Si la communauté est vide alors on la supprime
	pCom->weightInt -= node->weightNode;
	if(pCom->weightInt == 0){
	    if(is_first)
	        g->community = pCom->next;
	    else
		    prevCom->next = pCom->next;
		free(pCom);
	}

	//Ajout du membre dans la communauté
	node->myCom = community->label;
	node->next = community->member;
	node->previous = NULL;
	community->member->previous = node;
	community->member = node;
	community->weightInt += node->weightNode;
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

void showGraph(Graph *g){
	Community *pCom;
	Node *pNode;
	Edge *pEdge;
	if(g->community == NULL){
		printf("Graphe vide\n");
		return;
	}
	printf("Nombre Sommets: %lu\n Nombre Arc: %lu\n Poids total=%lf\n", g->nbNode, g->nbEdge, g->weightTot);
	pCom=g->community;
	while(pCom != NULL){
		printf("\n");
		printf("Communauté label: %lu\n Poids intérieur: %lf\n", pCom->label-1, pCom->weightInt);
		pNode=pCom->member;
		while(pNode != NULL){
			printf("Le sommet a un poids total de %lf\n", pNode->weightNode);
		    if (pNode->neighbours == NULL)
			    printf(" -> ce sommet n'a aucun arc sortant\n ");
		    else{
		    	pEdge=pNode->neighbours;
		    	while(pEdge != NULL){
		    		printf(" -> arc de %lu vers %lu avec l'info. %lf \n", pNode->myCom-1,pEdge->dest->myCom-1, pEdge->weight);
			    	pEdge=pEdge->next;
			    }
		    }
		    printf("\n");
		    pNode=pNode->next;
		}
	pCom=pCom->next;
	}
}

int readFile(char *filename, Graph *g){
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
				for (j=1; j<=nbS1; j++){
					addCommunity(g);
				}
				i=0; /* on relit la 1ere line */
			}
			node++; /* sommet courant & origine des arcs */
			nbEdge=0; /* controle le nombre d'arcs crees */
			while (line[i] != '\n'){
				weight=0;
				createEdge=1;
				//Lis le nombre/chiffre pour savoir le poids entre l'arc
				// %node et %nbEdge
				while (line[i] != ',' && line[i] != '\n'){
					while (line[i]==' ' || line[i]=='\t')
						i++;
					if ((line[i]>'9' || line[i]<'0') && line[i]!='x'){
						printf("Erreur à la line %d !\n",nbLine);
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
				printf("Erreur à la line %d !\n",nbLine);
				deleteGraph(g);
				return -1; /* pas le bon nombre de champs */
			}
		}
    }
	fclose(fp);
	return 0;
}

int main(int argc, char *argv[]){
    char filename[10]="graph.txt";
    Graph *g = malloc(sizeof(Graph));
    if(g==NULL)
        return -1;
    readFile(filename,g);
    showGraph(g);

	Node* pNode;
	
	
    //Ajout du sommet 14 dans communauté 2
	Community* pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	pNode = g->community->next->member;
	printf("De pNode: %lu à pCom: %lu\n", pNode->myCom-1, pCom->label-1);

	changeCommunity(g,pNode ,pCom);
	showGraph(g);

    //Ajout du sommet 15 dans communauté 2
    pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	pNode = g->community->member;
	printf("De pNode: %lu à pCom: %lu\n", pNode->myCom-1, pCom->label-1);
	changeCommunity(g,pNode ,pCom);
	showGraph(g);

    //Ajout du sommet 0 dans communauté 2
    pCom = g->community;
	while(pCom->label-1 != 0)
		pCom = pCom->next;
	pNode = pCom->member;
	
	pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	printf("De pNode: %lu à pCom: %lu\n", pNode->myCom-1, pCom->label-1);
	changeCommunity(g,pNode ,pCom);
	showGraph(g);

	pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	pNode = pCom->member->next;
	pCom = g->community;
	while(pCom->label-1 != 3)
		pCom = pCom->next;

	printf("De pNode: %lu à pCom: %lu\n", pNode->myCom-1, pCom->label-1);

	changeCommunity(g,pNode ,pCom);

	showGraph(g);
    deleteGraph(g);
	free(g);
    return 0;
}
