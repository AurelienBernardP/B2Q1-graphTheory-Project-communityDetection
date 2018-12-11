#include <stdio.h>
#include <stdlib.h>
#include "pass.h"
#include "graphes.h"

static double** makeAdjacenceMatrix(Graph* g){
    if(!g){return NULL;}
    
    //start allocation of adjacent matrix of graph g
    double** adjMatWithCommunity = malloc(g->nbMembers * sizeof( double*) );
    if(!adjMatWithCommunity){
        return NULL;
    }
    for(size_t i = 0; i < g->nbMembers; i++){
        adjMatWithCommunity[i] = calloc(g->nbMembers + 1, sizeof(double));//+1 because we are adding an extra column for the member's community.
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
    //list pointer is on first element of member list
    
    //go through all communities
    while(currentCommunity !=NULL){
        Member* currentMember = currentCommunity->member;
        //back up member pointer to first element of memeber list
        while(currentMember->previous != NULL){
            currentMember = currentMember->previous;
        }
        //member pointer is on the first element of the members of the community list
         
        //go through lost of members in the coomunity updating the adjacent matrix
        while(currentMember != NULL){
            //go through all neighbours of the member
            while(currentMember->neighbours != NULL){
                adjMatWithCommunity[(currentMember->label-1)][currentMember->neighbours->dest->label-1] = currentMember->neighbours->weight;
                currentMember->neighbours = currentMember->neighbours->next;
            }
            //add the number of the member's community in the matrix 
            adjMatWithCommunity[(currentMember->label-1)][g->nbMembers] = (double)currentMember->myCom->label;
            //matrix contains all the of the neighbours of the current member
            currentMember = currentMember->next;
        }
        currentCommunity = currentCommunity->next;
    }
    //matrix containss all the information needed from all the members

    return adjMatWithCommunity;
}

static int printGraphWithCommunities(Graph* g, double** AdjacenceMatrix){// we are maybe going to need the number of the pass to give different file names to the matrix printouts.
    if(!g || !AdjacenceMatrix){return -1;}
    
    FILE* fp = fopen("filename.txt", "w");//filename to be changed
    if(!fp){
        return -1;
    }
    fprintf(fp,"# number of communities = %lu\n",g->nbCommunity);
    fprintf(fp,"# number of members = %lu\n",g->nbMembers);
    fprintf(fp,"# number of edges = %lu\n",g->nbEdge);
    fprintf(fp,"# adjacence Matrix | community of member\n");
    for(size_t l=0; l<g->nbMembers; l++){//print adjacence matrix
        for(size_t c=0; l<g->nbMembers; c++){
            fprintf(fp,"%lf ",AdjacenceMatrix[l][c] );
        }
        fprintf(fp,"| %lu",(unsigned long int)AdjacenceMatrix[l][g->nbMembers] );//last column is the community of the member
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
    //init member pointer to first memeber of community
    Member* currentMember = community->member;
    //loop throught all memebers of community
    while(currentMember != NULL){
        //init edge pointer to first edge in the neightbours of current member
        Edge* currentMemberEdge = currentMember->neighbours;
        //loop through all edges of the current member and add the values to the table
        while(currentMemberEdge != NULL){
            communityConections[findIndexFromLabel(g, currentMemberEdge->dest->myCom->label, communityConections)].weight += currentMemberEdge->weight;
            currentMemberEdge = currentMemberEdge->next;
        }
        //all weights of the edges of current member have been summed to the array of connections
        currentMember = currentMember->next;
    }
    //all the weight of all the member's edges in the community has been added to the array
    return;
}

static int makeNewGraph(Graph* oldG, Graph* newG){
    if(!oldG || !newG) { return -1;}

    //init new graph structure
    newG->nbMembers = oldG->nbCommunity;
    newG->weightTot = oldG->weightTot;
    int errorCode = 0;

    //add the numbers of communities needed
    for(size_t i = 0; i<newG->nbMembers; i++){
        errorCode = addCommunity(newG);
        if(errorCode){
            deleteGraph(newG);
            return -1;
        }
    }
    //modify the label of the new communities and new members to keep the labels of the previous step graph
    Community* oldGCurrentCom = oldG->community;
    Community* newGCurrentCom = newG->community;
    //first put pointers to first element of list
    while(oldGCurrentCom->previous != NULL){
        oldGCurrentCom = oldGCurrentCom->previous;
    }
    while(newGCurrentCom->previous != NULL){
        newGCurrentCom = newGCurrentCom->previous;
    }
    //enumerate all communities and members and modify their labels
    while(oldGCurrentCom !=NULL && newGCurrentCom !=NULL){
        newGCurrentCom->member->label = newGCurrentCom->label = oldGCurrentCom->label;
        /*
            newGCurrentCom->member->weightmember;
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
        newGCurrentCom->member->weightmember = oldGCurrentCom->weightInt;

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