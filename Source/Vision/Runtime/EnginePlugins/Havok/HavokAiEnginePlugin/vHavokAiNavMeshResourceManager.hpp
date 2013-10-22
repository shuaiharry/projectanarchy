/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef __VHAVOK_AI_NAVMESH_RESOURCE_MANAGER_HPP
#define __VHAVOK_AI_NAVMESH_RESOURCE_MANAGER_HPP

#include <Vision/Runtime/EnginePlugins/Havok/HavokAiEnginePlugin/vHavokAiIncludes.hpp>

class vHavokAiNavMeshResource;

/// \brief
///   Resource manager for nav mesh resources. 
///
/// A global instance of this manager can be accessed via vHavokAiNavMeshResourceManager::GetInstance().
class vHavokAiNavMeshResourceManager : public VisResourceManager_cl
{
public:

	///
	/// @name Constructor / Destructor / Global Accessor
	/// @{
	///

	vHavokAiNavMeshResourceManager();
	virtual ~vHavokAiNavMeshResourceManager();
	VHAVOKAI_IMPEXP static vHavokAiNavMeshResourceManager* GetInstance() { return &g_GlobalManager; }

	///
	/// @}
	///

	///
	/// @name Initialization / deinitialization functions
	/// @{
	///

  /// \brief
	///   Should be called at plugin initialization time.
	void OneTimeInit();

  /// \brief
	///   Should be called at plugin de-initialization time.
	void OneTimeDeInit();

	///
	/// @}
	///

	///
	/// @name VisResourceManager_cl implementation
	/// @{
	///

	VHAVOKAI_IMPEXP virtual VManagedResource *CreateResource(const char *szFilename, 
    VResourceSnapshotEntry *pExtraInfo) HKV_OVERRIDE;

	///
	/// @}
	///

protected:
	/// one global instance of our manager
	static vHavokAiNavMeshResourceManager g_GlobalManager;
};

#endif	// __VHAVOK_AI_NAVMESH_RESOURCE_MANAGER_HPP

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
