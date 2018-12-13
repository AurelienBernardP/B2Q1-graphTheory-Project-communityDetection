#if defined(AGGREGATION_H)
#else
#define AGGREGATION_H

#include "optimisation.h"

/* communityAggregation
* Create a new file in $filename$ which keep a trace of the old graph $oldG$
* and apply communityAggregation (described in the article).
*
* [input]
* oldG : the graph which needs community aggregation
* filename : the path to a file
*
* [return]
* newG : a new graph which is a condensed version of the graph $oldG$
* NULL : an error occured
*
*/
Graph* communityAggregation(Graph* oldG,char* filename);

#endif
