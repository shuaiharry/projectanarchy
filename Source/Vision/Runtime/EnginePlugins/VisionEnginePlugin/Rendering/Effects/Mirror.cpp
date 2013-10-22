/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/VisionEnginePluginPCH.h>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/Effects/Mirror.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/Effects/MirrorRenderLoop.hpp>
#include <Vision/Runtime/Engine/SceneElements/VisApiSky.hpp>


VisMirrorManager_cl VisMirrorManager_cl::g_MirrorManager;
VisCallback_cl VisMirrorManager_cl::OnMirrorRenderHook;


#define MIRROR_VERSION_1                1
#define MIRROR_VERSION_2                2
#define MIRROR_VERSION_3                3
#define MIRROR_VERSION_4                4
#define MIRROR_VERSION_5                5
#define MIRROR_VERSION_6                6
#define MIRROR_VERSION_7                7
#define MIRROR_VERSION_8                8
#define MIRROR_VERSION_9                9
#define MIRROR_VERSION_10               10
#define MIRROR_VERSION_11               11
#define MIRROR_VERSION_12               12 // added render hook
#define MIRROR_VERSION_13               13 // render hook binary format changed
#define MIRROR_VERSION_14               14 // added m_bLODFromRefContext
#define MIRROR_VERSION_15               15 // render hook mismatch fix
#define MIRROR_CURRENT_VERSION          MIRROR_VERSION_15


// Helper function which checks whether the current context is registered in an active renderer node
static inline bool IsContextRegistered(VisRenderContext_cl *pContext)
{
  for (int iRendererNode=0; iRendererNode<Vision::Renderer.GetRendererNodeCount(); iRendererNode++)
  {
    IVRendererNode *pNode = Vision::Renderer.GetRendererNode(iRendererNode);
    if (pNode)
    {
      if (pNode->IsContextRegistered(pContext))
        return true;
    }
  }

  return false;
}

VisMirrorManager_cl::VisMirrorManager_cl()
{
}

void VisMirrorManager_cl::OneTimeInit()
{
  // we need the following callbacks:
  Vision::Callbacks.OnWorldDeInit += this;
  Vision::Callbacks.OnEnterBackground += this;
  Vision::Callbacks.OnRendererNodeChanged += this;
  Vision::Callbacks.OnRendererNodeSwitching += this;
  Vision::Callbacks.OnReassignShaders += this;
}

void VisMirrorManager_cl::OneTimeDeInit()
{
  Vision::Callbacks.OnWorldDeInit -= this;
  Vision::Callbacks.OnEnterBackground -= this;
  Vision::Callbacks.OnRendererNodeChanged -= this;
  Vision::Callbacks.OnRendererNodeSwitching -= this;
  Vision::Callbacks.OnReassignShaders -= this;
}

VisMirrorManager_cl &VisMirrorManager_cl::GlobalManager() 
{
  return g_MirrorManager;
}

void VisMirrorManager_cl::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
  if (pData->m_pSender==&Vision::Callbacks.OnRendererNodeChanged)
  {
    VisRendererNodeChangedDataObject_cl &data = *((VisRendererNodeChangedDataObject_cl *)pData);

    if (data.m_spAddedNode != NULL)
    {
      const int iMirrorCount = m_Instances.Count();
      for (int i=0; i<iMirrorCount; ++i)
      {
        data.m_spAddedNode->AddContext(m_Instances.GetAt(i)->m_spReflectionContext);
      }
    }

    if (data.m_spRemovedNode != NULL)
    {
      const int iMirrorCount = m_Instances.Count();
      for (int i=0; i<iMirrorCount; ++i)
      {
        data.m_spRemovedNode->RemoveContext(m_Instances.GetAt(i)->m_spReflectionContext);
      }
    }

    return;
  }

  if (pData->m_pSender==&Vision::Callbacks.OnRendererNodeSwitching)
  {
    // Handle all mirrors and see whether they have to be rendered
    VisRendererNodeDataObject_cl &data = *static_cast<VisRendererNodeDataObject_cl *>(pData);

    const int iMirrorCount = m_Instances.Count();
    for (int i=0; i<iMirrorCount; ++i)
    {
      VisMirror_cl *pMirror = m_Instances.GetAt(i);
      if (!pMirror->IsActive())
      {
        continue;
      }
      if (data.m_pRendererNode == NULL)
      {
        continue;
      }

      VisRenderContext_cl *pRefContext = data.m_pRendererNode->GetReferenceContext();
      if (pRefContext==NULL) // This should not happen unless the node is in invalid (= de-initialized) state
        continue;
      if ((pRefContext->GetRenderFlags() & (VIS_RENDERCONTEXT_FLAG_NOMIRRORS|VIS_RENDERCONTEXT_FLAG_NO_WORLDGEOM)))
        continue;
      if ((pRefContext->GetRenderFlags() & VIS_RENDERCONTEXT_FLAG_VIEWCONTEXT)==0)
        continue;

      VASSERT(data.m_pRendererNode->IsContextRegistered(pMirror->m_spReflectionContext));

      pMirror->HandleMirror(data);
    }
    return;
  }

  if (pData->m_pSender==&Vision::Callbacks.OnWorldDeInit)
  {
    int iMirrorCount = m_Instances.Count();
    for (int i=iMirrorCount-1;i>=0;i--) // Backwards to keep collection intact
    {
      VisMirror_cl *pMirror = m_Instances.GetAt(i);
      pMirror->ClearViewVisibilityCollectors();
      pMirror->DisposeObject();
    }
    m_Instances.Clear();
    return;
  }

  if (pData->m_pSender==&Vision::Callbacks.OnEnterBackground)
  {
    const int iMirrorCount = m_Instances.Count();
    for (int i=0; i<iMirrorCount; ++i)
    {
      m_Instances.GetAt(i)->ClearViewVisibilityCollectors();
    }
    return;
  }

  // We only need to respond to this callback outside the editor, because vForge has its own re-assignment callback.
  // also note that Vision::Callbacks.OnReassignShaders is not triggered during runtime
  if (pData->m_pSender==&Vision::Callbacks.OnReassignShaders && !Vision::Editor.IsInEditor())
  {
    const int iMirrorCount = m_Instances.Count();
    for (int i=0; i<iMirrorCount; ++i)
    {
      m_Instances.GetAt(i)->ReassignEffect();
    }
    return;
  }
}

int VisMirror_cl::GetNumber() const
{
  VASSERT(m_pParentManager!=NULL)
  return m_pParentManager->m_Instances.Find(this);
}

void VisMirror_cl::SetUseHDR(bool bUseHDR)
{
  m_bUseHDR = bUseHDR;
  SetResolution(m_iResolution);
}

