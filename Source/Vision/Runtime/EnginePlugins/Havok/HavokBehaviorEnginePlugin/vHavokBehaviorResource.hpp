/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef __VHAVOK_BEHAVIOR_RESOURCE_HPP
#define __VHAVOK_BEHAVIOR_RESOURCE_HPP

#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorIncludes.hpp>
#include <Behavior/Utilities/Utils/hkbOnHeapAssetLoader.h>
#include <Behavior/Utilities/Utils/hkbScriptAssetLoader.h>

class hkObjectResource;
class hkbProjectAssetManager;
class vHavokBehaviorComponent;
class vHavokBehaviorResourceManager;

/// \brief
///   This resource represents a Havok Behavior PROJECT file and all its dependencies
class vHavokBehaviorResource : public VManagedResource
{
	protected:
    ///
    /// @name Constructor / Destructor
    /// @{
    ///

    /// \brief
    ///   Constructor
    ///
		/// Called by vHavokBehaviorResourceManager::CreateResource
		VHAVOKBEHAVIOR_IMPEXP vHavokBehaviorResource(const char* filePath); 

	public:
		VHAVOKBEHAVIOR_IMPEXP virtual ~vHavokBehaviorResource();

    ///
    /// @}
    ///

    ///
    /// @name VManagedResource implementation
    /// @{
    ///

		VHAVOKBEHAVIOR_IMPEXP virtual BOOL Reload() HKV_OVERRIDE;
		VHAVOKBEHAVIOR_IMPEXP virtual BOOL Unload() HKV_OVERRIDE;
		VHAVOKBEHAVIOR_IMPEXP virtual void GetDependencies(VResourceSnapshot &snapshot) HKV_OVERRIDE;

    ///
    /// @}
    ///

	protected:

		friend class vHavokBehaviorResourceManager;

    /// \brief
		///   Common initialization function.
		void Init();

		hkArray<hkStringPtr> m_dependentFiles;  ///< List of files dependent on this resource
		hkArray<int> m_dependentFileSizes;      ///< Analogous array to m_dependentFiles, but tracks the size of the files
};

#if !defined(_VISION_DOC)

class vHavokBehaviorAssetLoader : public hkbOnHeapAssetLoader
{
	public:
		vHavokBehaviorAssetLoader(vHavokBehaviorResourceManager* resourceManager);

	protected:
		virtual void* loadFile(const hkStringBuf& fullPath, hkStreamReader* stream, void*& storingData) HK_OVERRIDE;

	public:
		vHavokBehaviorResourceManager* m_resourceManager;
};

class vHavokBehaviorScriptAssetLoader : public hkbScriptAssetLoader
{
public:
  vHavokBehaviorScriptAssetLoader(vHavokBehaviorResourceManager *resourceManager);

private:
  void loadScript(char const *filePath, bool forceLoad) HKV_OVERRIDE;

private:
  vHavokBehaviorResourceManager *m_resourceManager;
};

#endif

#endif	// __VHAVOK_BEHAVIOR_RESOURCE_HPP

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
