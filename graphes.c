#include <stdio.h>
#include <stdlib.h>

#include "aggregation.h"

typedef struct{
    double weight; 
    size_t destCommunityLabel;
}LinksToCommunity;

/* bubbleSort
* Sort the array $array$ which size is $size$
*
* [input]
* array : an initialised array
* size : the size of the array $array$
*
*/
static void bubbleSort(size_t *array, size_t size);

/* deleteMatrix
* Delete a 2-dimensional matrix of size ($size$ x $>=size$)
*
* [input]
* matrix: an initialised matrix
* size: the size of the mtatrix
*
*/
static void deleteMatrix(double** matrix, size_t size);

/* makeAdjacencyMatrix
* Create the adjacency matrix of the graph g
*
* [input]
* g : an initialised graph
*
* [return]
* NULL : Error at allocating memory for the matrix
* adjMat : the adjacency matrix of the graph g
*
*/
static double** makeAdjacencyMatrix(Graph* g);

/* printGraphWithCommunities
* Write the graph $g$ in the file $filename$
*
* [input]
* g : an initialised graph
* AdjacencyMatrix : the adjacency matrix of the graph $g$
* filename : the path to a non-existent file
*
* [return]
* 0 : The graph has been successfully saved in a file
* -1 : Error at applying the operation
*
*/
static int printGraphWithCommunities(Graph* g, double** AdjacencyMatrix, char* filename);

/* makeTrace
* Saving the graph $g$ in the file $filename$
*
* [input]
* g : an initialised graph
* filename : the path to a non-existent file
*
* [return]
* 0 : The graph has been successfully saved in a file
* -1 : Error at applying the operation
*
*/
static int makeTrace(Graph* g, char* filename);

/* findIndex
* Saving the graph $g$ in the file $filename$
*
* [input]
* g : an initialised graph
* filename : the path to a non-existent file
*
* [return]
* 0 : The graph has been successfully saved in a file
* -1 : Error at applying the operation
*
*/
static size_t findIndex(Graph* g, size_t communityLabel, LinksToCommunity* communityConnections);

/* condenseLinksBetweenCommunities 
* Sum the weight of all links from the members of $community$ to
* the members of all other communities
*
* [input]
* g : an initialised graph
* community : a community of the graph g
* communityConnections : initialised structure of type $LinksToCommunity$
*
*/
static void condenseLinksBetweenCommunities(Graph* g, Community* community, LinksToCommunity* communityConnections);

/* makeTrace
* Creating a condensed version of the graph $oldGraph$ to the graph $newGraph$
*
* [input]
* oldG : an initialised graph
* oldN : an initialised graph
*
* [return]
* 0 : A condensed version of $oldGraph$ has been successfully created
* -1 : Error at applying the operation
*
*/
static int makeNewGraph(Graph* oldGraph, Graph* newGraph);

static void bubbleSort(size_t *array, size_t size){
    size_t a, b, tmp; 
    for (a=0; a<size-1 ; a++){
        for (b=0; b<size-a-1 ; b++){
            if (array[b] > array[b+1]){
                tmp = array[b];
                array[b]=array[b+1];
                array[b+1]=tmp;
            }
        }
    }
}

static void deleteMatrix(double** matrix, size_t size){
    if(!matrix)
        return;
    for(size_t i = 0; i<size; i++)
        free(matrix[i]);
    free(matrix);
    return;
}

