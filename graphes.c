#include <stdio.h>
#include <stdlib.h>
#include "graphes.h"

static void initGraph(Graph *g);
static int addCommunity(Graph *g);
static int addEdge(Graph *g, long unsigned int a, long unsigned int b, double weight);

/*
Initialise le graphe mais n'alloue pas de mémoire)

input: g-> le graphe
*/

static void initGraph(Graph *g){
	g->nbCommunity=0;
	g->nbEdge=0;
	g->weightTot=0.0;
    g->community=NULL;
}

/*
Ajout au début de la liste $community$ une nouvelle Communauté qui possède
un unique membre.

input: g->un graphe initialisé
return : 0 si succès
		 1 si mémoire insuffisante pour allocation
*/
static int addCommunity(Graph *g){
	Node *member;
	Community* community;


	//Allocation d'une nouvelle communauté et d'un membre
	community=(Community *) malloc(sizeof(Community));
	if (community == NULL){
		printf("Erreur! Memoire insuffisante pour creer une community\n");
		return -1;
	}
	member=(Node *) malloc(sizeof(Node));
	if (member == NULL){
		free(community);
		printf("Erreur! Memoire insuffisante pour creer un membre\n");
		return -1;
	}

	//Mise à jour du nombre de communauté
	g->nbCommunity++;

	//Initialisation du membre
	member->neighbours=NULL;
	member->weightNode=0.0;
	member->next=NULL;
	member->previous = NULL;
	member->myCom = community;

	//Initialisation de la communauté
	community->label = g->nbCommunity;
	community->weightInt = 0.0;
	community->member=member;
	community->next=g->community;

	//Rajout de cette nouvelle communauté au début de la liste
	g->community = community;
	return 0;
}


/*
Ajout d'un arc de $start$ vers $end$ et de poids $weight$.
ATTENTION: Pré: Chaque sommet est associé à une communauté toutes différentes
		   Cette fonction est correcte qu'avec des communauté et non des sommets
 		   Puisqu'il n'existe pas de moyen pour identifier un membre
		   spécifiquement, on considère qu'il est possible de l'identifier
		   indépendamment et uniquement selon sa communauté.

input: g : un graphe déjà initialisé
	  start: la communauté d'origine de l'arc
	  end: la commaunté de destination de l'arc
	  weight : le poids de cet arc

return: 0 si succès
		-1 si la commaunté d'origine n'existe pas
		-2 si la commaunté de destination n'existe pas
		-3 si erreur allocation
*/
static int addEdge(Graph *g, long unsigned int start, long unsigned int end, double weight){
	Community *pCom, *pCom2;
	Edge *pEdge;

	/* on parcourt les communautés jusqu'à trouver $start$ */
	pCom=g->community;
	while (pCom != NULL && pCom->label != start)
		pCom=pCom->next;

	if (pCom == NULL){
		printf("Erreur! Création d'un arc dont l'origine n'existe pas\n");
		return -1;
	}

	/* on parcourt les communautés jusqu'à trouver $end$ */
	pCom2=g->community;
	while (pCom2 != NULL && pCom2->label != end) //Peut etre probleme is pCom == NULL car pCom->label n'existe pas et donc peut pas y avoir accès
		pCom2=pCom2->next;

	if (pCom2 == NULL){
		printf("Erreur! Création d'un arc dont l'extrémité n'existe pas\n");
		return -2;
	}

	//Allocation d'un arc
	pEdge=(Edge *) malloc(sizeof(Edge));
	if (pEdge == NULL){
		printf("Erreur! Mémoire insuffisante pour créer un arc\n");
		return -3;
	}

	//Initialisation d'un arc de $start$ à $end$
	pEdge->weight = weight;
	pEdge->dest = pCom2->member;

	//Ajout d'un arc au début de la liste du membre $start$
	pEdge->next = pCom->member->neighbours;
	pCom->member->neighbours = pEdge;

	//Mise à jour du graphe dû à l'ajout de l'arc
	pCom->member->weightNode += weight;
	pCom->weightInt += weight;
	g->weightTot += weight;

	g->nbEdge++;
	return 0;
}


//Rajouter weight tot dans sommet node?
//PRENDRE COMPTE DE LA MODIFICATION DE POIDS
//Comment remove le membre de la communauté dans supprimer celui-ci
int changeCommunity(Graph *g, Node* node, Community* community){
	if(node->myCom->label == community->label)
		return 0;

	Community* pCom, *prevCom;
	Node* pNode;
	unsigned is_first=1;
	unsigned i=0,nbMember=0;

	//Cherche la communauté du membre à déplacer
	prevCom = g->community;
	pCom = g->community;
	while(pCom->label != node->myCom->label){
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
		g->nbCommunity--;
	}

	//Ajout du membre dans la communauté
	node->myCom = community;
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
	printf("Nombre Sommets: %lu\n Nombre Arc: %lu\n Poids total=%lf\n", g->nbCommunity, g->nbEdge, g->weightTot);
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
		    		printf(" -> arc de %lu vers %lu avec l'info. %lf \n", pNode->myCom->label-1,pEdge->dest->myCom->label-1, pEdge->weight);
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

void stepOne(Graph* g){
	if(g==NULL)
		return;
	Community *pCom, *pComNext,*pNewCom;
	Node *pMember, *pMemberNext;
	Edge *pNeighbours;
	unsigned int i, has_imp;
	double maxQ, q;

	pCom = g->community;
	for(i=1; i <= g->nbCommunity; i++){
		if(pCom == NULL)
			pCom = g->community;
		maxQ = 0.0;
		pMember = pCom->member;
		has_imp = 0;
		while (pMember != NULL){
			pNeighbours = pMember->neighbours;
			while(pNeighbours != NULL){
				q = 0; //call delta q
				if (maxQ < q) {
					maxQ = q;
					pNewCom = pNeighbours->dest->myCom;
					has_imp = 1;
				}
				pNeighbours = pNeighbours->next;
			}
			//Le cas où la communauté n'existe plus. OK
			//On évite aussi de regarder les membres de la
			//nouvelle communauté qui a recu pMember
			pMemberNext = pMember->next;
			pComNext = pCom->next;
			if(has_imp){
				changeCommunity(g, pMember, pNewCom);
				i = 0; //on doit refaire un tour
			}
			pMember = pMemberNext;
		}
		pCom = pComNext;
	}
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
	printf("De pNode: %lu à pCom: %lu\n", pNode->myCom->label-1, pCom->label-1);

	changeCommunity(g,pNode ,pCom);
	showGraph(g);

    //Ajout du sommet 15 dans communauté 2
    pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	pNode = g->community->member;
	printf("De pNode: %lu à pCom: %lu\n", pNode->myCom->label-1, pCom->label-1);
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
	printf("De pNode: %lu à pCom: %lu\n", pNode->myCom->label-1, pCom->label-1);
	changeCommunity(g,pNode ,pCom);
	showGraph(g);

	//Ajout du 2ieme membre de le communauté 2 à la communauté 3
	pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	pNode = pCom->member->next;
	pCom = g->community;
	while(pCom->label-1 != 3)
		pCom = pCom->next;

	printf("De pNode: %lu à pCom: %lu\n", pNode->myCom->label-1, pCom->label-1);

	changeCommunity(g,pNode ,pCom);

	showGraph(g);
    deleteGraph(g);
	free(g);
    return 0;
}
