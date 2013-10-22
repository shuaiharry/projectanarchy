/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/FmodEnginePlugin.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/VFmodManager.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/VFmodEventGroup.hpp>

#include <Vision/Runtime/Base/System/Memory/VMemDbg.hpp>


VManagedResource* VFmodEventGroupManager::CreateResource(const char *szFilename, VResourceSnapshotEntry *pExtraInfo)
{
  VManagedResource *pRes = VFmodManager::GlobalManager().LoadEventGroup("NullEventGroup", "NullEventGroup");
  if (!pRes)
    return NULL;

  pRes->EnsureLoaded();

  return pRes;
}

// -------------------------------------------------------------------------- //
// Constructor/ Destructor                                                 
// -------------------------------------------------------------------------- //
VFmodEventGroup::VFmodEventGroup(VFmodEventGroupManager *pManager) : VManagedResource(pManager), m_iSysMem(0)
{
  m_pEventGroup = (FMOD::EventGroup*)0;
}

VFmodEventGroup::~VFmodEventGroup()
{
}


// -------------------------------------------------------------------------- //
// Instances                                                 
// -------------------------------------------------------------------------- //
VFmodEvent *VFmodEventGroup::CreateEvent(const char *szEventName, const hkvVec3 &vPos, int iFlags)
{
  VFmodEvent *pEvent = new VFmodEvent(szEventName, NULL, this, vPos, iFlags);
  return pEvent;
}


// -------------------------------------------------------------------------- //
// Resource property functions                                               
// -------------------------------------------------------------------------- //
bool VFmodEventGroup::GetProperty(const char *szPropertyName, void *pValue) const
{   
  return false;
}

bool VFmodEventGroup::GetProperty(int iPropertyIndex, void *pValue) const
{
  return false;
}

bool VFmodEventGroup::GetInfo(int *pGroupIndex, char **pszGroupName) const
{
  return false;
}

bool VFmodEventGroup::IsReady() const
{
  return true;
}




// -------------------------------------------------------------------------- //
// Overridden functions                                                 
// -------------------------------------------------------------------------- // 
BOOL VFmodEventGroup::Reload()
{
  return TRUE;
}

BOOL VFmodEventGroup::Unload()
{
  return TRUE;
}

#ifdef SUPPORTS_SNAPSHOT_CREATION

void VFmodEventGroup::GetDependencies(VResourceSnapshot &snapshot)
{
  VManagedResource::GetDependencies(snapshot); 
}

#endif

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
