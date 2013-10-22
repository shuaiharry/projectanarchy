/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HK_AI_PATHFINDING_UTILITIES_PATHFINDING_UTIL_H
#define HK_AI_PATHFINDING_UTILITIES_PATHFINDING_UTIL_H

#include <Ai/Pathfinding/Astar/hkaiAstarParameters.h>
#include <Ai/Pathfinding/Graph/hkaiAgentTraversalInfo.h>
#include <Ai/Pathfinding/Graph/hkaiGraphPathSearchParameters.h>
#include <Ai/Pathfinding/Character/hkaiPath.h>
#include <Ai/Pathfinding/NavMesh/hkaiNavMeshPathSearchParameters.h>

class hkaiDirectedGraphExplicitCost;
class hkaiAstarCostModifier;
class hkaiAstarEdgeFilter;
class hkaiStreamingCollection;
class hkaiNavMeshInstance;
class hkaiNavMesh;

/// A simple utility for pathfinding on a nav mesh.
/// Computes the edges that are traveled across, and (optionally) a smoothed path.
class hkaiPathfindingUtil
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiPathfindingUtil);
		HK_DECLARE_REFLECTION();
	
			/// Pathfinding input
		struct FindPathInput : public hkReferencedObject
		{
			// +version(10)
			HK_DECLARE_REFLECTION();
			HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR );

				/// Start point of the path
			hkVector4 m_startPoint;

				/// Goal point(s) of the path. The number of goal points is currently limited to 16 (see hkaiHierarchicalNavMeshHeuristic::MAX_GOALS)
			hkArray<hkVector4> m_goalPoints;

				/// Key of the starting face. m_startPoint should be contained in this face.
			hkaiPackedKey m_startFaceKey;

				/// Key(s) of the goal face(s). Each point in m_goalPoints should be contained in the corresponding face.
			hkArray<hkaiPackedKey> m_goalFaceKeys;

				/// The maximum number of A* iterations that should be performed.
				/// This defaults to a very large value, but can be reduced to avoid spending too much time per frame.
			int m_maxNumberOfIterations; //+default(100000)

				/// Width and filter information for a character.
				/// If an hkaiAstarCostModifier is specified, these can determine which edges are traversable,
				/// or change the cost of crossing certain faces
			hkaiAgentTraversalInfo m_agentInfo;
			
				/// Request search parameters
			hkaiNavMeshPathSearchParameters m_searchParameters;

			//
			// Pre-allocated memory buffers
			// If a pointer is not specified here, memory will be allocated during the search
			//

				/// Search buffer.
			hkaiSearchParameters::SearchBuffers m_searchBuffers; //+nosave
				/// Hierarchical search buffer.
			hkaiSearchParameters::SearchBuffers m_hierarchySearchBuffers; //+nosave

			FindPathInput( int numGoals = 0 );
			FindPathInput(hkFinishLoadedObjectFlag f);
			~FindPathInput()
			{

			}

				/// Internal determinism checks
			void checkDeterminism() const;

		private:
			// hidden because hkArray copy ctor is hidden
			FindPathInput(const FindPathInput& input); 

		};
		
			/// Pathfinding output
		struct FindPathOutput : public hkReferencedObject
		{
			HK_DECLARE_REFLECTION();
			HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR );

			FindPathOutput() {}
			FindPathOutput( hkFinishLoadedObjectFlag f);

				/// List of edges crossed during A*
			hkArray<hkaiPackedKey> m_visitedEdges;

				/// List of points for the shortest path
			hkArray<hkaiPath::PathPoint> m_pathOut;

				/// Results from A* search, e.g. # of iterations performed
			hkaiAstarOutputParameters m_outputParameters;

			void checkDeterminism() const;
		};

			/// Input information for hkaiPathfindingUtils::findNearestEdges() and hkaiPathfindingUtils::findNearestFaces()
		struct NearestFeatureInput
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR, NearestFeatureInput );
			NearestFeatureInput();

				/// Initial face key. If this is HKAI_INVALID_PACKED_KEY, the query will exit without doing anything.
			hkaiPackedKey m_startFaceKey; //+default( HKAI_INVALID_PACKED_KEY )

				/// Start point of the floodfill. If m_searchRadius is positive, the distance will be computed from here.
			hkVector4 m_startPoint;

				/// The maximum number of A* iterations that should be performed.
				/// This defaults to a very large value, but can be reduced to avoid spending too much time per frame.
			int m_maxNumberOfIterations;

				/// Maximum sizes used to store the open set and search state.
			hkaiSearchParameters::BufferSizes m_bufferSizes;

				/// Width and filter information for a character.
				/// If an hkaiAstarCostModifier is specified, these can determine which edges are traversal,
				/// or change the cost of crossing certain faces
			hkaiAgentTraversalInfo m_agentInfo;

			/// Optional pointer to hkaiAstarEdgeFilter, which can be used to reject nav mesh edges
			const hkaiAstarCostModifier* m_costModifier;

				/// Optional pointer to hkaiAstarEdgeFilter, which can be used to reject nav mesh edges
			const hkaiAstarEdgeFilter* m_edgeFilter;

				/// The maximum search region from the first search key - edges beyond this distance from the start won't be traversed.
				/// If this value is negative, then no check will be performed.
			hkReal m_searchRadius; //+default(-1.0f)

				/// Whether or not the sphere and capsule radius checks are projected in the "up" direction.
			hkBool m_projectedRadiusCheck; //+default(true) 

				/// Threshold at which point simple smoothing and traversal checks are applied.
				/// If the difference between the character radius and the mesh erosion are less than this,
				/// the simpler faster checks are used.
			hkReal m_simpleRadiusThreshold; //+default(0.01f)

				/// Up vector used for traversability checks.
			hkVector4 m_up;

		};

			
			/// A simple callback utility for use with findNearestEdges and findNearestFaces.
			/// These call edgeTraversed or faceTraversed respectively after each A* iteration.
			/// It is up to the user to terminate the search by returning 'false' from the callback;
			/// otherwise all faces/edges will be searched.
		class NearestFeatureCallback
		{
			public:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiPathfindingUtil::NearestFeatureCallback);
				virtual ~NearestFeatureCallback() {}

				virtual bool edgeTraversed(hkaiPackedKey edgeKey, const hkaiAgentTraversalInfo& agentInfo) = 0;

				virtual bool faceTraversed(hkaiPackedKey faceKey, const hkaiAgentTraversalInfo& agentInfo) = 0;
		};
			

			/// Input structure for findGraphPath()
		struct FindGraphPathInput
		{
			// +version(2)
			HK_DECLARE_REFLECTION();
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR, FindGraphPathInput );

			FindGraphPathInput();
			FindGraphPathInput(hkFinishLoadedObjectFlag f);

				/// Set the start node.
			void setStartNode( hkaiPackedKey startNode );

				/// Creates two start nodes: the specified startNodeKey, and the target of the edge specified by
				/// startEdgeKey. The initial cost for the first node will be fraction * oppositeEdgeCost, and the
				/// initial cost for the second node will be (1-fraction) * edgeCost.
			void setStartEdge( const hkaiStreamingCollection* collection, hkaiPackedKey startNodeKey, hkaiPackedKey startEdgeKey, hkReal fraction );

				/// Set the goal node.
			void setGoalNode( hkaiPackedKey goalNode );

				/// Creates two goal nodes: the specified goalNodeKey, and the target of the edge specified by
				/// goalEdgeKey. The final cost for the first node will be fraction * edgeCost, and the
				/// initial cost for the second node will be (1-fraction) * oppositeEdgeCost.
			void setGoalEdge( const hkaiStreamingCollection* collection, hkaiPackedKey goalNodeKey, hkaiPackedKey goalEdgeKey, hkReal fraction );

				/// Keys of the starting nodes. Only the first is considered by the hierarchical heuristic, so make sure
				/// they're nearby
			hkArray<hkaiPackedKey> m_startNodeKeys;

				/// Initial costs for the corresponding start nodes
			hkArray<hkReal> m_initialCosts;

				/// Key of the goal node.
			hkArray<hkaiPackedKey> m_goalNodeKeys;

				/// Final costs, added to the path cost for the corresponding goal nodes
			hkArray<hkReal> m_finalCosts;

				/// The maximum number of A* iterations that should be performed.
				/// This defaults to a very large value, but can be reduced to avoid spending too much time per frame.
			int m_maxNumberOfIterations; //+default(100000)

				/// Width and filter information for a character.
				/// If an hkaiAstarCostModifier is specified, these can determine which edges are traversable,
				/// or change the cost of crossing certain faces
			hkaiAgentTraversalInfo m_agentInfo;

				/// Request search parameters
			hkaiGraphPathSearchParameters m_searchParameters;

			//
			// Pre-allocated memory buffers
			// If a pointer is not specified here, memory will be allocated during the search
			//

				/// Search buffer.
			hkaiSearchParameters::SearchBuffers m_searchBuffers; //+nosave
				/// Hierarchical search buffer.
			hkaiSearchParameters::SearchBuffers m_hierarchySearchBuffers; //+nosave
		};

		/// Output structure for findGraphPath()
		struct FindGraphPathOutput
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR, FindGraphPathOutput );

			/// Sequence of nodes in returned path
			hkArray<hkaiPackedKey> m_pathNodes;

			/// Results from A* search, e.g. # of iterations performed
			hkaiAstarOutputParameters m_outputParameters;
		};


			/// Compute a path from the points given in the input, and store the results in the output.
			/// If FindPathInput::m_startFaceKey is set to HKAI_INVALID_PACKED_KEY, or FindPathInput::m_goalFaceKeys is empty,
			/// the search will early out without performing any A* iterations.
		static void HK_CALL findPath(const hkaiStreamingCollection& collection, const FindPathInput& input, FindPathOutput& output);
		
			/// Same as above, but using a single nav mesh.
			/// Hierarchical heuristic is not supported
		static void HK_CALL findPath(const hkaiNavMesh& navMesh, const FindPathInput& input, FindPathOutput& output);
		
			/// Check whether or not a path exists without turning a corner.
			/// If FindPathInput::m_startFaceKey or FindPathInput::m_goalFaceKeys[goalIndex] is set to HKAI_INVALID_PACKED_KEY,
			/// the check will early out without performing any iterations.
		static bool HK_CALL checkLineOfSight(const hkaiStreamingCollection& collection, const FindPathInput& input, int goalIndex, FindPathOutput* output = HK_NULL);

			/// Walk out from the start point and fire a callback for each edge crossed.
			/// The search will only terminate when
			/// - NearestFeatureCallback::edgeTraversed returns false
			/// - All edges within NearestFeatureInput::m_searchRadius of the starting point are explored (if the radius is positive)
			/// - The number of iterations in NearestFeatureInput::m_maxNumberOfIterations is reached
		static void HK_CALL findNearestEdges( const hkaiStreamingCollection& collection, const NearestFeatureInput& input, NearestFeatureCallback* callback, hkaiAstarOutputParameters* output = HK_NULL);

			/// Walk out from the start point and fire a callback for each face crossed.
			/// The search will only terminate when
			/// - NearestFeatureCallback::faceTraversed returns false
			/// - All faces within NearestFeatureInput::m_searchRadius of the starting point are explored (if the radius is positive)
			/// - The number of iterations in NearestFeatureInput::m_maxNumberOfIterations is reached
		static void HK_CALL findNearestFaces( const hkaiStreamingCollection& collection, const NearestFeatureInput& input, NearestFeatureCallback* callback, hkaiAstarOutputParameters* output = HK_NULL);

			/// Utility function used internally to determine whether a path on the initial face can be used
		static hkBool32 HK_CALL _checkInitialFace(const hkaiStreamingCollection& collection, const FindPathInput& input, int& closestGoalIndex );



			/// Find a path between two nodes in the mesh cluster graphs.
		static void HK_CALL findGraphPath(const hkaiStreamingCollection& collection, const FindGraphPathInput& input, FindGraphPathOutput& output, const hkaiStreamingCollection* hierarchyCollection = HK_NULL);

			/// Same as above, but using a single mesh cluster graph.
		static void HK_CALL findGraphPath(const hkaiDirectedGraphExplicitCost& graph, const FindGraphPathInput& input, FindGraphPathOutput& output, const hkaiDirectedGraphExplicitCost* clusterGraph = HK_NULL);

	protected:

			/// What type of feature the query is collecting.
		enum NearestFeatureType
		{
			CALLBACK_EDGE,
			CALLBACK_FACE
		};

		static void HK_CALL _findNearestFeature(const hkaiStreamingCollection& collection, const NearestFeatureInput& input, NearestFeatureCallback* callback, NearestFeatureType callbackType, hkaiAstarOutputParameters* output = HK_NULL);

};

#endif // HK_AI_PATHFINDING_UTILITIES_PATHFINDING_UTIL_H

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