void VisMirror_cl::SetResolution(int iRes)
{
  if (iRes==m_iResolution && m_spRenderTarget_Refl)
    return;

  m_iResolution = iRes;

  // Create render target
  VisRenderableTextureConfig_t config;
  config.m_iHeight = m_iResolution;
  config.m_iWidth  = m_iResolution;
  if (!m_bUseHDR)
  {
    config.m_eFormat = VTextureLoader::DEFAULT_RENDERTARGET_FORMAT_32BPP;
  }
  else
  {
#if defined (_VISION_XENON)
    config.m_eFormat = VTextureLoader::R10G10B10A2F_RT;
#else
    config.m_eFormat = VTextureLoader::R16G16B16A16F;
#endif
  }
  
  VisRenderableTexture_cl *pTex =  Vision::TextureManager.CreateRenderableTexture("<Mirror>",config);
  VASSERT(pTex); // There is no alternative to renderable texture
  if (!pTex)
  {
    m_bSupported = false;
    return;
  }
  pTex->SetResourceFlag(VRESOURCEFLAG_AUTODELETE);
  m_spRenderTarget_Refl = pTex;

  // Create depth stencil target
  config.m_eFormat = VVideo::GetSupportedDepthStencilFormat(VTextureLoader::D24S8, *Vision::Video.GetCurrentConfig());
  config.m_bIsDepthStencilTarget = true;
  config.m_bRenderTargetOnly = true;

  VisRenderableTexture_cl *pDepthTex =  Vision::TextureManager.CreateRenderableTexture("<MirrorDepthStencil>",config);
  VASSERT(pDepthTex); // There is no alternative to renderable texture
  if (!pDepthTex)
  {
    m_bSupported = false;
    return;
  }
  pDepthTex->SetResourceFlag(VRESOURCEFLAG_AUTODELETE);
  m_spDepthStencilTarget_Refl = pDepthTex;

  // If the mirror is already initialized, update the target textures in the context too
  if (m_spReflectionContext)
  {
    m_spReflectionContext->SetRenderTarget(0, m_spRenderTarget_Refl);
    m_spReflectionContext->SetDepthStencilTarget(m_spDepthStencilTarget_Refl);
    if(m_spMeshObj && m_spMeshObj->GetCurrentMeshBuffer())
    {
      m_spMeshObj->GetCurrentMeshBuffer()->SetBaseTexture(m_spRenderTarget_Refl);
    }	
  }
}

void VisMirror_cl::InitMirror()
{
  SetUseEulerAngles(FALSE);
  VisMeshBufferObject_cl::SetEnableSubOrderSorting(TRUE); // Turn this globally on to have correct sorting across mirror objects
  if (m_pParentManager)
    m_pParentManager->m_Instances.AddUnique(this);

  m_iViewContextCount = 0;

  m_bVisibleThisFrame = false;
  m_spDynamicMesh = NULL;
  m_vModelScale.set(1.f,1.f,1.f);

  // The shape of the mirror:
  m_vLocalMirrorVert[0].set(-0.5f, 0.5f, 0.f);
  m_vLocalMirrorVert[1].set( 0.5f, 0.5f, 0.f);
  m_vLocalMirrorVert[2].set( 0.5f,-0.5f, 0.f);
  m_vLocalMirrorVert[3].set(-0.5f,-0.5f, 0.f);

  SetResolution(m_iResolution);

  // Add mirror context to global list of active contexts create a render context
  m_spReflectionContext = new VisRenderContext_cl();
  m_spReflectionContext->SetUsageHint(VIS_CONTEXTUSAGE_MIRROR);
  m_spReflectionContext->SetName("Mirror");

  int iFlags = VIS_RENDERCONTEXT_FLAGS_SECONDARYCONTEXT;
  // Remove the following flags from context:
  iFlags &= ~VIS_RENDERCONTEXT_FLAG_SHOW_DEBUGOUTPUT;
  // We typically don't want occlusion query in mirrors
  iFlags &= ~VIS_RENDERCONTEXT_FLAG_USE_OCCLUSIONQUERY;

  m_spReflectionContext->SetCamera(new VisContextCamera_cl());
  m_spReflectionContext->SetRenderFlags(iFlags);
  m_spReflectionContext->SetRenderTarget(0, m_spRenderTarget_Refl);
  m_spReflectionContext->SetDepthStencilTarget(m_spDepthStencilTarget_Refl);
  m_spReflectionContext->SetRenderLoop(new MirrorRenderLoop_cl(this));
  m_spReflectionContext->SetUserData(this); // set the mirror pointer as user data

  m_pSourceContext = Vision::Contexts.GetMainRenderContext();

  // Add the reflection context to all currently registered renderer nodes. If nodes are added/removed later on,
  // the OnRendererNodeChanged callback will take care of them.
  int iRendererNodeCount = Vision::Renderer.GetRendererNodeCount();
  for (int i=0; i<iRendererNodeCount; i++)
  {
    IVRendererNode *pRendererNode = Vision::Renderer.GetRendererNode(i);
    if (pRendererNode != NULL)
    {
      pRendererNode->AddContext(m_spReflectionContext);
    }
  }

  m_spReflectionContext->SetPriority(VIS_RENDERCONTEXTPRIORITY_MIRROR);
  m_spReflectionContext->SetRenderingEnabled(true);
  SetRenderFilterMask(VIS_ENTITY_VISIBLE_IN_MIRROR);
  SetVisibleBitmask(VIS_ENTITY_VISIBLE_IN_WORLD);
  m_uiRenderHook = VRH_DECALS;
  SetShowDebugMirrorTexture(false);
}

VisMirror_cl::VisMirror_cl(VisMirrorManager_cl *pManager, int iResolution, bool bUseHDR)
{
  m_pParentManager = pManager;
  m_pWorldSurface = NULL;
  m_pReferenceObject = NULL;
  m_iResolution = iResolution;
  m_bUseHDR = bUseHDR;
  m_bActive = m_bSupported = true;
  m_bValidFX = false;
  m_bLODFromRefContext = true;
  m_fSizeX = m_fSizeY = 100.f;
  m_fFarClipDist = -1.f;
  m_eReflectionShaderMode = AlwaysSimple;
  m_bExecuteRenderHooks = false;
  m_bDoubleSided = false;
  m_bCameraOnFrontSide = true;
  m_fObliqueClippingPlaneOffset = 0.0f;
  m_fFovScale = 1.0f; // Will always remain at 1.0 when using mirrors (not water)
}

VisMirror_cl::VisMirror_cl()
{
  SetUseEulerAngles(FALSE);
  m_pParentManager = &VisMirrorManager_cl::GlobalManager();
  m_eReflectionShaderMode = AlwaysSimple;
  m_pWorldSurface = NULL;
  m_pReferenceObject = NULL;
  m_iResolution = 512;
  m_bUseHDR = false;
  m_bActive = m_bSupported = true;
  m_bValidFX = false;
  m_bLODFromRefContext = true;
  m_fSizeX = m_fSizeY = 100.f;
  m_fFarClipDist = -1.f;
  m_bExecuteRenderHooks = false;
  m_bDoubleSided = false;
  m_bCameraOnFrontSide = true;
  m_fObliqueClippingPlaneOffset = 0.0f;
}

