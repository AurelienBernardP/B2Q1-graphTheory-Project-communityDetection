#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pass.h"

typedef struct{
    double weight; 
    size_t destCommunityLabel;
}LinksToCommunity;

void bubbleSort(size_t *array, size_t size){
    size_t a, b, tmp; 
    for (a = 0 ; a < size-1; a++){
        for (b = 0 ; b < size-a-1; b++){
            if (array[b] > array[b+1]){
                tmp = array[b];
                array[b]=array[b+1];
                array[b+1]=tmp;
            }
        }
    }
}

static double** makeAdjacenceMatrix(Graph* g){
    if(!g)
        return NULL;

    //Allocation of the adjacent matrix of graph g
    double** adjMat = malloc(g->nbMembers * sizeof( double*) );
    if(!adjMat)
        return NULL;

    size_t i;
    for(i = 0; i < g->nbMembers; i++){
        adjMat[i] = calloc(g->nbMembers + 1, sizeof(double));//adding an extra column for the member's community
        if(!adjMat[i])
            return NULL;
    }
    size_t* index = malloc(g->nbMembers *  sizeof(size_t));
    if(!index)
        //free mat
        return NULL;    
    Community* community =  g->community;
    Member* member;

    i=0;
    while(community != NULL){
        member = community->member;
        while(member!=NULL){
            index[i] = member->label;
            member=member->next;
            i++;
        }
        community = community->next;
    }
    bubbleSort(index,g->nbMembers);
    for(i=0;i<g->nbMembers-1;i++)
        if(index[i]>index[i+1])
            printf("nopeeeeeeee index pas trie\n");
    //Browsing all communities
    community =  g->community;    
    while(community !=NULL){
        //Browing all members of the community
        member = community->member;
        while(member != NULL){
            //Browsing all neighbours of the member
            Edge *neighbours = member->neighbours;
            
            size_t row = 0;
            while(member->label != index[row])
                row++;
            while(neighbours != NULL){
                //Adding the weight to the matrix
                size_t column = 0;
                while(neighbours->dest->label != index[column])
                    column++;
                adjMat[row][column] = neighbours->weight;
                neighbours = neighbours->next;
            }
            //Add the label of the member's community in the matrix 
            adjMat[row][g->nbMembers] = (double)member->myCom->label;
            member = member->next;

        }
        community = community->next;
    }
    return adjMat;
}

static int printGraphWithCommunities(Graph* g, double** AdjacenceMatrix, char* filename){
    if(!g || !AdjacenceMatrix)
        return -1;
    
    FILE* fp = fopen(filename, "w");
    if(!fp)
        return -1;
    fprintf(fp,"# number of communities = %lu\n",g->nbCommunity);
    fprintf(fp,"# number of members = %lu\n",g->nbMembers);
    fprintf(fp,"# number of edges = %lu\n",g->nbEdge);
    fprintf(fp,"# weights of all edges = %lf\n",g->weightTot);
    fprintf(fp,"# adjacence Matrix | community of member\n");

    //Writing the adjacence matrix in the file
    for(size_t l=0; l<g->nbMembers; l++){
        for(size_t c=0; c<g->nbMembers; c++)
            fprintf(fp,"%lf ",AdjacenceMatrix[l][c]);
        fprintf(fp,"| %lu",(unsigned long int)AdjacenceMatrix[l][g->nbMembers] );//last column is the community of the member
        fprintf(fp,"\n");//next line of file
    }

    fclose(fp);
    return 0;
}

static int makeTrace(Graph* g, char* filename){
    if(!g)
        return -1;

    double** adjMat = makeAdjacenceMatrix(g);   
    if(!adjMat)
        return -1;

    int errorCode = printGraphWithCommunities(g, adjMat, filename);
    if(errorCode)
        return -1;

    return 0;
}

static size_t findIndex(Graph* g, size_t communityLabel, LinksToCommunity* communityConnections){
    size_t start = 0;
    size_t end = g->nbCommunity-1;
    size_t middle = g->nbCommunity/2;

    //Finding the index of $communityConnections$ which corresponds to the $communityLabel$
    while(start <= end){
        if(communityConnections[middle].destCommunityLabel == communityLabel)
            return middle;
        if(communityConnections[middle].destCommunityLabel < communityLabel)
            end = middle - 1;        
        if(communityConnections[middle].destCommunityLabel > communityLabel)
            start = middle + 1;       

        middle = (end + start) / 2;
    }

    printf("Index pas trouvé\n");
    return 0;
}

static void condenseLinksBetweenCommunities(Graph* g, Community* community, LinksToCommunity* communityConnections){
    if(!g || !community || !communityConnections)
        return;

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

static int makenewGraph(Graph* oldGraph, Graph* newGraph){
    if(!oldGraph || !newGraph)
        return -1;

    int errorCode = 0;

    //Add the numbers of communities needed
    for(size_t i = 0; i<oldGraph->nbCommunity; i++){
        errorCode = addCommunity(newGraph);
        if(errorCode){
            deleteGraph(newGraph);
            return -2;
        }
    }

    //Match the label of the new communities and new members with the labels of the previous graph
    Community* oldGraphCom = oldGraph->community;
    Community* newGraphCom = newGraph->community;
    while(oldGraphCom !=NULL){
        newGraphCom->label = oldGraphCom->label;
        newGraphCom->member->label = oldGraphCom->label;
        oldGraphCom = oldGraphCom->next;
        newGraphCom = newGraphCom->next;
    }

    //Initialise an array which contains the weight between communities
    oldGraphCom = oldGraph->community;
    newGraphCom = newGraph->community;
    LinksToCommunity* communityConnections = malloc(oldGraph->nbCommunity * sizeof(LinksToCommunity));
    if(!communityConnections)
        return -2;

    //Saving all community labels of the new graph
    size_t j = newGraph->nbCommunity-1;
    while(newGraphCom != NULL){
        communityConnections[j].destCommunityLabel = newGraphCom->label; 
        newGraphCom = newGraphCom->next;
        j--;
    }

    newGraphCom = newGraph->community;
    while(oldGraphCom != NULL){
        //Resetting all weight connections
        for(size_t i = 0; i < newGraph->nbCommunity ; i++)
            communityConnections[i].weight = 0;

        //Make an array of the weight of the connections between community and the others
        condenseLinksBetweenCommunities(oldGraph, oldGraphCom, communityConnections);

        //Make edges with the values collected earlier
        for(size_t i = newGraph->nbCommunity-1; (newGraph->nbCommunity+i) >= newGraph->nbCommunity; i--)
            addEdge(newGraph,newGraphCom->member->label, communityConnections[i].destCommunityLabel, communityConnections[i].weight);

        oldGraphCom = oldGraphCom->next;
        newGraphCom = newGraphCom->next;       
    }
    free(communityConnections);
    deleteGraph(oldGraph);
    return 0;
}

Graph* pass(Graph* oldGraph, unsigned int nbPass){
    if(!oldGraph)
        return NULL;

    char filename[100];
    sprintf(filename, "./Pass/pass%u.txt", nbPass);
    int errorCode = 0;
    errorCode = makeTrace(oldGraph,filename);
    if(errorCode){
        deleteGraph(oldGraph);
        return NULL;
    }

    Graph* newGraph= initGraph();
    if(!newGraph){
        deleteGraph(oldGraph);
        return NULL;
    }

    errorCode = makenewGraph(oldGraph, newGraph);
    if(errorCode){
        deleteGraph(oldGraph);
        deleteGraph(newGraph);
        return NULL;
    }
    return newGraph;
}
