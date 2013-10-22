/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef HKAI_GRAPH_UTILS_H
#define HKAI_GRAPH_UTILS_H

#include <Ai/Pathfinding/hkaiBaseTypes.h>
#include <Ai/Pathfinding/NavMesh/Streaming/hkaiStreamingSet.h>

class hkaiDirectedGraphExplicitCost;
class hkaiDirectedGraphInstance;
class hkaiStreamingCollection;

/// Utilities for manipulating hkaiDirectedGraphExplicitCost
class hkaiGraphUtils 
{
public:
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE, hkaiGraphUtils);

	static hkaiPackedKey HK_CALL getEdgeBetween( const hkaiDirectedGraphExplicitCost* graph, hkaiPackedKey nodeIn, hkaiPackedKey targetNode );
	static hkaiPackedKey HK_CALL getEdgeBetween( const hkaiStreamingCollection* collectionIn, hkaiPackedKey nodeIn, hkaiPackedKey targetNode );

	/// Asserts if the graph isn't symmetric
	static void HK_CALL checkSymmetric(const hkaiDirectedGraphExplicitCost* graph);

	/// Adds an edge to the corresponding directed graph instances, or increments its reference count
	static void HK_CALL addDirectedGraphEdgeForUserEdge( hkaiPackedKey startFaceKey, hkaiPackedKey userEdgeKey, hkaiStreamingCollection* collection );
	static void HK_CALL removeDirectedGraphEdgeForUserEdge( hkaiPackedKey startFaceKey, hkaiPackedKey userEdgeKey, hkaiStreamingCollection* collection );

	/// Removes all graph user edges connected to the section.
	static void HK_CALL removeDirectedUserEdgesToSection( hkaiStreamingCollection* collection, hkaiRuntimeIndex sectionIndex);

	static void HK_CALL compactOwnedEdges( hkaiDirectedGraphInstance& graph );

		// Remove nodes with no edges. If otherGraphs is specified, the hkaiStreamingSets will be updated
	static hkResult HK_CALL removeEmptyNodes( hkaiDirectedGraphExplicitCost& graph, hkArray<hkaiDirectedGraphExplicitCost*>* otherGraphs = HK_NULL);
	static hkResult HK_CALL removeNodes( hkaiDirectedGraphExplicitCost& graph, hkArray<int>::Temp& nodesToRemove, bool invalidateOppEdges=true, hkArray<hkaiDirectedGraphExplicitCost*>* otherGraphs = HK_NULL);

	static void HK_CALL createEdgeToNodeMap(const hkaiDirectedGraphExplicitCost* graph, hkArray<int>& nodeIndices);
	static void HK_CALL createEdgeToNodeMap(const hkaiDirectedGraphInstance* graph, hkArray<int>& nodeIndices);

		/// Whether an edge is being added or removed.
	enum Adjustment
	{
		GRAPH_ADD_EDGE = 1,
		GRAPH_REMOVE_EDGE = -1,
	};

	static void HK_CALL _adjustDirectedGraphEdge( hkaiPackedKey startFaceKey, hkaiPackedKey userEdgeKey, hkaiStreamingCollection* collection, Adjustment adj );


};

#endif // HKAI_GRAPH_UTILS_H

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
