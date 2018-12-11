#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "graphes.h"

/*
Initialise le graphe mais n'alloue pas de memoire)

input: g-> le graphe
*/
static void initGraph(Graph *g);
/*
Supprime une communaute du graphe

input: un graphe g deja initialise et une de ses communautes
*/
static void deleteCommunity(Graph *g, Community* community);

//Affiche un graphe different element d'un graphe
static void showGraph(Graph *);

//Cree un graphe à partir d'un fichier.
static int readFile(char *filename, Graph *);

/*
Ajout du membre $member$ dans la communauté $community$.
Apres l'ajout, si l'ancienne communauté de $member$ est vide, on la supprime

input: g : un graphe initialisé
	  member: un membre d'une communauté
	  community: la nouvelle communauté du membre $member$
*/
static void changeCommunity(Graph *g, Member* member, Community* community);

/*Summing the weights of all links from a member to a community

input:member: a member of a community (!= NULL)
		label: the label of a community

return: >=0: the weights of the links from $member$ to community $label$
		 -1: the member does not exist
*/
static double weightToCommunity(Member* member, size_t label);

//Take off a member from its community
static void deleteMemberFromCommunity(Graph *g, Member *member);

static double gainModularity(Graph* g, Member* member, Community* com);

static void initGraph(Graph *g){
	if(g == NULL){
		printf("Allouer le graphe avant de l'initialiser\n");
		return;
	}
	g->nbCommunity=0;
	g->nbEdge=0;
	g->nbMembers=0;
	g->weightTot=0.0;
   g->community=NULL;
}

void deleteGraph(Graph *g){
	if(g == NULL)
		return;
	Community *pCom,*tmpCom;
	Member* pMember, *tmpMember;
	Edge *pEdge,*tmpEdge;

	//Delete all communities including its edges and its members
	pCom=g->community;
	while (pCom != NULL){
	    pMember=pCom->member;
	    while(pMember != NULL){
		    pEdge=pMember->neighbours;
		    while(pEdge != NULL){
			    tmpEdge=pEdge;
			    pEdge=pEdge->next;
			    free(tmpEdge);
			}
		    tmpMember = pMember;
		    pMember = pMember->next;
		    free(tmpMember);
		}
		tmpCom=pCom;
		pCom=pCom->next;
		free(tmpCom);
	}
	initGraph(g);
}

static void showGraph(Graph *g){
	Community *pCom;
	Member *pMember;
	Edge *pEdge;
	if(g->community == NULL){
		printf("Graphe vide\n");
		return;
	}
	printf("Nombre Sommets: %lu\n Nombre Arc: %lu\n Poids total=%lf\n", g->nbCommunity, g->nbEdge, g->weightTot);
	pCom=g->community;
	while(pCom != NULL){
		printf("\n");
		printf("Communaute label: %lu\n Poids total: %lf\n Poids interieur: %lf\n", pCom->label-1, pCom->weightTot, pCom->weightInt);
		pMember=pCom->member;
		while(pMember != NULL){
			printf("Le sommet a un poids total de %lf\n", pMember->weightMember);
		    if (pMember->neighbours == NULL)
			    printf(" -> ce sommet n'a aucun arc sortant\n ");
		    else{
		    	pEdge=pMember->neighbours;
		    	while(pEdge != NULL){
		    		printf(" -> arc de %lu vers %lu avec l'info. %lf \n", pMember->myCom->label-1,pEdge->dest->myCom->label-1, pEdge->weight);
			    	pEdge=pEdge->next;
			    }
		    }
		    printf("\n");
		    pMember=pMember->next;
		}
	pCom=pCom->next;
	}
}

