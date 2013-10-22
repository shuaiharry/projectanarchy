/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

// Note: "cluster" and "chunk" are used interchangeably below.





inline hkSimdReal hkaiHierarchicalGraphHeuristic::getMinDistanceToGoals( hkVector4Parameter nodePosition ) const
{
	hkSimdReal minDistance = hkSimdReal_Max;

	// Check each goal, choose the one with the closest Euclidean distance
	for (int iGoal=0; iGoal < m_numGoals; iGoal++)
	{
		hkSimdReal distance_i = nodePosition.distanceTo(m_goalPositions[iGoal]);
		minDistance.setMin(minDistance, distance_i);
	}

	return minDistance;
}

inline hkaiHierarchicalGraphHeuristic::ClusterKey hkaiHierarchicalGraphHeuristic::getClusterForNode( NodeKey nodeKey ) const
{	
	hkaiRuntimeIndex sectionIndex = hkaiGetRuntimeIdFromPacked(nodeKey);
	hkaiDirectedGraphExplicitCost::NodeIndex nodeIndex = hkaiGetIndexFromPacked(nodeKey);

	m_originalGraph->setGeneralAccessor(sectionIndex);
	const hkaiDirectedGraphExplicitCost::NodeData* nodeDataPtr = m_originalGraph->getGeneralAccessor()->getNodeDataPtr( nodeIndex );

	HK_ASSERT(0x13e90f6b, nodeDataPtr);
	return nodeDataPtr ? hkaiGetPackedKey(sectionIndex, nodeDataPtr[0]) : HKAI_INVALID_PACKED_KEY;
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
