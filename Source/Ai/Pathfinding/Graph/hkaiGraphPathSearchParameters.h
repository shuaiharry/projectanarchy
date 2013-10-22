#include <Ai/Pathfinding/Astar/Search/hkaiSearchParams.h>
/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HK_AI_PATHFINDING_GRAPH_PATH_SEARCH_PARAMETERS_H
#define HK_AI_PATHFINDING_GRAPH_PATH_SEARCH_PARAMETERS_H

class hkaiAstarCostModifier;
class hkaiAstarEdgeFilter;

/// Parameters for controlling path searches on directed graphs.
/// This input data may apply to a batch of path requests.
struct hkaiGraphPathSearchParameters
{
	// +version(2)
	HK_DECLARE_REFLECTION();
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR, hkaiGraphPathSearchParameters );
	

	hkaiGraphPathSearchParameters();
	hkaiGraphPathSearchParameters(hkFinishLoadedObjectFlag f);

	hkReal m_heuristicWeight; //+default(1.0f)

		/// Whether or not the search should use hierarchical A*
		/// Hierarchical searching for graphs is in beta.
	hkBool m_useHierarchicalHeuristic; //+default(true)

		/// Optional pointer to hkaiAstarCostModifier, which can be used to modify costs based on the hkaiAgentTraversalInfo
	const hkaiAstarCostModifier* m_costModifier; //+nosave

		/// Optional pointer to hkaiAstarEdgeFilter, which can be used to reject edges
	const hkaiAstarEdgeFilter* m_edgeFilter; //+nosave

	//
	// Memory control parameters
	// Reducing these numbers will reduce the size of memory allocations during the search,
	// but may cause the search to terminate early
	//

		/// Maximum memory size for the search.
	hkaiSearchParameters::BufferSizes m_bufferSizes;

		/// Maximum memory size for the hierarchical heuristic subsearch.
	hkaiSearchParameters::BufferSizes m_hierarchyBufferSizes;
};

#include <Ai/Pathfinding/Graph/hkaiGraphPathSearchParameters.inl>

#endif // HK_AI_PATHFINDING_GRAPH_PATH_SEARCH_PARAMETERS_H

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
