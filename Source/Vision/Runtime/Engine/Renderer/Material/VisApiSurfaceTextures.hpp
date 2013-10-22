/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VisApiSurfaceTextures.hpp

#ifndef VISAPISURFACETEXTURES_HPP_INCLUDED
#define VISAPISURFACETEXTURES_HPP_INCLUDED

//#include <Vision/Runtime/Engine/Renderer/Shader/VisApiShaderEffect.hpp>

/// \brief
///   Class that provides material textures for surfaces.
/// 
/// The VisSurface_cl class is derived from this class to inherit the material textures.
/// 
/// Furthermore, entities can replace material sets via the VisSurfaceTextureSet_cl class.
/// 
/// See VisBaseEntity_cl::SetCustomTextureSet
class VisSurfaceTextures_cl
{
public:

  /// \brief
  ///   Constructor
  VisSurfaceTextures_cl() : m_vLightmapScaleOfs(1.0f, 1.0f, 0.0f, 0.0f)
  {
    m_iIndex = 0;
    m_spAuxiliaryTextures = NULL;
    m_iAuxiliaryTextureCount = 0;
  }

  ~VisSurfaceTextures_cl()
  {
    V_SAFE_DELETE_ARRAY(m_spAuxiliaryTextures);
  }

  /// \brief Enum specifying the usage of a texture.
  enum VTextureType_e
  {
    VTT_Diffuse,      ///< The texture is used as a diffuse color map.
    VTT_NormalMap,    ///< The texture is used as a normal map.
    VTT_SpecularMap   ///< The texture is used as a specular map.
  };
 
  ///
  /// @name Access Textures
  /// @{
  ///
   
  /// \brief
  ///   Helper function to load and set a specific texture in this set.
  VISION_APIFUNC void SetTextureFile (VTextureType_e eType, const char *pszFileName);

  /// \brief
  ///   Helper function to return the texture filename string (or NULL) of the texture in the specified texture slot.
  VISION_APIFUNC const char *GetTextureFile(VTextureType_e eType) const;

  /// \brief
  ///   Sets a specific texture object in the passed slot (diffuse/normalmap/specularmap). If the diffuse texture is changed (VTT_Diffuse), then the texture animation instance is changed.
  VISION_APIFUNC void SetTexture (VTextureType_e eType, VTextureObject *pTex);

  /// \brief
  ///   Return the texture object in the specified texture slot.
  VISION_APIFUNC VTextureObject* GetTexture (VTextureType_e eType) const;


  /// \brief
  ///   Returns the file name of the base texture.
  inline const char *GetBaseTexture() const
  {
    if (m_spDiffuseTexture)
      return m_spDiffuseTexture->GetOriginalFilename();
    return NULL;
  }

  /// \brief
  ///   Returns the file name of the normal map texture.
  inline const char *GetNormalMapTexture() const
  {
    if (m_spNormalMap)
      return m_spNormalMap->GetOriginalFilename();
    return NULL;
  }


  /// \brief
  ///   Returns the file name of the specular texture.
  inline const char *GetSpecularMapTexture() const
  {
    if (m_spSpecularMap)
      return m_spSpecularMap->GetOriginalFilename();
    return NULL;
  }


  /// \brief
  ///   Returns a pointer to the base texture object that represents the diffuse base texture of the surface.
  inline VTextureObject *GetBaseTextureObject() const 
  {
    return m_spDiffuseTexture;
  }

  /// \brief
  ///   Sets the diffuse base texture on this material and creates a texture animation instance if the texture is animated.
  VISION_APIFUNC void SetBaseTexture(VTextureObject *pTex);

  /// \brief
  ///   Returns a pointer to the texture object that represents the normal map texture of the
  ///   surface.
  inline VTextureObject *GetNormalMapTextureObject() const 
  {
    return m_spNormalMap;
  }

  /// \brief
  ///   Returns the specular map texture object.
  inline VTextureObject *GetSpecularMapTextureObject() const 
  {
    return m_spSpecularMap;
  }


  /// \brief
  ///   Return the animation instance that is associated with the diffuse texture. If the diffuse texture is not animated, this function returns NULL.
  VisTextureAnimInstance_cl *GetDiffuseTextureAnimation() const
  {
    return m_spDiffuseAnim;
  }

  /// \brief
  ///   Returns the number of additional auxiliary textures associated with this material.
  inline int GetAuxiliaryTextureCount() const {return m_iAuxiliaryTextureCount;}

