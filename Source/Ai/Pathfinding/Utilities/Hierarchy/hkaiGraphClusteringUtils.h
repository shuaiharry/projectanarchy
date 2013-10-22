/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef HKAI_GRAPH_CLUSTERING_UTILS_H
#define HKAI_GRAPH_CLUSTERING_UTILS_H

#include <Ai/Pathfinding/hkaiBaseTypes.h>
#include <Ai/Pathfinding/Graph/hkaiDirectedGraphExplicitCost.h>

class hkaiDirectedGraphExplicitCostBuilder;
class hkaiDirectedGraphExplicitCost;
class hkcdDynamicAabbTree;
class hkaiStreamingCollection;

	/// Graph clustering utilities.
class hkaiGraphClusteringUtils
{
public:
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI, hkaiGraphClusteringUtils);

	enum 
	{
		INVALID_REGION_INDEX = -1,
	};

		/// How the closest cluster center to a node is determined.	
	enum ClusterMethod
	{
			/// Naive linear search.
		CLUSTER_NAIVE,
			/// Build an AABB tree and query that.
		CLUSTER_TREE
	};

		/// How the edge cost in the cluster graph is determined.
	enum ClusterCostCalculation
	{
		/// Edge cost is the path distance between the closest nodes to each center.
		/// This is fast but might be inaccurate.
		COST_CENTER_TO_CENTER,

		/// Edge cost is the average cost between all pairs of nodes. This can be expensive to compute.
		COST_ALL_PAIRS,

		/// Edge cost is the average cost from the closest node to the center to all nodes in the other cluster.
		COST_CENTER_TO_ALL,
	};

		/// Cluster generation settings.
	struct ClusterSettings
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI, ClusterSettings);
		ClusterSettings();
		hkEnum<ClusterMethod, hkUint8> m_method;	
		hkEnum<ClusterCostCalculation, hkUint8> m_costCalculation;
		hkBool m_symmetricCosts;
		int m_nodesPerCluster;

		//
		// Multithreading options
		//

			/// Clustering will only be split until multiple jobs if the region is larger than this
		int m_minSplitSize; //+default(100)

			/// Number of jobs to split into
		int m_numJobs; //+default(16)

		class hkJobQueue* m_jobQueue;
		class hkJobThreadPool* m_threadPool;
	};

	typedef hkaiDirectedGraphExplicitCost::NodeIndex NodeIndex;
	static hkaiDirectedGraphExplicitCost* HK_CALL clusterGraph( const hkaiDirectedGraphExplicitCost* graph, const ClusterSettings& settings, hkArray<int>& clusterIndicesOut );
	static hkaiStreamingCollection* HK_CALL clusterCollection( hkaiStreamingCollection* collection, const ClusterSettings& settings );
	static int HK_CALL getNodeRegions( const hkaiDirectedGraphExplicitCost* graph, hkArray<int>& groupsOut );
	static void HK_CALL clusterGraphRegion( const hkaiDirectedGraphExplicitCost* graph, const ClusterSettings& settings, hkArrayBase<NodeIndex>& nodeIndices, hkArrayBase<int>& clusterIndicesOut, hkArrayBase<hkVector4>& centersOut);

	static void HK_CALL _kMeansUpdateCenters( const hkaiDirectedGraphExplicitCost* graph, const hkArrayBase<NodeIndex>& nodeIndices, const hkArrayBase<int>& clusterIndicesOut, hkArrayBase<hkVector4>& centersOut);
	static void HK_CALL _kMeansUpdateClustersNaive( const hkaiDirectedGraphExplicitCost* graph, const hkArrayBase<NodeIndex>& nodeIndices, hkArrayBase<int>& clusterIndicesOut, const hkArrayBase<hkVector4>& centersOut );
	static void HK_CALL _kMeansUpdateClustersTree( const hkaiDirectedGraphExplicitCost* graph, const ClusterSettings& settings, const hkArrayBase<NodeIndex>& nodeIndices, hkArrayBase<int>& clusterIndicesOut, const hkArrayBase<hkVector4>& centersOut );
	static void HK_CALL _kMeansUpdateSingleCluster( const hkaiDirectedGraphExplicitCost* graph, const hkArrayBase<NodeIndex>& nodeIndices, hkArrayBase<int>& clusterIndicesOut, const hkArrayBase<hkVector4>& centersOut, const hkcdDynamicAabbTree* tree, int n );

	static hkaiDirectedGraphExplicitCost* _buildClusterGraph( const hkaiDirectedGraphExplicitCost* graph, const ClusterSettings& settings, const hkArrayBase<NodeIndex>& nodeIndices, const hkArrayBase<hkVector4>& centers);
	static hkReal _getClusterPathCost( const hkaiDirectedGraphExplicitCost* graph, const hkcdDynamicAabbTree* tree, const ClusterSettings& settings, int clusterA, int clusterB, const hkArrayBase<NodeIndex>& nodeIndices, const hkArrayBase<hkVector4>& centers);

	/// Clean up any disconnected clusters
	static void HK_CALL fixupClusters( const hkaiDirectedGraphExplicitCost* graph, const ClusterSettings& settings, hkArray<NodeIndex>& nodeIndices, hkArray<hkVector4>& centers);
	/// Ensure each node has a neighbor in the same cluster (doesn't create new clusters)
	/// Returns true if any modifications were made.
	static bool HK_CALL _checkHasNeighborInCluster(const hkaiDirectedGraphExplicitCost* graph, const ClusterSettings& settings, hkArray<NodeIndex>& nodeIndices, hkArray<hkVector4>& centers);
	/// Ensures there's a path from each node in its cluster to each other node. If not, split the cluster (can create new clusters).
	/// Returns true if any modifications were made.
	static bool HK_CALL _checkClustersAreConnected(const hkaiDirectedGraphExplicitCost* graph, const ClusterSettings& settings, hkArray<NodeIndex>& nodeIndices, hkArray<hkVector4>& centers);

	/// Returns the number of connected components in the cluster
	static int HK_CALL _isClusterConnected( const hkaiDirectedGraphExplicitCost* graph, const hkArray<hkaiDirectedGraphExplicitCost::NodeIndex>& nodesInCluster, hkArray<int>& subClusters );
	static void HK_CALL _fixDisconnectedClusters( const hkaiDirectedGraphExplicitCost* graph, const hkArray<hkaiDirectedGraphExplicitCost::NodeIndex>& nodesInCluster, const hkArray<int>& subClusters, int numSubClusters, hkArray<NodeIndex>& nodeIndices, hkArray<hkVector4>& centers );
};

#endif // HKAI_GRAPH_CLUSTERING_UTILS_H

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
