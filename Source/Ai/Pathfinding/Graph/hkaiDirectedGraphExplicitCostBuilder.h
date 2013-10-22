/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef HKAI_ASTAR_DIRECTEDGRAPHEXPLICITCOST_BUILDER_H
#define HKAI_ASTAR_DIRECTEDGRAPHEXPLICITCOST_BUILDER_H

#include <Ai/Pathfinding/Utilities/Hierarchy/hkaiHierarchyUtils.h>
#include <Ai/Pathfinding/Graph/hkaiDirectedGraphExplicitCost.h>

/// Interface for constructing graphs.
class hkaiGraphBuilder
{
public:
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI, hkaiGraphBuilder);
	typedef int PositionId;
	typedef int EdgeKey;
	virtual ~hkaiGraphBuilder() {}
	virtual PositionId addPosition( hkVector4Parameter p ) = 0;
	virtual EdgeKey addEdge( PositionId a, PositionId b, hkReal w ) = 0;
	virtual void extraPositionData( PositionId, int tag, void* value ) {}
	virtual void extraEdgeData( EdgeKey, int tag, void* value ) {}

		// calls addEdge(a,b,w) and addEdge(b,a,w)
	void addSymmetricEdge( PositionId a, PositionId b, hkReal w );
};

	/// Utility class for constructing graphs
class hkaiDirectedGraphExplicitCostBuilder : public hkaiGraphBuilder
{
public:
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiDirectedGraphExplicitCostBuilder);
	typedef hkaiDirectedGraphExplicitCost::Node Node;
	typedef hkaiDirectedGraphExplicitCost::Position Position;
	typedef hkaiDirectedGraphExplicitCost::Edge Edge;

		/// Graph building options.
	struct BuildOptions
	{
		BuildOptions();

			/// Whether or not to allow duplicated edges.
			/// If false, duplicates will be ignored, and the lower-cost edge will be kept.
		hkBool m_allowDuplicateEdges;
	};
	
		/// A potential edge to be constructed in the graph.
	struct Link
	{
		int a; 
		int b; 
		hkReal w;
	};

	virtual PositionId addPosition( hkVector4Parameter p ) HK_OVERRIDE;
	virtual EdgeKey addEdge( PositionId a, PositionId b, hkReal w ) HK_OVERRIDE;

	hkaiDirectedGraphExplicitCost* build(hkaiDirectedGraphExplicitCost* d);
	hkArray<Position> m_positions;
	hkArray<Link> m_links;

	BuildOptions m_options;

protected:
	int removeDuplicateEdges();
};

	/// Works the same as hkaiDirectedGraphExplicitCostBuilder, but avoids duplicated positions
class hkaiHashingDirectedGraphBuilder : public hkaiDirectedGraphExplicitCostBuilder
{
public: 
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiHashingDirectedGraphBuilder);
	hkaiHashingDirectedGraphBuilder() {m_collisions = 0;}

	virtual PositionId addPosition( hkVector4Parameter p ) HK_OVERRIDE;

	typedef hkPointerMap<hkUint64, int> KeyToIndexMap;
	KeyToIndexMap m_keyToIndexMap;
	int m_collisions;
};

#endif // HKAI_ASTAR_DIRECTEDGRAPHEXPLICITCOST_BUILDER_H

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
