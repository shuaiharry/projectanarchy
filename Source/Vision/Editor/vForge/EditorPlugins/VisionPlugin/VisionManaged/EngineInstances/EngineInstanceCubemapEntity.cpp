/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Editor/vForge/EditorPlugins/VisionPlugin/VisionManaged/VisionManagedPCH.h>

#include <Vision/Editor/vForge/EditorPlugins/VisionPlugin/VisionManaged/EngineInstances/EngineInstanceCubemapEntity.hpp>

#include <Vision/Runtime/Base/Graphics/Shader/vShaderConstantHelper.hpp>

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/Effects/CubeMapHandle.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/MobileForwardRenderer/VFakeSpecularGenerator.hpp>

namespace VisionManaged
{
  CubeMapHandle_cl* EngineInstanceCubemapEntity::GetEntity()
  {
    return vstatic_cast<CubeMapHandle_cl*>(EngineInstanceEntity::GetEntity());
  }

  EngineInstanceCubemapEntity::EngineInstanceCubemapEntity(Shape3D ^shape) : EngineInstanceEntity("CubeMapHandle_cl", "gizmo_link.model", shape, nullptr, true)
  {
    SetPreviewVisible(false);
    VisBaseEntity_cl *pEntity = GetEntity();
    VDynamicMesh *pModel = pEntity->GetMesh();
    pEntity->SetLightInfluenceBitMask(0);
    pModel->SetAllowShaderAssignment(false); // also never update shader assignment
    pModel->SetSupportMaterialEditing(false); // dont show in the material editor
  }
  

  void EngineInstanceCubemapEntity::DisposeObject()
  {
    EngineInstanceEntity::DisposeObject();
  }
 
  void EngineInstanceCubemapEntity::SetPreviewVisible(bool bStatus)
  {
    VisBaseEntity_cl *pEntity = GetEntity();
    pEntity->SetVisibleBitmask(bStatus ? 1:0);
  }

  void EngineInstanceCubemapEntity::SetRendererConfig(String^ rendererConfig)
  {
    VString configFile;
    ConversionUtils::StringToVString(rendererConfig, configFile);

    IVFileInStream* pStream = Vision::File.Open(configFile.AsChar());

    CubeMapHandle_cl* pCubeMap = GetEntity();

    if(pStream)
    {
      IVRendererNode* pNode = IVRendererNode::ReadFromStream(pStream);
      pCubeMap->SetRendererNode(pNode);
      pStream->Close();
    }
    else
    {
      pCubeMap->SetRendererNode(NULL);
    }
  }


  void EngineInstanceCubemapEntity::AssignCubemapShader(const char *szCubemapKey)
  {
    if (!szCubemapKey || !szCubemapKey[0])
      return;

    char szParamStr[1024];
    sprintf(szParamStr,"Cubemap=%s;PassType=%s;",szCubemapKey, VPassTypeToString(VPT_TransparentPass));
    VisBaseEntity_cl *pEntity = GetEntity();
    Vision::Shaders.LoadShaderLibrary("\\Shaders\\Cubemap.ShaderLib",SHADERLIBFLAG_HIDDEN);
    VCompiledTechnique *pTechnique = Vision::Shaders.CreateTechnique("Cubemap",szParamStr);
    VDynamicMesh *pModel = pEntity->GetMesh();
    if (pModel && pTechnique)
    {
      VisShaderSet_cl *pSet = new VisShaderSet_cl();
      pSet->BuildForDynamicMesh(pModel,NULL,pTechnique);
      pEntity->SetShaderSet(pSet);
    }
  }

  void EngineInstanceCubemapEntity::SetBlurPasses(int iBlurPasses)
  {
    CubeMapHandle_cl *pEntity = GetEntity();
    pEntity->SetBlurPasses(iBlurPasses);
  }

  void EngineInstanceCubemapEntity::SetAutoGenMipMaps(bool bStatus)
  {
    CubeMapHandle_cl *pEntity = GetEntity();
    pEntity->SetAutoGenMipMaps(bStatus);
  }

