#if defined(PASS_H)
#else
#define PASS_H

#include "graphes.h"

typedef struct{
    double weight; 
    size_t destCommunityLabel;
}BetweenCommunityEdge;

static double** makeAdjacenceMatrix(Graph* g);

static int printGraphWithCommunities(Graph* g, double** AdjacenceMatrix);// we are maybe going to need the number of the pass to give different file names to the matrix printouts.

static int makeTrace(Graph* g);

static size_t findIndexFromLabel(Graph* g, size_t communityLabel, BetweenCommunityEdge* communityConections);

static void condenseLinksBetweenCommunities(Graph* g, Community* community, BetweenCommunityEdge* communityConections );

static int makeNewGraph(Graph* oldG, Graph* newG);

int pass(Graph* oldG);

#endif
