/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HK_AI_PATHFINDING_NAVMESH_PATH_SEARCH_PARAMETERS_H
#define HK_AI_PATHFINDING_NAVMESH_PATH_SEARCH_PARAMETERS_H

#include <Common/Base/Math/Vector/hkPackedVector3.h>
#include <Ai/Pathfinding/Astar/Search/hkaiSearchParams.h>

class hkaiAstarCostModifier;
class hkaiAstarEdgeFilter;

/// Parameters for controlling path searches on nav mesh.
/// This input data may apply to a batch of path requests.
struct hkaiNavMeshPathSearchParameters
{
	// +version(10)
	HK_DECLARE_REFLECTION();
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR, hkaiNavMeshPathSearchParameters );

		/// Flags to control how the output path should be computed.
	enum OutputPathFlags
	{
			/// Whether or not the path should be smoothed.
			/// If this flag isn't set, only the list of edges will be returned, and the other flags are ignored.
		OUTPUT_PATH_SMOOTHED = 1,

			/// Whether or not to project the path points onto the nav mesh faces.
		OUTPUT_PATH_PROJECTED = 2,

			/// Whether or not the face normals should be computed.
			/// If this flag is not setup, the search's up vector will be used for normals instead.
		OUTPUT_PATH_COMPUTE_NORMALS = 4
	};

		/// How user edge traversability should be checked.
	enum UserEdgeTraversalTestType
	{
			/// No traversal clearance tests are performed on user edges
		USER_EDGE_TRAVERSAL_TEST_DISABLED,

			/// User edges are tested for clearance just like normal edges
		USER_EDGE_TRAVERSAL_TEST_ENABLED,
	};

	hkaiNavMeshPathSearchParameters();

	hkaiNavMeshPathSearchParameters(hkFinishLoadedObjectFlag f);

	hkBool32 operator ==( const hkaiNavMeshPathSearchParameters& o ) const;

	//
	// Simplfied accessors
	//

	inline hkBool32 shouldPerformLineOfSightCheck() const;
	inline hkBool32 shouldSmoothPath() const;
	inline hkBool32 shouldProjectPath() const;
	inline hkBool32 shouldComputePathNormals() const;

	inline void setUp( hkVector4Parameter up );

		/// World up vector.
	hkPackedUnitVector<3> m_up; //+overridetype(hkInt16[3])

		/// Optional pointer to hkaiAstarCostModifier, which can be used to modify costs based on the hkaiAgentTraversalInfo
	const hkaiAstarCostModifier* m_costModifier; //+nosave

		/// Optional pointer to hkaiAstarEdgeFilter, which can be used to reject nav mesh edges
	const hkaiAstarEdgeFilter* m_edgeFilter; //+nosave

		/// When this flag is enabled, assertions will be raised for any invalid input values.
		/// Whether or not this flag is enabled, invalid input values will be removed/ignored.
	hkBool m_validateInputs; //+default(true)

		/// Controls various path output parameters. See hkaiNavMeshPathSearchParameters::OutputPathFlags for more information.
	hkFlags<OutputPathFlags, hkUint8> m_outputPathFlags; //+default(hkaiNavMeshPathSearchParameters::OUTPUT_PATH_SMOOTHED | hkaiNavMeshPathSearchParameters::OUTPUT_PATH_PROJECTED)


		/// How line of sight should be checked during the search.
	enum LineOfSightFlags 
	{
			/// Don't perform any line-of-sight check before A*
		NO_LINE_OF_SIGHT_CHECK = 0,
			
			/// Perform a line-of-sight check in order to try to avoid a full A* search.
			/// If the line-of-sight check between the start and end points succeeds, this is returned as the shortest path.
			/// If this flag is set, a line-of-sight check will be performed if no cost modifier is specified.
		CHECK_LINE_OF_SIGHT_IF_NO_COST_MODIFIER = 1,

			/// Enable the optional internal vertex-handling code during the line-of-sight check.
			/// This increases the cost of the line-of-sight check, but means that it will succeed in more cases,
			/// thus reducing the need for a full A* check in some cases.
			/// This has no effect if the CHECK_LINE_OF_SIGHT_IF_NO_COST_MODIFIER or CHECK_LINE_OF_SIGHT_ALWAYS bits aren't set.
		HANDLE_INTERNAL_VERTICES = 2,

			/// Always do a line-of-sight checked, regardless of whether or not a cost modifier is specified.
			/// This check should be disabled when using a cost modifier to increase the cost of certain path edges, 
			/// as it will find any straight-line path, even if a non-LOS path is actually preferred due to modified edge costs.
		CHECK_LINE_OF_SIGHT_ALWAYS = 4
	};

		/// Controls whether a line-of-sight check is performed before A*, and how the check is configured.
		/// See LineOfSightFlags for more details.
	hkFlags<LineOfSightFlags, hkUint8> m_lineOfSightFlags; //+default(hkaiNavMeshPathSearchParameters::CHECK_LINE_OF_SIGHT_IF_NO_COST_MODIFIER)

		/// Whether or not the search should use hierarchical A*
	hkBool m_useHierarchicalHeuristic; //+default(false)

		/// Whether or not the sphere and capsule radius checks are projected in the "up" direction.
	hkBool m_projectedRadiusCheck; //+default(true)

		/// Whether or not to test clearance on user edges
	hkEnum<UserEdgeTraversalTestType, hkUint8> m_userEdgeTraversalTestType; //+default(hkaiNavMeshPathSearchParameters::USER_EDGE_TRAVERSAL_TEST_DISABLED)

		/// Whether or not to use the more advanced distance calculation. This can result in smoother paths through long edges.
	hkBool m_useGrandparentDistanceCalculation; //+default(true)

		/// Weight to apply to the heuristic function. Values greater than 1
		/// result in faster but less accurate searches (non-optimal paths may
		/// be returned). Defaults to 1.
	hkReal m_heuristicWeight; //+default(1.0f)

		/// Threshold at which point simple smoothing and traversal checks are applied.
		/// If the difference between the character radius and the mesh erosion are less than this,
		/// the simpler faster checks are used.
	hkReal m_simpleRadiusThreshold; //+default(0.01f)

		/// Maximum path length allowed during A*. Any node beyond this distance will be ignored.
		/// Note that this distance is computed from the midpoint of the nav mesh edges and accounts for the cost modifier.
	hkReal m_maximumPathLength; //+default(HK_REAL_MAX)

		/// A* edges will be rejected if they do not intersect a sphere of this radius, centered on the start point.
		/// If this value is negative, the check will be skipped
	hkReal m_searchSphereRadius; //+default(-1.0f)

		/// A* edges will be rejected if they do not intersect any capsules of this radius, whose axes extend from the start point to each end point.
		/// If this value is negative, the check will be skipped
	hkReal m_searchCapsuleRadius; //+default(-1.0f)

	//
	// Memory control parameters
	// Reducing these numbers will reduce the size of memory allocations during the search,
	// but may cause the search to terminate early
	//
	
		/// Maximum memory size for the search.
	hkaiSearchParameters::BufferSizes m_bufferSizes;
		
		/// Maximum memory size for the hierarchical heuristic subsearch.
	hkaiSearchParameters::BufferSizes m_hierarchyBufferSizes;

		/// Internal determinism checks
	inline void checkDeterminism() const;
};

#include <Ai/Pathfinding/NavMesh/hkaiNavMeshPathSearchParameters.inl>

#endif // HK_AI_PATHFINDING_NAVMESH_PATH_SEARCH_PARAMETERS_H

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
