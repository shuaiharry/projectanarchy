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
VFmodCollisionMeshInstance::VFmodCollisionMeshInstance(VFmodManager *pManager)
{
  m_bActive = true;
  m_bDoubleSided = false;
  m_vScale.set(1.0f, 1.0f, 1.0f);
  m_fDirectOcclusion = m_fReverbOcclusion = 1.0f;

  m_pGeometry = NULL;
}

VFmodCollisionMeshInstance::~VFmodCollisionMeshInstance()
{
}

void VFmodCollisionMeshInstance::DisposeObject()
{
  VisObject3D_cl::DisposeObject();
}


// -------------------------------------------------------------------------- //
// Overridden functions                                                 
// -------------------------------------------------------------------------- //
void VFmodCollisionMeshInstance::OnObject3DChanged(int iO3DFlags)
{
  VisObject3D_cl::OnObject3DChanged(iO3DFlags);
}


#ifdef SUPPORTS_SNAPSHOT_CREATION

void VFmodCollisionMeshInstance::GetDependencies(VResourceSnapshot &snapshot)
{
  VisObject3D_cl::GetDependencies(snapshot);

  if (m_spMesh)
    m_spMesh->GetDependencies(snapshot);
}

#endif


// -------------------------------------------------------------------------- //
// Serialization                                                
// -------------------------------------------------------------------------- //
V_IMPLEMENT_SERIAL( VFmodCollisionMeshInstance, VisObject3D_cl, 0, &g_FmodModule );

void VFmodCollisionMeshInstance::Serialize( VArchive &ar )
{
  if (ar.IsLoading())
  {
    char iVersion = -1;
    bool bActive;
    int iMeshType;
    ar >> iVersion; 
    VASSERT(iVersion >= V_FMODCOLLISIONMESHINSTANCE_VERSION_0 && iVersion <= V_FMODCOLLISIONMESHINSTANCE_VERSION_CURRENT);

    VisObject3D_cl::Serialize(ar);
    ar >> bActive;
    m_vScale.SerializeAsVec3(ar);
    ar >> m_fDirectOcclusion >> m_fReverbOcclusion;
    ar >> iMeshType; // mesh type; 0==VBaseMesh

    if (iMeshType==0)
    {
      VBaseMesh *pMesh = (VBaseMesh *)ar.ReadProxyObject();
      SetCollisionMesh(pMesh, m_fDirectOcclusion, m_fReverbOcclusion);
    } 
    else
    {
      VASSERT(!"Yet unsupported mesh type for sound collision geometry");
    }

    SetActive(bActive);
  } 
  else
  {
    VASSERT(!"Saving is not supported by the NULL plugin!");
  }
}


// -------------------------------------------------------------------------- //
// Mesh definition                                                
// -------------------------------------------------------------------------- //
bool VFmodCollisionMeshInstance::SetCollisionMesh(VSimpleCollisionMesh *pMesh, float fDirectOcclusion, float fReverbOcclusion, int iMeshFlags)
{
  return true;
}

bool VFmodCollisionMeshInstance::SetCollisionMesh(VBaseMesh *pMesh, float fDirectOcclusion, float fReverbOcclusion)
{
  m_spMesh = pMesh;
  return true;
}

bool VFmodCollisionMeshInstance::SetCollisionMesh(int iVertexCount, int iTriangleCount, hkvVec3 *pVertices, void *pIndices, float fDirectOcclusion, float fReverbOcclusion, int iMeshFlags, int iIndexType)
{
  return true;
}


// -------------------------------------------------------------------------- //
// CollisionMeshInstance property functions                                              
// -------------------------------------------------------------------------- //
void VFmodCollisionMeshInstance::SetActive(bool bStatus)
{
  m_bActive = bStatus;
}

void VFmodCollisionMeshInstance::SetScaling(const hkvVec3 &vScale) 
{ 
  m_vScale = vScale; 
  OnObject3DChanged(VIS_OBJECT3D_ALLCHANGED);
}

void VFmodCollisionMeshInstance::SetOcclusion(float fDirectOcclusion, float fReverbOcclusion)
{
  m_fDirectOcclusion = fDirectOcclusion;
  m_fReverbOcclusion = fReverbOcclusion;
}


VFmodCollisionMeshInstance* VFmodCollisionMeshInstanceCollection::SearchCollisionMesh(const char* szName) const
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
