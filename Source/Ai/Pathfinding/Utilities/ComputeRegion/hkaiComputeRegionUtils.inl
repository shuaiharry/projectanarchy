/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/Algorithm/Sort/hkSort.h>
#include <Common/Base/Container/LocalArray/hkLocalBuffer.h>
#include <Common/Base/Algorithm/UnionFind/hkUnionFind.h>

class hkaiNavVolume;
class hkaiDirectedGraphExplicitCost;


template <typename MeshType, typename AreaType >
hkResult HK_CALL hkaiComputeRegionUtils::computeRegions( const MeshType& mesh, bool sortRegionsByArea, hkArrayBase<int>& regionsOut, int& numRegionsOut, hkArray<AreaType>* areasOut )
{
	typedef Adapter<MeshType> Graph;
	Graph graph(mesh);

	HK_TIME_CODE_BLOCK("computeRegions", HK_NULL);
	HK_ASSERT(0x27a98f58, regionsOut.getSize() >= graph.getNumNodes() );

	// Only need to expand this if we're sorting, otherwise we'll write directly to the output
	hkArray<int>::Temp regionsUnsorted;
	if(sortRegionsByArea)
	{
		hkResult res = regionsUnsorted.trySetSize( graph.getNumNodes() );
		HKAI_CHECK_SUCCESS(res);
	}

	// Flood fill and assign region Ids
	const int numNodes = graph.getNumNodes();
	int numRegions = 0;

	// Set up the union find
	{
		hkLocalBuffer<int> parents(numNodes);
		hkUnionFind unionFind( parents, numNodes);

		hkArray<int> regionSizes;
		hkArray<int> regions;

		unionFind.beginAddEdges();
		{
			for (int n=0; n<graph.getNumNodes(); n++)
			{
				const typename Graph::Node& node = graph.getNode(n);
				// Include any connected faces in this region
				for (int e=graph.edgesBegin(node); graph.edgesHasNext(node, e); e = graph.edgesNext(node, e) )
				{
					const typename Graph::Edge& edge = graph.getEdge(e);
					if ( edge.isExternal() )
					{
						continue;
					}

					if ( graph.hasOpposite(edge) )
					{
						unionFind.addEdge(n, graph.getOppositeNodeIndex(edge));
					}
				}
			}
		}
		unionFind.endAddEdges();

		hkResult assignRes = unionFind.assignGroups(regionSizes);
		HKAI_CHECK_SUCCESS(assignRes);

		hkResult sortRes = unionFind.sortByGroupId(regionSizes, regions);
		HKAI_CHECK_SUCCESS(sortRes);

		numRegions = regionSizes.getSize();

		// If not sorting, we can write directly to the output here.
		hkArrayBase<int>& regionsByNode = (sortRegionsByArea ? regionsUnsorted : regionsOut);

		int index = 0;
		for(int regionIdx = 0; regionIdx < numRegions; regionIdx++)
		{
			int regionSize = regionSizes[regionIdx];
			for (int i=0; i<regionSize; i++)
			{
				int nodeIndex = regions[index++];
				regionsByNode[nodeIndex] = regionIdx;
			}
		}
	}

	typename hkArray< RegionSort< typename Graph::Area > >::Temp regionAreas;
	if( sortRegionsByArea )
	{
		{
			hkResult res = regionAreas.trySetSize(numRegions);
			HKAI_CHECK_SUCCESS(res);
		}

		for (int r=0; r < numRegions; r++)
		{
			regionAreas[r].m_area = typename Graph::Area(0);
			regionAreas[r].m_idx = r;
		}

		for (int n=0; n < numNodes; n++ )
		{
			typename Graph::Area nodeArea = graph.getAreaForNode(n);

			int nodeRegion = regionsUnsorted[n];
			regionAreas[ nodeRegion ].m_area += nodeArea;
		}

		hkSort( regionAreas.begin(), regionAreas.getSize() );

		// Copy areas if required
		if( areasOut )
		{
			hkResult res = areasOut->trySetSize( numRegions );
			HKAI_CHECK_SUCCESS(res);

			for(int i = 0; i < numRegions; i++ )
			{
				(*areasOut)[i] = regionAreas[i].m_area;
			}
		}

		// Build remap index
		hkArray< int >::Temp regionRemap;
		{
			hkResult res = regionRemap.trySetSize( numRegions );
			HKAI_CHECK_SUCCESS(res);

			for (int r=0; r < regionRemap.getSize(); r++)
			{
				regionRemap[ regionAreas[r].m_idx ] = r;
			}
		}

		for (int n=0; n < numNodes; n++ )
		{
			regionsOut[n] = regionRemap[ regionsUnsorted[n] ];
		}
	}

	numRegionsOut = numRegions;
	return HK_SUCCESS;

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
