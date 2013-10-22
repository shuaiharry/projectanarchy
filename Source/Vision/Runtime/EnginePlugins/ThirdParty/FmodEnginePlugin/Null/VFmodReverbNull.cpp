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


V_IMPLEMENT_SERIALX(VFmodReverbProps);

void VFmodReverbProps::SerializeX( VArchive &ar )
{
  if (ar.IsLoading())
  {
    char iVersion = -1;
    ar >> iVersion; 
    VASSERT(iVersion >= V_FMODREVERBPROPS_VERSION_0 && iVersion <= V_FMODREVERBPROPS_VERSION_CURRENT);

    ar >> m_iEnvironment;                  
    ar >> m_iRoom;
    ar >> m_iRoomHF;
    ar >> m_iRoomLF; 
    ar >> m_fDecayTime;   
    ar >> m_fDecayHFRatio;
    ar >> m_iReflections;
    ar >> m_fReflectionsDelay;     
    ar >> m_iReverb;
    ar >> m_fReverbDelay;
    ar >> m_fHFReference;
    ar >> m_fLFReference;          
    ar >> m_fDiffusion;
    ar >> m_fDensity;
  }
  else
  {
	VASSERT(!"Saving is not supported by the null plugin!");
  }
}


// -------------------------------------------------------------------------- //
// Constructor/ Destructor                                                 
// -------------------------------------------------------------------------- //
VFmodReverb::VFmodReverb(VFmodManager *pManager)
{
  m_bActive = true;
  m_fReverbMinDistance = m_fReverbMaxDistance = 0.0f;

  m_pReverb = NULL;
}

VFmodReverb::~VFmodReverb()
{
}

void VFmodReverb::DisposeObject()
{
  VisObject3D_cl::DisposeObject();
}


// -------------------------------------------------------------------------- //
// Overridden functions                                                 
// -------------------------------------------------------------------------- //
void VFmodReverb::OnObject3DChanged(int iO3DFlags)
{
  VisObject3D_cl::OnObject3DChanged(iO3DFlags);
}


// -------------------------------------------------------------------------- //
// Serialization                                                
// -------------------------------------------------------------------------- //
V_IMPLEMENT_SERIAL( VFmodReverb, VisObject3D_cl, 0, &g_FmodModule );

void VFmodReverb::Serialize( VArchive &ar )
{
  if (ar.IsLoading())
  {
    char iVersion = -1;
    bool bActive;
    ar >> iVersion; 
    VASSERT(iVersion >= V_FMODREVERB_VERSION_0 && iVersion <= V_FMODREVERB_VERSION_CURRENT);

    VisObject3D_cl::Serialize(ar);
    ar >> bActive;
    ar >> m_fReverbMinDistance >> m_fReverbMaxDistance;
    ar >> m_reverbProp;

    Init(m_reverbProp, m_fReverbMinDistance, m_fReverbMaxDistance);

    SetActive(bActive);
  } 
  else
  {
    VASSERT(!"Saving is not supported by the null plugin!");
  }
}


// -------------------------------------------------------------------------- //
// Initialization                                              
// -------------------------------------------------------------------------- //
bool VFmodReverb::Init(VFmodReverbProps &reverbProperties, float fReverbMinDistance, float fReverbMaxDistance)
{
  m_fReverbMinDistance = fReverbMinDistance;
  m_fReverbMaxDistance = fReverbMaxDistance;

  OnObject3DChanged(VIS_OBJECT3D_ALLCHANGED); // set new position etc. on new geometry object
  return true;
}


// -------------------------------------------------------------------------- //
// Reverb property functions                                              
// -------------------------------------------------------------------------- //
void VFmodReverb::SetActive(bool bStatus)
{
  m_bActive = bStatus;
}

void VFmodReverb::SetReverbDistances(float fReverbMinDistance, float fReverbMaxDistance)
{
  m_fReverbMinDistance = fReverbMinDistance;
  m_fReverbMaxDistance = fReverbMaxDistance;

  OnObject3DChanged(VIS_OBJECT3D_ALLCHANGED); // update 3d attributes
}

void VFmodReverb::SetReverbProperties(VFmodReverbProps &reverbProperties)
{
  m_reverbProp = reverbProperties;
}

// -------------------------------------------------------------------------- //
// Misc                                              
// -------------------------------------------------------------------------- //


VFmodReverb* VFmodReverbCollection::SearchReverb(const char* szName) const
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
