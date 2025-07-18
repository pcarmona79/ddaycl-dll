
#include "../g_local.h"
#include "ai_local.h"

//==========================================
// 
// 
//==========================================

static short int alist[MAX_NODES];	//list contains all studied nodes, Open and Closed together
static int alist_numNodes;

enum {
	NOLIST,
	OPENLIST,
	CLOSEDLIST
};

typedef struct
{
	short int	parent;
	int		G;
	int		H;

	short int	list;

} astarnode_t;

astarnode_t	astarnodes[MAX_NODES];

struct astarpath_s *Apath;
//==========================================
// 
// 
//==========================================
static short int originNode;
static short int goalNode;
static short int currentNode;

static int ValidLinksMask;
#define DEFAULT_MOVETYPES_MASK (LINK_MOVE|LINK_STAIRS|LINK_FALL|LINK_WATER|LINK_WATERJUMP|LINK_JUMPPAD|LINK_PLATFORM|LINK_TELEPORT);
//==========================================
// 
// 
// 
//==========================================

int	AStar_nodeIsInClosed( int node )
{
	if( astarnodes[node].list == CLOSEDLIST )
		return 1;

	return 0;
}

int	AStar_nodeIsInOpen( int node )
{
	if( astarnodes[node].list == OPENLIST )
		return 1;

	return 0;
}

static void AStar_InitLists (void)
{
	int i;

	for ( i=0; i<MAX_NODES; i++ )
	{
		astarnodes[i].G = 0;
		astarnodes[i].H = 0;
		astarnodes[i].parent = 0;
		astarnodes[i].list = NOLIST;
	}

	if( Apath )
		Apath->numNodes = 0;

	alist_numNodes = 0;
	memset( alist, -1, sizeof(alist));//jabot092
}

static int AStar_PLinkDistance( int n1, int n2 ) // Faltaba definir como int estas 2 variables. El compilador lo hace por defecto pero tira un warning feo >:( - ZeRo
{
	int	i;

	for ( i=0; i<pLinks[n1].numLinks; i++)
	{
		if( pLinks[n1].nodes[i] == n2 )
			return (int)pLinks[n1].dist[i];
	}

	return -1;
}

static int	Astar_HDist_ManhatanGuess( int node )
{
	vec3_t	DistVec;
	int		i;
	int		HDist;

	//teleporters are exceptional
	if( nodes[node].flags & NODEFLAGS_TELEPORTER_IN )
		node++; //it's tele out is stored in the next node in the array

	for (i=0 ; i<3 ; i++)
	{
		DistVec[i] = fabs(nodes[goalNode].origin[i] - nodes[node].origin[i]);
	}

	HDist = (int)(DistVec[0] + DistVec[1] + DistVec[2]);
	return HDist;
}

static void AStar_PutInClosed( int node )
{
	if( !astarnodes[node].list ) {
		alist[alist_numNodes] = node;
		alist_numNodes++;
	}

	astarnodes[node].list = CLOSEDLIST;
}

static void AStar_PutAdjacentsInOpen( int node ) // Faltaba definir como int a node. El compilador lo hace por defecto pero tira un warning feo >:( - ZeRo
{
	int	i;
	int rand_ignore = 0;

	for ( i=0; i<pLinks[node].numLinks; i++)
	{
		int	addnode;

		//ignore invalid links
		if( !(ValidLinksMask & pLinks[node].moveType[i]) )
			continue;

		addnode = pLinks[node].nodes[i];

		//ignore self
		if( addnode == node )
			continue;

		//ignore if it's already in closed list
		if( AStar_nodeIsInClosed( addnode ) )
			continue;

		//faf: randomly ignore some nodes to get bots to take alternate routes
		if (dropnodes== true && //rand_ignore < (int)(nav.num_broams/10) && 
			rand()%(1+(int)rand()%15) == 1)
			//rand()%((int)(nav.num_broams/30)) == 1)
		{	
			rand_ignore++;
			continue;//faf
		}

		//if it's already inside open list
		if( AStar_nodeIsInOpen( addnode ) )
		{
			int plinkDist;
			
			plinkDist = AStar_PLinkDistance( node, addnode );
			//if( plinkDist == -1)
				//gi.dprintf("WARNING: AStar_PutAdjacentsInOpen:164 - Couldn't find distance between nodes\n");

			//compare G distances and choose best parent
			//else
			if ( astarnodes[addnode].G > (astarnodes[node].G + plinkDist) )
			{
				astarnodes[addnode].parent = node;
				astarnodes[addnode].G = astarnodes[node].G + plinkDist;
			}
			
		} else {	//just put it in

			int plinkDist;

			plinkDist = AStar_PLinkDistance( node, addnode );
			if( plinkDist == -1)
			{
				plinkDist = AStar_PLinkDistance( addnode, node );
				if( plinkDist == -1)
				{
					plinkDist = 999;//jalFIXME

					//ERROR
					//gi.dprintf("WARNING: AStar_PutAdjacentsInOpen:186 - Couldn't find distance between nodes\n");
				}
			}

			//put in global list
			if( !astarnodes[addnode].list ) {
				alist[alist_numNodes] = addnode;
				alist_numNodes++;
			}

			astarnodes[addnode].parent = node;
			astarnodes[addnode].G = astarnodes[node].G + plinkDist;
			astarnodes[addnode].H = Astar_HDist_ManhatanGuess( addnode );
			astarnodes[addnode].list = OPENLIST;
		}
	}
}

static int AStar_FindInOpen_BestF ( void )
{
	int	i;
	int	bestF = -1;
	int best = -1;

	for ( i=0; i<alist_numNodes; i++ )
	{
		int node = alist[i];

		if( astarnodes[node].list != OPENLIST )
			continue;

		if ( bestF == -1 || bestF > (astarnodes[node].G + astarnodes[node].H) ) {
			bestF = astarnodes[node].G + astarnodes[node].H;
			best = node;
		}
	}
	//printf("BEST:%i\n", best);
	return best;
}

static void AStar_ListsToPath ( void )
{
	int count = 0;
	int cur = goalNode;
	short int	*pnode;

	Apath->numNodes = 0;
	pnode = Apath->nodes;
	while ( cur != originNode ) 
	{
		*pnode = cur;
		pnode++;
		cur = astarnodes[cur].parent;
		count++;
	}

	Apath->numNodes = count-1;
}

static int AStar_FillLists ( void )
{
	//put current node inside closed list
	AStar_PutInClosed( currentNode );
	
	//put adjacent nodes inside open list
	AStar_PutAdjacentsInOpen( currentNode );
	
	//find best adjacent and make it our current
	currentNode = AStar_FindInOpen_BestF();

	return (currentNode != -1);	//if -1 path is bloqued
}

static int AStar_ResolvePath ( int n1, int n2, int movetypes )
{
	ValidLinksMask = movetypes;
	if ( !ValidLinksMask )
		ValidLinksMask = DEFAULT_MOVETYPES_MASK;

	AStar_InitLists();

	originNode = n1;
	goalNode = n2;
	currentNode = originNode;

	while ( !AStar_nodeIsInOpen(goalNode) )
	{
		if( !AStar_FillLists() )
			return 0;	//failed
	}

	AStar_ListsToPath();

	return 1;
}

int AStar_GetPath( int origin, int goal, int movetypes, struct astarpath_s *path )
{
	Apath = path;

	if( !AStar_ResolvePath ( origin, goal, movetypes ) )
		return 0;

	path->originNode = origin;
	path->goalNode = goal;
	return 1;
}



