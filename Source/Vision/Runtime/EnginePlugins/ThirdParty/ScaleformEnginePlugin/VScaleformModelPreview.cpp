/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/ScaleformEnginePlugin.hpp>

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/GUI/VMenuIncludes.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformManager.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformMovie.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformModelPreview.hpp>

#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Player.h"
#include "GFx/GFx_Resource.h"
#include "GFx/GFx_ImageResource.h"

#if defined(WIN32) || defined(_VISION_XENON) || defined(_VISION_PS3)

VScalefromModelPreview::VScalefromModelPreview(VScaleformMovieInstance * pMainMovieInstance,
                                               VisBaseEntity_cl *pPreviewEntity,
                                               const char * szTextureName,
                                               const char * szMoviePathAndName,
                                               int iSizeX, int iSizeY, float fFovH, float fFovV) :
    m_pMainMovieInstance(pMainMovieInstance),
    m_sTextureName(szTextureName),
    m_sMoviePathAndName(szMoviePathAndName)
{
  VASSERT_MSG(pMainMovieInstance!=NULL, "The main movie is required for VScalefromModelPreview!");
  VASSERT_MSG(pPreviewEntity!=NULL, "Specify an entity for the VScalefromModelPreview!");
  VASSERT_MSG(szTextureName!=NULL, "Specify a texture for the VScalefromModelPreview!");

  //Scaleform::StringBuffer buffer2;
  //Scaleform::Memory::pGlobalHeap->MemReport(buffer2, Scaleform::MemoryHeap::MemReportFull);
  //Vision::Error.Warning("\n-----------------\n1st report:\n\n%s\n-----------------\n", buffer2.ToCStr());

  VString sTargetName;
  sTargetName.Format("ScaleformModelPreviewComponent:%p", pPreviewEntity);
  m_spModelPreview = new VModelPreviewComponent(sTargetName.AsChar());

  m_spModelPreview->CreateEmptyLightGrid();

  m_spModelPreview->SetPreviewEntity(pPreviewEntity);

#ifdef _VR_DX11
  //this switch is required for Scaleform DX11, because it cannot handle typeless textures so far!
  bool bTypedRenderTargetsActive = Vision::Renderer.GetUseTypedRenderTargets()!=0;
  if(!bTypedRenderTargetsActive)
    Vision::Renderer.SetUseTypedRenderTargets(true);
#endif

#ifdef HK_DEBUG_SLOW
  bool bSuccess =
#endif
  m_spModelPreview->InitComponent(iSizeX, iSizeY, fFovH, fFovV);
#ifdef HK_DEBUG_SLOW
  VASSERT_MSG(bSuccess, "Failed to initialize internal model preview component!");
#endif

#ifdef _VR_DX11
  if(!bTypedRenderTargetsActive)
    Vision::Renderer.SetUseTypedRenderTargets(false);
#endif

  Reassign();

  //Scaleform::StringBuffer buffer;
  //Scaleform::Memory::pGlobalHeap->MemReport(buffer, Scaleform::MemoryHeap::MemReportFull);
  //Vision::Error.Warning("\n-----------------\n2nd report:\n\n%s\n-----------------\n", buffer.ToCStr());

  Vision::Callbacks.OnEnterForeground += this;
}

VScalefromModelPreview::~VScalefromModelPreview()
{
  Vision::Callbacks.OnEnterForeground -= this;
}

//-----------------------------------------------------------------------------------

void VScalefromModelPreview::Reassign()
{
  VASSERT_MSG(m_pMainMovieInstance!=NULL, "Main movie instance invalid!")

  Scaleform::Ptr<Scaleform::GFx::MovieDef> spMovieDef;

  //use movie def of the main movie if so separate movie has been specified
  if(m_sMoviePathAndName.IsEmpty())
  {
    spMovieDef = *m_pMainMovieInstance->GetGFxMovieInstance()->GetMovieDef();
  }
  else
  {
    spMovieDef = *VScaleformManager::GlobalManager().GetLoader()->CreateMovie(
      m_sMoviePathAndName, Scaleform::GFx::Loader::LoadAll|Scaleform::GFx::Loader::LoadWaitCompletion);
  }

  VASSERT_MSG(spMovieDef.GetPtr()!=NULL, "Couldn't find movie definition for child swf");

  //grab the image, which will be our target in the SWF file
  Scaleform::GFx::Resource* pResource = spMovieDef->GetResource(m_sTextureName);
  VASSERT_MSG(pResource, "Cannot find target texture inside specified SWF file!");
  VASSERT_MSG(pResource->GetResourceType() == Scaleform::GFx::Resource::RT_Image, "SWF resource if of wrong type!");

  //It is an image...
  Scaleform::GFx::ImageResource* pImageResource = (Scaleform::GFx::ImageResource*)pResource;

  //grab the target texture
  VisRenderableTexture_cl *pTexture = m_spModelPreview->GetRenderableTexture();
  Scaleform::Ptr<Scaleform::Render::TextureImage> spNewTexture = *VScaleformManager::GlobalManager().ConvertTexture(pTexture);
  VASSERT_MSG(spNewTexture.GetPtr()!=NULL, "Unable to convert texture");

  // Note:
  // =====
  // Scaling could causes problems in release builds, since gfxExport by default bakes images together.
  // This leads to a wrong representation in the release. If you configure gfxExport in the right way,
  // so that it does not bake images together for the movie containing the renderable texture you can enable this code:
  //
  //Scaleform::Render::ImageBase* pImageBase = pImageResource->GetImage();
  //VASSERT_MSG(pImageBase, "Image base of specified resource is not valid!");
  //Scaleform::Render::ImageSize renderTargetSize(iSizeX, iSizeY);
  //Scaleform::Render::ImageSize swfImageSize = pImageBase->GetSize();
  //Scaleform::GFx::Size<float> scaleParameters(((float)swfImageSize.Width) / ((float)renderTargetSize.Width), ((float)swfImageSize.Height) / ((float)renderTargetSize.Height));
  //Scaleform::GFx::Matrix2F textureMatrix = Scaleform::GFx::Matrix2F::Scaling(scaleParameters.Width, scaleParameters.Height );
  //spNewTexture->SetMatrix(textureMatrix);

  pImageResource->SetImage(spNewTexture);
  m_pMainMovieInstance->GetGFxMovieInstance()->ForceUpdateImages();
}

void VScalefromModelPreview::Update(float fTimeDiff)
{
  m_spModelPreview->Update(fTimeDiff);
}

void VScalefromModelPreview::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
  // Reassign texture after device loss / backgrounding
  // (Scaleform sets the texture to NULL internally)
  if (pData->m_pSender == &Vision::Callbacks.OnEnterForeground)
  {
    Reassign();
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