int addEdge(Graph *g, size_t start, size_t end, double weight){
	if(start == 0 || start > g->nbMembers || weight == 0 || g == NULL
	   			   						|| end == 0 || end > g->nbMembers){
		printf("Preconditions non respectees pour addEdge\n");
		return -4;
	}
	Community *pComStart, *pComEnd;
	Member* pMemberStart, *pMemberEnd;
	Edge *pEdge;

	//Searching for the member $start$
	pMemberStart = NULL;
	pComStart=g->community;
	while (pComStart != NULL && pMemberStart == NULL){
		pMemberStart = pComStart->member;
		while(pMemberStart!=NULL && pMemberStart->label != start)
			pMemberStart = pMemberStart->next;
		pComStart=pComStart->next;
	}

	if (pMemberStart == NULL){
		printf("Erreur! Creation d'un arc dont l'origine n'existe pas\n");
		return -1;
	}

	//Searching for the member $end$
	pMemberEnd = NULL;
	pComEnd=g->community;
	while (pComEnd != NULL && pMemberEnd == NULL){
		pMemberEnd = pComEnd->member;
		while(pMemberEnd!=NULL && pMemberEnd->label != end){
			pMemberEnd = pMemberEnd->next;
		}
		pComEnd=pComEnd->next;
	}

	if (pMemberEnd == NULL){
		printf("Erreur! Creation d'un arc dont la destination n'existe pas\n");
		return -1;
	}

	//Allocating an edge
	pEdge=(Edge *) malloc(sizeof(Edge));
	if (pEdge == NULL){
		printf("Erreur! Memoire insuffisante pour creer un arc\n");
		return -3;
	}

	//Initialisation of an edge from $start$ to $end$
	pEdge->weight = weight;
	pEdge->dest = pMemberEnd;
	pEdge->next = pMemberStart->neighbours;
	pMemberStart->neighbours = pEdge;

	//Updating the community due to the addition of the edge
	pMemberStart->weightMember += weight;
	pMemberStart->myCom->weightTot += weight;

	//If the edge is a loop, the weight are doubled
   if(start == end){
		pMemberStart->myCom->weightInt += 2*weight;
		pMemberStart->myCom->weightTot += weight;
		g->weightTot += weight;
	}

	//Updating the graph
	g->weightTot += weight;
	g->nbEdge++;
	return 0;
}

int addCommunity(Graph *g){
	if(g == NULL){
		printf("Preconditions non respectees pour addCommunity\n");
		return -2;
	}
	Member *member;
	Community *community;


	//Allocating new community
	community=(Community *) malloc(sizeof(Community));
	if (community == NULL){
		printf("Erreur! Memoire insuffisante pour creer une community\n");
		return -1;
	}

	//Allocating new member
	member=(Member *) malloc(sizeof(Member));
	if (member == NULL){
		free(community);
		printf("Erreur! Memoire insuffisante pour creer un membre\n");
		return -1;
	}

	//Updating number of communities and members
	g->nbCommunity++;
	g->nbMembers++;

	//Initialisation of the member
	member->label = g->nbMembers;
	member->neighbours = NULL;
	member->weightMember = 0.0;
	member->next = NULL;
	member->previous = NULL;
	member->myCom = community;

	//Initialisation of the community and adding the member in it
	community->label = g->nbCommunity;
	community->weightTot = 0.0;
	community->weightInt = 0.0;
	community->member=member;
	community->previous =NULL;
	community->next=g->community;

	//Adding the new community at the beginning of the list
	if(g->community != NULL)
		g->community->previous = community;
	g->community = community;
	return 0;

}

static void deleteCommunity(Graph *g, Community* community){
	if(g == NULL || community == NULL)
		return;

	//If unique community
	if(community->previous == NULL && community->next == NULL)
		g->community = NULL;
	else{
		//If first community of the list
		if(community->previous == NULL){
			g->community = community->next;
			community->next->previous = NULL;
		}
		else{
			//If last community of the list		
			if(community->next == NULL)
				community->previous->next = community->next;
			//If middle community of the list
			else{
				community->previous->next = community->next;
				community->next->previous = community->previous;
			}
		}
	}
	g->nbCommunity--;
	free(community);
}

static void deleteMemberFromCommunity(Graph *g, Member *member){
	if(g == NULL || member == NULL)
		return;
	double weightIn;
	//If unique member, then we delete community
	if(member->previous == NULL && member->next == NULL){
		member->myCom->member = NULL;
		printf("myCom: %lu ", member->myCom->label-1);
		printf("got deleted\n");
		deleteCommunity(g, member->myCom);
	}
	//If the member is not unique, then we just take it off
	else{
		//If first member from the list
		if(member->previous == NULL){
			member->next->previous = NULL;
			member->myCom->member = member->next;
		}
		else{
			//If last member from the list
			if(member->next == NULL)
				member->previous->next = NULL;
			//If middle member from the list
			else{
			member->previous->next = member->next;
			member->next->previous = member->previous;
			}
		}
		//Taking the weights off the community due to the calling off
		weightIn = weightToCommunity(member, member->myCom->label);
		member->myCom->weightTot -= member->weightMember;
		member->myCom->weightTot += weightIn;        
		member->myCom->weightInt -= weightIn;
	}
}

