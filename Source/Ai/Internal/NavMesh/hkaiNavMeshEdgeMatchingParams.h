/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef HKAI_NAV_MESH_EDGE_MATCHING_PARAMS_H
#define HKAI_NAV_MESH_EDGE_MATCHING_PARAMS_H


extern const class hkClass hkaiNavMeshEdgeMatchingParametersClass;


		/// Tolerances for edge matching. This is used to connect edges then might not be exact, e.g., two stairs
struct hkaiNavMeshEdgeMatchingParameters
{
	// +version(9)
	HK_DECLARE_REFLECTION();
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI_NAVMESH, hkaiNavMeshEdgeMatchingParameters);

	hkaiNavMeshEdgeMatchingParameters();
	hkaiNavMeshEdgeMatchingParameters(hkFinishLoadedObjectFlag f) {}

		/// The maximum step height
	hkReal m_maxStepHeight;	//+default(.5f)
							//+hk.RangeReal(absmin=0,softmax=10)
							//+hk.Description("The maximum vertical distance allowed between edges.")
							//+hk.Semantics("DISTANCE")

		/// Maximum allowed separation when considering connecting a pair of edges
	hkReal m_maxSeparation;		//+default(.1f)
								//+hk.RangeReal(absmin=0,softmax=10)
								//+hk.Description("The maximum horizontal distance allowed between edges.")
								//+hk.Semantics("DISTANCE")

		/// Maximum allowed overhang when considering connecting a pair of edges
	hkReal m_maxOverhang;	//+default(1e-2f)
							//+hk.RangeReal(absmin=0,softmax=10)
							//+hk.Description("The maximum overhang allowed between edges.")
							//+hk.Semantics("DISTANCE")

		/// Maximum amount that a face can be behind the opposite edge.
	hkReal m_behindFaceTolerance;	//+default(1e-4f)
									//+hk.RangeReal(absmin=0,softmax=1)
									//+hk.Description("The maximum amount that a face can be behind the opposite edge.")
									//+hk.Semantics("DISTANCE")

		/// Minimum planar alignment required when considering connecting a pair of edges
	hkReal m_cosPlanarAlignmentAngle;	//+default(0.99619472f)
										//+hk.RangeReal(absmin=-1,absmax=1)
										//+hk.Description("The minimum planar alignment required when considering connecting a pair of edges.")
										//+hk.Semantics("COSINE_ANGLE")

		/// Minimum vertical alignment required when considering connecting a pair of edges
	hkReal m_cosVerticalAlignmentAngle;		//+default(.5f)
											//+hk.RangeReal(absmin=-1,absmax=1)
											//+hk.Description("The minimum vertical alignment required when considering connecting a pair of edges.")
											//+hk.Semantics("COSINE_ANGLE")

		/// Minimum overlap required when considering connecting a pair of edges
	hkReal m_minEdgeOverlap;	//+default(.02f)
								//+hk.RangeReal(absmin=0,softmin=1)
								//+hk.Description("The minimum overlap required when considering connecting a pair of edges.")

		/// Horizontal epsilon used for edge connection raycasts
	hkReal m_edgeTraversibilityHorizontalEpsilon;	//+default(.01f)
													//+hk.RangeReal(absmin=0,softmin=1)
													//+hk.Description("Internal Use. Horizontal traversabilty-test epsilon")

		/// Vertical epsilon used for edge connection raycasts
	hkReal m_edgeTraversibilityVerticalEpsilon;	//+default(.01f)
												//+hk.RangeReal(absmin=0,softmin=1)
												//+hk.Description("Internal Use. Vertical traversabilty-test epsilon")

		/// Minimum face normal alignment required when considering connecting a pair of edges (used for wall-climbing only)
	hkReal m_cosClimbingFaceNormalAlignmentAngle;	//+default(-0.5f),
													//+hk.RangeReal(absmin=-1,absmax=1)
													//+hk.Description("The minimum face normal alignment required when considering connecting a pair of edges.")
													//+hk.Semantics("COSINE_ANGLE")

		/// Minimum edge alignment required when considering connecting a pair of edges (used for wall-climbing only)
	hkReal m_cosClimbingEdgeAlignmentAngle;	//+default(0.99619472f)
											//+hk.RangeReal(absmin=-1,absmax=1)
											//+hk.Description("The minimum edge alignment required when considering connecting a pair of edges.")
											//+hk.Semantics("COSINE_ANGLE")

		/// In the case where two surfaces meet (e.g. a wall and a floor), if the angle between them is acute, they may be prevented from connecting.
		/// To alleviate these issues, the normals of the two surfaces can be rotated slightly towards each other. This normal correction will be
		/// performed if the angle between the surfaces is less than 90 degrees and greater than the angle (in radians) specified below. The default
		/// value is 87 degrees which means that, for example, the edges where a wall at an angle of 88 degrees meets a floor will still get connected.
		/// (This is used for wall-climbing only.)
	hkReal m_minAngleBetweenFaces;	//+default(87.f*HK_REAL_DEG_TO_RAD)
									//+hk.RangeReal(absmin=0,absmax=1.57)
									//+hk.Description("The minimum allowed angle (in radians) between faces when considering connecting a pair of edges.")
									//+hk.Semantics("ANGLE")

		/// Tolerance used to determine when edges are parallel for the purposes of "partially" matching them.
		/// In general, this doesn't need to be adjusted, but setting to HK_REAL_MAX will effectively disable this.
		
	hkReal m_edgeParallelTolerance; //+default(1e-5f)

		/// If enabled, a more accurate calculation is performed as an alternative to the m_edgeTraversibilityHorizontalEpsilon value.
		/// This can avoid missed connections on skinny triangles, but it also tends to result in connections in narrow corridors.
		/// This is disabled by default and should generally only be enabled in specific areas via override settings.
	hkBool m_useSafeEdgeTraversibilityHorizontalEpsilon; //+default(false)
};

#endif	// HKAI_NAV_MESH_EDGE_MATCHING_PARAMS_H

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