VisMirror_cl::~VisMirror_cl()
{
  if (m_spReflectionContext != NULL)
  {
    int iRendererNodeCount = Vision::Renderer.GetRendererNodeCount();
    for (int i=0; i<iRendererNodeCount; i++)
    {
      IVRendererNode *pRendererNode = Vision::Renderer.GetRendererNode(i);
      if (pRendererNode != NULL)
        pRendererNode->RemoveContext(m_spReflectionContext);
    }
    m_spReflectionContext = NULL;
  }

  m_spDynamicMesh = NULL;

  if (m_pParentZone)
  {
    VisTypedEngineObject_cl::DisposeObject();
    // Note that this has to be done for mirror shapes only since they are purged by the resource manager and disposed by a zone
  }

}

void VisMirror_cl::Init()
{
  InitMirror();

  CreateMesh();
  UpdateMirror();
  AddDefaultVisibilityObject();
}

void VisMirror_cl::DisposeObject()
{
  if (IsObjectFlagSet(VObjectFlag_Disposing))
    return;
  SetActive(false);
  m_spMeshObj = NULL;
  VisTypedEngineObject_cl::DisposeObject();
  m_pParentManager->m_Instances.SafeRemove(this);
}

#ifdef SUPPORTS_SNAPSHOT_CREATION
void VisMirror_cl::GetDependencies(VResourceSnapshot &snapshot)
{
  VisObject3D_cl::GetDependencies(snapshot);

  if (m_spMirrorTechnique)
    m_spMirrorTechnique->GetDependencies(snapshot,NULL);
  if (m_spDynamicMesh)
    m_spDynamicMesh->GetDependencies(snapshot);
}
#endif

void VisMirror_cl::AddDefaultVisibilityObject()
{
  if (!m_spDefaultBBoxVisObj)
    m_spDefaultBBoxVisObj = new VisVisibilityObjectAABox_cl(VISTESTFLAGS_PERFORM_ALL_TESTS|VISTESTFLAGS_ACTIVE);

  UpdateDefaultVisibilityObject();
  m_VisibilityObjects.AddUnique(m_spDefaultBBoxVisObj);
  m_spDefaultBBoxVisObj->SetVisibleBitmask(m_iVisibleBitmask);
  m_uiVisibilityRefreshFrame = VisRenderContext_cl::GetGlobalTickCount();
}

void VisMirror_cl::RemoveDefaultVisibilityObject()
{
  if (m_spDefaultBBoxVisObj)
  {
    RemoveVisibilityObject(m_spDefaultBBoxVisObj);
    m_spDefaultBBoxVisObj = NULL;
  }
}

void VisMirror_cl::UpdateDefaultVisibilityObject()
{
  VASSERT(m_spDefaultBBoxVisObj);
  hkvAlignedBBox visBox = GetBoundingBox();
  visBox.addBoundary(hkvVec3 (2.f*Vision::World.GetGlobalUnitScaling())); // Make it slightly larger to prevent self occluding
  m_spDefaultBBoxVisObj->SetWorldSpaceBoundingBox(visBox);
  if (!IsBusySerializing()) // During serialization we assume the vis assignment has been correctly de-serialized.
    m_spDefaultBBoxVisObj->ReComputeVisibility();
}

void VisMirror_cl::RecreateRenderTarget()
{
  m_spRenderTarget_Refl->EnsureLoaded();
  m_spReflectionContext->SetRenderTarget(0, m_spRenderTarget_Refl);
  m_spReflectionContext->SetDepthStencilTarget(m_spRenderTarget_Refl);
}

void VisMirror_cl::FreeRenderTarget()
{
  m_spRenderTarget_Refl->SetResourceFlag(VRESOURCEFLAG_ALLOWUNLOAD);
  m_spReflectionContext->SetRenderTarget(0, NULL);
  m_spReflectionContext->SetDepthStencilTarget(NULL);
  m_spRenderTarget_Refl->EnsureUnloaded();
}

VTextureObject* VisMirror_cl::GetMirrorTexture()
{
  return m_spReflectionContext->GetRenderTarget();
}

void VisMirror_cl::OnObject3DChanged(int iO3DFlags)
{
  VisObject3D_cl::OnObject3DChanged(iO3DFlags);
  UpdateMirror();
}

void VisMirror_cl::SetActive(bool bStatus)
{
  m_bActive = bStatus;
  m_spReflectionContext->SetRenderingEnabled(IsActive());
}

void VisMirror_cl::SetSize(float x, float y)
{
  m_fSizeX = x;
  m_fSizeY = y;
  UpdateMirror();
}

void VisMirror_cl::GetWorldSpaceVertices(hkvVec3* pVert) const
{
  const hkvMat3 &rotMat(m_cachedRotMatrix);
  for (int i=0;i<4;i++)
  {
    pVert[i] = m_vLocalMirrorVert[i];
    pVert[i].x *= m_fSizeX;
    pVert[i].y *= m_fSizeY;

    pVert[i] = rotMat * pVert[i];
    pVert[i] += m_vPosition;
  }
}

void VisMirror_cl::DebugRender(IVRenderInterface* pRenderer, VColorRef iColor) const
{
  hkvVec3 vWorldPos[4];
  GetWorldSpaceVertices(vWorldPos);
  for (int i=0;i<4;i++)
  {
    hkvVec3 p1 = vWorldPos[i];
    hkvVec3 p2 = vWorldPos[(i+1)%4];
    pRenderer->DrawLine(p1,p2,iColor,2.f);
  }
}

const hkvAlignedBBox& VisMirror_cl::GetBoundingBox()
{
  m_BoundingBox.setInvalid();
  hkvVec3 vWorldPos[4];
  GetWorldSpaceVertices(vWorldPos);
  if (m_spDynamicMesh != NULL)
  {
    hkvAlignedBBox temp(hkvNoInitialization);
    m_spDynamicMesh->GetVisibilityBoundingBox(temp);
    temp.transformFromOrigin (hkvMat4 (GetRotationMatrix(),GetPosition()));
    m_BoundingBox.expandToInclude (temp);
  } 
  else
  {
    for (int i=0;i<4;i++)
      m_BoundingBox.expandToInclude(vWorldPos[i]);
  }
  m_Plane.setFromPoints(vWorldPos[0],vWorldPos[1],vWorldPos[2], hkvTriangleOrientation::ClockWise);
  return m_BoundingBox;
}

