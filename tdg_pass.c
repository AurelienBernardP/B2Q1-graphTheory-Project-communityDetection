#include <stdio.h>
#include <stdlib.h>
#include "graphes.h"

typedef struct{
    double weight; 
    size_t destCommunityLabel;
}BetweenCommunityEdge;

static double** makeAdjacenceMatrix(Graph* g){
    if(!g){return NULL;}
    
    //start allocation of adjacent matrix of graph g
    double** adjMatWithCommunity = malloc(g->nbNodes * sizeof( double*) );
    if(!adjMatWithCommunity){
        return NULL;
    }
    for(size_t i = 0; i < g->nbNodes; i++){
        adjMatWithCommunity[i] = calloc(g->nbNodes + 1, sizeof(double));//+1 because we are adding an extra column for the node's community.
        if(!adjMatWithCommunity[i]){
            return NULL;
        }
    }
    //end allocation of adjacent matrix of graph g
    
    //back list pointer to first element of community list
    Community* currentCommunity = g->community;
    while(currentCommunity->previous != NULL){
        currentCommunity = currentCommunity->previous;
    }
    //list pointer is on first element of node list
    
    //go through all communities
    while(currentCommunity !=NULL){
        Node* currentNode = currentCommunity->member;
        //back up node pointer to first element of memeber list
        while(currentNode->previous != NULL){
            currentNode = currentNode->previous;
        }
        //node pointer is on the first element of the members of the community list
         
        //go through lost of nodes in the coomunity updating the adjacent matrix
        while(currentNode != NULL){
            //go through all neighbours of the node
            while(currentNode->neighbours != NULL){
                adjMatWithCommunity[(currentNode->label-1)][currentNode->neighbours->dest->label-1] = currentNode->neighbours->weight;
                currentNode->neighbours = currentNode->neighbours->next;
            }
            //add the number of the node's community in the matrix 
            adjMatWithCommunity[(currentNode->label-1)][g->nbNodes] = (double)currentNode->myCom->label;
            //matrix contains all the of the neighbours of the current node
            currentNode = currentNode->next;
        }
        currentCommunity = currentCommunity->next;
    }
    //matrix containss all the information needed from all the nodes

    return adjMatWithCommunity;
}

static int printGraphWithCommunities(Graph* g, double** AdjacenceMatrix){// we are maybe going to need the number of the pass to give different file names to the matrix printouts.
    if(!g || !AdjacenceMatrix){return -1;}
    
    FILE* fp = fopen("filename.txt", "w");//filename to be changed
    if(!fp){
        return -1;
    }
    fprintf(fp,"# number of communities = %lu\n",g->nbCommunity);
    fprintf(fp,"# number of nodes = %lu\n",g->nbNodes);
    fprintf(fp,"# number of edges = %lu\n",g->nbEdge);
    fprintf(fp,"# adjacence Matrix | community of node\n");
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
        return -1;
    }

    return 0;
}

static size_t findIndexFromLabel(Graph* g, size_t communityLabel, BetweenCommunityEdge* communityConections){
    size_t start = 0;
    size_t end = g->nbCommunity-1;
    size_t middle = g->nbCommunity/2;
   
    while(start <= end){
        if(communityConections[middle].destCommunityLabel == communityLabel){
            printf("index found\n");
            return middle;
        }
        if(communityConections[middle].destCommunityLabel < communityLabel){
            end = middle - 1;
        }
        if(communityConections[middle].destCommunityLabel > communityLabel){
            start = middle + 1;       
        }    
        middle = (end + start) / 2;
    }

    printf("*******index NOT found********\n");
    return 0;
}






static void condenseLinksBetweenCommunities(Graph* g, Community* community, BetweenCommunityEdge* communityConections ){
    if(!g || !community || !communityConections){
        return;
    }
    //init node pointer to first memeber of community
    Node* currentNode = community->member;
    //loop throught all memebers of community
    while(currentNode != NULL){
        //init edge pointer to first edge in the neightbours of current node
        Edge* currentNodeEdge = currentNode->neighbours;
        //loop through all edges of the current node and add the values to the table
        while(currentNodeEdge != NULL){
            communityConections[findIndexFromLabel(g, currentNodeEdge->dest->myCom->label, communityConections)].weight += currentNodeEdge->weight;
            currentNodeEdge = currentNodeEdge->next;
        }
        //all weights of the edges of current node have been summed to the array of connections
        currentNode = currentNode->next;
    }
    //all the weight of all the node's edges in the community has been added to the array
    return;
}





