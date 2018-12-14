#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "optimisation.h"
#include "aggregation.h"


/* deleteCommunity
* Delete the community $community$ from the graph g
*
* [input]
* g: an initialised graph
* community : an initialised community inside the graph g
*
*/
static void deleteCommunity(Graph *g, Community* community);

/* displayGraph
* Show all elements of the graph
*
* [input]
* g: a graph
* showEdges: a boolean which indicates if all edges must be displayed or not
*
*/
static void displayGraph(Graph *g, bool showEdges);

/* readFile
* Reading the file $filename$  to create a graph -the format of the file is specific.
*
* [input]
* filename : the name of the file which describes a graph
* g : an initialised empty graph
*
* [return]
* 0 : The graph described in the file $filename$ has been successfully created
* -1 : The format of the file $filename$ is incorrect.
* -2 : The file $filename$ does not exist.
*
*/
static int readFile(char *filename, Graph * g);

/* changeMemberToCommunity
* Move the member $member$ to the community $community$ of the graph
*
* [input]
* g : an initialised graph
* member : an initialised member inside a community of the graph g
* community : a community of graph g
*
*/
static void changeMemberToCommunity(Graph *g, Member* member, Community* community);

/* weightToCommunity
* Summing the weight of all links from the member $member$ to all members 
* of the community which is labelled $label$
*
* [input]
* member : an initialised member of a graph
* label : the label of a community of the same graph
*
* [return]
* >=0 : the sum of the weight of all links from $member$ to community labelled $label$
* 0 : The member does not exist
*/
static double weightToCommunity(Member* member, size_t label);

/* deleteMemberFromCommunity
* Take off the member $member$ from its community and delete the community if 
* it is empty after the withdrawal
*
* [input]
* g: an initialised graph
* member : an initialised member inside a community of the graph g
*
*/
static void deleteMemberFromCommunity(Graph *g, Member *member);

/* gainModularity
* Calculate the gain of modularity when the member $member$ is moved
* to the community $com$
*
* [input]
* g : an initialised graph
* member : an initialised member inside a community of the graph g
* com : a community of graph g
*
* [return]
* [-1;1] : the gain of modularity
*
*/
static double gainModularity(Graph* g, Member* member, Community* com);

/* power
*Calculate the power of $x$ to $y$
*
* [input]
* x : the base
* y : the power
* [return]
* the power of x to $y$
*
*/
static double power(double x, int y);

/* modularityOptimisation
* Applying a local modularity optimisation.
*
* [input]
* g : an initialised graph
*
* [return]
* true : the communities of g have changed
* false : the communities of g haven't changed
*
*/
static bool modularityOptimisation(Graph* g);

Graph *initGraph(){
	Graph *g = malloc(sizeof(Graph));	
	if(!g){
		printf("Erreur allocation de graphe\n");
		return NULL;
	}
	g->nbCommunity=0;
	g->nbEdge=0;
	g->nbMembers=0;
	g->weightTot=0.0;
   g->community=NULL;
	return g;
}

static double power(double x, int y){
	if (!y)
		return x;
	return x * power(x, y-1);
}

void deleteGraph(Graph *g){
	if(!g)
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

	free(g);
}

static void displayGraph(Graph *g, bool showEdges){
	if(!g || !g->community){
		printf("Graphe vide\n");
		return;
	}
	Community *com;
	Member *member;

	printf("Nombre de communautes: %lu\n",g->nbCommunity);
	printf("Nombre de membres: %lu\n",g->nbMembers);
	printf("Nombre d'arcs: %lu\n",g->nbEdge);
	printf("Poids total de tous les arcs: %lf\n \n",g->weightTot);

	printf("Format d'affichage:\n");
	printf("Communaute: Membre(s)");
	if(showEdges)
		printf("\t ->Arc(s) [Poids]");
	printf("\n");

	com=g->community;
	while(com != NULL){
		printf("\n%lu: ",com->label);

		member = com->member;
		while(member != NULL){
			printf("%lu",member->label);
			if(showEdges){
				Edge *neighbours = member->neighbours;
				while(neighbours != NULL){
				printf("\t->%lu [%lf]\n",neighbours->dest->label,neighbours->weight);
				neighbours = neighbours->next;
				}
				if(member->next != NULL)
					printf("\n%lu: ",com->label);
			}
			if(member->next != NULL && !showEdges)
				printf(",");
			member = member->next;
		}
		com = com->next;
	}
	printf("\n");
}