void VisMirror_cl::UpdateMirror()
{
  // Build mirror plane and absolute bounding box
  GetBoundingBox();
  const hkvMat3 &rotMat(m_cachedRotMatrix);
 
  if (m_spMeshObj)
  {
    m_spMeshObj->SetPosition(m_vPosition);
    hkvMat3 mat;

    if (m_spDynamicMesh != NULL)
      mat.set(m_vModelScale.x,0,0, 0,m_vModelScale.y,0, 0,0,m_vModelScale.z);
    else
      mat.set(m_fSizeX,0,0, 0,m_fSizeY,0, 0,0,1);

    mat = rotMat.multiply (mat);

    m_spMeshObj->SetRotationMatrix(mat);

    // Render order
    int iSubOrder = (int)(GetPosition().z * 100.f); // Sort bottom to top
    m_spMeshObj->SetOrder(m_uiRenderHook, -iSubOrder);
  }
  if (m_spDefaultBBoxVisObj)
    UpdateDefaultVisibilityObject();
}

void VisMirror_cl::SetShowPlaneObject(bool bStatus)
{
  if (bStatus)
  {
    if (m_spMeshObj) m_spMeshObj->SetVisibleBitmask(m_iVisibleBitmask);
    return;
  }
  if (m_spMeshObj) m_spMeshObj->SetVisibleBitmask(0);
}

void VisMirror_cl::SetRenderFilterMask(unsigned int uiMask)
{
  m_iContextBitmask = uiMask;
  if (m_spReflectionContext)
    m_spReflectionContext->SetRenderFilterMask(uiMask);
}

void VisMirror_cl::SetVisibleBitmask(unsigned int uiMask)
{
  m_iVisibleBitmask = uiMask;
  if (m_spMeshObj) 
    m_spMeshObj->SetVisibleBitmask(m_iVisibleBitmask);

  int iCount = m_VisibilityObjects.Count();
  for (int i=0;i<iCount;i++)
    m_VisibilityObjects.GetAt(i)->SetVisibleBitmask(m_iVisibleBitmask);

  m_uiVisibilityRefreshFrame = VisRenderContext_cl::GetGlobalTickCount();
}

void VisMirror_cl::SetRenderHook(unsigned int uiRenderHook)
{
  m_uiRenderHook = uiRenderHook;
  UpdateMirror();
}

void VisMirror_cl::SetUseLODFromRefContext(bool bLODFromRefContext)
{
  m_bLODFromRefContext = bLODFromRefContext;
  if (m_spReflectionContext != NULL)
    m_spReflectionContext->SetLODReferenceContext(m_bLODFromRefContext? m_pSourceContext : NULL);
}

void VisMirror_cl::SetTechnique(VCompiledTechnique *pTechnique)
{
  m_spMirrorTechnique = pTechnique;
  if (m_spMeshObj)
  {
    m_spMeshObj->SetTechnique(m_spMirrorTechnique);
  }
  m_bDoubleSided = false;

  if (pTechnique)
  {
    // Analyze the shaders in the technique
    for (int i=0;i<pTechnique->GetShaderCount();i++)
    {
      const VStateGroupRasterizer& rasterState = pTechnique->GetShader(i)->GetRenderState()->GetRasterizerState();
      if (rasterState.m_cCullMode == CULL_NONE || rasterState.m_cCullMode == CULL_FRONT)
      {
        m_bDoubleSided = true;
      }
    }
    m_bValidFX = true;
  }
}

void VisMirror_cl::SetEffect(VCompiledEffect *pEffect)
{
  m_MirrorEffect.SetEffect(pEffect);

  if (pEffect)
  {
    VTechniqueConfig config;
    
#ifdef _VISION_XENON
    if (m_bUseHDR)
    {
      config.AddInclusionTag("HDR_ON_XBOX");
    }
#endif    

    SetTechnique(pEffect->FindCompatibleTechnique(&config));
  }
  else
  {
    SetTechnique(NULL);
  }
}

void VisMirror_cl::ReassignEffect()
{
  m_MirrorEffect.ReAssignEffect();
  SetEffect(m_MirrorEffect.GetEffect()); // Re-apply the technique
}

void VisMirror_cl::SetReferenceObject(VisObject3D_cl *pRefObject)
{
  m_pReferenceObject = pRefObject;
}

void VisMirror_cl::ApplyToWorldSurface(VisSurface_cl *pWorldSrf, VisStaticMeshInstance_cl *pInst)
{
  m_pWorldSurface = pWorldSrf;
  m_spStaticMeshInst = pInst;
  if (!pWorldSrf)
  {
    return;
  }
  m_pWorldSurface->SetTechnique(m_spMirrorTechnique);
  m_pWorldSurface->m_spDiffuseTexture = GetMirrorTexture();
  SetShowPlaneObject(false);
}

void VisMirror_cl::SetModelFile(const char *szModel)
{
  m_spDynamicMesh = NULL;
  if (szModel && szModel[0])
  {
    m_spDynamicMesh = Vision::Game.LoadDynamicMesh(szModel, true, false);
    if (m_spDynamicMesh == NULL)
    {
      Vision::Error.Warning("Could not load mirror model %s", szModel);
      return;
    }
    VisMeshBuffer_cl *pMesh = new VisMeshBuffer_cl();
    VisMBVertexDescriptor_t storeDesc;
    VVertexBuffer *pVB = m_spDynamicMesh->GetMeshBuffer()->GetVertexBuffer();
    m_spDynamicMesh->GetMeshBuffer()->GetVertexDescriptor(storeDesc);
    pMesh->SetVertexBuffer(pVB, storeDesc, m_spDynamicMesh->GetNumOfVertices(), VIS_MEMUSAGE_STATIC, 0);

    int iIndexCount = m_spDynamicMesh->GetMeshBuffer()->GetIndexCount();
    VIndexBuffer *pIB = m_spDynamicMesh->GetMeshBuffer()->GetIndexBuffer();
    pMesh->SetIndexBuffer(pIB, iIndexCount, VIS_MEMUSAGE_STATIC, 0);   

    pMesh->SetPrimitiveCount(iIndexCount/3);
    pMesh->SetPrimitiveType(VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST);

    pMesh->SetDefaultTransparency(VIS_TRANSP_NONE);

    pMesh->SetBaseTexture(m_spRenderTarget_Refl);
    #ifdef HK_DEBUG
        pMesh->SetFilename("<PlanarMirrorMeshFromModel>");
    #endif

    CreateMeshBufferObject(pMesh);
  }
  else
  {
    if (m_spMeshObj)
      m_spMeshObj->SetVisible(true);
    else
      CreateMesh();
  }

  UpdateMirror();
}