  /// \brief
  ///   Returns the additional auxiliary texture with index iIndex. The index must be in valid
  ///   range [0..GetAuxiliaryTextureCount()-1].
  ///
  /// \param iIndex
  ///  The index of the auxiliary texture.
  ///   
  /// The return value can be NULL is no auxiliary texture was set at the given index.
  VTextureObject *GetAuxiliaryTexture(int iIndex) const 
  {
    VASSERT(m_spAuxiliaryTextures!=NULL && iIndex>=0 && iIndex<m_iAuxiliaryTextureCount);
    return m_spAuxiliaryTextures[iIndex];
  }

  /// \brief
  ///   Allocates an array of iCount auxiliary textures for this material. See GetAuxiliaryTexture or SetAuxiliaryTexture.
  ///
  /// Auxiliary textures can be used to associate an arbitrary number of custom textures with a material without any hardcoded
  /// semantic (e.g. specular). Since auxiliary textures can be addressed in shader texture samplers, this is a convenient way
  /// to render geometry with identical shader setup but different texture sets.
  /// Allocating a new array does not preserve the old array in this material.
  ///
  /// \param iCount
  ///   new number of auxiliary textures. Can be 0
  VISION_APIFUNC void AllocateAuxiliaryTextures(int iCount);

  /// \brief
  ///   Associates an auxiliary texture with this material. The index must be in valid range defined by GetAuxiliaryTextureCount (which is the number of allocated textures via AllocateAuxiliaryTextures).
  void SetAuxiliaryTexture(int iIndex, VTextureObject *pTexture) const 
  {
    VASSERT(m_spAuxiliaryTextures!=NULL && iIndex>=0 && iIndex<m_iAuxiliaryTextureCount);
    m_spAuxiliaryTextures[iIndex] = pTexture;
  }

  ///
  /// @}
  ///

  /// \brief
  ///   Implements the SerializeX function to serialize the content of this set.
  VISION_APIFUNC void SerializeX( VArchive &ar );
  V_DECLARE_SERIALX( VisSurfaceTextures_cl, VISION_APIFUNC );


  /// \brief
  ///   Assignment operator; assigns the texture smart pointers componentwise.
  VisSurfaceTextures_cl &operator = (const VisSurfaceTextures_cl &other)
  {
    if(this == &other)
      return *this;
  
    m_iIndex = other.m_iIndex;
    m_spDiffuseTexture = other.m_spDiffuseTexture;
    m_spNormalMap = other.m_spNormalMap;
    m_spSpecularMap = other.m_spSpecularMap;
    m_spModelLightmaps[0] = other.m_spModelLightmaps[0];
    m_spModelLightmaps[1] = other.m_spModelLightmaps[1];
    m_spModelLightmaps[2] = other.m_spModelLightmaps[2];
    m_spModelLightmaps[3] = other.m_spModelLightmaps[3];
    m_vLightmapScaleOfs = other.m_vLightmapScaleOfs;

    //copy auxiliary textures
    V_SAFE_DELETE_ARRAY(m_spAuxiliaryTextures);
    m_iAuxiliaryTextureCount = other.m_iAuxiliaryTextureCount;
    if (m_iAuxiliaryTextureCount>0)
    {
      int i;
      m_spAuxiliaryTextures = new VTextureObjectPtr[m_iAuxiliaryTextureCount];
      for (i=0;i<m_iAuxiliaryTextureCount;i++)
        m_spAuxiliaryTextures[i] = other.m_spAuxiliaryTextures[i];
    }


    return *this;
  }

  /// \brief
  ///   Get the 0-based index of this entry in the owner's list.
  inline int GetIndex() const {return m_iIndex;}

  /// \brief
  ///   Internal function - do not use
  inline void SetIndex(int iIndex) {m_iIndex=iIndex;}

#ifdef SUPPORTS_SNAPSHOT_CREATION
  /// \brief
  ///   Gather relevant resource information that pOwner relies on. In this case, add all textures.
  VISION_APIFUNC void GetDependencies(VResourceSnapshot &snapshot, VManagedResource *pOwner);
#endif
  int m_iIndex; // index

  hkvVec4 m_vLightmapScaleOfs;               ///< xy = scale, zw = ofsset for uv => lightmap uv transform
  VTextureObjectPtr m_spDiffuseTexture;      ///< the diffuse base texture of the material
  VTextureObjectPtr m_spNormalMap;           ///< the normalmap texture of the material
  VTextureObjectPtr m_spSpecularMap;         ///< the reflection map texture of the material
  VTextureObjectPtr m_spModelLightmaps[4];   ///< 0: diffuse, 1-3: dot3 lightmaps

  VisTextureAnimInstancePtr m_spDiffuseAnim; ///< texture animation instance for diffuse texture animation

  short m_iAuxiliaryTextureCount;
  VTextureObjectPtr *m_spAuxiliaryTextures;
};

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
