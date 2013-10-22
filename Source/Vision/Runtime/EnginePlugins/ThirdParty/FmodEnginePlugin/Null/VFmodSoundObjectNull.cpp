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
VFmodSoundObject::VFmodSoundObject() :
  VisObject3D_cl(),
  m_iFlags(0),
  m_iPriority(0),
  m_fVolume(1.0f),
  m_fPan(0.0f),
  m_fConeOutside(-1.0f), // no cone
  m_fConeInside(-1.0f),  // no cone
  m_fFadeMin(1.0f),
  m_fFadeMax(10000.0f),
  m_fVolumeFadeTime(0.0f),
  m_fVolumeFadeDuration(0.0f),
  m_bFading(false),
  m_fPitch(1.0f), // unmodified pitch
  m_pChannel(NULL),
  m_fInitialFreq(0.0f),
  m_bIsPlaying(false),
  m_bPlayedOnce(false),
  m_bUnpause(false)
{
  m_pOwner = NULL;
  m_pChannel = (FMOD::Channel*)0;
}

VFmodSoundObject::VFmodSoundObject(VFmodSoundObjectCollection* pOwner, VFmodSoundResource* pResource, const hkvVec3 &vPos, int iFlags, int iPriority) :
  VisObject3D_cl(),
  m_iFlags(iFlags),
  m_iPriority(iPriority),
  m_fVolume(1.0f),
  m_fPan(0.0f),
  m_fConeOutside(-1.0f), // no cone
  m_fConeInside(-1.0f),  // no cone
  m_fFadeMin(1.0f),
  m_fFadeMax(10000.0f),
  m_fStartTime(0.0f),
  m_fVolumeFadeTime(0.0f),
  m_fVolumeFadeDuration(0.0f),
  m_bFading(false),
  m_fPitch(1.0f), // unmodified pitch
  m_pChannel(NULL),
  m_fInitialFreq(0.0f),
  m_bIsPlaying(false),
  m_bPlayedOnce(false),
  m_bUnpause(false)
{
  SetPosition(vPos);

  m_pChannel = (FMOD::Channel*)0;

  if(Vision::Editor.IsAnimatingOrPlaying() && (iFlags & VFMOD_FLAG_PAUSED) == 0)
    Play();
}

VFmodSoundObject::~VFmodSoundObject()
{
  OnDisposeObject();
}

void VFmodSoundObject::DisposeObject()
{
  VFmodSoundObjectPtr keepRef = this; // ensure the object gets deleted at the end of the function

  Stop();

  VisObject3D_cl::DisposeObject();
}


// -------------------------------------------------------------------------- //
// Sound property functions                                               
// -------------------------------------------------------------------------- //
void VFmodSoundObject::Play(float fStartTime, bool bAlsoInEditor)
{
  m_fStartTime = fStartTime;
  Helper_SetFlag(VFMOD_FLAG_PAUSED, !IsPlaying());
}

void VFmodSoundObject::Stop()
{
  if (!IsPlaying())
    return;

  m_bUnpause = false;
  m_bIsPlaying = false;
  m_fStartTime = 0.0f;
}

bool VFmodSoundObject::IsPlaying() const
{
  return m_bIsPlaying;
}

bool VFmodSoundObject::IsReady() const
{
  return true;
}

float VFmodSoundObject::GetCurrentPosition(bool bAbsolute) const
{
  return 0.0f;
}

void VFmodSoundObject::SetPaused(bool bStatus)
{
  if (IsPlaying() != bStatus)
    return;

  Helper_SetFlag(VFMOD_FLAG_PAUSED, !IsPlaying());
}

void VFmodSoundObject::SetMuted(bool bStatus)
{
  Helper_SetFlag(VFMOD_FLAG_MUTE, bStatus); 
}

void VFmodSoundObject::SetVolume(float fVol)
{
  m_fVolume = fVol;
}

void VFmodSoundObject::SetPan(float fPan)
{
  VASSERT(GetResource()->Is2D());
  m_fPan = fPan;
}

void VFmodSoundObject::Set3DFadeDistance(float fMin, float fMax)
{
  m_fFadeMin = fMin;
  m_fFadeMax = fMax;
  VASSERT(m_fFadeMax>=m_fFadeMin);
}

void VFmodSoundObject::FadeTo(float fNewVolume, float fDuration)
{
  if (fDuration <= 0.0f)
  {
    SetVolume(fNewVolume);
    return;
  }

  m_fVolumeFadeStart = m_fVolume;
  m_fVolumeFadeTarget = fNewVolume;
  m_fVolumeFadeTime = 0.0f;
  m_fVolumeFadeDuration = fDuration;
  m_bFading = true;

  Play();
}

void VFmodSoundObject::FadeOut(float fDuration)
{
  if(fDuration <= 0.0f)
  {
    SetVolume(m_fVolume);
    return;
  }

  m_fVolumeFadeStart = m_fVolume;
  m_fVolumeFadeTarget = 0.0f;
  m_fVolumeFadeTime = 0.0f;
  m_fVolumeFadeDuration = fDuration;
  m_bFading = true;
}