void VisMirror_cl::CreateMesh()
{
  m_spDynamicMesh = NULL;

  VisMeshBuffer_cl *pMesh = new VisMeshBuffer_cl();

  VisMBVertexDescriptor_t desc;
  desc.m_iStride = sizeof(MirrorVertex_t);
  desc.m_iPosOfs = offsetof(MirrorVertex_t, pos);
  desc.m_iNormalOfs = offsetof(MirrorVertex_t, normal);
  desc.m_iTexCoordOfs[0] = offsetof(MirrorVertex_t, tex);
  desc.m_iColorOfs = offsetof(MirrorVertex_t, color);
  desc.SetFormatDefaults();

  GetBoundingBox(); // Necessary so we can obtain the normal
  hkvVec3 vNormal = m_Plane.m_vNormal;
  pMesh->AllocateVertices(desc, 4);
  MirrorVertex_t *pVert = (MirrorVertex_t *)pMesh->LockVertices(0);
  for (int i = 0; i < 4; i++, pVert++)
  {
    pVert->pos[0]=m_vLocalMirrorVert[i].x;
    pVert->pos[1]=m_vLocalMirrorVert[i].y;
    pVert->pos[2]=m_vLocalMirrorVert[i].z;
    pVert->color = V_RGBA_RED;
    pVert->tex[0] = m_vLocalMirrorVert[i].x;
    pVert->tex[1] = m_vLocalMirrorVert[i].y;
    pVert->normal[0] = vNormal.x;
    pVert->normal[1] = vNormal.y;
    pVert->normal[2] = vNormal.z;
  }
  pMesh->UnLockVertices();

  pMesh->SetPrimitiveType(VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST);
  pMesh->AllocateIndexList(6);
  unsigned short *pIndex = (unsigned short *)pMesh->LockIndices(0);
  pIndex[0] = 2;
  pIndex[1] = 1;
  pIndex[2] = 0;
  pIndex[3] = 3;
  pIndex[4] = 2;
  pIndex[5] = 0;

  pMesh->UnLockIndices();
  pMesh->SetDefaultTransparency(VIS_TRANSP_NONE);

  pMesh->SetBaseTexture(m_spRenderTarget_Refl);
  #ifdef HK_DEBUG
    pMesh->SetFilename("<PlanarMirrorMesh>");
  #endif

  CreateMeshBufferObject(pMesh);
}

void VisMirror_cl::CreateMeshBufferObject(VisMeshBuffer_cl* pMesh)
{
  m_spMeshObj = new VisMeshBufferObject_cl(pMesh);
  m_spMeshObj->SetTechnique(m_spMirrorTechnique);
  m_spMeshObj->SetObjectFlag(VObjectFlag_AutoDispose);
}

void VisMirror_cl::SetShowDebugMirrorTexture(bool bStatus)
{
  if (bStatus)
  {
    if (m_spDebugMask!=NULL)
      return;

    // Create the mirror screenmask
    int iIndexX = GetNumber() % 4;
    int iIndexY = GetNumber() / 4;
    m_spDebugMask = new VisScreenMask_cl();
    m_spDebugMask->SetTextureObject(m_spRenderTarget_Refl);
    m_spDebugMask->SetPos((float)iIndexX*256.f,(float)iIndexY*256.f);
    m_spDebugMask->SetTargetSize(256,256);
    m_spDebugMask->SetVisibleBitmask(VIS_ENTITY_VISIBLE_IN_WORLD); // not in our context
  } 
  else
  {
    m_spDebugMask = NULL;
  }
}

float VisMirror_cl::TraceMirror(const hkvVec3& vStart, const hkvVec3& vEnd, bool bDoubleSided) const
{
  // Make a quad out of two triangles and trace it
  hkvVec3 vWorldPos[4];
  GetWorldSpaceVertices(vWorldPos);
  VTriangle tri;

  hkvVec3 vDiff = vEnd - vStart;
  hkvVec3 vDir = vDiff;
  float fDist;
  tri.SetPoints((hkvVec3* )&vWorldPos[2],(hkvVec3* )&vWorldPos[1],(hkvVec3* )&vWorldPos[0]);
  if (tri.GetTraceIntersection((hkvVec3& )vStart,vDir,bDoubleSided, fDist))
    return fDist*vDir.getLength();

  tri.SetPoints((hkvVec3* )&vWorldPos[3],(hkvVec3* )&vWorldPos[2],(hkvVec3* )&vWorldPos[0]);
  if (tri.GetTraceIntersection((hkvVec3& )vStart,vDir,bDoubleSided, fDist))
    return fDist*vDir.getLength();

  return -1.f;
}

float VisMirror_cl::GetActualFarClipDistance() const
{
  float fNearPlane,fFarPlane;
  m_spReflectionContext->GetClipPlanes(fNearPlane,fFarPlane);
  return fFarPlane;
}

void AddObliqueClippingPlane (hkvMat4& mMatrix, const hkvPlane& eyeSpacePlane)
{
#ifdef CLIPSPACE_DEPTH_IS_MINUSONE_TO_ONE
  const float fAdd = 1.0f;
#else
  const float fAdd = 0.0f;
#endif

  // Calculate the clip-space corner point opposite the clipping plane
  // as (sgn(eyeSpacePlane.x), sgn(eyeSpacePlane.y), 1, 1) and
  // transform it into camera space by multiplying it
  // by the inverse of the projection matrix
  hkvVec4 q;
  q.x = (hkvMath::sign (eyeSpacePlane.m_vNormal.x) - mMatrix.m_Column[2][0]) / mMatrix.m_Column[0][0];
  q.y = (hkvMath::sign (eyeSpacePlane.m_vNormal.y) - mMatrix.m_Column[2][1]) / mMatrix.m_Column[1][1];
  q.z = 1.0f;
  q.w = (1 - mMatrix.m_Column[2][2]) / mMatrix.m_Column[3][2];

  // Calculate the scaled plane vector
  float fDot4 = eyeSpacePlane.m_vNormal.x*q.x + eyeSpacePlane.m_vNormal.y*q.y + eyeSpacePlane.m_vNormal.z*q.z + eyeSpacePlane.m_fNegDist*q.w;
  float f = (1.0f + fAdd) / fDot4;
  hkvVec4 c(eyeSpacePlane.m_vNormal.x*f, eyeSpacePlane.m_vNormal.y*f, eyeSpacePlane.m_vNormal.z*f, eyeSpacePlane.m_fNegDist*f);

  // Replace the third row of the projection matrix
  mMatrix.m_Column[0][2] = c.x;
  mMatrix.m_Column[1][2] = c.y;
  mMatrix.m_Column[2][2] = c.z - fAdd;
  mMatrix.m_Column[3][2] = c.w;
}

