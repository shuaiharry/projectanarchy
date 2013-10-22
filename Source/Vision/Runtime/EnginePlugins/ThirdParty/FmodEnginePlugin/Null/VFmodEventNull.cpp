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


// -------------------------------------------------------------------------- //
// Constructor/ Destructor                                                 
// -------------------------------------------------------------------------- //

// for serialization
VFmodEvent::VFmodEvent() :
  VisObject3D_cl(),
  m_iFlags(0),
  m_sEventName(),
  m_pEvent(NULL),
  m_bIsPlaying(false),
  m_bPlayedOnce(false),
  m_bInfoOnly(false),
  m_bStartPlayback(false)
{
  m_pOwner = NULL;
  m_pEvent = (FMOD::Event*)0;
}

VFmodEvent::VFmodEvent(const char *szEventName, VFmodEventCollection* pOwner, VFmodEventGroup* pEventGroup, const hkvVec3 &vPos, int iFlags) :
  VisObject3D_cl(),
  m_iFlags(iFlags),
  m_sEventName(szEventName),
  m_pEvent(NULL),
  m_bIsPlaying(false),
  m_bPlayedOnce(false),
  m_bInfoOnly(false),
  m_bStartPlayback(false)
{
  VASSERT(szEventName!=NULL && pOwner!=NULL && pEventGroup!=NULL);

  SetPosition(vPos);
  m_spEventGroup = pEventGroup;

  m_pOwner = pOwner;

  m_pEvent = (FMOD::Event*)0;
  
  if(Vision::Editor.IsAnimatingOrPlaying() && (iFlags & VFMOD_FLAG_PAUSED) == 0)
    Start();
}

VFmodEvent::~VFmodEvent()
{
  OnDisposeObject();
}

void VFmodEvent::DisposeObject()
{
  Stop();

  VisObject3D_cl::DisposeObject();
}


// -------------------------------------------------------------------------- //
// Event property functions                                               
// -------------------------------------------------------------------------- //
void VFmodEvent::Start(bool bAlsoInEditor)
{
  if (IsPlaying() || m_bInfoOnly)
    return;   
  
  if (!bAlsoInEditor && !Vision::Editor.IsAnimatingOrPlaying())
    return;

  m_bIsPlaying = true;
  m_bPlayedOnce = true;

  Helper_SetFlag(VFMOD_FLAG_PAUSED, !IsPlaying());
}

void VFmodEvent::Stop(bool bImmediate)
{
  m_bStartPlayback = false;
  m_bIsPlaying = false;
}

bool VFmodEvent::IsPlaying() const
{
  return m_bIsPlaying;
}

bool VFmodEvent::IsReady() const
{
  return true;
}

void VFmodEvent::SetPaused(bool bStatus)
{
  m_bIsPlaying = !bStatus;
  Helper_SetFlag(VFMOD_FLAG_PAUSED, !IsPlaying());
}

void VFmodEvent::SetMuted(bool bStatus)
{
  Helper_SetFlag(VFMOD_FLAG_MUTE, bStatus);
}

bool VFmodEvent::GetInfo(int *pEventIndex, char **pszEventName, FMOD_EVENT_INFO *pInfo) const
{
  if(pszEventName)
    *pszEventName = "NullEvent";
  return true;
}

// -------------------------------------------------------------------------- //
// Overridden functions                                                 
// -------------------------------------------------------------------------- //
void VFmodEvent::OnDisposeObject()
{
}

void VFmodEvent::OnObject3DChanged(int iO3DFlags)
{
  VisObject3D_cl::OnObject3DChanged(iO3DFlags);
}

static int VFMOD_TRIGGER_ID_PAUSE = -1;
static int VFMOD_TRIGGER_ID_RESUME = -1;

void VFmodEvent::MessageFunction(int iID, INT_PTR iParamA, INT_PTR iParamB)
{
  VisObject3D_cl::MessageFunction(iID, iParamA, iParamB);
}

#ifdef SUPPORTS_SNAPSHOT_CREATION

void VFmodEvent::GetDependencies(VResourceSnapshot &snapshot)
{
  VisObject3D_cl::GetDependencies(snapshot);
  if (m_spEventGroup)
    m_spEventGroup->GetDependencies(snapshot);
}

#endif


// -------------------------------------------------------------------------- //
// Serialization                                                
// -------------------------------------------------------------------------- //
V_IMPLEMENT_SERIAL( VFmodEvent, VisObject3D_cl, 0, &g_FmodModule );

void VFmodEvent::Serialize( VArchive &ar )
{
  VisObject3D_cl::Serialize(ar);
  char iVersion = -1;
  if (ar.IsLoading())
  {
    ar >> iVersion; 
    VASSERT(iVersion >= VFMOD_EVENT_VERSION_0 && iVersion <= VFMOD_EVENT_VERSION_CURRENT);

    // load resource 
    char szEventProjectPath[FS_MAX_PATH+1];
    char szEventGroupName[FS_MAX_PATH+1];
    ar.ReadStringBinary(szEventProjectPath, FS_MAX_PATH);
    ar.ReadStringBinary(szEventGroupName, FS_MAX_PATH);

    // event properties
    ar >> m_sEventName;

    // @@@ Force events to be non-disposable
    //m_iFlags |= VFMOD_FLAG_NODISPOSE;
    
    ar >> m_iFlags;

    if(iVersion >= VFMOD_EVENT_VERSION_1)
    {
      ar >> m_bPlayedOnce;
    }
  }
  else
  {
    VASSERT(!"Saving is not supported for the null plugin!");
  }
}

void VFmodEvent::OnDeserializationCallback(const VSerializationContext &context)
{
  VisObject3D_cl::OnDeserializationCallback(context);

  if(Vision::Editor.IsAnimatingOrPlaying() && (m_iFlags & VFMOD_FLAG_PAUSED) == 0)
    Start();  
}


// -------------------------------------------------------------------------- //
// Internal functions                                                
// -------------------------------------------------------------------------- //
void VFmodEvent::SetPlaying(bool bStatus) 
{ 
  m_bIsPlaying = bStatus; 
}

void VFmodEventCollection::Update()
{
}

void VFmodEventCollection::PurgeNotPlaying()
{ 
}


void VFmodEventCollection::StopAll(bool bReasonIsUnloadWorld)
{
}

VFmodEvent* VFmodEventCollection::SearchEvent(const char* szName) const
{
  return NULL;
}

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
