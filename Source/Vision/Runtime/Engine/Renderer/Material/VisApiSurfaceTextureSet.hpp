/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VisApiSurfaceTextureSet.hpp

#ifndef VISAPISURFACETEXTURESET_HPP_INCLUDED
#define VISAPISURFACETEXTURESET_HPP_INCLUDED

#include <Vision/Runtime/Engine/Renderer/Shader/VisApiShaderEffect.hpp>
#include <Vision/Runtime/Engine/Renderer/Material/VisApiSurfaceTextures.hpp>
#include <Vision/Runtime/Engine/System/VisApiCallbacks.hpp>

/// \brief
///   Provides an array of VisSurfaceTextures_cl instances to match the material set of a model.
/// 
/// An instance of this class can be used to override the model material textures on a per-entity basis; see VisBaseEntity_cl::SetCustomTextureSet.
/// 
/// Once a set is initialized for a model, each entry in the m_pSurfaceTextures array corresponds
/// to a surface in the mesh, i.e. pSet->GetTextures(i) <-> pMesh->GetSurfacebyIndex(i).
/// This has to be taken into account when modifying the replacement textures.
class VisSurfaceTextureSet_cl : public IVSerializationProxyCreator, public VRefCounter
{
public:

  /// \brief
  ///   Constructor that initializes an empty set. Should not be used, use one of the other constructors instead.
  VISION_APIFUNC VisSurfaceTextureSet_cl();

  /// \brief
  ///   Constructor that initializes this set with the mesh's surface array.
  VISION_APIFUNC VisSurfaceTextureSet_cl(VBaseMesh *pModel, bool bCreateSrfInstances=false);

  /// \brief
  ///   Constructor that initializes this set with the entity model's surface array.
  VISION_APIFUNC VisSurfaceTextureSet_cl(VisBaseEntity_cl *pEntity, bool bCreateSrfInstances=false);

  /// \brief
  ///   Constructor that initializes this set with the static mesh's surface array.
  VISION_APIFUNC VisSurfaceTextureSet_cl(VisStaticMeshInstance_cl *pMeshInst);

  /// \brief
  ///   Destructor. Frees the allocated array.
  VISION_APIFUNC virtual ~VisSurfaceTextureSet_cl();

  /// \brief
  ///   Initializes this set with the mesh's surface array.
  /// 
  /// It allocates an array of VisSurfaceTextures_cl or VisSurface_cl instances using the same amount as the mesh surfaces.
  /// Each array element gets properly cloned from the original mesh materials.
  /// A set that has been created through this function can be used for functions VisBaseEntity_cl::SetCustomTextureSet or 
  /// VisStaticMeshInstance_cl::SetCustomSurfaceSet. If used for static meshes, the bCreateSrfInstances parameter has to be true.
  ///
  /// \param pMesh
  ///   Mesh to use.
  ///
  /// \param bCreateSrfInstances
  ///   If true, an array of VisSurface_cl is created, an array of the lightweight parent class VisSurfaceTextures_cl otherwise.
  ///   Static meshes can only work on VisSurface_cl so in this case true has to be passed here.
  ///
  /// \return
  ///   true if successful.
  VISION_APIFUNC bool CreateForMesh(VBaseMesh *pMesh, bool bCreateSrfInstances);

  /// \brief
  ///   Wrapper function for backwards compatibility. Simply calls CreateForMesh.
  VISION_APIFUNC bool CreateForModel(VDynamicMesh *pMesh, bool bCreateSrfInstances=false);

  /// \brief
  ///   Implements the SerializeX function to serialize the content of this set.
  VISION_APIFUNC void SerializeX( VArchive &ar );
  V_DECLARE_SERIALX( VisSurfaceTextureSet_cl, VISION_APIFUNC );

  /// \brief
  ///   Implements IVSerializationProxyCreator.
  VISION_APIFUNC VOVERRIDE IVSerializationProxy *CreateProxy();

  /// \brief
  ///   Internal name that is serialized and which can be used to identify a variant by name. Used by vForge.
  VISION_APIFUNC void SetVariantName(const VString& sName)
  {
    m_sVariantName = sName;
  }

#ifdef SUPPORTS_SNAPSHOT_CREATION
  /// \brief
  ///   Gather relevant resource information that pOwner relies on. In this case, add all textures
  ///   from all sets.
  void GetDependencies(VResourceSnapshot &snapshot, VManagedResource *pOwner)
  {
    for (int i=0;i<m_iSurfaceCount;i++)
      GetTextures(i)->GetDependencies(snapshot,pOwner);
  }
#endif

  /// \brief
  ///   Returns the number of allocated textures / surfaces in this set. This matches pMesh->GetSurfaceCount of the owner mesh.
  int GetSurfaceCount() const
  {
    return m_iSurfaceCount;
  }

  /// \brief
  ///   Returns the textures of surface with specified index, where index must be in valid range [0..GetSurfaceCount()-1].
  VisSurfaceTextures_cl* GetTextures(int iIndex) const;

  /// \brief
  ///   Indicates whether the array has been allocated with VisSurface_cl instances, i.e. bCreateSrfInstances = true was passed during creation.
  bool UsesSurfaces() const
  {
    return m_pSurfaceRefArray!=NULL;
  }

  /// \brief
  ///   Returns the array as an array of VisSurface_cl instances. This function requires that UsesSurfaces() == true.
  inline VisSurface_cl** AsSurfaceArray() const
  {
    VASSERT_MSG(UsesSurfaces(), "Must not call this function when UsesSurfaces indicates false");
    return m_pSurfaceRefArray;
  }

  /// \brief
  ///   Returns the mesh this set was created for. A texture set is only compatible with one specific mesh.
  ///
  /// \return
  ///   The pointer of the mesh used in CreateForMesh.
  ///
  /// \sa CreateForMesh
  inline VBaseMesh* GetMesh() const { return m_pMesh; }

  /// \brief
  ///   Sets the mesh this set was created for during de-serialization. Used internally, do not use.
  inline void SetMesh(VBaseMesh* pMesh) { m_pMesh = pMesh; }

protected:
  void DeleteArray();

  VBaseMesh* m_pMesh; ///< Reference to the mesh this set was created from.
  short m_iSurfaceCount; ///< Number of materials in this set. Automatically allocated by constructor
  VisSurfaceTextures_cl *m_pSurfaceTextureArray;  ///< Pointer to the array of material textures. The array holds m_iSurfaceCount elements
  VisSurface_cl *m_pSurfaceArray;  ///< Pointer to the array of surfaces. The array holds m_iSurfaceCount elements
  VisSurface_cl **m_pSurfaceRefArray;  ///< Pointer to the array of surface pointers. The array holds m_iSurfaceCount elements
  VString m_sVariantName;
};

typedef VSmartPtr<VisSurfaceTextureSet_cl> VisSurfaceTextureSetPtr;

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
