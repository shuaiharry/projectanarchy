/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/FmodEnginePlugin.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/VFmodManager.hpp>

#include <Vision/Runtime/Base/System/Memory/VMemDbg.hpp>


VManagedResource* VFmodSoundResourceManager::CreateResource(const char *szFilename, VResourceSnapshotEntry *pExtraInfo)
{
  VManagedResource *pRes = VFmodManager::GlobalManager().LoadSoundResource(szFilename, 0);
  return pRes;
}


// -------------------------------------------------------------------------- //
// Constructor/ Destructor                                                 
// -------------------------------------------------------------------------- //
VFmodSoundResource::VFmodSoundResource(VFmodSoundResourceManager *pManager,int iUsageFlags) : VManagedResource(pManager)
{
  m_iSysMem = 0;
  m_iSoundFlags = iUsageFlags;
  m_pSound = NULL;

  if (IsStreaming()) // since unique
    SetResourceFlag(VRESOURCEFLAG_AUTODELETE);
}

VFmodSoundResource::~VFmodSoundResource()
{
}


// -------------------------------------------------------------------------- //
// Instances                                                 
// -------------------------------------------------------------------------- //
VFmodSoundObject *VFmodSoundResource::CreateInstance(const hkvVec3 &vPos, int iFlags, int iPriority)
{
  VFmodSoundObject *pInst = new VFmodSoundObject(NULL, this, vPos, iFlags, iPriority);
  return pInst;
}


// -------------------------------------------------------------------------- //
// Resource property functions                                               
// -------------------------------------------------------------------------- //
bool VFmodSoundResource::IsReady() const
{
  return true;
}


// -------------------------------------------------------------------------- //
// Overridden functions                                                 
// -------------------------------------------------------------------------- //
BOOL VFmodSoundResource::Reload()
{
  return TRUE;
}

BOOL VFmodSoundResource::Unload()
{
  return TRUE;
}

int VFmodSoundResource::GetAdditionalOutputString(char *szDestBuffer, int iMaxChars)
{
  return 0;
}

#ifdef SUPPORTS_SNAPSHOT_CREATION

void VFmodSoundResource::GetDependencies(VResourceSnapshot &snapshot)
{
  if (!IsStreaming())
  {
    VManagedResource::GetDependencies(snapshot);
    IVFileInStream *pFile = Vision::File.Open(GetFilename());
    if (pFile)
    {
      // patch the file size afterwards
      VResourceSnapshotEntry *pEntry = snapshot.FindResourceEntry(this);
      VASSERT(pEntry!=NULL)
        if (pEntry)
        {
          pEntry->m_iFileSize = pFile->GetSize();
          pEntry->SetCustomIntValue(0,m_iSoundFlags);
        }
        pFile->Close();
    }
  }
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