void VisMirror_cl::UpdateCamera(VisRenderContext_cl *pRefContext)
{
  VASSERT(m_pSourceContext);
  VisContextCamera_cl *pMainCam = m_pSourceContext->GetCamera();
  VASSERT(pMainCam);
  hkvVec3 vMainCamPos;

  // This one is 1 frame delayed (update core status not performed yet). But it would be nicer to use this
  vMainCamPos = pMainCam->GetPosition();

  m_bVisibleThisFrame = this->m_iContextBitmask != 0;

  m_bCameraOnFrontSide = (m_Plane.getDistanceTo (vMainCamPos) >= 0.0f);

  if (!m_bDoubleSided && !m_bCameraOnFrontSide) // Camera on wrong side? -> don't render
  {
    m_bVisibleThisFrame = false;
  }

  int iCount = m_VisibilityObjects.Count();
  for (int i=0;i<iCount;i++)
  {
    m_VisibilityObjects.GetAt(i)->SetOcclusionQueryPixelThreshold(32);
  }

  // Do not check visibility if either the renderer wasn't updated recently (e.g. when it was reinitialized) and hasn't even run visibility collection,
  // or when our visibility objects have changed (e.g. when the mirror is new).
  if(Vision::Renderer.GetCurrentRendererNode()->GetReferenceContext()->WasRecentlyRendered() && m_uiVisibilityRefreshFrame != VisRenderContext_cl::GetGlobalTickCount())
  {
  if (m_bVisibleThisFrame && iCount>0 && !m_VisibilityObjects.IsAnyVisible(pRefContext))
    {
    m_bVisibleThisFrame = false;
    }
  }
  
  if (m_bVisibleThisFrame && IsActive())
  {
    m_spReflectionContext->SetRenderingEnabled(true);
  }
  else
  {
    m_spReflectionContext->SetRenderingEnabled(false);
    return;
  }
  
  float fNearPlane, fFarPlane;
  m_pSourceContext->GetClipPlanes(fNearPlane, fFarPlane);

  // Far clip plane override
  if (m_fFarClipDist > 0.0f)
  {
    fFarPlane = m_fFarClipDist;
  }

  if (fNearPlane > (fFarPlane - 1.0f))
  {
    return;
  }
  
  // The effective plane offset can differ from m_fObliqueClippingPlaneOffset. If the camera is
  // closer to the plane than m_fObliqueClippingPlaneOffset (above OR below) , we have to limit 
  // the offset manually. We scale it a bit, because we don't want it to actually be ON the 
  // plane or rendering issues can result.
  float fEffectivePlaneOffset = m_fObliqueClippingPlaneOffset;
  float fCamDist = fabs(m_Plane.getDistanceTo(vMainCamPos)) * 0.7f;
  
  // NOTE: Handle the case for both above and below the mirror surface. The abs() above
  //       makes sure fCamDist moves away from 0 in both cases.
  if (fCamDist < -fEffectivePlaneOffset)
  {
    fEffectivePlaneOffset = -fCamDist * Vision::World.GetGlobalUnitScaling();
  }
  else if (fCamDist < fEffectivePlaneOffset)
  {
    fEffectivePlaneOffset = fCamDist * Vision::World.GetGlobalUnitScaling();
  }

  hkvVec3 v1,v2,v3;

  // Create the mirrored camera position and orientation
  hkvVec3 vMirrorNrml = m_Plane.m_vNormal;
  m_vMirrorPos = m_Plane.getMirrored (vMainCamPos);

  const hkvMat3 mainCamRot = pMainCam->GetRotationMatrix();
  mainCamRot.getAxisXYZ(&v1,&v2,&v3);
  v1 = v1.getReflected(vMirrorNrml);
  v2 = v2.getReflected(vMirrorNrml);
  v3 = v3.getReflected(vMirrorNrml);
  m_MirrorCamRot.setLookInDirectionMatrix (v1, v3);

  VisContextCamera_cl *pMirrorCam = m_spReflectionContext->GetCamera();
  VASSERT(pMirrorCam);
  pMirrorCam->Set(m_MirrorCamRot, m_vMirrorPos);

  // Set FOV as in main context
  float fAngleX,fAngleY;
  m_pSourceContext->GetFinalFOV(fAngleX,fAngleY);
  m_spReflectionContext->SetCustomProjectionMatrix(NULL); // Force building a real projection matrix
  float fScaledX = fAngleX * m_fFovScale;
  float fScaledY = fAngleY * m_fFovScale;
  m_spReflectionContext->SetFOV(hkvMath::Min(fScaledX,179.f), hkvMath::Min(fScaledY,179.f));

  // We need the water plane in camera space
  hkvMat3 mWorldToCamRotation = pMirrorCam->GetWorldToCameraRotation();

  hkvVec3 vCSNrml = mWorldToCamRotation * vMirrorNrml;
  hkvVec3 vCSPos = mWorldToCamRotation * (m_vPosition + vMirrorNrml * fEffectivePlaneOffset - m_vMirrorPos);
  if (!m_bCameraOnFrontSide)
  {
    vCSNrml = -vCSNrml;
  }

  hkvPlane planeCS; planeCS.setFromPointAndNormal (vCSPos,vCSNrml);
  m_ObliqueClippingPlane = planeCS;

  // Apply oblique clipping plane - make sure we fetch the raw projection matrix from the view properties (without y-Flipping applied)
  m_ObliqueClippingProjection = m_spReflectionContext->GetViewProperties()->getProjectionMatrix(hkvClipSpaceYRange::MinusOneToOne);
  AddObliqueClippingPlane (m_ObliqueClippingProjection, planeCS);
  m_spReflectionContext->SetCustomProjectionMatrix(m_ObliqueClippingProjection.getPointer(), false);

  // Note: Setting the near and far plane after the oblique clipping projection has been created ensures that these clip plane values
  //       are only used for visibility, this is a result of the circumstance that when using oblique clipping projection the far plane
  //       does not have the same meaning than in an normal perspective projection.
  m_spReflectionContext->SetClipPlanes(fNearPlane,fFarPlane);
}

IVisVisibilityCollector_cl *VisMirror_cl::GetVisibilityCollectorForView(VisRenderContext_cl *pView)
{
  // We need a visibility collector for each view, because visibility computation is decoupled

  // See whether we have stored the pair already
  for (int i=0;i<m_iViewContextCount;i++)
    if (m_pViewContext[i]==pView)
    {
      // For more than one context we have to mark the camera as teleported
      if (m_iViewContextCount>1)
        m_spReflectionContext->GetCamera()->ReComputeVisibility();
      return m_spViewVisibility[i];
    }

  // Cleanup the ones we do not need anymore (i.e. not registered)
  int iOldCount = m_iViewContextCount;
  m_iViewContextCount = 0;
  for (int i=0;i<iOldCount;i++)
  {
    if (!IsContextRegistered(m_pViewContext[i]))
      continue;

    m_pViewContext[m_iViewContextCount] = m_pViewContext[i];
    m_spViewVisibility[m_iViewContextCount] = m_spViewVisibility[i];
    m_iViewContextCount++;
  }
  for (int i=m_iViewContextCount;i<iOldCount;i++)
  {
    m_pViewContext[i] = NULL;
    m_spViewVisibility[i] = NULL;
  }

  if (m_iViewContextCount>=MIRROR_MAX_VIEWCONTEXTS)
  {
    VASSERT(!"Number of maximum view contexts exceeded.");
    return NULL;
  }

  // Create a new one:
  VisionVisibilityCollector_cl *pCollector = new VisionVisibilityCollector_cl();

  m_pViewContext[m_iViewContextCount] = pView;
  m_spViewVisibility[m_iViewContextCount] = pCollector;
  pCollector->SetOcclusionQueryRenderContext(m_spReflectionContext);

  m_iViewContextCount++;

  return pCollector;
}

