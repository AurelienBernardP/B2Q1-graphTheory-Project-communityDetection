#if defined(AGGREGATION_H)
#else
#define AGGREGATION_H

#include "optimisation.h"

/* communityAggregation
* Create a new file in $filename$ which keeps a trace of a graph $oldG$
* and create a condensed version of that graph.
*
* [input]
* oldG : the graph which needs to be condense
* filename : the path to a file
*
* [return]
* newG : a new graph which is a condensed version of the graph $oldG$
* NULL : an error occured
*
*/
Graph* communityAggregation(Graph* oldG,char* filename);

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
int makeTrace(Graph* g, char* filename);

#endif
