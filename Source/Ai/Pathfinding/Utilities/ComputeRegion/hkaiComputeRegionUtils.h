/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef HKAI_COMPUTE_REGION_UTILS_H
#define HKAI_COMPUTE_REGION_UTILS_H

#include <Ai/Pathfinding/hkaiBaseTypes.h>

	/// Utility to find connected components of a graph.
namespace hkaiComputeRegionUtils
{
		/// Constants for regions.
	enum Constants
	{
		INVALID_REGION_INDEX = -1,
	};
	
		/// Helper class. Never actually instantiated.
	template<typename T>
	struct Adapter
	{
		Adapter(const T&) { m_dummy = 0; }
		
		typedef int Node;
		typedef int Edge;
		typedef int NodeIndex;
		typedef int EdgeIndex;

		int getNumNodes( ) const { return -1; }
		int getNumEdges( ) const { return -1; }
		const Node& getNode( NodeIndex n ) const {return m_dummy; }
		const Edge& getEdge( EdgeIndex e ) const {return m_dummy; }

		NodeIndex edgesBegin( const Node& node) const { return -1; }
		bool edgesHasNext(const Node& node, NodeIndex e) const { return false; }
		NodeIndex edgesNext( const Node& node, NodeIndex e ) const { return e+1; }
		bool hasOpposite( const Edge& edge) const {return false; }
		NodeIndex getOppositeNodeIndex( const Edge& edge) const {return 0; }

		typedef int Area;
		int getAreaForNode( NodeIndex n ) const { return 1; }

		int m_dummy;

	};

		/// Sort operation.
	template< typename Area >
	struct RegionSort
	{
		HK_DECLARE_POD_TYPE();

		Area m_area;
		int m_idx;

		HK_FORCE_INLINE bool operator < ( const RegionSort& other ) const { return other.m_area < m_area; } // Note actually > not < to sort in descending order
	};

		/// Find connected components of the mesh (or graph).
	template<typename MeshType, typename AreaType>
	static hkResult HK_CALL computeRegions( const MeshType& mesh, bool sortRegionsByArea, hkArrayBase<int>& regionsOut, int& numRegionsOut, hkArray<AreaType>* areasOut = HK_NULL);
}


#include <Ai/Pathfinding/Utilities/ComputeRegion/hkaiComputeRegionUtils.inl>

#endif //HKAI_COMPUTE_REGION_UTILS_H

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
