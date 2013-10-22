/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef HKAI_ASTAR_HIERARCHICAL_GRAPH_HEURISTIC_H
#define HKAI_ASTAR_HIERARCHICAL_GRAPH_HEURISTIC_H

#include <Ai/Pathfinding/Astar/hkaiAstar.h>
#include <Ai/Pathfinding/Astar/hkaiAstarParameters.h>
#include <Ai/Pathfinding/Astar/Search/DirectedGraphSearch/hkaiDirectedGraphEuclideanSearch.h>

#include <Ai/Pathfinding/Graph/hkaiDirectedGraphExplicitCost.h>
#include <Ai/Pathfinding/NavMesh/hkaiGeneralAccessor.h>

// If this is defined, some extra distance information will be used in the heuristic
// This overestimates the cost slightly, but can greatly reduce the number of iterations taken
#define HKAI_USE_EXTRA_DISTANCE_INFORMATION

struct hkaiSearchMemoryInfo;

/// An improved heuristic for use in A* searches.
struct hkaiHierarchicalGraphHeuristic
{
	public:
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiHierarchicalGraphHeuristic);
		typedef int SearchIndex;
		typedef hkReal PathCost;
		typedef struct hkaiDirectedGraphVisitor OriginalGraph;
		typedef hkaiDirectedGraphVisitor CoarseGraph;

		typedef hkaiPackedKey NodeKey;
		typedef hkaiPackedKey EdgeKey;
		typedef hkaiPackedKey ClusterKey;
		typedef int ClusterIndex;


		enum { HEURISTIC_IS_INVARIANT = 1};

		enum { MAX_GOALS = 16 };

//		hkaiHierarchicalGraphHeuristic();
		hkaiHierarchicalGraphHeuristic(const hkaiSearchMemoryInfo& memInfo);

		~hkaiHierarchicalGraphHeuristic() {}

			/// Initialize with hierarchy information
		void init( OriginalGraph* originalGraph, CoarseGraph* clusterGraph, NodeKey originalStartNodeKey, NodeKey originalgoalNodeKey );

			/// Initialize with hierarchy information and multiple goals
		void init( OriginalGraph* originalGraph, CoarseGraph* clusterGraph, NodeKey originalStartNodeKey, const NodeKey* originalGoalNodeKeys, int numGoals );

			/// Get the approximate cost from an edge to the goal.
		hkReal getHeuristic(NodeKey nodeId) const;


		inline ClusterKey getClusterForNode(NodeKey nodeKey) const;
		void getNodePosition(NodeKey nodeKey, hkVector4& posOut) const;
		void getClusterPosition(ClusterKey key, hkVector4& posOut) const;

	protected:
		/// Returns the Euclidean distance to the closest goal
		inline hkSimdReal getMinDistanceToGoals( hkVector4Parameter nodePosition ) const;

		// sets goalIndexOut to the index of the goal cluster that the node is adjacent to, or -1 if not adjacent
		bool isFaceChunkAdjacentToGoalChunk(CoarseGraph::NodeIndex nodeIndex, int& goalClusterOut, CoarseGraph::EdgeCost& costOut) const;

		hkSimdReal adjacentDistance(hkVector4Parameter startPosition, hkVector4Parameter startCenter,
			hkVector4Parameter endPosition, hkVector4Parameter endCenter, CoarseGraph::EdgeCost edgeCost) const;

	public:
		HK_PAD_ON_SPU(OriginalGraph*) m_originalGraph;
		HK_PAD_ON_SPU( CoarseGraph*) m_clusterGraph;
	
		HK_PAD_ON_SPU(const hkaiStreamingCollection::InstanceInfo*) m_originalStreamingInfo;
		HK_PAD_ON_SPU(const hkaiStreamingCollection::InstanceInfo*) m_clusterStreamingInfo;
		
		HK_PAD_ON_SPU(int) m_numGoals;
		HK_PAD_ON_SPU(bool) m_hasValidGoalCluster;
		HK_PAD_ON_SPU(ClusterKey) m_originalStartCluster;

		ClusterKey		m_endClusterKeys[MAX_GOALS];
		hkVector4		m_endClusterPositions[MAX_GOALS];
		
		HK_PAD_ON_SPU(NodeKey) m_startNodeKey;
		
		mutable hkaiDirectedGraphEuclideanSearch m_coarseSearch;
		
		//
		// Goal information
		//

		HK_PAD_ON_SPU(const NodeKey*) m_goalNodeKeys;
		hkVector4		m_goalPositions[MAX_GOALS];
		
};

#include <Ai/Pathfinding/Astar/Heuristic/hkaiHierarchicalGraphHeuristic.inl>
#endif // HKAI_ASTAR_HIERARCHICAL_HEURISTIC_H

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