void VisMirror_cl::ClearViewVisibilityCollectors()
{
  // reset the list
  for (int i=0;i<m_iViewContextCount;i++)
  {
    m_pViewContext[i] = NULL;
    m_spViewVisibility[i] = NULL;
  }
  m_iViewContextCount = 0;
  m_pSourceContext = NULL;
}

void VisMirror_cl::HandleMirror(VisRendererNodeDataObject_cl &data)
{
  VASSERT(data.m_pRendererNode != NULL)
  
  VisRenderContext_cl *pViewContext = data.m_pRendererNode->GetReferenceContext();

  // When rendering multiple times per frame we must turn off occlusion query, otherwise objects are culled
  int iFlags = m_spReflectionContext->GetRenderFlags();
  if (m_pSourceContext && m_pSourceContext!=pViewContext && (iFlags&VIS_RENDERCONTEXT_FLAG_USE_OCCLUSIONQUERY))
  {
    iFlags &= ~(VIS_RENDERCONTEXT_FLAG_USE_OCCLUSIONQUERY);
    m_spReflectionContext->SetRenderFlags(iFlags);
  }

  m_pSourceContext = pViewContext;

  // Add this mirror context to the list of dependent contexts that are rendered before next frame
  IVisVisibilityCollector_cl *pOldVisColl = m_spReflectionContext->GetVisibilityCollector();

  // If we have multiple views:
  // Wait for pending visibility computations in the previous vis collector to finish
  // before we start overwriting any values in the context it references... [#20062]
  if (m_iViewContextCount>1)
  {
    if (pOldVisColl)
      pOldVisColl->WaitForAllTasks();
  }

  UpdateCamera(pViewContext);

  // Add this mirror context to the list of dependent contexts that are rendered before next frame
  IVisVisibilityCollector_cl *pVisColl = GetVisibilityCollectorForView(m_pSourceContext);
  if (!m_bVisibleThisFrame)
  {
    // We have to clear the old result so old scene elements don't hang around in it. [#18302]
    // This is critical if the source context changed the status for m_bVisibleThisFrame between the two callbacks
    ((VisionVisibilityCollector_cl *)pVisColl)->ClearVisibilityData();
  }

  if (pVisColl!=pOldVisColl)
  {
    m_spReflectionContext->SetVisibilityCollector(pVisColl);
    m_spReflectionContext->SetLODReferenceContext(m_bLODFromRefContext? m_pSourceContext : NULL);
  }

  PrepareProjectionPlanes();
}

void VisMirror_cl::SetupSingleShaderProjection(VCompiledShaderPass *shader, const hkvVec3& campos, const hkvMat3 &camrot)
{
  // Cull the whole shader?
  if (((shader->GetRenderState ()->GetRasterizerState ().m_cCullMode == CULL_FRONT) &&  m_bCameraOnFrontSide) || 
      ((shader->GetRenderState ()->GetRasterizerState ().m_cCullMode == CULL_BACK)  && !m_bCameraOnFrontSide))
  {
    shader->GetRenderState ()->SetRenderFlags (0); // Prevent rendering
    return;
  }
  shader->GetRenderState ()->SetRenderFlags ((unsigned int) -1);

  hkvVec3 vDir(hkvNoInitialization),vRight(hkvNoInitialization),vUp(hkvNoInitialization);
  camrot.getAxisXYZ(&vDir,&vRight,&vUp);

  float fAngleX,fAngleY;
  m_spReflectionContext->GetFinalFOV(fAngleX,fAngleY);
  
  // Setup cone width and height modifiers to match the projection
  float tx = - 1.0f / hkvMath::tanDeg (fAngleX / 2.0f);
  float ty = 1.0f / hkvMath::tanDeg (fAngleY / 2.0f);

  float s[4];
  s[0] = tx*vRight.x;
  s[1] = tx*vRight.y;
  s[2] = tx*vRight.z;
  s[3] = - (s[0]*campos.x+s[1]*campos.y+s[2]*campos.z);
  shader->GetConstantBuffer (VSS_VertexShader)->SetSingleParameterF("refPlaneS", s);

  float t[4];
  t[0] = ty*vUp.x;
  t[1] = ty*vUp.y;
  t[2] = ty*vUp.z;
  t[3] = - (t[0]*campos.x+t[1]*campos.y+t[2]*campos.z);
  shader->GetConstantBuffer (VSS_VertexShader)->SetSingleParameterF("refPlaneT", t);

  float q[4];
  q[0] = vDir.x;
  q[1] = vDir.y;
  q[2] = vDir.z;
  q[3] = - (q[0]*campos.x+q[1]*campos.y+q[2]*campos.z);
  shader->GetConstantBuffer (VSS_VertexShader)->SetSingleParameterF("refPlaneQ", q);

  hkvMat3 mat(hkvNoInitialization);
  hkvVec3 pos(hkvNoInitialization); 
  if (m_pReferenceObject)
  {
    m_pReferenceObject->GetRotationMatrix(mat);
    pos = m_pReferenceObject->GetPosition();
  }
  else if (m_spStaticMeshInst)
  {
    mat = m_spStaticMeshInst->GetTransform().getRotationalPart();
    pos = m_spStaticMeshInst->GetTransform().getTranslation();
  } 
  else if (m_pWorldSurface)
  {
    mat.setIdentity();
    pos.set(0,0,0);
  } 
  else if (m_spMeshObj)
  {
    m_spMeshObj->GetRotationMatrix(mat);
    pos = m_spMeshObj->GetPosition();
  } 
  else 
  {
    VASSERT(false);
  }

  float mmv[4];
  mmv[0] = mat.m_ElementsCM[0]; mmv[1] = mat.m_ElementsCM[3]; mmv[2] = mat.m_ElementsCM[6]; mmv[3] = 0.0f;
  shader->GetConstantBuffer (VSS_VertexShader)->SetSingleParameterF("mmv0", mmv);

  mmv[0] = mat.m_ElementsCM[1]; mmv[1] = mat.m_ElementsCM[4]; mmv[2] = mat.m_ElementsCM[7]; mmv[3] = 0.0f;
  shader->GetConstantBuffer (VSS_VertexShader)->SetSingleParameterF("mmv1", mmv);

  mmv[0] = mat.m_ElementsCM[2]; mmv[1] = mat.m_ElementsCM[5]; mmv[2] = mat.m_ElementsCM[8]; mmv[3] = 0.0f;
  shader->GetConstantBuffer (VSS_VertexShader)->SetSingleParameterF("mmv2", mmv);

  mmv[0] = pos[0]; mmv[1] = pos[1]; mmv[2] = pos[2]; mmv[3] = 0.0f;
  shader->GetConstantBuffer (VSS_VertexShader)->SetSingleParameterF("mmvpos", mmv);

  shader->GetConstantBuffer(VSS_PixelShader)->SetSingleParameterF("TexSize", (float)m_spRenderTarget_Refl->GetTextureWidth(), (float)m_spRenderTarget_Refl->GetTextureHeight(),
    1.0f / m_spRenderTarget_Refl->GetTextureWidth(), 1.0f / m_spRenderTarget_Refl->GetTextureHeight());

  shader->m_bModified = true; // Avoid early-out in engine [#18302]
}

