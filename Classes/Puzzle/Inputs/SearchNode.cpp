
//#define SEARCH_NODE_LINEAR_PATH

#include "SearchNode.h"
#include <cmath>

#include "cocos2d.h"
USING_NS_CC;

float SearchNode::GoalDistanceEstimate( SearchNode &nodeGoal )
{
#ifdef SEARCH_NODE_LINEAR_PATH
	// Prefer straight-line paths. From: http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html#breaking-ties
	float dx1 = float( ( (float)x - (float)nodeGoal.x ) );
	float dy1 = float( ( (float)y - (float)nodeGoal.y) );
	float dx2 = searchProvider->getDxy().x;
	float dy2 = searchProvider->getDxy().y;

	float manhattan = fabs(dx1) + fabs(dy1);
	float cross = fabs(dx1*dy2 - dx2*dy1);
	float p = cross * 0.00005f; // Must satisfy for shortest path: p < MIN(GetCost()) / MAX_PATH_LEN where MAX_PATH_LEN = 100 in our case. 
	float heuristic = manhattan * (1.0f + p);
	return heuristic;
#else
    float dx1 = float( ( (float)x - (float)nodeGoal.x ) );
	float dy1 = float( ( (float)y - (float)nodeGoal.y) );
    float manhattan = fabs(dx1) + fabs(dy1);
    float p = 1.0f / (float)searchProvider->getMaxPathLength(); // Must satisfy for shortest path: p < MIN(GetCost()) / MAX_PATH_LEN.
    float heuristic = manhattan * (1.0f + p);
	return heuristic;
#endif
}

bool SearchNode::IsGoal( SearchNode &nodeGoal )
{
	return IsSameState(nodeGoal);
}

bool SearchNode::GetSuccessors( AStarSearch<SearchNode> *astarsearch, SearchNode *parent_node, int numAncestors )
{
	if (searchProvider && astarsearch)
	{
		Coord parentPos = parent_node ? cmc(parent_node->x, parent_node->y) : cmc(-1, -1);
		Coord successors[4];
		int numSuccessors = searchProvider->getSuccessors(cmc(x, y), parentPos, &successors[0], numAncestors);
		assert(numSuccessors <= 4);
		for (int i = 0; i < numSuccessors; ++i)
        {
            SearchNode successor = SearchNode(successors[i].x, successors[i].y, searchProvider);
			astarsearch->AddSuccessor(successor);
        }
	}

	return true;
}

float SearchNode::GetCost( SearchNode &successor )
{
	if (cost == -1)
	{
		cost = IsGoal(successor) ? 0 : 1 + 1000 * searchProvider->getSearchWeighting(cmc(x, y));
	}
	return cost;
}

bool SearchNode::IsSameState( SearchNode &rhs )
{
	return (x == rhs.x && y == rhs.y);
}