static void changeCommunity(Graph *g, Member* member, Community* community){
	if(g == NULL || member == NULL || community == NULL){
		printf("Preconditions non respectees pour changeCommunity, communaute inchangee\n");
		return;
	}

	//If member is already in the community then we do nothing
	if(member->myCom->label == community->label)
		return;

	//Taking the member off its community
	deleteMemberFromCommunity(g,member);

	//Addingg the member in the new community
	member->myCom = community;
	member->next = community->member;
	member->previous = NULL;
	community->member->previous = member;
	community->member = member;

	//Updating the weight of the new community
	double weightIn;
	weightIn = weightToCommunity(member,community->label);
	community->weightInt += weightIn;
	community->weightTot += member->weightMember;
	community->weightTot -= weightIn;


	return;
}

static double weightToCommunity(Member* member, size_t label){
	if(member == NULL){
		printf("Le membre n'existe pas (weightToCommunity)");
		return -1;
	}
   Edge* pNeighbours = member->neighbours;
   double weightToCommunity = 0.0;

   while(pNeighbours != NULL){
		if(pNeighbours->dest->myCom->label == label)
			weightToCommunity += pNeighbours->weight;
		pNeighbours = pNeighbours->next;
   }

   return weightToCommunity;
}

//Est il necessaire de calculer le gain entre membre de communaute?
//Oui car sinon le membre changera de communaute bcp plus de fois
//Si non 
//		-> mettre if(member->myCom->label == com->label) return 0.0;
static double gainModularity(Graph* g, Member* member, Community* com){
	if(g == NULL || com == NULL || member == NULL){
		printf("Preconditions non respectees pour gainModularity\n");
		return -1;
	}
   printf("from %lu to %lu\n",member->myCom->label-1 ,com->label-1);
   double weightTot;
   double comWeightInt, comWeightExt;
   double memberWeight, memberWeightCom;
   Member* pMember;
   Edge* pNeightbours;

   //sum of the weights of the links inside C
   comWeightInt = com->weightInt;
   //sum of the weights of the links incident to Members in C
   comWeightExt = com->weightTot - com->weightInt;

   //sum of the weights links incident to Members in i
   memberWeight = member->weightMember;
   //sum of the weights of the links from i to Members in c
   memberWeightCom = weightToCommunity(member, com->label);

   //sum of the weights of all the links in the network
   weightTot = g->weightTot;

   printf("sum_in = %lf, sum_tot = %lf, k_in = %lf, k_i = %lf, m = %lf\n",comWeightInt,comWeightExt,memberWeightCom,memberWeight,weightTot);
   double t1 = (comWeightInt + memberWeightCom)/(weightTot);
   double t2 = (comWeightExt + memberWeight)/(weightTot);
   double t3 = t1 - (t2 * t2);

   double s1 = (comWeightExt/(weightTot))*(comWeightExt/(weightTot));
   double s2 = (memberWeight/(weightTot))*(memberWeight/(weightTot));
   double s3 = (comWeightInt/(weightTot))-s1-s2;

   printf("q = %lf \n",t3-s3);
   return (t3 - s3);
}

void stepOne(Graph* g){
	if(g==NULL)
		return;
	Community *pCom, *pComNext,*pNewCom;
	Member *pMember, *pMemberNext;
	Edge *pNeighbours;
	unsigned int i, has_imp;
	double maxQ, q;

	pCom = g->community;

	//Browsing each community
	for(i=1; i <= g->nbCommunity; i++){
		if(pCom == NULL)
			pCom = g->community;
		maxQ = 0.0;
		has_imp = 0;

		//Browsing each member of the community
		pMember = pCom->member;
		while (pMember != NULL){
			//Browsing each neightours of the member
			pNeighbours = pMember->neighbours;
			while(pNeighbours != NULL){
				q = gainModularity(g, pMember, pNeighbours->dest->myCom);
				if (maxQ < q) {
					maxQ = q;
					pNewCom = pNeighbours->dest->myCom;
					has_imp = 1;
				}
				pNeighbours = pNeighbours->next;
			}

			//(pMember might be in other community or pCom might be deleted)
			//Keeping the order of the browsing
			pMemberNext = pMember->next;
			pComNext = pCom->next;
			if(has_imp && pMember->myCom->label != pNewCom->label){
				printf("**************************From %lu to %lu because %lf\n", pMember->myCom->label-1, pNewCom->label-1,maxQ);
				changeCommunity(g, pMember, pNewCom);
				i = 0;//If a member has changed of community, then we have to recheck all communities
			}
			pMember = pMemberNext;
		}
		pCom = pComNext;
	}
}