static double** makeAdjacencyMatrix(Graph* g){
    if(!g)
        return NULL;
    size_t i, row, column;
    Community* community;
    Member* member;
    //Allocating of the adjacent matrix of graph g of size $g->nbMembers$x$g->nbMembers+1$
    double** adjMat = malloc(g->nbMembers * sizeof(double*) );
    if(!adjMat){
        printf("Erreur lors de l'allocation memoire pour la fonction makeAdjacencyMatrix\n");
        return NULL;
    }
    for(i = 0; i < g->nbMembers; i++){
        adjMat[i] = calloc(g->nbMembers + 1, sizeof(double));//adding an extra column for the member's community
        if(!adjMat[i]){
            printf("Erreur lors de l'allocation memoire pour la fonction makeAdjacencyMatrix\n");
            return NULL;
        }
    }

    //Allocating a vector to keep in order in which the adcency matrix should be completed (more details on the report)
    size_t* index = malloc(g->nbMembers *  sizeof(size_t));
    if(!index){
        printf("Erreur lors de l'allocation memoire pour la fonction makeAdjacencyMatrix\n");
        deleteMatrix(adjMat, g->nbMembers);
        return NULL;
    }   

    //Adding all community labels in a vector $index$
    i=0;
    community =  g->community;
    while(community != NULL){
        member = community->member;
        while(member!=NULL){
            index[i] = member->label;
            member=member->next;
            i++;
        }
        community = community->next;
    }
    //Sorting the vector $index$
    bubbleSort(index,g->nbMembers);

    //Browsing all communities
    community =  g->community;    
    while(community !=NULL){

        //Browing all members of the community
        member = community->member;
        while(member != NULL){

            //Browsing all neighbours of the member
            Edge *neighbours = member->neighbours;

            //Searching for the row of the adjacency matrix which corresponds to the community
            row = 0;
            while(member->label != index[row])
                row++;
            while(neighbours != NULL){

                //Searching for the column of the adjacency matrix which corresponds to the community
                column = 0;
                while(neighbours->dest->label != index[column])
                    column++;

                //Adding the weight to the matrix
                adjMat[row][column] = neighbours->weight;
                neighbours = neighbours->next;
            }
            //Add the label of the member's community in the matrix 
            adjMat[row][g->nbMembers] = (double)member->myCom->label;
            member = member->next;
        }
        community = community->next;
    }
    free(index);
    return adjMat;
}

static int printGraphWithCommunities(Graph* g, double** AdjacencyMatrix, char* filename){
    if(!g || !AdjacencyMatrix){
        printf("Le graphe ou la matrix d'adjacence n'existe pas. (printGraphWithCommunities)\n");
        return -1;
    }
    
    FILE* fp = fopen(filename, "w");
    if(!fp){
        printf("Le fichier %s n'existe pas. (printGraphWithCommunities)\n",filename);
        return -1;
    }
    fprintf(fp,"# nombre de communautes = %lu\n",g->nbCommunity);
    fprintf(fp,"# nombre de membres = %lu\n",g->nbMembers);
    fprintf(fp,"# nombre d'arcs = %lu\n",g->nbEdge);
    fprintf(fp,"# poids de tous les arcs = %lf\n",g->weightTot);
    fprintf(fp,"# Matrice d'adjacence | le membre de la communaute\n");

    //Writing the Adjacency matrix in the file
    for(size_t l=0; l<g->nbMembers; l++){
        for(size_t c=0; c<g->nbMembers; c++)
            fprintf(fp,"%lf ",AdjacencyMatrix[l][c]);
        fprintf(fp,"| %lu",(size_t)AdjacencyMatrix[l][g->nbMembers]);//last column is the community of the member
        fprintf(fp,"\n");//next line of file
    }

    fclose(fp);
    return 0;
}

static int makeTrace(Graph* g, char* filename){
    if(!g || !filename){
        printf("Le graphe ou le fichier n'existe pas. (makeTrace) \n");
        return -1;
    }

    //Creating the adjacency matrix of the graph g
    double** adjMat = makeAdjacencyMatrix(g);   
    if(!adjMat)
        return -1;

    //Saving the graph g in a file
    int errorCode = printGraphWithCommunities(g, adjMat, filename);
    if(errorCode)
        return -1;

    deleteMatrix(adjMat,g->nbMembers);
    return 0;
}

static size_t findIndex(Graph* g, size_t communityLabel, LinksToCommunity* communityConnections){
    //Finding the index of $communityConnections$ which corresponds to the $communityLabel$
    size_t i = 0;
    while(communityConnections[i].destCommunityLabel != communityLabel && i < g->nbCommunity)
        i++;
    if(i == g->nbCommunity)
        printf("L'index pas trouve. (findIndex)\n");
    return i;
}

