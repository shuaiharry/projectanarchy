/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef __VHAVOK_AI_NAVMESH_LINKER_HPP
#define __VHAVOK_AI_NAVMESH_LINKER_HPP

#include <Vision/Runtime/EnginePlugins/Havok/HavokAiEnginePlugin/vHavokAiIncludes.hpp>

class vHavokAiNavMeshInstance;

class vHavokAiNavMeshLinker
{
public:
  struct NavMeshLinkSetting
  {
    float m_edgeMatchTolerance;         ///< AABB expansion tolerance used when detecting nearby edges.
    float m_maxStepHeight;              ///< The maximum step height.
    float m_maxSeparation;              ///< Maximum allowed separation when considering connecting a pair of edges.
    float m_maxOverhang;                ///< Maximum allowed overhang when considering connecting a pair of edges.
    float m_cosPlanarAlignmentAngle;    ///< Minimum planar alignment required when considering connecting a pair of edges.
    float m_cosVerticalAlignmentAngle;  ///< Minimum vertical alignment required when considering connecting a pair of edges.
    float m_minEdgeOverlap;             ///< Minimum overlap required when considering connecting a pair of edges.
  };

  ///
  /// @name Constructor / Destructor
  /// @{
  ///

	VHAVOKAI_IMPEXP vHavokAiNavMeshLinker();
	VHAVOKAI_IMPEXP ~vHavokAiNavMeshLinker();

  ///
  /// @}
  ///

  /// \brief
  ///   Removes all nav meshes from the linker.
	VHAVOKAI_IMPEXP void ClearNavMeshes();

  /// \brief
  ///   Add nav mesh for linking.
	VHAVOKAI_IMPEXP void AddNavMesh(vHavokAiNavMeshInstance* navMesh);

  /// \brief
  ///   Stitches together the nav meshes that have been added previously.
  VHAVOKAI_IMPEXP bool LinkNavMeshes();

  /// \brief
  ///   Stitches together the nav meshes that have been added previously.
  ///
  /// \param navMeshLinkSetting
  ///   Custom nav mesh link settings.
  VHAVOKAI_IMPEXP bool LinkNavMeshes(const NavMeshLinkSetting& navMeshLinkSetting);

protected:
	hkArray<vHavokAiNavMeshInstance*>	m_navMeshes;
};

#endif	// __VHAVOK_AI_NAVMESH_BUILDER_HPP

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
