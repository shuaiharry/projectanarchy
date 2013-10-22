/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HK_AI_GRAPH_HIERARCHICAL_SEARCH_H
#define HK_AI_GRAPH_HIERARCHICAL_SEARCH_H

#include <Ai/Pathfinding/Astar/hkaiAstarParameters.h>
#include <Ai/Pathfinding/Astar/Heuristic/hkaiHierarchicalGraphHeuristic.h>

#include <Ai/Pathfinding/Astar/SearchState/hkaiHashSearchState.h>
#include <Ai/Pathfinding/Graph/hkaiDirectedGraphExplicitCost.h>

#include <Ai/Pathfinding/Astar/OpenSet/hkaiHeapOpenSet.h>

struct hkaiSearchMemoryInfo;

	/// Utility for performing A* searches on a graph.
struct hkaiDirectedGraphSearch
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiDirectedGraphSearch);
	typedef hkaiDirectedGraphVisitor Graph;

	typedef hkaiDirectedGraphVisitor ClusterGraph;

	typedef struct hkaiHierarchicalGraphHeuristic Heuristic;
	typedef hkaiHashSearchState SearchState;

	typedef hkaiHeapOpenSet OpenSet;

	typedef hkaiPackedKey SearchIndex;
	typedef hkaiPackedKey NodeIndex;


		/// Start point and goal information
	struct StartGoalInfo
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiDirectedGraphSearch::StartGoalInfo);

		// Only one start point, but can have multiple nodes near it.
		const hkaiPackedKey* m_startNodeKeys;
		const hkReal* m_initialCosts;
		int m_numStartNodeKeys;

		const hkaiPackedKey* m_goalNodeKeys;
		const hkReal* m_finalCosts;
		int m_numGoals;
	};

	hkaiDirectedGraphSearch( const hkaiSearchMemoryInfo& memInfo, const hkaiSearchMemoryInfo& hierarchyMemInfo );
	

		/// Initialize the search
	void init(Graph* graph, const hkaiStreamingCollection::InstanceInfo* clusterGraphInfo, const StartGoalInfo& goalInfo);

	inline hkReal getCost(int nid)
	{
		m_state.nextNode( nid );
		return m_state.getCost(nid);
	}
	
public:

	/// Do one iteration of A*
	hkaiAstarOutputParameters::SearchStatus iteration();
		
		/// Pointer to the hkaiNavMeshGraph being searched
	hkPadSpu<Graph*> m_graph;

		/// A* search state
	SearchState m_state;

		/// A* open set
	OpenSet m_openSet;

		/// A* Heuristic
	Heuristic m_heuristic;

		/// The most recent search node that was processed by A*.
		/// This will be a nav mesh edge, the start pseudo-node, or a goal-pseudo node
	SearchIndex m_lastClosedNode;

		/// Hierarchy graph (if applicable)
	ClusterGraph m_clusterGraph;

		/// Character diameter
	hkReal m_diameter;
};

#include <Ai/Pathfinding/Astar/Search/DirectedGraphSearch/hkaiDirectedGraphSearch.inl>
#endif // HK_AI_GRAPH_HIERARCHICAL_SEARCH_H

/*
 * Havok SDK - Base file, BUILD(#20131019)
 * 
 * Confidential Information of Havok.  (C) Copyright 1999-2013
 * Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
 * Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
 * rights, and intellectual property rights in the Havok software remain in
 * Havok and/or its suppliers.
 * 
 * Use of this software for evaluation purposes is subject to and indicates
 * acceptance of the End User licence Agreement for this product. A copy of
 * the license is included with this software and is also available from salesteam@havok.com.
 * 
 */