  void EngineInstanceCubemapEntity::SetCubemapKey(String^ handleKey, String^ materialKey, int iEdgeSize)
  {
    CubeMapHandle_cl *pEntity = GetEntity();

    VString sHandleKey;
    ConversionUtils::StringToVString(handleKey, sHandleKey);
    pEntity->SetCubemapKey(sHandleKey, iEdgeSize);


    VString sMaterialKey;
    ConversionUtils::StringToVString(materialKey, sMaterialKey);
    AssignCubemapShader(sMaterialKey);
  }

  void EngineInstanceCubemapEntity::SetRenderingType(CubeMapRenderingType_e eRenderingType)
  {
    CubeMapHandle_cl *pEntity = GetEntity();
    switch(eRenderingType)
    {
      case CubeMapRenderingType_e::RendererNode:
        pEntity->SetRenderLoop(NULL);
        break;

      case CubeMapRenderingType_e::Specular:
        pEntity->SetRendererNode(NULL);
        pEntity->SetRenderLoop(new VFakeSpecularGenerator);
        break;

      case CubeMapRenderingType_e::Scene:
        pEntity->SetRendererNode(NULL);
        pEntity->SetRenderLoop(new VisionRenderLoop_cl);
        break;
    }
  }

  void EngineInstanceCubemapEntity::SetSpecularPower(float fSpecularPower)
  {
    CubeMapHandle_cl *pEntity = GetEntity();
    if(pEntity->GetRenderLoop()->GetTypeId() == VFakeSpecularGenerator::GetClassTypeId())
    {
      static_cast<VFakeSpecularGenerator*>(pEntity->GetRenderLoop())->SetSpecularPower(fSpecularPower);
    }
  }

  void EngineInstanceCubemapEntity::SetRenderFilterMask(unsigned int iMask)
  {
    CubeMapHandle_cl *pEntity = GetEntity();
    pEntity->SetRenderFilterMask(iMask);
  }

  void EngineInstanceCubemapEntity::SetClipPlanes(float fNear, float fFar)
  {
    CubeMapHandle_cl *pEntity = GetEntity();
    pEntity->SetClipPlanes(fNear, fFar);
  }

  void EngineInstanceCubemapEntity::SetUpdateParams(bool bContinuous, float fInterval, bool bAlternate)
  {
    CubeMapHandle_cl *pEntity = GetEntity();
    pEntity->SetContinuousUpdate(bContinuous);
    pEntity->SetUpdateParams(fInterval);
    pEntity->SetAlternatingUpdate(bAlternate);
  }


  void EngineInstanceCubemapEntity::UpdateCubemap()
  {
    CubeMapHandle_cl *pEntity = GetEntity();
    pEntity->Invalidate();
  }


  bool EngineInstanceCubemapEntity::OnExport(SceneExportInfo ^info) 
  {
    if(!m_bExport)
    {
      return true;
    }

    VisBaseEntity_cl *pEntity = GetEntity();
    if (pEntity == NULL)
      return true;

    // export without model assigned:
    VDynamicMeshPtr spOldModel = pEntity->GetMesh();
    pEntity->SetMesh(static_cast<VDynamicMesh *>(NULL));
    VArchive &ar = *((VArchive *)info->NativeShapeArchivePtr.ToPointer());
    ar << pEntity;
    pEntity->SetMesh(spOldModel);
    return true;
  }


