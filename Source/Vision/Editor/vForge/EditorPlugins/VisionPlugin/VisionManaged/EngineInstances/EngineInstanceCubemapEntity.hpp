/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#pragma once

#include <Vision/Editor/vForge/EditorPlugins/VisionPlugin/VisionManaged/EngineInstances/EngineInstanceEntity.hpp>

class CubeMapHandle_cl;

namespace VisionManaged
{
  public ref class EngineInstanceCubemapEntity : public EngineInstanceEntity
  {
  public:
    enum class CubeMapRenderingType_e {
      Scene,
      Specular,
      RendererNode
    };

    EngineInstanceCubemapEntity(Shape3D ^shape);
    virtual void DisposeObject() override;

    // overridden entity functions
    virtual bool OnExport(SceneExportInfo ^info) override;
    CubeMapHandle_cl* GetEntity();

    void SetBlurPasses(int iNumPasses);
    void SetAutoGenMipMaps(bool bStatus);
    void SetCubemapKey(String^ handleKey, String^ materialKey, int iEdgeSize);
    void SetPreviewVisible(bool bStatus);
    void SetRendererConfig(String^ rendererConfig);
    void UpdateCubemap();

    // special functions:
    void SetRenderFilterMask(unsigned int iMask);
    void SetClipPlanes(float fNear, float fFar);
    void SetUpdateParams(bool bContinuous, float fInterval, bool bAlternate);
    bool SaveToFile(String ^filename);
    void SetExportAsEntity(bool bStatus) {m_bExport=bStatus;}
    void AssignCubemapShader(const char *szCubemapKey);
    void SetRenderingType(CubeMapRenderingType_e eRenderingType);
    void SetSpecularPower(float fSpecularPower);

   bool m_bExport;
  };
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