int addEdge(Graph *g, size_t start, size_t end, double weight){
	if(!g){
		printf("Impossible d'ajouter un arc car le graphe n'existe pas\n");
		return -4;
	}
	Community *pComStart, *pComEnd;
	Member* pMemberStart, *pMemberEnd;
	Edge *pEdge;

	//Searching for the member $start$
	pMemberStart = NULL;
	pComStart=g->community;
	while (pComStart != NULL && !pMemberStart){
		pMemberStart = pComStart->member;
		while(pMemberStart != NULL && pMemberStart->label != start)
			pMemberStart = pMemberStart->next;
		pComStart=pComStart->next;
	}

	if (!pMemberStart){
		printf("Erreur! Creation d'un arc dont l'origine n'existe pas\n");
		return -1;
	}

	//Searching for the member $end$
	pMemberEnd = NULL;
	pComEnd=g->community;
	while (pComEnd != NULL && !pMemberEnd){
		pMemberEnd = pComEnd->member;
		while(pMemberEnd!=NULL && pMemberEnd->label != end){
			pMemberEnd = pMemberEnd->next;
		}
		pComEnd=pComEnd->next;
	}

	if (!pMemberEnd){
		printf("Erreur! Creation d'un arc dont la destination n'existe pas\n");
		return -2;
	}

	//Allocating an edge
	pEdge=(Edge *) malloc(sizeof(Edge));
	if (!pEdge){
		printf("Erreur! Memoire insuffisante pour creer un arc\n");
		return -3;
	}

	//Initialisation of an edge from $start$ to $end$
	pEdge->weight = weight;
	pEdge->dest = pMemberEnd;
	pEdge->next = pMemberStart->neighbours; //Adding at the beginning of the neighbours of $start$
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
	if(!g){
		printf("Impossible d'ajouter une communaute car le graphe n'existe pas\n");
		return -2;
	}
	Member *member;
	Community *community;


	//Allocating new community
	community=(Community *) malloc(sizeof(Community));
	if (!community){
		printf("Erreur! Memoire insuffisante pour creer une community\n");
		return -1;
	}

	//Allocating new member
	member=(Member *) malloc(sizeof(Member));
	if (!member){
		free(community);
		printf("Erreur! Memoire insuffisante pour creer un membre\n");
		return -1;
	}

	//Updating number of communities and members of the graph
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
	if(!g || !community){
		printf("Le graphe ou la communaute n'existe pas.(deleteCommunity)\n");
		return;
	}

	//If unique community
	if(!community->previous && !community->next)
		g->community = NULL;
	else{
		//If first community of the list
		if(!community->previous){
			g->community = community->next;
			community->next->previous = NULL;
		}
		else{
			//If last community of the list		
			if(!community->next)
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
	if(!g || !member){
		printf("Le graphe ou le membre n'existe pas (deleteMemberFromCommunity).\n");
		return;
	}
	double weightIn;

	//If unique member, then we delete community
	if(!member->previous && !member->next){
		member->myCom->member = NULL;
		deleteCommunity(g, member->myCom);
	}
	//If the member is not unique, then we just take it off
	else{
		//If first member from the list
		if(!member->previous){
			member->next->previous = NULL;
			member->myCom->member = member->next;
		}
		else{
			//If last member from the list
			if(!member->next)
				member->previous->next = NULL;
			//If middle member from the list
			else{
			member->previous->next = member->next;
			member->next->previous = member->previous;
			}
		}
		//Taking the weights off the community due to the calling off
		weightIn = weightToCommunity(member, member->myCom->label);
		member->myCom->weightTot -= (member->weightMember - weightIn);        
		member->myCom->weightInt -= weightIn;
	}
}

static void changeMemberToCommunity(Graph *g, Member* member, Community* community){
	if(!g || !member || !community){
		printf("Preconditions non respectees pour changeMemberToCommunity, communaute inchangee\n");
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
	community->weightTot += (member->weightMember - weightIn);

	return;
}

static double weightToCommunity(Member* member, size_t label){
	if(!member){
		printf("Le membre n'existe pas (weightToCommunity)\n");
		return 0;
	}
   Edge* pNeighbours;
   double weightToCommunity;
	
	//Browsing all neightbours of the member
	pNeighbours	= member->neighbours;
	weightToCommunity = 0.0;
   while(pNeighbours != NULL){
		if(pNeighbours->dest->myCom->label == label)
			weightToCommunity += pNeighbours->weight;
		pNeighbours = pNeighbours->next;
   }

   return weightToCommunity;
}

static double gainModularity(Graph* g, Member* member, Community* com){
	if(!g || !com || !member){
		printf("Preconditions non respectees pour gainModularity\n");
		return 0;
	}
   double weightTot;
   double comWeightInt, comWeightExt;
   double memberWeight, memberWeightCom;
	//sum of the weight of the links inside the community $com$
	comWeightInt = com->weightInt;
	
	//sum of the weight of the links incident to members of the community $com$
	comWeightExt = com->weightTot;

	//sum of the weight of all links originated from $member$
	memberWeight = member->weightMember;

	//sum of the weight of the links from $member$ to members of the community $com$
	memberWeightCom = weightToCommunity(member, com->label);

   //sum of the weight of all links of the graph
   weightTot = g->weightTot;

	//Calculating the gain of modularity
   double t1 = (comWeightInt + memberWeightCom)/(weightTot);
   double t2 = (comWeightExt + memberWeight)/(weightTot);
   double t3 = t1 - (t2 * t2);

   double s1 = (comWeightExt/(weightTot))*(comWeightExt/(weightTot));
   double s2 = (memberWeight/(weightTot))*(memberWeight/(weightTot));
   double s3 = (comWeightInt/(weightTot))-s1-s2;

   return (t3 - s3);
}

static bool modularityOptimisation(Graph* g){
	if(!g)
		return false;
	Community *pCom, *pComNext,*pNewCom;
	Member *pMember, *pMemberNext;
	Edge *pNeighbours;
	bool has_imp, has_changed;
	double maxQ, q;

	has_changed = false;
	//Browsing each community
	pCom = g->community;
	for(size_t i=1; i <= g->nbCommunity; i++){
		if(!pCom)
			pCom = g->community;
		maxQ = 0.0;
		has_imp = false;

		//Browsing each member of the community
		pMember = pCom->member;
		while (pMember != NULL){

			//Browsing each neightours of the member
			pNeighbours = pMember->neighbours;
			while(pNeighbours != NULL){

				//Calculating the gain modularity if the member $pMember$ was in its neighbours' community
				q = gainModularity(g, pMember, pNeighbours->dest->myCom);
				if (maxQ < q) {
					maxQ = q;
					pNewCom = pNeighbours->dest->myCom;
					has_imp = true;
				}
				pNeighbours = pNeighbours->next;
			}

			//(pMember might be in other community or pCom might be deleted)
			//Keeping the order of the browsing
			pMemberNext = pMember->next;
			pComNext = pCom->next;
			if(has_imp && pMember->myCom->label != pNewCom->label){
				has_changed = true;
				changeMemberToCommunity(g, pMember, pNewCom);
				i = 0;//If a member has changed of community, then we have to recheck all communities
			}
			pMember = pMemberNext;
		}
		pCom = pComNext;
	}
	return has_changed;
}

static int readFile(char *filename, Graph *g){
	if(!g){
		printf("Le graphe doit etre alloue avant de lire le fichier\n");
		return -1;
	}
	FILE *fp;
	char line[MAX+1];
	int i,j,nbS1,nbLine,member,nbEdge,createEdge;
	double weight;

	//Open the file in read mode
	fp=fopen(filename,"r");
	if(!fp){
		printf("Le fichier %s n'existe pas.\n",filename);
		return -2;
	}

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
						printf("Erreur a la ligne %d !\n",nbLine);
						deleteGraph(g);
						return -1;
					}
					if (line[i]=='x')
						createEdge=0;

					weight=10*weight+line[i]-'0';
					i++;
					if(line[i] == '.'){
						i++;
						int dec = 0;
						while(line[i] != ',' && line[i] != '\n' && line[i] != 13){
							weight += ((line[i]-'0')/ power(10,dec) );
							dec++;
							i++;
						}
					}
					while (line[i]==' ' || line[i]=='\t' || line[i] == 13)
						i++;
				}
				if (line[i] == ',')
					i++;
				nbEdge++;
				if (nbEdge<=nbS1 && createEdge==1)
					addEdge(g,member,nbEdge,weight);
			}
			if (nbEdge != nbS1){
				printf("Erreur a la ligne %d !\n",nbLine);
				deleteGraph(g);
				return -1;
			}
		}
    }
	fclose(fp);
	return 0;
}