  bool EngineInstanceCubemapEntity::SaveToFile(String ^filename)
  {
    CubeMapHandle_cl *pEntity = GetEntity();
    if (!pEntity || !pEntity->GetCubeMapTexture())
    {
      return false;
    }

    String ^absFilename = EditorManager::Project->MakeAbsolute(filename);
    VString sFilename,sAbsFilename;
    ConversionUtils::StringToVString(filename,sFilename);
    ConversionUtils::StringToVString(absFilename,sAbsFilename);
    HRESULT res = S_FALSE;

    ManagedBase::RCS::GetProvider()->EditFile(absFilename);

    VTextureObject* pTexture = pEntity->GetCubeMapTexture();

    int iSize = pTexture->GetTextureWidth();

    VASSERT_MSG(pTexture->GetTextureHeight() == pTexture->GetTextureWidth(), "Cubemap must be square");

    int iDataSize = 6 * pEntity->GetCubeMapTexture()->GetMipLevelSize(0);

    // Create a buffer to store the DDS file data
    array<byte>^ ddsData = gcnew array<byte>(sizeof(V_DDSHEADER) + sizeof(V_DDSHEADER_DXT10) + iDataSize);
    pin_ptr<unsigned char> pBuffer = &ddsData[0];

    unsigned char* pOut = pBuffer + sizeof(V_DDSHEADER) + sizeof(V_DDSHEADER_DXT10);

    // Manually read back into the DDS buffer. Currently we have no read back for cubemaps in the Engine,
    // but feel free to move this code once it's used more often.
#if defined(_VR_DX11)

    ID3D11Texture2D* pOriginalTexture = static_cast<ID3D11Texture2D*>(pTexture->GetD3DResource());

    if(!pOriginalTexture)
    {
      return false;
    }

    D3D11_TEXTURE2D_DESC desc;
    pOriginalTexture->GetDesc(&desc);

    int iMipLevelsSrc = desc.MipLevels;

    desc.BindFlags = 0;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;
    desc.ArraySize = 1;
    desc.MipLevels = 1;
    
    // Create a temporary texture for CPU readback
    VSmartPtr<ID3D11Texture2D> spStagingTexture;
    res = VVideo::GetD3DDevice()->CreateTexture2D(&desc, NULL, &spStagingTexture.m_pPtr);

    if(FAILED(res))
    {
      return false;
    }

    // Copy each face texture into the DDS buffer
    for(int iFace = 0; iFace < 6; iFace++)
    {
        VVideo::GetD3DDeviceContext()->CopySubresourceRegion(spStagingTexture, 0, 0, 0, 0, pOriginalTexture, iFace * iMipLevelsSrc, NULL);

        D3D11_MAPPED_SUBRESOURCE subresource;
        res = VVideo::GetD3DDeviceContext()->Map(spStagingTexture, 0, D3D11_MAP_READ, 0, &subresource);

        if(FAILED(res))
        {
          return false;
        }

        unsigned char* pIn = reinterpret_cast<unsigned char*>(subresource.pData);
        
        int iDestPitch = iSize * 4;
        int iSrcPitch = subresource.RowPitch;

        for(int iRow = 0; iRow < iSize; iRow++)
        {
          memcpy(pOut, pIn, iDestPitch);

          pOut += iDestPitch;
          pIn += iSrcPitch;
        }

        VVideo::GetD3DDeviceContext()->Unmap(spStagingTexture, 0);
    }
    
#else

    IDirect3DCubeTexture9* pOriginalTexture = static_cast<IDirect3DCubeTexture9*>(pTexture->GetD3DInterface());

    if(!pOriginalTexture)
    {
      return false;
    }

    // Create a temporary texture for CPU readback
    VSmartPtr<IDirect3DTexture9> spStagingTexture;
    res = Vision::Video.GetD3DDevice()->CreateTexture(iSize, iSize, 1,  D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &spStagingTexture.m_pPtr, NULL);

    if(FAILED(res))
    {
      return false;
    }

    // Copy each face texture into the DDS buffer
    for(int iFace = 0; iFace < 6; iFace++)
    {
      VSmartPtr<IDirect3DSurface9> spSrcSurface;
      pOriginalTexture->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(iFace), 0, &spSrcSurface.m_pPtr);

      VSmartPtr<IDirect3DSurface9> spDestSurface;
      res = spStagingTexture->GetSurfaceLevel(0, &spDestSurface.m_pPtr);

      if(FAILED(res))
      {
        return false;
      }

      res = Vision::Video.GetD3DDevice()->GetRenderTargetData(spSrcSurface, spDestSurface);

      if(FAILED(res))
      {
        return false;
      }

      D3DLOCKED_RECT lockedRect;
      res = spDestSurface->LockRect(&lockedRect, NULL, D3DLOCK_READONLY);

      if(FAILED(res))
      {
        return false;
      }

      unsigned char* pIn = reinterpret_cast<unsigned char*>(lockedRect.pBits);

      int iDestPitch = iSize * 4;
      int iSrcPitch = lockedRect.Pitch;

      for(int iRow = 0; iRow < iSize; iRow++)
      {
        memcpy(pOut, pIn, iDestPitch);

        pOut += iDestPitch;
        pIn += iSrcPitch;
      }

      spDestSurface->UnlockRect();
    }

#endif