static int makeNewGraph(Graph* oldG, Graph* newG){
    if(!oldG || !newG) { return -1;}

    //init new graph structure
    newG->nbNodes = oldG->nbCommunity;
    newG->weightTot = oldG->weightTot;
    int errorCode = 0;

    //add the numbers of communities needed
    for(size_t i = 0; i<newG->nbNodes; i++){
        errorCode = addCommunity(newG);
        if(errorCode){
            deleteGraph(newG);
            return -1;
        }
    }
    //modify the label of the new communities and new nodes to keep the labels of the previous step graph
    Community* oldGCurrentCom = oldG->community;
    Community* newGCurrentCom = newG->community;
    //first put pointers to first element of list
    while(oldGCurrentCom->previous != NULL){
        oldGCurrentCom = oldGCurrentCom->previous;
    }
    while(newGCurrentCom->previous != NULL){
        newGCurrentCom = newGCurrentCom->previous;
    }
    //enumerate all communities and nodes and modify their labels
    while(oldGCurrentCom !=NULL && newGCurrentCom !=NULL){
        newGCurrentCom->member->label = newGCurrentCom->label = oldGCurrentCom->label;
        /*
            newGCurrentCom->member->weightNode;
            can't remember what the above line did but i think its not usefull ^-^" we will see when tested
        */    
        oldGCurrentCom = oldGCurrentCom->next;
        newGCurrentCom = newGCurrentCom->next;
    }

    oldGCurrentCom = oldG->community;
    newGCurrentCom = newG->community;
    while(oldGCurrentCom->previous !=NULL){
        oldGCurrentCom = oldGCurrentCom->previous;
    }
    while(newGCurrentCom->previous !=NULL){
        newGCurrentCom = newGCurrentCom->previous;
    }
 /*   //establish the weight inside de comunity with a loop 
    while(oldGCurrentCom != NULL && newGCurrentCom != NULL){
        Edge* newLoop = (Edge*) malloc(sizeof(Edge));
        if(!newLoop){
            return -1;
        }
        newLoop->weight = oldGCurrentCom->weightInt;
        newLoop->dest = newGCurrentCom->member;
        newLoop->next = NULL;
        newGCurrentCom->member->neighbours = newLoop;
        newGCurrentCom->member->weightNode = oldGCurrentCom->weightInt;

        oldGCurrentCom = oldGCurrentCom->next;
        newGCurrentCom = newGCurrentCom->next;

    }
 */   //calculate the weight of al edges from one comunity to an other

//////////all the commented code above is done in the part of the function below 

    oldGCurrentCom = oldG->community;
    newGCurrentCom = newG->community;
    while(oldGCurrentCom->previous != NULL){
        oldGCurrentCom = oldGCurrentCom->previous;
    }
    while(newGCurrentCom->previous != NULL){
        newGCurrentCom = newGCurrentCom->previous;
    }
    //initilize an array with all the communitie labels to fill in with the weight of the links of a community with the others 
    BetweenCommunityEdge* communityConections = malloc(oldG->nbCommunity * sizeof(BetweenCommunityEdge));////////////////////////////////////////////////////////////alloc
    if(!communityConections){
            return -1;
    }
    size_t j = newG->nbCommunity -1;
    while(newGCurrentCom != NULL ){
        communityConections[j].destCommunityLabel = newGCurrentCom->label;// placed in ascending label order 
        newGCurrentCom = newGCurrentCom->next;
        j--;
    }

    newGCurrentCom = newG->community;
    while(oldGCurrentCom != NULL && newGCurrentCom != NULL){
        for(size_t i = 0; i < newG->nbCommunity ; i++){//set all weights to 0
            communityConections[i].weight = 0;
        }
        //make a table of the weight of the connections between current community and the others
        condenseLinksBetweenCommunities(oldG, oldGCurrentCom, communityConections);
        //make edges with the values collected earlier
        for(size_t i = newG->nbCommunity-1; (newG->nbCommunity+i) >= newG->nbCommunity; i-- ){
            addEdge(newG,newGCurrentCom->member->label, communityConections[i].destCommunityLabel, communityConections[i].weight);
        }

        oldGCurrentCom = oldGCurrentCom->next;
        newGCurrentCom = newGCurrentCom->next;       
    }
    free(communityConections);

    //free old graph
    ////
    ////
    //return new one


    return 0;
}

int pass(Graph* oldG){
    if(!oldG) { return -1;}

    int errorCode = 0;

    errorCode = makeTrace(oldG);
    if(errorCode){
        //delete old graph??
        return -1;
    }
    Graph* newG= malloc(sizeof(Graph));
    if(!newG){
        //delete old graph???
        return -1;
    }
    errorCode = makeNewGraph(oldG, newG);
    if(errorCode){
        //delete old graph??
        return -1;
    }


    return 0;
}