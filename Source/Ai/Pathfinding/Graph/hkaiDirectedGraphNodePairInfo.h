/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HK_AI_GRAPH_NODE_PAIR_INFO_H
#define HK_AI_GRAPH_NODE_PAIR_INFO_H

#include <Ai/Pathfinding/Graph/hkaiDirectedGraphExplicitCost.h>
#include <Ai/Pathfinding/NavMesh/Streaming/hkaiStreamingCollection.h>

class hkaiStreamingCollection;

/// Information regarding a pair of nodes, and the edge between them.
struct hkaiDirectedGraphNodePairInfo
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI,hkaiDirectedGraphNodePairInfo);


	/// Information regarding an node being examined.
	struct NodeInfo
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI,NodeInfo);

		/// The key of the node.
		HK_PAD_ON_SPU( hkaiPackedKey ) m_nodeKey;

		/// A pointer to the node
		HK_PAD_ON_SPU( const hkaiDirectedGraphExplicitCost::PaddedNode* ) m_node;

		/// The position in the node (or the start/end position of the search)
		hkVector4 m_position;

		inline NodeInfo();
		inline void validate( const hkaiStreamingCollection::InstanceInfo* collection = HK_NULL ) const;

		inline void setNode(hkaiPackedKey nIdx, const hkaiDirectedGraphExplicitCost::PaddedNode* node, hkVector4Parameter pos);
	};

	/// Edge information for the current node.
	NodeInfo m_currentNodeInfo;

	/// Edge information for the adjacent node.
	NodeInfo m_adjacentNodeInfo;

	/// Index of the edge
	HK_PAD_ON_SPU( hkaiPackedKey ) m_edgeKey;

	/// Pointer to the edge
	HK_PAD_ON_SPU( const hkaiDirectedGraphExplicitCost::Edge*	) m_edge;

	inline hkaiDirectedGraphNodePairInfo();
	inline void validate( const hkaiStreamingCollection::InstanceInfo* collection ) const;
};

#include <Ai/Pathfinding/Graph/hkaiDirectedGraphNodePairInfo.inl>

#endif // HK_AI_NAVVOLUME_CELL_PAIR_INFO_H

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
