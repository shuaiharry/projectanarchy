/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file vScaleformManager.hpp
#ifndef __VSCALEFORMMODELPREVIEW_HPP_INCLUDED
#define __VSCALEFORMMODELPREVIEW_HPP_INCLUDED

#if defined(WIN32) || defined(_VISION_XENON) || defined(_VISION_PS3)

//forward decls
namespace Scaleform
{
  namespace GFx
  {
    class MovieDef;
    class ImageResource;
  }

  namespace Render
  {
    class TextureImage;
  }
}


/// \brief Scalefrom model preview: You can use this class to render an
/// 			 entity into a texture of a Scalefrom movie
class VScalefromModelPreview : public VRefCounter, public IVisCallbackHandler_cl
{
public:

  /// \brief Constructor: Create a new model preview within a texture of a Scaleform movie.
  /// \note It is important that all involved resources are loaded with the sample Loader object,
  /// 			ImageCreator object and in case of loaded movies via ActionScript: to use the same
  /// 			path format (e.g. use 'some/path/movie.swf' in AS and in C++)
  ///
  /// \param pMainMovieInstance The movie, which contains the texture to replace - OR -
  /// 													the movie, which loaded the movie containing the texture to replace.
  /// \param pPreviewEntity     The entity to render
  /// \param szTextureName      The name of the texture to replace
  /// \param szMoviePathAndName  (\b optional) The path and name of the movie which contains the target
  /// 													 texture (e.g. 'some/movie.swf'). This parameter is just required if the movie,
  /// 													 which contains the texture was loaded by another movie (the main movie).
  /// \param iSizeX (\b optional) x size of the render target, default is 512.
  /// \param iSizeY (\b optional) y size of the render target, default is 512.
  /// \param fFovH (\b optional) Horizontal Field-Of-View, default is 90.
  /// \param fFovV (\b optional) Vertical Field-Of-View, will be adjusted to the current aspect if 0.
  SCALEFORM_IMPEXP VScalefromModelPreview(VScaleformMovieInstance * pMainMovieInstance, VisBaseEntity_cl *pPreviewEntity,
    const char * szTextureName, const char * szMoviePathAndName = NULL, int iSizeX = 512, int iSizeY = 512, float fFovH = 90, float fFovV = 0);

  /// \brief Destructor.
  SCALEFORM_IMPEXP virtual ~VScalefromModelPreview();

  /// \brief Call the update function when rendering.
  /// \param fTimeDiff The time difference for the update process.
  SCALEFORM_IMPEXP virtual void Update(float fTimeDiff);

  /// \brief Reassign renderable texture to Scaleform movie.
  SCALEFORM_IMPEXP virtual void Reassign();

  /// \brief Gets the preview component in order to access it's properties.
  /// \return The model preview component.
  inline VModelPreviewComponent * GetPreviewComponent() { return m_spModelPreview; }

  /// \brief Callback handler implementation
  SCALEFORM_IMPEXP virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

protected:
  VScaleformMovieInstance * m_pMainMovieInstance;
  VSmartPtr<VModelPreviewComponent> m_spModelPreview;
  VString m_sMoviePathAndName;
  VString m_sTextureName;
};

#endif

#endif //__VSCALEFORMMODELPREVIEW_HPP_INCLUDED

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
