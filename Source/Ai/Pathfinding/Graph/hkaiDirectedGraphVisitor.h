/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HKAI_DIRECTEDGRAPH_VISITOR_H
#define HKAI_DIRECTEDGRAPH_VISITOR_H

#include <Ai/Pathfinding/Graph/hkaiDirectedGraphInstance.h>
#include <Ai/Pathfinding/Astar/Heuristic/hkaiGraphDistanceHeuristic.h>
#include <Ai/Pathfinding/Astar/SearchState/hkaiHashSearchState.h>
#include <Ai/Pathfinding/NavMesh/Streaming/hkaiStreamingCollection.h>

#include <Ai/Pathfinding/Graph/hkaiAgentTraversalInfo.h>
#include <Ai/Pathfinding/Astar/CostModifier/hkaiAstarCostModifier.h>
#include <Ai/Pathfinding/Astar/EdgeFilter/hkaiAstarEdgeFilter.h>

	/// Directed graph A* interface
struct hkaiDirectedGraphVisitor
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI_ASTAR, hkaiDirectedGraphVisitor);

	typedef hkaiGraphDistanceHeuristic<hkaiDirectedGraphVisitor> Heuristic;
	HK_FORCE_INLINE hkaiDirectedGraphVisitor();
	HK_FORCE_INLINE hkaiDirectedGraphVisitor(const hkaiDirectedGraphAccessor* accessor);

	//HK_FORCE_INLINE void init(const hkaiDirectedGraphExplicitCost* mainMemGraph);

	enum 
	{
		MAX_VERTEX_DEGREE = 24
	};


#ifdef HK_PLATFORM_SPU
	enum 
	{ 
		EDGE_DMA_GROUP = 1
	};
#endif

	typedef hkaiHashSearchState SearchState; 

	//
	// Begin hkaiAstar interface
	//

	typedef hkaiDirectedGraphExplicitCost::SearchIndex SearchIndex;
	typedef hkaiDirectedGraphExplicitCost::NodeIndex NodeIndex;
	typedef hkaiDirectedGraphExplicitCost::EdgeIndex EdgeKey;
	typedef hkaiDirectedGraphExplicitCost::EdgeCost EdgeCost;
	typedef hkaiDirectedGraphExplicitCost::Position Position;

	HK_FORCE_INLINE hkBool32 isGoal( SearchIndex nit ) const;
	HK_FORCE_INLINE int getMaxNeighborCount( SearchIndex nit ) const;
	HK_FORCE_INLINE void getNeighbors( SearchIndex nit, hkArrayBase< EdgeKey >& neighbors ) const;
	HK_FORCE_INLINE SearchIndex edgeTarget( SearchIndex nit, EdgeKey eit ) const;
	inline EdgeCost getTotalCost(SearchIndex nit, SearchIndex adj, EdgeKey eit, const EdgeCost costToParent) const;

	//
	// End hkaiAstar interface
	//

	HK_FORCE_INLINE hkBool32 isValidEdgeTarget( SearchIndex eit ) const;
	HK_FORCE_INLINE hkBool32 isEdgeTraversable(SearchIndex nit, SearchIndex adj, EdgeKey eit) const;
	HK_FORCE_INLINE void getPositionForHeuristic( SearchIndex a, hkVector4& v ) const;
	HK_FORCE_INLINE void getCurrentPosition( SearchIndex a, hkVector4& v ) const;
	HK_FORCE_INLINE hkSimdReal distanceFromPosition(hkVector4Parameter posA, hkVector4Parameter posB) const;
	HK_FORCE_INLINE void setSearchState( SearchState* state );


	//
	// Listener interface
	// Most of these are empty, but a few are used to cache nodes/edges locally
	//
	HK_FORCE_INLINE void nextNode(SearchIndex nid, hkBool32 updateSearchState = true);
	HK_FORCE_INLINE void nextEdge(SearchIndex nid, EdgeKey ekey, hkBool32 updateSearchState = true);
	HK_FORCE_INLINE void edgeAccepted(SearchIndex nid, EdgeKey ekey) {}
	HK_FORCE_INLINE void setParent(SearchIndex nid, SearchIndex parent, EdgeKey edge) const;
	HK_FORCE_INLINE void keepParent(SearchIndex nid, SearchIndex parent, EdgeKey edge) const;
	HK_FORCE_INLINE void nodeOpened(SearchIndex) const;
	HK_FORCE_INLINE void openNodeAdjusted(SearchIndex) const;
	HK_FORCE_INLINE void nodeClosed(SearchIndex) const;

	//
	// end listener interface
	//

	HK_FORCE_INLINE void init( 
		const hkaiStreamingCollection::InstanceInfo* streamingInfo, 
		const hkaiAstarCostModifier* costModifier,
		const hkaiAstarEdgeFilter* edgeFilter
		);

	HK_FORCE_INLINE void setTraversalInfo ( const hkaiAgentTraversalInfo& info );

	HK_FORCE_INLINE void setGoals( const hkaiPackedKey* goalKeys, const hkReal* finalCosts, int numGoals);

	// Streaming collection
	HK_PAD_ON_SPU( const struct hkaiStreamingCollection::InstanceInfo* ) m_streamingInfo;

	HK_FORCE_INLINE const hkaiDirectedGraphAccessor* getIncomingAccessor() const;
	HK_FORCE_INLINE const hkaiDirectedGraphAccessor* getOutgoingAccessor() const;
	HK_FORCE_INLINE const hkaiDirectedGraphAccessor* getGeneralAccessor() const;

	//HK_FORCE_INLINE void setIncomingAccessor(const hkaiDirectedGraphExplicitCost* mainMemGraph);
	//HK_FORCE_INLINE void setOutgoingAccessor(const hkaiDirectedGraphExplicitCost* mainMemGraph);
	HK_FORCE_INLINE void setIncomingAccessor(int sectionId);
	HK_FORCE_INLINE void setOutgoingAccessor(int sectionId);
	HK_FORCE_INLINE void setGeneralAccessor(int sectionId);
	HK_FORCE_INLINE void setOutgoingSectionFromPacked( hkaiPackedKey );

	HK_ON_CPU( const hkaiDirectedGraphAccessor* m_incomingGraph; )
	HK_ON_CPU( const hkaiDirectedGraphAccessor* m_outgoingGraph; )
	HK_ON_CPU( const hkaiDirectedGraphAccessor* m_generalGraph; )
	HK_ON_SPU( hkaiDirectedGraphAccessor m_incomingAccessor; )
	HK_ON_SPU( hkaiDirectedGraphAccessor m_outgoingAccessor; )
	HK_ON_SPU( hkaiDirectedGraphAccessor m_generalAccessor; )

	// Don't do these on SPU, since we don't actually support graph queries there.
