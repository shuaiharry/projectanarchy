/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/ScaleformEnginePlugin.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformTexture.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformInternal.hpp>

int VScaleformTexture::s_iResourceCount = 0;

VScaleformTexture::VScaleformTexture()
  : VTextureObject(&Vision::TextureManager.GetManager())
  , m_spMovie(NULL)
  , m_spCamera(NULL)
  , m_spContext(NULL)
  , m_spRenderTarget(NULL)
{
  s_iResourceCount++;
}

VScaleformTexture::~VScaleformTexture()
{
  s_iResourceCount--;
}

BOOL VScaleformTexture::Reload()
{
  VASSERT(m_spMovie == NULL);
  m_spMovie = VScaleformManager::GlobalManager().LoadMovie(GetFilename());

  if (m_spMovie == NULL)
    return FALSE;

  m_spMovie->m_bIsTexture = true;

  CreateRenderTarget();
  m_spCamera = new VisContextCamera_cl();
  m_spContext = new VisRenderContext_cl(m_spCamera, 90.0f, 90.0f, 
    m_spRenderTarget->GetTextureWidth(), m_spRenderTarget->GetTextureHeight(), 5.0f, 30000.0f);

  m_spContext->SetRenderTarget(0, m_spRenderTarget);

  m_spContext->SetRenderingEnabled(true);
  m_spContext->SetRenderLoop(new VScaleformMovieExclusiveRenderLoop(m_spMovie));
  m_spContext->SetRenderFilterMask(VIS_ENTITY_VISIBLE_IN_TEXTURE);
  Vision::Contexts.AddContext(m_spContext);

  m_spMovie->SetHandleInput(false);

  // Do not call SetDimensions here since it would call UpdateViewport with the wrong render context.
  m_spMovie->m_iPosX = 0;
  m_spMovie->m_iPosY = 0;
  m_spMovie->m_iWidth = SF_MOVIE_SIZE_RENDERCONTEXT;
  m_spMovie->m_iHeight = SF_MOVIE_SIZE_RENDERCONTEXT;

  m_spMovie->SetVisibleBitmask(VIS_ENTITY_VISIBLE_IN_TEXTURE);

  // advance the movie once
  m_spMovie->m_pAdvanceTask->Schedule(0.05f); // has to be > 0
  m_spMovie->m_pAdvanceTask->WaitUntilFinished();

  // Do not call base implementation (VTextureObject::Reload()) since this tries to load from file.
  return TRUE;
}

BOOL VScaleformTexture::Unload()
{
  if (m_spContext != NULL)
  {
    m_spContext->SetRenderTarget(0,NULL);
    Vision::Contexts.RemoveContext(m_spContext);
  }

  m_spRenderTarget = NULL;
  m_spCamera = NULL;
  m_spContext = NULL;

  if (m_spMovie != NULL)
  {
    VScaleformManager::GlobalManager().UnloadMovie(m_spMovie);
    m_spMovie = NULL;
  }

  return VTextureObject::Unload();
}

void VScaleformTexture::CreateRenderTarget()
{
  VisRenderableTextureConfig_t config;
  m_spMovie->GetAuthoredSize(config.m_iWidth, config.m_iHeight); // take original size from movie
  config.m_eFormat = VTextureLoader::DEFAULT_RENDERTARGET_FORMAT_32BPP;
  
  m_spRenderTarget = Vision::TextureManager.CreateRenderableTexture("<ScaleformTarget>",config);

#if defined(_VR_DX9)
  m_spD3Dinterface = m_spRenderTarget->GetD3DInterface();
#elif defined(_VR_DX11)
  m_spD3DResource = m_spRenderTarget->GetD3DResource();
  m_spShaderResourceView = m_spRenderTarget->GetShaderResourceView();
#elif defined(_VR_GLES2)
  m_GLFormat = m_spRenderTarget->m_GLFormat;
  m_GLHandle = m_spRenderTarget->m_GLHandle;
#endif

  // copy some properties:
  m_iSizeX = m_spRenderTarget->GetTextureWidth();
  m_iSizeY = m_spRenderTarget->GetTextureHeight();
  m_eTextureFormat = m_spRenderTarget->GetTextureFormat();
  m_eTextureType = m_spRenderTarget->GetTextureType();
}

void VScaleformTexture::OnEnterBackground()
{
  // Do not unload the whole resource,
  // only destroy the graphics resources used.

#if defined(_VR_DX9)
  m_spD3Dinterface = NULL;
#elif defined(_VR_DX11)
  m_spD3DResource = NULL;
  m_spShaderResourceView = NULL;
#elif defined(_VR_GLES2)
  m_GLHandle = 0;
#endif

  m_spRenderTarget = NULL;

  if (m_spContext != NULL)
    m_spContext->SetRenderTarget(0, NULL);
}

void VScaleformTexture::OnLeaveBackground()
{
  if (IsLoaded())
    CreateRenderTarget();

  if (m_spContext != NULL)
    m_spContext->SetRenderTarget(0, m_spRenderTarget);
}

VTextureObject* VScaleformTextureFormatProvider::CreateTexture(const char *szFilename, int &iFlags)
{
  VScaleformTexture* pTexture = new VScaleformTexture();
  pTexture->SetFilename(szFilename);
  pTexture->EnsureLoaded();
  return pTexture;
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