void VisMirror_cl::PrepareProjectionPlanes()
{
  if (!m_spMirrorTechnique)
    return;

  const int iShaderCount = m_spMirrorTechnique->m_Shaders.Count();

  // Set planes for all shaders in the effect
  for (int i=0;i<iShaderCount;i++)
    SetupSingleShaderProjection(m_spMirrorTechnique->m_Shaders.GetAt(i), m_vMirrorPos, m_MirrorCamRot);
}

V_IMPLEMENT_SERIAL( VisMirror_cl, VisObject3D_cl, 0, &g_VisionEngineModule );
void VisMirror_cl::Serialize( VArchive &ar )
{
  VisEffectConfig_cl fxConfig;
  if (ar.IsLoading())
  {
    char iVers;

    char szModelFile[FS_MAX_PATH];
    VisVisibilityObjectAABox_cl *pDefaultVisObj = NULL;
    szModelFile[0] = 0;

    ar >> iVers; VASSERT(iVers<=MIRROR_CURRENT_VERSION);
    if (iVers>=MIRROR_VERSION_8)
      VisObject3D_cl::Serialize(ar); // Version 8 uses object3d for position/orientation

    ar >> m_iResolution;
    if (iVers>=MIRROR_VERSION_11)
      ar >> m_bUseHDR;

    if (iVers<MIRROR_VERSION_8) // Old code path
    {
      hkvVec3 vPos;
      hkvMat3 vRotation;
        
      vPos.SerializeAsVisVector (ar);
      ar >> vRotation;

      // Translate by custom offset defined in the archive
      hkvVec3 vOffset(hkvNoInitialization),vEulerOfs(hkvNoInitialization);
      hkvMat3 mRotationOfs(hkvNoInitialization);
      if (ar.GetCustomShapeTransformation(vOffset,mRotationOfs,vEulerOfs))
      {
        vPos = mRotationOfs * vPos;
        vPos += vOffset;
        vRotation = mRotationOfs.multiply (vRotation);
      }
      SetPosition(vPos);
      SetRotationMatrix(vRotation);
    }

    ar >> m_fSizeX >> m_fSizeY;
    if (iVers>=MIRROR_VERSION_1)
      ar.ReadStringBinary(szModelFile,FS_MAX_PATH);

    InitMirror();
    ar >> fxConfig;
    if (szModelFile[0])
      SetModelFile(szModelFile);
    else
      CreateMesh();

    SetEffect(fxConfig.GetEffect(0));
    if (iVers>=MIRROR_VERSION_2)
      ar >> m_VisibilityObjects;
    if (iVers>=MIRROR_VERSION_4)
      ar >> pDefaultVisObj;
    m_spDefaultBBoxVisObj = pDefaultVisObj;
    if (iVers>=MIRROR_VERSION_3)
      ar >> m_fFarClipDist;
    unsigned int iContextMask = VIS_ENTITY_VISIBLE_IN_TEXTURE;
    if (iVers>=MIRROR_VERSION_5)
      ar >> iContextMask;

    if (iVers>=MIRROR_VERSION_6)
    {
      int iTemp = 0;
      ar >> iTemp; m_eReflectionShaderMode = (VReflectionShaderSets_e)iTemp; 
    }

    if (iVers>=MIRROR_VERSION_7)
    {
      ar >> m_fObliqueClippingPlaneOffset;
      ar >> m_fFovScale;
    }
    else
    {
      m_fObliqueClippingPlaneOffset = 0.0f;
      m_fFovScale = 1.0f;
    }

    if (iVers>=MIRROR_VERSION_9)
      ar >> m_bExecuteRenderHooks;
    if (iVers>=MIRROR_VERSION_10)
      m_vModelScale.SerializeAsVec3(ar);

    if (iVers >= MIRROR_VERSION_15)
    {
      ar >> m_uiRenderHook;
    }
    else if (iVers >= MIRROR_VERSION_13)
    {
      unsigned int iOrder;
      ar >> iOrder;

      // Toggle VRH_ADDITIVE_PARTICLES with VRH_TRANSLUCENT_VOLUMES so that the VRenderHook_e enum order matches the execution order
      if (iOrder == VRH_ADDITIVE_PARTICLES)       m_uiRenderHook = VRH_TRANSLUCENT_VOLUMES;
      else if (iOrder == VRH_TRANSLUCENT_VOLUMES) m_uiRenderHook = VRH_ADDITIVE_PARTICLES;
      else                                        m_uiRenderHook = iOrder;
    }
    else if (iVers == MIRROR_VERSION_12)
    {
      unsigned int uiTempRenderHook;
      ar >> uiTempRenderHook;
      m_uiRenderHook = UpdateRenderHook(uiTempRenderHook);
    }

    if (iVers >= MIRROR_VERSION_14)
      ar >> m_bLODFromRefContext;

    SetRenderFilterMask(iContextMask);

    UpdateMirror();

    //SetShowDebugMirrorTexture(true);
  } 
  else
  {
    const char *szModelFile = NULL;
    if (m_spDynamicMesh)
      szModelFile = m_spDynamicMesh->GetFilename();

    ar << (char)MIRROR_CURRENT_VERSION;

    VisObject3D_cl::Serialize(ar);        // Version. 8 uses object3d for position/orientation

    ar << m_iResolution;
    ar << m_bUseHDR;                      // Version 11
    ar << m_fSizeX << m_fSizeY;
    ar << szModelFile;
    ar << m_MirrorEffect;
    ar << m_VisibilityObjects;            // Version 2
    ar << m_spDefaultBBoxVisObj;          // Version 4
    ar << m_fFarClipDist;                 // Version 3
    ar << m_iContextBitmask;              // Version 5
    ar << (int)m_eReflectionShaderMode;   // Version 6
    ar << m_fObliqueClippingPlaneOffset;  // Version 7
    ar << m_fFovScale;                    // Version 7
    ar << m_bExecuteRenderHooks;          // Version 9
    m_vModelScale.SerializeAsVec3 (ar);   // Version 10
    ar << m_uiRenderHook;                 // Version 15
    ar << m_bLODFromRefContext;           // Version 14
  }
}

VisMirror_cl::VReflectionShaderSets_e g_TempMode = VisMirror_cl::AlwaysSimple;

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