void VFmodSoundObject::FadeIn(float fDuration)
{
  if(fDuration <= 0.0f)
  {
    SetVolume(m_fVolume);
    return;
  }
  if(m_fVolume < 0.001f)
  {
    m_fVolume = 1.0f;
  }

  m_fVolumeFadeStart = 0.0f;
  m_fVolumeFadeTarget = m_fVolume;
  m_fVolumeFadeTime = 0.0f;
  m_fVolumeFadeDuration = fDuration;
  m_bFading = true;

  SetVolume(0.0f);
  Play();
}

void VFmodSoundObject::FadeFromTo(float fStartVolume, float fTargetVolume, float fDuration)
{
  if(fDuration <= 0.0f)
  {
    SetVolume(m_fVolume);
    return;
  }

  m_fVolumeFadeStart = fStartVolume;
  m_fVolumeFadeTarget = fTargetVolume;
  m_fVolumeFadeTime = 0.0f;
  m_fVolumeFadeDuration = fDuration;
  m_bFading = true;
}

void VFmodSoundObject::SetPitch(float fPitch)
{
  m_fPitch = fPitch;
}

void VFmodSoundObject::SetConeAngles(float fInside, float fOutside)
{
  m_fConeOutside = fOutside;
  m_fConeInside = fInside;
}

#define SO_EULER_CONST 2.718281828459045f // exp(1)
#define SO_AIR_ATT_COEF 1.3f

float VFmodSoundObject::CalculateAttenuatedVolume(const hkvVec3& vListenerPos) const
{ 
  return 1.0f;
}

int VFmodSoundObject::GetChannelIndex() const
{
  return 0;
}


// -------------------------------------------------------------------------- //
// Overridden functions                                                 
// -------------------------------------------------------------------------- //
void VFmodSoundObject::OnDisposeObject()
{
}

void VFmodSoundObject::OnObject3DChanged(int iO3DFlags)
{
  VisObject3D_cl::OnObject3DChanged(iO3DFlags);
}

void VFmodSoundObject::MessageFunction(int iID, INT_PTR iParamA, INT_PTR iParamB)
{
  VisObject3D_cl::MessageFunction(iID, iParamA, iParamB);
}

#ifdef SUPPORTS_SNAPSHOT_CREATION

void VFmodSoundObject::GetDependencies(VResourceSnapshot &snapshot)
{
  VisObject3D_cl::GetDependencies(snapshot);
  if (m_spResource)
    m_spResource->GetDependencies(snapshot);
}

#endif


// -------------------------------------------------------------------------- //
// Serialization                                                
// -------------------------------------------------------------------------- //
V_IMPLEMENT_SERIAL( VFmodSoundObject, VisObject3D_cl, 0, &g_FmodModule );

void VFmodSoundObject::Serialize( VArchive &ar )
{
  VisObject3D_cl::Serialize(ar);
  char iVersion = -1;
  if (ar.IsLoading())
  {
    ar >> iVersion; 
    VASSERT(iVersion >= VFMOD_SOUNDOBJECT_VERSION_0 && iVersion <= VFMOD_SOUNDOBJECT_VERSION_CURRENT);

    // first, load resource file
    int iResourceFlags;
    char szResourceFile[FS_MAX_PATH+1];
    ar >> iResourceFlags;
    ar.ReadStringBinary(szResourceFile, FS_MAX_PATH); 
    m_spResource = VFmodManager::GlobalManager().LoadSoundResource(szResourceFile, iResourceFlags);

    // sound properties
    ar >> m_iFlags;
    ar >> m_iPriority;
    ar >> m_fVolume >> m_fPan >> m_fConeInside >> m_fConeOutside;
    ar >> m_fFadeMin >> m_fFadeMax;
    ar >> m_bFading;
    ar >> m_fVolumeFadeTime >> m_fVolumeFadeDuration;
    ar >> m_fVolumeFadeStart >> m_fVolumeFadeTarget;
    ar >> m_fPitch;

    if(iVersion >= VFMOD_SOUNDOBJECT_VERSION_1)
    {
      ar >> m_bPlayedOnce;
    }

    // add to global list
    m_pOwner = &VFmodManager::GlobalManager().SoundInstances();
    m_pOwner->Add(this);
  }
  else
  {
    VASSERT(!"Saving not supported by null plugin!");
  }
}

void VFmodSoundObject::OnDeserializationCallback(const VSerializationContext &context)
{
  VisObject3D_cl::OnDeserializationCallback(context);

  if (Vision::Editor.IsAnimatingOrPlaying() && (m_iFlags & VFMOD_FLAG_PAUSED) == 0)
    Play();
}


// -------------------------------------------------------------------------- //
// Internal functions                                                
// -------------------------------------------------------------------------- //
void VFmodSoundObject::Update()
{
}


void VFmodSoundObjectCollection::Update()
{
}

void VFmodSoundObjectCollection::PurgeNotPlaying()
{ 
}


void VFmodSoundObjectCollection::StopAll(bool bReasonIsUnloadWorld)
{
}

VFmodSoundObject* VFmodSoundObjectCollection::SearchObject(const char* szName) const
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
