#include <stdio.h>
#include <stdlib.h>
#include"graphes.h"
static void freeAdjacenceMatrix(Graph* g, double** adjacenceMatrix){
    if(!g || !AdjacenceMatrix){return;}
        
    for(size_t i = 0; i<g->nbNodes; i++){
        free(adjacenceMatrix[i]);
    }
    free(adjacenceMatrix);
        
    return;    
}
static double** makeAdjacenceMatrix(Graph* g){
    if(!g){return NULL;}
    
    //start allocation of adjacent matrix of graph g
    double** adjMatWithCommunity = malloc(g->nbNodes * sizeof( double*) );
    if(!adjMatWithCommunity){
        return NULL;
    }
    for(size_t i = 0; i < g->nbNodes; i++){
        adjMatWithCommunity[i] = calloc(g->nbNodes + 1, double);//+1 because we are adding an extra column for the node's community.
        if(!adjMatWithCommunity[i]){
            free(adjMatWithCommunity);
            return NULL;
        }
    }
    //end allocation of adjacent matrix of graph g
    
    //back list pointer to first element of node list
    node* currentNode = g->community->member;
    while(currentNode->previous != NULL){
        currentNode = currentNode->previous;
    }
    //list pointer is on first element of node list

    //go through lost of nodes updating the adjacent matrix
    while(currentNode != NULL){
        //go through all neighbours of the node
        while(currentNode->neighbours != NULL){
            adjMatWithCommunity[(currentNode->label-1)][currentNode->neighbours->dest->label-1] = currentNode->neighbours->weight;
            currentNode->neighbours = currentNode->neighbours->next;
        }
        //add the number of the node's community in the matrix 
        adjMatWithCommunity[(currentNode->label-1)][g->nbNodes] = (double)currentNode->community->label;
        //matrix contains all the of the neighbours of the current node
        currentNode = currentNode->next
    }
    //matrix containss all the information needed from all the nodes

    return adjMatWithCommunity;
}

static int printGraphWithCommunities(Graph* g, double** AdjacenceMatrix){// we are maybe going to need the number of the pass to give different file names to the matrix printouts.
    if(!g || !AdjacenceMatrix){return -1;}
    
    FILE* fp = fopen("filename.txt", 'w');//filename to be changed
    if(!fp){
        return -1;
    }
    fprintf(fp,"# number of communities = %lu\n",g->nbCommunity);
    fprintf(fp,"# number of nodes = %lu\n",g->nbNodes);
    fprintf(fp,"# number of edges = %lu\n",g->nbEdge);
    fprintf(fp,"# adjacence Matrix | community of node\n",g->nbEdge);
    for(size_t l=0; l<g->nbNodes; l++){//print adjacence matrix
        for(size_t c=0; l<g->nbNodes; c++){
            fprintf(fp,"%lf ",AdjacenceMatrix[l][c] );
        }
        fprintf(fp,"| %lu",(unsigned long int)AdjacenceMatrix[l][g->nbNodes] );//last column is the community of the node
        fprintf(fp,"\n");//nex line of file
    }
    fclose(fp);
    return 0;
}

static int makeTrace(Graph* g){
    if(!g){return -1;}

    double** adjMatWithCommunity = makeAdjacenceMatrix(g);   
    if(!adjMatWithCommunity){
        return -1;
    }

    int errorCode = printGraphWithCommunities(g, adjMatWithCommunity);
    if(errorCode){
        freeAdjacenceMatrix(g, adjMatWithCommunity);
        return -1;
    }
    freeAdjacenceMatrix(g, adjMatWithCommunity);

    return 0;
}


static int makeNewGraph(Graph* oldG, Graph* newG){
    if(!oldG || !newG) { return -1;}


    return 0;
}



int pass(Graph* oldG, Graph* newG){
    if(!oldG || !newG) { return -1;}
    int errorCode = 0;

    errorCode = makeTrace(oldG);
    if(errorCode){
        return -1;
    }

    errorCode = makeNewGraph(oldG, newG);
    if(errorCode){
        return -1;
    }


    return 0;
}