#include <stdio.h>
#include <stdlib.h>
#include "pass.h"

typedef struct{
    double weight; 
    size_t destCommunityLabel;
}LinksToCommunity;

static double** makeAdjacenceMatrix(Graph* g){
    if(!g)
        return NULL;

    //Allocation of the adjacent matrix of graph g
    double** adjMat = malloc(g->nbMembers * sizeof( double*) );
    if(!adjMat)
        return NULL;

    for(size_t i = 0; i < g->nbMembers; i++){
        adjMat[i] = calloc(g->nbMembers + 1, sizeof(double));//adding an extra column for the member's community
        if(!adjMat[i])
            return NULL;
    }
    
    Community* community =  g->community;    
    //Browsing all communities
    while(community !=NULL){
        Member* member = community->member;
        //Browing all members of the community
        while(member != NULL){
            Edge *neighbours = member->neighbours;
            //Browsing all neighbours of the member
            while(neighbours != NULL){
                //Adding the weight to the matrix
                adjMat[(member->label-1)][(neighbours->dest->label-1)] = neighbours->weight;
                neighbours = neighbours->next;
            }
            //Add the label of the member's community in the matrix 
            adjMat[(member->label-1)][g->nbMembers] = (double)member->myCom->label;
            member = member->next;
        }
        community = community->next;
    }

    return adjMat;
}

static int printGraphWithCommunities(Graph* g, double** AdjacenceMatrix){// we are maybe going to need the number of the pass to give different file names to the matrix printouts.
    if(!g || !AdjacenceMatrix)
        return -1;
    
    FILE* fp = fopen("filename.txt", "w");//filename to be changed
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

static int makeTrace(Graph* g){
    if(!g)
        return -1;

    double** adjMat = makeAdjacenceMatrix(g);   
    if(!adjMat)
        return -1;

    int errorCode = printGraphWithCommunities(g, adjMat);
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

static int makenewGraphraph(Graph* oldGraph, Graph* newGraph){
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

Graph* pass(Graph* oldGraph){
    if(!oldGraph)
        return NULL;

    int errorCode = 0;
    errorCode = makeTrace(oldGraph);

    if(errorCode){
        deleteGraph(oldGraph);
        return NULL;
    }

    Graph* newGraph= initGraph();
    if(!newGraph){
        deleteGraph(oldGraph);
        return NULL;
    }

    errorCode = makenewGraphraph(oldGraph, newGraph);
    if(errorCode){
        deleteGraph(oldGraph);
        deleteGraph(newGraph);
        return NULL;
    }

    return newGraph;
}