int main(int argc, char *argv[]){
	bool showEdges, hasChanged;
	int error;
	unsigned int nbPass;
	char deleteFilename[MAX_NAME], filename[MAX_NAME];

	//Verifying the number of arguments given in the command line
	if(argc != 3){
		printf("Le nombre d'arguments de la commande est incorrect.\n");
		return 0;
	}

	//Checking the thrid input to determine how to show the graph
	if(strcmp(argv[2],"true") || strcmp(argv[2],"True"))
		showEdges = true;
	else
		if(strcmp(argv[2],"False") || strcmp(argv[2],"False"))
			showEdges = false;
		else{
			printf("%s n'est pas un booleen.\n",argv[2]);
			return 0;
		}

	//Allocatating and initialisation of a graph
   Graph *g = initGraph();
   if(!g){
		printf("Erreur allocation de mémoire pour le graphe.\n");
   	return -1;
	}

	//Creating the graph described in the file
   error = readFile(argv[1],g);
	if(error != 0){
		printf("Erreur lors de la lecture du fichier.\n");
		deleteGraph(g);
		return -1;
	}

	//Deleting all previously created files in the folder 'Pass'
	nbPass = 1;
	remove("./Pass/finalGraph.txt");
	sprintf(deleteFilename, "./Pass/pass%u.txt", nbPass);
	while(remove(deleteFilename) != -1){
		nbPass++;
		sprintf(deleteFilename, "./Pass/pass%u.txt", nbPass);
	}

	//Creating the folder 'Pass'
   mkdir("Pass", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	//Applying the method described in the article
	nbPass=0;
	hasChanged = true;
	while(hasChanged){
		//Applying modularity optimisation
		hasChanged = modularityOptimisation(g);

		//Verifying if any modularity optimisation occured
		if(hasChanged){
			nbPass++;
			printf("Pass %d \n \n",nbPass);
			printf("<Optimisation de la modularité> effectue\n \n");

			//Applying community aggregation
		   sprintf(filename, "./Pass/pass%u.txt", nbPass);
			g = communityAggregation(g,filename);
			printf("<Regroupement des communautes> effectue\n \n");

			//Showing the graph
			displayGraph(g,showEdges);
		}

	}
	makeTrace(g,"./Pass/finalGraph.txt");
	printf("\n Nombre de <pass> effectues: %u\n",nbPass);
	deleteGraph(g);
   return 0;
}
