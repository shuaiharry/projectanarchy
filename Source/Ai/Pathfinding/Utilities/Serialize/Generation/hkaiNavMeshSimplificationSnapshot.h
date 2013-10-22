/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef HK_AI_NAV_MESH_SIMP_SNAPSHOT_H
#define HK_AI_NAV_MESH_SIMP_SNAPSHOT_H

#include <Ai/Internal/NavMesh/hkaiNavMeshGenerationSettings.h>
#include <Common/Base/Container/BitField/hkBitField.h>

extern const hkClass hkaiNavMeshSimplificationSnapshotClass;

struct hkGeometry;
class hkaiVolume;

	/// A simple container class for saving the data necessary to reproduce nav mesh simplification
	/// without needing to run generation.
class hkaiNavMeshSimplificationSnapshot
{
public:
	HK_DECLARE_REFLECTION();
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AI_NAVMESH, hkaiNavMeshSimplificationSnapshot);

	hkaiNavMeshSimplificationSnapshot( );
	hkaiNavMeshSimplificationSnapshot(hkFinishLoadedObjectFlag f);
	~hkaiNavMeshSimplificationSnapshot();

	void setRaycasterInformation( const hkGeometry* triMesh, const hkArrayBase< hkRefPtr<const hkaiVolume> >& carvers, const hkBitField& cuttingTriangles);
	void setMeshAndSettings( const hkaiNavMesh& mesh, const hkaiNavMeshGenerationSettings& settings);

		/// Save this snapshot to the filename specified in the settings.
	void save() const;

		/// Converted trimesh, different from the nav mesh generation input
	hkRefPtr< const hkGeometry > m_geometry;

		/// Possibly different from the nav mesh generation settings (one extra for the bounds AABB)
	hkArray< hkRefPtr<const hkaiVolume> > m_carvers;

		/// Bitfield to indicate which triangles are cutting.
	hkBitField m_cuttingTriangles;

		/// Nav Mesh generation input
	struct hkaiNavMeshGenerationSettings m_settings;

		/// The nav mesh before it gets simplified.
	hkRefPtr< const hkaiNavMesh > m_unsimplifiedNavMesh;

};

#endif // HK_AI_NAV_MESH_SIMP_SNAPSHOT_H

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
