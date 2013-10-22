/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef HKAI_NAVIGATION_MESH_GENERATION_SETTINGS_H
#define HKAI_NAVIGATION_MESH_GENERATION_SETTINGS_H

#include <Ai/Internal/NavMesh/hkaiNavMeshSimplificationUtils.h>
#include <Ai/Internal/NavMesh/hkaiNavMeshEdgeMatchingParams.h>
#include <Ai/Pathfinding/Common/hkaiMaterialPainter.h>
#include <Common/Base/Reflection/Attributes/hkAttributes.h>

extern const class hkClass hkaiNavMeshGenerationSettingsOverrideSettingsClass;

extern const class hkClass hkaiNavMeshGenerationSettingsRegionPruningSettingsClass;

extern const class hkClass hkaiNavMeshGenerationSettingsMaterialConstructionPairClass;

extern const class hkClass hkaiNavMeshGenerationSettingsClass;

class hkaiVolume;

/// This input structure controls all of the major settings for automatic nav mesh generation
struct hkaiNavMeshGenerationSettings
{
	// +version(25)
	HK_DECLARE_REFLECTION();
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI_NAVMESH, hkaiNavMeshGenerationSettings);

	/// Flags to determine whether the triangles for a material are walkable, extruded, etc.
	enum ConstructionFlagsBits
	{
			/// If this bit is specified for a material, the triangles with the material will be considered for the walkable surface.
		MATERIAL_WALKABLE = 1,
	
			/// If this bit is specified for a material, the triangles with the material will be extruded and used to cut the walkable surfaces
		MATERIAL_CUTTING = 2,

			/// The default settings for materials. Triangles with these materials will be both considered for the walkable surface and used for cutting.
		MATERIAL_WALKABLE_AND_CUTTING = 3
	};

	typedef hkFlags<ConstructionFlagsBits, hkUint32> ConstructionFlags;

		/// Storage for material properties. All triangles with the specified index will be treated with the specified flags.
	struct MaterialConstructionPair
	{
		HK_DECLARE_REFLECTION();
		int m_materialIndex;
		ConstructionFlags m_flags;
	};

	typedef hkaiNavMeshEdgeMatchingParameters EdgeMatchingParameters;


		/// How the character width (m_minCharacterWidth) is used to remove sections of the nav mesh.
	enum CharacterWidthUsage
	{
			/// The character diameter is ignored during nav mesh generation.
		NONE,

			/// Edges which are less than the m_minCharacterWidth and adjacent to bounday edges are blocked.
			/// This can help prune out areas that are connected by thin "bridges"
		BLOCK_EDGES,

			/// All boundaries of the nav mesh are moved "inwards" by .5*m_minCharacterWidth.
			/// This allows characters less than or equal to this width to use simpler (and faster) checks during A* and path smoothing.
			/// This feature is in beta. Behavior may change in future releases.
		SHRINK_NAV_MESH
	};

		/// Localized or material-based override settings for nav mesh generation
	struct OverrideSettings
	{
		// +version(3)
		HK_DECLARE_REFLECTION();
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI_NAVMESH, OverrideSettings);

		OverrideSettings();
		OverrideSettings(hkFinishLoadedObjectFlag f);
		~OverrideSettings();

			/// Initialize all the values in the override settings to match those of a master "base" settings.
			/// This is useful for when you are creating an override for one feature (ex. simplification settings),
			/// but do not want to affect the others (ex. edge matching)
		OverrideSettings( const hkaiNavMeshGenerationSettings& baseSettings );

			/// Sets the simplification values to avoid almost all simplification.
			/// This is often used to avoid simplifying borders when setting up streaming.
		void setForBorderPreservation(); 

			/// Volume within which override settings apply
		hkRefPtr<const hkaiVolume> m_volume;

			/// Material to which override settings apply. This is only used if the m_volume is HK_NULL.
		int m_material;

			/// Override the m_characterWidthUsage for the region.
			/// Currently the only supported use for this is preventing small edge blocking in an area.
			/// This is useful when splitting up nav mesh generation into tiles, to avoid incorrect blockage near the
			/// tile boundary.
		hkEnum<CharacterWidthUsage, hkUint8> m_characterWidthUsage; //+default(hkaiNavMeshGenerationSettings::BLOCK_EDGES)

			/// Max walkable slope in the region or for the material.
		hkReal m_maxWalkableSlope;

			/// Edge matching parameters to be used in the region or for the material.
		hkaiNavMeshEdgeMatchingParameters m_edgeMatchingParams;

			/// Simplification settings to be used in the region or for the material.
		struct hkaiNavMeshSimplificationUtils::Settings m_simplificationSettings;
	};

	hkaiNavMeshGenerationSettings();
	hkaiNavMeshGenerationSettings(hkFinishLoadedObjectFlag f);
	~hkaiNavMeshGenerationSettings();

	void setConstructionPropertiesForMaterial( int materialIndex, hkUint32 constructionFlags );

	void setSimplificationSettingsForMaterial( int materialIndex, const hkaiNavMeshSimplificationUtils::Settings& settings );

	/// The height of the character used to extrude the geometry when generating the nav mesh. The resulting nav mesh AABB is also expanded by this amount.
	hkReal m_characterHeight;		//+default(1.75f)
									//+hk.RangeReal(absmin=0,softmax=10)
									//+hk.Description("The character height used to extrude the input geometry.")

	/// The up vector for the character
	hkVector4 m_up;

	/// The size of the grid to which the input vertices are snapped
	hkReal m_quantizationGridSize;

	/// The maximum allowable surface given in radians.
	hkReal m_maxWalkableSlope;		//+default( HK_REAL_PI / 3.0f )
									//+hk.RangeReal(absmin=0,absmax=1.57)
									//+hk.Description("The maximum walkable slope. Stored in radians.")
									//+hk.Semantics("ANGLE")

		/// The winding of the walkable triangles with respect to the up vector.
		/// Internally, Havok deals with counter-clockwise winding.
		/// If clockwise winding is specified, the winding will be flipped on a copy of the input geometry.
	enum TriangleWinding
	{
			/// Walkable triangles are wound counter-clockwise w.r.t. the up vector.
		WINDING_CCW,
			
			/// Walkable triangles are wound clockwise w.r.t. the up vector.
		WINDING_CW,
	};

	hkEnum< TriangleWinding, hkUint8 > m_triangleWinding; //+default(hkaiNavMeshGenerationSettings::WINDING_CCW)

	/// The threshold for removal of 'degenerate' triangles after triangulation (but before shared edge identification and convexification)
	hkReal m_degenerateAreaThreshold;

	/// The threshold for removal of 'degenerate' triangles after triangulation (but before shared edge identification and convexification)
	hkReal m_degenerateWidthThreshold;

	/// The threshold for convex piece merging (Hertel-Mehlhorn). Max (reflex) angle in degrees allowed between adjacent edges
	hkReal m_convexThreshold;

	///	For convex piece merging (Hertel-Mehlhorn), only merges that result in faces with less than this parameter will be accepted.
	/// Note that in the final mesh, faces with more edges may be created due to splitting of edges during edge matching or generating streaming information.
	int m_maxNumEdgesPerFace; //+default(255)

	/// The edge matching params
	hkaiNavMeshEdgeMatchingParameters m_edgeMatchingParams;

		/// How conflicting edges are prioritized.
	enum EdgeMatchingMetric
	{
		/// Order by overlap first - this prioritizes large overlapping edges
		ORDER_BY_OVERLAP = 1,

		/// Order by distance first - this prioritizies close edges
		ORDER_BY_DISTANCE = 2,
	};

	/// The order by which potential edge pairs are sorted. 
	hkEnum<EdgeMatchingMetric, hkUint32> m_edgeMatchingMetric; //+default(hkaiNavMeshGenerationSettings::ORDER_BY_DISTANCE)

	/// Number of passes of the edge matching code.
	int m_edgeConnectionIterations;		//+default(2)
										//+hk.RangeInt32(absmin=1,softmax=10)

		/// Parameters to control how regions are removed from the nav mesh
	struct RegionPruningSettings
	{
		// +version(2)
		HK_DECLARE_REFLECTION();
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI_NAVMESH, RegionPruningSettings);

		RegionPruningSettings();
		RegionPruningSettings( hkFinishLoadedObjectFlag f )
			: m_regionSeedPoints(f)
		{

		}

			/// Computes the amount to expand face AABBs by
		void getFaceAabbExpansion( hkVector4Parameter up, hkVector4& expansion ) const;

			/// Determines whether the region's AABB is adjacent to the mesh boundary.
		static bool HK_CALL isRegionNearBoundary( const hkAabb& regionAabb, const hkAabb& meshBoundary );

			/// Any regions with area less than this threshold will be automatically removed
		hkReal m_minRegionArea;		//+default(5.0f)
									//+hk.RangeReal(absmin=0)

			/// Minimum acceptable distance to the region seed points (below).
			/// If no face in a region is within the minimum distance to at least one seed point, the region will be removed.
		hkReal m_minDistanceToSeedPoints; //+default(1.0f)

			/// Tolerance to preserve small regions near the boundary AABB of the nav mesh.
			/// Any region within this distance of the nav mesh AABB will always be preserved.
			/// This is recommended e.g. when setting up streaming, as small regions may connect to adjacent sections.
			/// Note that this may result in some regions being left in the nav mesh unexpectedly.
			/// To disable this feature, set this to 0 or a negative value.
		hkReal m_borderPreservationTolerance;	//+default(.1f)
												//+hk.RangeReal(softmin=-10, softmax=10)

			/// Whether or not border preservation should be applied in the "up" direction.
			/// Generally this should be left false to avoid unexpected regions above or below the main mesh.
			/// However it may be desirable if connecting regions vertically, e.g. stories in a skyscraper.
		hkBool m_preserveVerticalBorderRegions; //+default(false);

			/// Whether or not to perform a pruning pass before CSG and triangulation.
			/// This is highly recommended because it can eliminate large amounts of triangles from being processed.
		hkBool m_pruneBeforeTriangulation; //+default(true)

			/// If points are specified here, any regions not connected to the points will be removed
		hkArray<hkVector4> m_regionSeedPoints;
	};

		/// Parameters to control how regions are removed from the nav mesh.
	RegionPruningSettings m_regionPruningSettings;


		/// Generation settings that are specific to wall-climbing generation.
	struct WallClimbingSettings
	{
			//+version(1)
		HK_DECLARE_REFLECTION();
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI_NAVMESH, WallClimbingSettings);

		WallClimbingSettings();
		WallClimbingSettings( hkFinishLoadedObjectFlag f )
		{
		}
			
			/// Whether or not to enable wall-climbing generation
		hkBool m_enableWallClimbing; //+default(false)

			/// Whether or not to exclude triangles which are walkable for non-climbing nav mesh generation.
			/// This can be useful to generate disjoint walking and climbing meshes from the same input geometry.
		hkBool m_excludeWalkableFaces; //+default(false)
	};

		/// Generation settings that are specific to wall-climbing generation.
	WallClimbingSettings m_wallClimbingSettings;

	/// Bounding AABB - if this box is set to be non-empty, then the generated nav mesh will be confined to this bounding volume
	hkAabb m_boundsAabb;

	/// Carver volumes used to remove sections of the input mesh - see "Reducing the Detail of Input Geometry" in the User Guide for more details.
	/// If the carver fully contains faces from the geometry being carved, then those faces will be removed from processing.
	/// In places where a carver intersects a walkable surface, the carver will cut out the walkable surface such that the character cannot enter the area.
	hkArray< hkRefPtr<const hkaiVolume> > m_carvers;

	/// Painters set the material for faces that they overlap. This can be used to ensure a specific outline appears in the final nav mesh.
	hkArray< hkRefPtr<const hkaiMaterialPainter> > m_painters;

	/// Callback to resolve material painting where two or more material painters overlap.
	/// If this callback is NULL, then the painter with the highest index in the m_painters array will take precedence.
	hkaiMaterialPainter::ResolveOverlapCallback* m_painterOverlapCallback; //+nosave

	/// Default ConstructionFlag
	ConstructionFlags m_defaultConstructionProperties; //+default( hkaiNavMeshGenerationSettings::MATERIAL_WALKABLE_AND_CUTTING );

	/// Material-to-ConstructionFlags map - determines which properties should be used for which materials.
	/// If the material index is not found in the map, then m_defaultConstructionProperties is used
	hkArray<struct hkaiNavMeshGenerationSettings::MaterialConstructionPair> m_materialMap;

	/// Set if the input vertices should be welded before nav mesh construction
	hkBool m_weldInputVertices;

	/// The threshold to use when welding input geometry
	hkReal m_weldThreshold;

	/// The minimum character diameter that will be used on this nav mesh.
	/// This value is used to prune the nav mesh, removing regions that are untraversable based on this value,
	/// or, if erosion is enabled, to shrink the nav mesh by half of the specified value.
	hkReal m_minCharacterWidth;		//+default(0.0f)
									//+hk.RangeReal(absmin=0,softmax=10)

	/// How the minimum character width (m_minCharacterWidth) is used to simplify the nav mesh.
	hkEnum<CharacterWidthUsage, hkUint8> m_characterWidthUsage; //+default(hkaiNavMeshGenerationSettings::BLOCK_EDGES)


	/// If set, simplification will be applied using hkaiNavMeshSimplificationUtils
	hkBool m_enableSimplification;


	/// Holds the settings that control the amount of simplification to apply
	struct hkaiNavMeshSimplificationUtils::Settings m_simplificationSettings;

	//
	// Face triangulation
	//
	
		/// Reserved material used internally to mark faces which are going to be carved out.
		/// Set this to an unused material. Defaults to 0x8fffffff.
		/// Note: this field is only used for deprecated generation and will be removed in a future release.
	int m_carvedMaterialDeprecated;

		/// Reserved material used internally to mark cutting faces which are going to be carved out.
		/// Set this to an unused material, different from m_carvedMaterial. Defaults to 0x8ffffffe.
		/// Note: this field is only used for deprecated generation and will be removed in a future release.
	int m_carvedCuttingMaterialDeprecated;

	/// If set, the edge ordering within some faces will be changed, to 
	/// improve the accuracy of traversability checks.
	hkBool m_setBestFaceCenters; //+default(true)

	//
	// Debugging Information
	//

		/// Whether or not to perform internal consistency checks after CSG
	hkBool m_checkEdgeGeometryConsistency; //+default(false)
	
		/// If this flag is set, the input data (hkGeometry and Settings) will be serialized out.
	hkBool m_saveInputSnapshot;

		/// Base filename of the saved input snapshot. 
	hkStringPtr m_snapshotFilename;

	//
	// Override settings
	//

		/// Override settings based on volume regions or material
	hkArray<struct OverrideSettings> m_overrideSettings;

		/// Checks whether not the face (or triangle) is under the max walkable slope.
	hkBool32 isWalkableBySlope(hkVector4Parameter normal, const hkArrayBase<hkVector4>& points, int material ) const;
		
		/// Gets the value of the character width usage for the point and the face material
	CharacterWidthUsage getCharacterWidthUsage(hkVector4Parameter point, int material) const;

		/// Gets the value of the max walkable slope for the points and the triangle material
	hkReal getMaxWalkableSlope(const hkArrayBase<hkVector4>& points, int material) const;
	
	const EdgeMatchingParameters& getEdgeMatchingParameters(const hkArrayBase<hkVector4>& points, int materialA, int materialB ) const;
	const hkaiNavMeshSimplificationUtils::Settings& getSimplificationSettings(const hkArrayBase<hkVector4>& points, int materialSettingsIndex = -1) const;

protected:
		/// Returns the index of the override settings to use for a given set of points and material, or -1 if the global settings should be used instead.
		/// If the points overlap multiple override settings volumes and/or the material is overridden then the one with the smallest index will be returned.
		/// In other words, it is up to the user to sort the elements in m_overrideSettings to give the desired priority.
	int getFirstOverrideSettingIndex(const hkArrayBase<hkVector4>& points, int materialA, int materialB, bool checkMaterial = true) const;
};

#endif	// HKAI_NAVIGATION_MESH_GENERATION_SETTINGS_H

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
