/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VisApiSurfaceTextureSetManager.hpp

#ifndef VISAPISURFACETEXTURESETMANAGER_HPP_INCLUDED
#define VISAPISURFACETEXTURESETMANAGER_HPP_INCLUDED

#include <Vision/Runtime/Engine/Renderer/Material/VisApiSurfaceTextureSet.hpp>
#include <Vision/Runtime/Engine/System/VisApiCallbacks.hpp>

/// \brief
///   Holds a reference to all VisSurfaceTextureSet_cl instances in order to apply mesh
///   modifications to any dependent VisSurfaceTextureSet_cl.
class VSurfaceTextureSetManager :  public IVisCallbackHandler_cl
{
public:
  ///
  /// @name IVisCallbackHandler_cl Overrides
  /// @{
  ///

  /// \brief
  ///   Implements the listener function
  VISION_APIFUNC virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

  ///
  /// @}
  ///


  ///
  /// @name VSurfaceTextureSetManager Public Functions
  /// @{
  ///

  /// \brief
  ///   Called once by the Engine for initialization
  VISION_APIFUNC void OneTimeInit();

  /// \brief
  ///   Called once by the Engine for de-initialization
  VISION_APIFUNC void OneTimeDeInit();

  /// \brief
  ///   Adds a VisSurfaceTextureSet_cl instance (will be called by VisSurfaceTextureSet_cl::ctr)
  VISION_APIFUNC void AddSurfaceTextureSet(VisSurfaceTextureSet_cl* pTextureSet);

  /// \brief
  ///   Remove a VisSurfaceTextureSet_cl instance (will be called automatically by VisSurfaceTextureSet_cl::d'tor)
  VISION_APIFUNC void RemoveSurfaceTextureSet(VisSurfaceTextureSet_cl* pTextureSet);

  /// \brief
  ///   Access one global instance of a VSurfaceTextureSetManager
  VISION_APIFUNC static VSurfaceTextureSetManager& GlobalManager();

  ///
  /// @}
  ///

private:
  VArray<VisSurfaceTextureSet_cl*> m_instances;
  static VSurfaceTextureSetManager s_manager;
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