static int readFile(char *filename, Graph *g){
	if(g == NULL){
		printf("Le graphe doit être alloue avant de lire le fichier\n");
		return -1;
	}
	FILE *fp;
	char line[MAX+1];
	int i,j,nbS1,nbLine,member,nbEdge,createEdge;
	double weight;

	initGraph(g);
	//Open the file in read mode
	fp=fopen(filename,"r");

	nbLine=0;
	member=0;
	nbS1=0;


 	while(fgets(line,MAX,fp) != NULL){
		//Counting the number of lines in the file
		nbLine++;
		//Denying all empty lines
		if (line[0] != '\n'){
			i=0;
			//Check the number of all nodes given by the first line
			if (!nbS1){
				nbS1=1;
				while (line[i] != '\n'){
						if (line[i] == ',')
							nbS1++;
						i++;
				}
				for (j=1; j<=nbS1; j++){
					addCommunity(g);
				}
				//Reading the first line again
				i=0;
			}
			//The current node and the origin of the links
			member++; 
			//The number of edges created
			nbEdge=0;
			while (line[i] != '\n'){
				weight=0;
				createEdge=1;
				//Reading the number for the weight of the link
				while (line[i] != ',' && line[i] != '\n'){
					while (line[i]==' ' || line[i]=='\t')
						i++;
					//If no numbers, then delete
					if ((line[i]>'9' || line[i]<'0') && line[i]!='x'){
						printf("Erreur a la line %d !\n",nbLine);
						deleteGraph(g);
						return -1;
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
					addEdge(g,member,nbEdge,weight);
			}
			if (nbEdge != nbS1){
				printf("Erreur a la line %d !\n",nbLine);
				deleteGraph(g);
				return -1;
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
	Community* tmp;
	Community* com = g->community;
	//Reverse list
	while(com != NULL){
		tmp = com->previous;
		com->previous = com->next;
		com->next = tmp;
		if(com->previous == NULL)
			g->community = com;
		com = com->previous;
	}
	showGraph(g);
/*	Member* pMember;


    //Ajout du sommet 14 dans communaute 2
	Community* pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	pMember = g->community->next->member;
	printf("De pMember: %lu a pCom: %lu\n", pMember->myCom->label-1, pCom->label-1);

	changeCommunity(g,pMember ,pCom);
	showGraph(g);

    //Ajout du sommet 15 dans communaute 2
    pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	pMember = g->community->member;
	printf("De pMember: %lu a pCom: %lu\n", pMember->myCom->label-1, pCom->label-1);
	changeCommunity(g,pMember ,pCom);
	showGraph(g);

    //Ajout du sommet 0 dans communaute 2
    pCom = g->community;
	while(pCom->label-1 != 0)
		pCom = pCom->next;
	pMember = pCom->member;

	pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	printf("De pMember: %lu a pCom: %lu\n", pMember->myCom->label-1, pCom->label-1);
	changeCommunity(g,pMember ,pCom);
	showGraph(g);

	//Ajout du 2ieme membre de le communaute 2 a la communaute 3
	pCom = g->community;
	while(pCom->label-1 != 2)
		pCom = pCom->next;
	pMember = pCom->member->next;
	pCom = g->community;
	while(pCom->label-1 != 3)
		pCom = pCom->next;

	printf("De pMember: %lu a pCom: %lu\n", pMember->myCom->label-1, pCom->label-1);

	changeCommunity(g,pMember ,pCom);

	showGraph(g);
	*/
	stepOne(g);
	showGraph(g);
    deleteGraph(g);
	free(g);
    return 0;
}
