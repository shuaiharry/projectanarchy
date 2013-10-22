/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/VisionEnginePluginPCH.h>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/VDebugPathRenderer.hpp>

#define DEBUGPATHRENDERER_SERIALIZATION_VERSION_0        0   // initial version
#define DEBUGPATHRENDERER_SERIALIZATION_VERSION_CURRENT  DEBUGPATHRENDERER_SERIALIZATION_VERSION_0


/////////////////////////////////////////////////////////////////////
// VDebugPathRenderer
/////////////////////////////////////////////////////////////////////
V_IMPLEMENT_SERIAL(VDebugPathRenderer, VPathRendererBase, 0, &g_VisionEngineModule);

VDebugPathRenderer::VDebugPathRenderer(int iComponentFlags) :
  VPathRendererBase(iComponentFlags),
  RenderColor(V_RGBA_YELLOW)
{
}

VDebugPathRenderer::~VDebugPathRenderer()
{
}

void VDebugPathRenderer::OnRender(IVisCallbackDataObject_cl* pCallbackData)
{
  IVPathRenderingData* pData = m_spPathRenderingData;
  if (pData == NULL)
    return;

  float fLinkExtent = (pData->GetLinkLength() - pData->GetLinkGap()) / 2.0f;
  for (unsigned int i = 0; i < pData->GetNumLinks(); ++i)
  {
    hkvVec3 vTranslation;
    hkvMat3 mRotation;
    if (!pData->GetLinkTransform(i, mRotation, vTranslation))
      continue;

    hkvVec3 v1 = hkvVec3(-fLinkExtent, 0.0f, 0.0f);
    hkvVec3 v2 = -v1;

    v1 = (mRotation * v1) + vTranslation;
    v2 = (mRotation * v2) + vTranslation;

    Vision::Game.DrawSingleLine(v1, v2, RenderColor);
  }
}

void VDebugPathRenderer::Serialize(VArchive &ar)
{
  VPathRendererBase::Serialize(ar);

  if (ar.IsLoading())
  {
    unsigned int iVersion = 0;
    ar >> iVersion;
    VASSERT_MSG(iVersion <= DEBUGPATHRENDERER_SERIALIZATION_VERSION_CURRENT, "Invalid version of serialized data!");

    ar >> RenderColor;
  }
  else
  {
    ar << (unsigned int)DEBUGPATHRENDERER_SERIALIZATION_VERSION_CURRENT;
    ar << RenderColor;
  }
}

// ----------------------------------------------------------------------------
START_VAR_TABLE(VDebugPathRenderer, VPathRendererBase, "Debug path renderer (renders each link as a simple line)", VVARIABLELIST_FLAGS_NONE, "Path Renderer (Debug)")
  DEFINE_VAR_COLORREF(VDebugPathRenderer, RenderColor, "Color for rendering the path.", "255,255,0,255", 0, NULL);
END_VAR_TABLE

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