    // Write the rest of the DDS header. This is a bit tricky, since we need to make sure to write
    // a DDS file exactly so that all tools we use for transformation later understand them.
    // We're writing the file as a DXT10 DDS file, since both PVRTexTool and texconv can read them.
    V_DDSHEADER* pHeader = reinterpret_cast<V_DDSHEADER*>(pBuffer);

    memset(pHeader, 0, sizeof(V_DDSHEADER) + sizeof(V_DDSHEADER_DXT10));

    pHeader->dwMagic = ' SDD';
    pHeader->dwSize = 124;
    pHeader->dwFlags = DDS_HEADER_FLAGS_TEXTURE;
    pHeader->dwHeight = iSize;
    pHeader->dwWidth = iSize;
    pHeader->dwPitchOrLinearSize = iSize * 4;
    pHeader->dwDepth = 1;
    pHeader->dwMipMapCount = 1;
    pHeader->ddpfPixelFormat.dwSize = 32;
    pHeader->ddpfPixelFormat.dwFlags = V_DDS_FOURCC;
    pHeader->ddpfPixelFormat.dwFourCC = '01XD';
    pHeader->ddpfPixelFormat.dwRGBBitCount = 32;
    pHeader->ddsCaps.dwCaps1 = 0x1008;  // DDSCAPS_COMPLEX | DDSCAPS_TEXTURE
    pHeader->ddsCaps.dwCaps2 = 0xFE00;  // DDSCAPS2_CUBEMAP | DDS_CUBEMAP_ALLFACES

    V_DDSHEADER_DXT10* pHeaderDx10 = reinterpret_cast<V_DDSHEADER_DXT10*>(pBuffer + sizeof(V_DDSHEADER));
    pHeaderDx10->dxgiFormat = 87;       // DXGI_FORMAT_B8G8R8A8_UNORM
    pHeaderDx10->resourceDimension = 3; // DDS_DIMENSION_TEXTURE2D 
    pHeaderDx10->miscFlags = 0x04;      // DDS_RESOURCE_MISC_TEXTURECUBE
    pHeaderDx10->arraySize = 1;

    // Write out, locking so that the asset management won't accidentally start transforming the unfinished file
    try
    {
      System::IO::Directory::CreateDirectory(System::IO::Path::GetDirectoryName(absFilename));
      System::IO::FileStream^ stream = gcnew System::IO::FileStream(absFilename, System::IO::FileMode::Create, System::IO::FileAccess::Write, System::IO::FileShare::None);
      stream->Write(ddsData, 0, ddsData->Length);
      delete stream;
    }
    catch(Exception^ ex)
    {
      EditorManager::DumpException(ex);
      return false;
    }

    VFileTime newFileTime;
    if (VFileHelper::GetModifyTime(sAbsFilename.AsChar(), newFileTime))
    {
      pEntity->GetCubeMapTexture()->SetTimeStamp(newFileTime);
    }

    ManagedBase::RCS::GetProvider()->AddFile(absFilename, true);
    return true;
  }
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