#ifndef HK_PLATFORM_SPU
	const hkaiAstarCostModifier*  m_costModifier;
	const hkaiAstarEdgeFilter*  m_edgeFilter;
#endif

		//
		// Cached information
		//

	hkaiDirectedGraphExplicitCost::PaddedNode m_cachedNode;
	hkaiDirectedGraphExplicitCost::PaddedNode m_cachedInstanceNode;
	hkVector4  m_cachedPosition;

	hkaiAgentTraversalInfo	m_agentInfo;

	SearchState* m_searchState;
	HK_PAD_ON_SPU( hkaiDirectedGraphExplicitCost::Edge* ) m_currentEdge;
	HK_PAD_ON_SPU( int ) m_cachedIncomingSectionId; 
	HK_PAD_ON_SPU( int ) m_cachedOutgoingSectionId; 
	HK_PAD_ON_SPU( int ) m_cachedGeneralSectionId; 
	HK_PAD_ON_SPU( hkaiPackedKey ) m_cachedNodeKey; 

	// Goal and final cost information
	// These are additional costs applied to the corresponding goal nodes.
	HK_PAD_ON_SPU( const hkaiPackedKey* )	m_goalNodes;
	HK_PAD_ON_SPU( const hkReal* )			m_finalCosts;
	HK_PAD_ON_SPU( int )					m_numGoals;

	// need extra due to alignment restrictions on DMA gets
	HK_ALIGN16( hkaiDirectedGraphExplicitCost::Edge m_localEdges[2*MAX_VERTEX_DEGREE + 2] );

	// Debug info for asserts
	HK_ON_DEBUG( hkaiDirectedGraphExplicitCost::Edge* m_localEdgeStart; )
};

#include <Ai/Pathfinding/Graph/hkaiDirectedGraphVisitor.inl>

#endif

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
