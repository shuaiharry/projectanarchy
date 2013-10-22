/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VScaleformTexture.hpp

#ifndef VSCLAEFORMRENDERTARGET_HPP_INCLUDED
#define VSCLAEFORMRENDERTARGET_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformManager.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformMovie.hpp>

///
/// \brief  
///   Use a Scaleform movie as texture
///
class VScaleformTexture : public VTextureObject
{
public:
  /// \brief  
  ///   Constructor
  VScaleformTexture();

  /// \brief  
  ///   Destructor
  virtual ~VScaleformTexture();

  SCALEFORM_IMPEXP virtual BOOL Reload() HKV_OVERRIDE;

  SCALEFORM_IMPEXP virtual BOOL Unload() HKV_OVERRIDE;

  SCALEFORM_IMPEXP virtual void OnEnterBackground() HKV_OVERRIDE;

  SCALEFORM_IMPEXP virtual void OnLeaveBackground() HKV_OVERRIDE;

  static inline int GetResourceCount() 
  {
    return s_iResourceCount;
  }

private:
  // Create a render target for the texture
  void CreateRenderTarget();

  // Member variables
  VScaleformMovieInstancePtr m_spMovie;     ///< The Scaleform movie to use
  VisContextCameraPtr m_spCamera;           ///< Internally used camera to render the movie
  VisRenderContextPtr m_spContext;          ///< The used render context
  VisRenderableTexturePtr m_spRenderTarget; ///< The internally used renderable texture

  static int s_iResourceCount;
};

///
/// \brief  
///   Texture format provider for Scaleform (SWF/GFX) movies
///
class VScaleformTextureFormatProvider : public IVTextureFormatProvider
{
public:
  SCALEFORM_IMPEXP virtual const char** GetSupportedFileExtensions(int &iListCount) HKV_OVERRIDE
  {
    static const char* formats[] = { "swf", "gfx" };

    iListCount = V_ARRAY_SIZE(formats);
    return formats;
  }

  SCALEFORM_IMPEXP virtual VTextureObject* CreateTexture(const char *szFilename, int &iFlags) HKV_OVERRIDE;
};


#endif // VSCLAEFORMRENDERTARGET_HPP_INCLUDED

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
