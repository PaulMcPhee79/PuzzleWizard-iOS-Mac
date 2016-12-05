#ifndef __SEARCH_NODE_H__
#define __SEARCH_NODE_H__

#include <Utils/Astar.h>
#include <Interfaces/ISearchProvider.h>
#include <Utils/CMTypes.h>

class SearchNode
{
public:
	int x;
	int y;
	ISearchProvider* searchProvider;

	SearchNode() : x(0), y(0), searchProvider(NULL), cost(-1) { };
	SearchNode(int tileX, int tileY, ISearchProvider* provider) : x(tileX), y(tileY), searchProvider(provider), cost(-1) { };
	~SearchNode(void) { searchProvider = NULL; }

	float GoalDistanceEstimate( SearchNode &nodeGoal );
	bool IsGoal( SearchNode &nodeGoal );
	bool GetSuccessors( AStarSearch<SearchNode> *astarsearch, SearchNode *parent_node, int numAncestors );
	float GetCost( SearchNode &successor );
	bool IsSameState( SearchNode &rhs );

private:
	int cost;
};
#endif // __SEARCH_NODE_H__
