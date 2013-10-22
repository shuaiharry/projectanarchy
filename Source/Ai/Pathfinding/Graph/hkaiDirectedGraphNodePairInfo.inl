/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */



inline hkaiDirectedGraphNodePairInfo::NodeInfo::NodeInfo()
:	m_nodeKey(HKAI_INVALID_PACKED_KEY),
	m_node(HK_NULL)
{
}

inline hkaiDirectedGraphNodePairInfo::hkaiDirectedGraphNodePairInfo()
:	m_edgeKey(HKAI_INVALID_PACKED_KEY),
	m_edge(HK_NULL)
{

}

inline void hkaiDirectedGraphNodePairInfo::NodeInfo::validate( const hkaiStreamingCollection::InstanceInfo* streamingInfo ) const
{
#ifdef HK_DEBUG
	HK_ASSERT(0x630f01f0, m_nodeKey == HKAI_INVALID_PACKED_KEY || ( m_node ) );
 
 	if (streamingInfo && m_node)
 	{
		const hkaiDirectedGraphExplicitCost::Node& node =  hkaiStreamingCollection::getNodeFromPacked( streamingInfo, m_nodeKey );
 		HK_ASSERT(0x334051d7, node.m_numEdges == m_node->m_numEdges );
 		HK_ASSERT(0x334051d7, node.m_startEdgeIndex== m_node->m_startEdgeIndex );
 	}
#endif
}

inline void hkaiDirectedGraphNodePairInfo::validate( const hkaiStreamingCollection::InstanceInfo* streamingInfo ) const
{
#ifdef HK_DEBUG
 	HK_ASSERT(0x56f31f14, (m_edgeKey != HKAI_INVALID_PACKED_KEY) == (m_edge != HK_NULL) );
 	m_currentNodeInfo.validate( streamingInfo );
 	m_adjacentNodeInfo.validate( streamingInfo );
 	// Make sure the nodes aren't the same (unless the edge is NULL)
 	HK_ASSERT(0x2ad0e697, (m_currentNodeInfo.m_nodeKey != m_adjacentNodeInfo.m_nodeKey) || m_edgeKey == HKAI_INVALID_PACKED_KEY);
 	HK_ASSERT(0x6ec88eb1, (m_currentNodeInfo.m_node != m_adjacentNodeInfo.m_node) || m_edge == HK_NULL);

	if (streamingInfo && m_edge)
 	{
		const hkaiDirectedGraphExplicitCost::Edge& edge = hkaiStreamingCollection::getGraphEdgeFromPacked(streamingInfo, m_edgeKey );
 		HK_ASSERT(0x5a3fef83, edge.m_flags == m_edge->m_flags);
		HK_ASSERT(0x5a3fef83, edge.getOppositeNodeIndex() == m_edge->getOppositeNodeIndex() );
		HK_ASSERT(0x5a3fef83, edge.m_cost == m_edge->m_cost );
 	}
#endif
}

inline void hkaiDirectedGraphNodePairInfo::NodeInfo::setNode(hkaiPackedKey nIdx, const hkaiDirectedGraphExplicitCost::PaddedNode* node, hkVector4Parameter pos)
{
	m_nodeKey = nIdx;
	m_node = node;
	m_position = pos;
}

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