static void condenseLinksBetweenCommunities(Graph* g, Community* community, LinksToCommunity* communityConnections){
    if(!g || !community || !communityConnections){
        printf("Le graphe ou la communauté n'existe pas. (condenseLinksBetweenCommunities)\n");
        return;
    }

    //Browsing all members of the community
    Member* member = community->member;
    while(member != NULL){
        //Browsing all neightbours of the member
        Edge* neighbours = member->neighbours;
        while(neighbours != NULL){
            //Adding the weights of all links toward other communities
            communityConnections[findIndex(g, neighbours->dest->myCom->label, communityConnections)].weight += neighbours->weight;
            neighbours = neighbours->next;
        }
        member = member->next;
    }
    return;
}

static int makeNewGraph(Graph* oldGraph, Graph* newGraph){
    if(!oldGraph || !newGraph){
        printf("L'un des graphes n'existe pas. (makeNewGraph)\n");
        return -1;
    }

    int errorCode = 0;
    size_t i;
    Community *oldGraphCom, *newGraphCom;
    LinksToCommunity *communityConnections;

    //Create the numbers of communities needed for the new graph $newGraph$
    for(size_t i = 0; i<oldGraph->nbCommunity; i++){
        errorCode = addCommunity(newGraph);
        if(errorCode){
            printf("Erreur lors de l'ajout d'une communaute. (makeNewGraph)\n");
            deleteGraph(newGraph);
            return -1;
        }
    }

    //Match the label of the new communities and new members with the label of those from the previous graph
    oldGraphCom = oldGraph->community;
    newGraphCom = newGraph->community;
    while(oldGraphCom !=NULL){
        newGraphCom->label = oldGraphCom->label;
        newGraphCom->member->label = oldGraphCom->label;
        oldGraphCom = oldGraphCom->next;
        newGraphCom = newGraphCom->next;
    }

    //Allocate an array of type $communityConnections$
    oldGraphCom = oldGraph->community;
    newGraphCom = newGraph->community;
    communityConnections = malloc(oldGraph->nbCommunity * sizeof(LinksToCommunity));
    if(!communityConnections){
        printf("Erreur d'allocation de memoire. (makeNewGraph)\n");
        return -1;
    }

    //Saving all community labels of the new graph in an array of type $communityConnections$
    size_t j = newGraph->nbCommunity-1;
    while(newGraphCom != NULL){
        communityConnections[j].destCommunityLabel = newGraphCom->label; 
        newGraphCom = newGraphCom->next;
        j--;
    }

    newGraphCom = newGraph->community;
    while(oldGraphCom != NULL){
        //Resetting all weight connections
        for(i = 0; i < newGraph->nbCommunity ; i++)
            communityConnections[i].weight = 0;

        //Initialise the array which contains the sum of the weight of all links between the community $oldGraphCom$ and the others
        condenseLinksBetweenCommunities(oldGraph, oldGraphCom, communityConnections);

        //Create edges between the community $newGraphCom$ and all the others
        for(i = newGraph->nbCommunity-1; (newGraph->nbCommunity+i) >= newGraph->nbCommunity; i--)
            addEdge(newGraph,newGraphCom->member->label, communityConnections[i].destCommunityLabel, communityConnections[i].weight);

        oldGraphCom = oldGraphCom->next;
        newGraphCom = newGraphCom->next;       
    }
    free(communityConnections);
    deleteGraph(oldGraph);
    return 0;
}

Graph* communityAggregation(Graph* oldGraph, char* filename){
    if(!oldGraph)
        return NULL;

    int errorCode = 0;
    errorCode = makeTrace(oldGraph,filename);
    if(errorCode){
        deleteGraph(oldGraph);
        return NULL;
    }

    Graph* newGraph= initGraph();
    if(!newGraph){
        printf("Erreur lors de l'allocation de memoire d'un nouveau graphe.(communityAggregation)\n");
        deleteGraph(oldGraph);
        return NULL;
    }

    errorCode = makeNewGraph(oldGraph, newGraph);
    if(errorCode){
        deleteGraph(oldGraph);
        deleteGraph(newGraph);
        return NULL;
    }
    return newGraph;
}
