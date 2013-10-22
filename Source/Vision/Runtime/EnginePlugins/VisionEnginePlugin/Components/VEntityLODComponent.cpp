/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/VisionEnginePluginPCH.h>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Components/VEntityLODComponent.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Animation/Transition/VTransitionStateMachine.hpp>
#include <Vision/Runtime/Engine/Animation/VisApiAnimNormalizeMixerNode.hpp>
#include <Vision/Runtime/Base/System/Memory/VMemDbg.hpp>

/// =========================================================================== ///
/// Entity LOD Level Info Methods                                               ///
/// =========================================================================== ///
bool VEntityLODLevelInfo::SetModelFile(const char *szFilename, bool bAppyMotionDelta)
{
  // Load model
  m_spMesh = Vision::Game.LoadDynamicMesh(szFilename, true);
  m_spAnimConfig = NULL;
  if (m_spMesh == NULL)
    return false;

  // Create anim config only for models with skeleton
  if (m_spMesh->GetSkeleton() == NULL)
    return true;

  // Create a new final result for this config
  VisAnimFinalSkeletalResult_cl* pFinalSkeletalResult;
  m_spAnimConfig = VisAnimConfig_cl::CreateSkeletalConfig(m_spMesh, &pFinalSkeletalResult);

  if (bAppyMotionDelta)
    m_spAnimConfig->SetFlags(m_spAnimConfig->GetFlags() | APPLY_MOTION_DELTA);
  else
    m_spAnimConfig->SetFlags(m_spAnimConfig->GetFlags() & ~APPLY_MOTION_DELTA);

  m_spFinalSkeletalResult = pFinalSkeletalResult;

  return true;
}

/// =========================================================================== ///
/// Entity LOD Component Methods                                                ///
/// =========================================================================== ///

// Register the class in the engine module so it is available for RTTI
#ifdef SUPPORTS_LOD_HYSTERESIS_THRESHOLDING
  V_IMPLEMENT_SERIAL(VEntityLODComponent, IVLODHysteresisComponent, 0, &g_VisionEngineModule);
#else
  V_IMPLEMENT_SERIAL(VEntityLODComponent, IVObjectComponent, 0, &g_VisionEngineModule);
#endif //SUPPORTS_LOD_HYSTERESIS_THRESHOLDING

// One global instance of our manager
VEntityLODComponentManager VEntityLODComponentManager::g_GlobalManager;

VEntityLODComponent::VEntityLODComponent(int iComponentFlags)
#if defined(SUPPORTS_LOD_HYSTERESIS_THRESHOLDING)
  : IVLODHysteresisComponent(iComponentFlags)
#else
  : IVObjectComponent(0, iComponentFlags)
#endif // SUPPORTS_LOD_HYSTERESIS_THRESHOLDING
  , LOD_LevelMode(VLOD_AUTO)
  , LOD_LevelCount(1)
  , Level_Medium_Mesh()
  , Level_Low_Mesh()
  , Level_UltraLow_Mesh()
  , Level_Medium_Distance(500.0f * Vision::World.GetGlobalUnitScaling())
  , Level_Low_Distance(1000.0f * Vision::World.GetGlobalUnitScaling())
  , Level_UltraLow_Distance(1500.0f * Vision::World.GetGlobalUnitScaling())
  , m_iCurrentLevel(-1)
  , m_pLevels(NULL)
{
}

VEntityLODComponent::~VEntityLODComponent()
{
  V_SAFE_DELETE_ARRAY(m_pLevels);
}

void VEntityLODComponent::CommonInit()
{
  VisBaseEntity_cl* pOwnerEntity = vstatic_cast<VisBaseEntity_cl*>(GetOwner());
  if (pOwnerEntity == NULL)
    return;

  // Get number of LOD levels
  V_SAFE_DELETE_ARRAY(m_pLevels);
  m_pLevels = new VEntityLODLevelInfo[LOD_LevelCount + 1];

  // LOD Level High: Owner entity mesh
  VDynamicMesh *pMesh = pOwnerEntity->GetMesh();

  if (pMesh != NULL)
  {
    if (Level_High_Mesh.IsEmpty())
      Level_High_Mesh = pMesh->GetFilename();

    InitializeLODLevelInfo(VLOD_HIGH, Level_High_Mesh, 0.0f, Level_Medium_Distance);
  }

  // LOD Level Medium
  if (LOD_LevelCount >= VLOD_MEDIUM)
    InitializeLODLevelInfo(VLOD_MEDIUM, Level_Medium_Mesh, Level_Medium_Distance, Level_Low_Distance);

  // LOD Level Low
  if (LOD_LevelCount >= VLOD_LOW)
    InitializeLODLevelInfo(VLOD_LOW, Level_Low_Mesh, Level_Low_Distance, Level_UltraLow_Distance);

  // LOD Level UltraLow
  if (LOD_LevelCount >= VLOD_ULTRALOW)
    InitializeLODLevelInfo(VLOD_ULTRALOW, Level_UltraLow_Mesh, Level_UltraLow_Distance, hkvMath::FloatMaxPos());

  ConnectToExistingAnimConfig();
  
  // Set LOD level
  m_iCurrentLevel = -1;
  if (LOD_LevelMode == VLOD_NONE)
    SetLODLevel(VLOD_HIGH);
  else if (LOD_LevelMode == VLOD_AUTO)
    UpdateLOD();
  else if (LOD_LevelMode <= LOD_LevelCount)
    SetLODLevel(static_cast<VEntityLODLevel_e>(LOD_LevelMode));
}

void VEntityLODComponent::InitializeLODLevelInfo(int iLevel, const char* szMeshFilename, float fMinDistance, float fMaxDistance)
{
  if (VStringUtil::IsEmpty(szMeshFilename))
  {
    return;
  }
  else if (!m_pLevels[iLevel].SetModelFile(szMeshFilename))
  {
    Vision::Error.Warning("Model '%s' could not be loaded for LOD Level [%i]", szMeshFilename, iLevel);
    return;
  }

  VisBaseEntity_cl* pOwnerEntity = vstatic_cast<VisBaseEntity_cl*>(GetOwner());
  const float fFarClipDistance = pOwnerEntity->GetFarClipDistance();

  m_pLevels[iLevel].SetSwitchDistance(fMinDistance, (iLevel < LOD_LevelCount) ? fMaxDistance : fFarClipDistance);
  m_pLevels[iLevel].m_pEntity = pOwnerEntity;
}

void VEntityLODComponent::PerFrameUpdate()
{
  if (LOD_LevelMode == VLOD_AUTO && (vstatic_cast<VisBaseEntity_cl*>(m_pOwner))->WasVisibleInAnyLastFrame())
    UpdateLOD();
}

void VEntityLODComponent::SetLODLevel(VEntityLODLevel_e newLevel)
{
  LOD_LevelMode = newLevel;

  // Early out if owner has not been set yet.
  if (GetOwner() == NULL)
    return;

  if (LOD_LevelMode == VLOD_NONE)
  {
    ApplyLOD(VLOD_HIGH);
  }
  else if (LOD_LevelMode == VLOD_AUTO)
  {
    UpdateLOD();
  }
  else
  {
    ApplyLOD(static_cast<int>(LOD_LevelMode));
  }
}

void VEntityLODComponent::UpdateLOD()
{
  float fDistance = GetDistanceToCamera();

  // compute LOD level
  int iLevel = 0;
#ifdef SUPPORTS_LOD_HYSTERESIS_THRESHOLDING
  float fThreshold = VLODHysteresisManager::GetThreshold(VLHT_ENTITIES);

  // Only use thresholding if current level is initialized.
  if (fThreshold > 0.0f && m_iCurrentLevel != -1)
  {
    iLevel = m_iCurrentLevel;

    for (;;)
    {
      const float fNearSwitchDistance = m_pLevels[iLevel].m_fMinSwitchDistance;
      const float fFarSwitchDistance = m_pLevels[iLevel].m_fMaxSwitchDistance + fThreshold;
      bool bNear = fDistance < fNearSwitchDistance;
      bool bFar = fDistance >= fFarSwitchDistance;

      if (!bNear && !bFar)
      {
        break;
      }
      else if (bNear)
      {
        if (iLevel > 0)
          iLevel--;
        else
          break;
      }
      else if (bFar)
      {
        if (iLevel < LOD_LevelCount)
          iLevel++;
        else
          break;
      }
    }

    VisBaseEntity_cl *pEntity = vstatic_cast<VisBaseEntity_cl*>(m_pOwner);
    if (pEntity != NULL)
      pEntity->SetClipSettings(pEntity->GetNearClipDistance(), pEntity->GetFarClipDistance(), &pEntity->GetPosition());
  }
  else
#endif //SUPPORTS_LOD_HYSTERESIS_THRESHOLDING
  {
    for (int i = 0; i <= LOD_LevelCount; i++)
    {
      if (m_pLevels[i].m_fMinSwitchDistance <= fDistance)
        iLevel = i;
    }
  }

  ApplyLOD(iLevel);
}

void VEntityLODComponent::ApplyLOD(int newLevel)
{
  if (m_iCurrentLevel == newLevel)
    return;

  m_iCurrentLevel = newLevel;

  VisBaseEntity_cl *pEntity = vstatic_cast<VisBaseEntity_cl*>(m_pOwner);
  VEntityLODLevelInfo &info = GetLevelInfo(newLevel);

  if (info.m_spMesh != NULL)
  {
    pEntity->SetMesh(info.m_spMesh, info.m_spAnimConfig);
  }
}

bool VEntityLODComponent::ConnectToExistingAnimConfig()
{
  // First check if there is a transition state machine
  VTransitionStateMachine *pTSM = m_pOwner->Components().GetComponentOfType<VTransitionStateMachine>();
  if (pTSM != NULL)
  {
    SetSkeletalAnimRootNode(pTSM->GetTransitionMixer(), true);
    return true;
  }

  // Otherwise get the owner entity's anim config.
  VisBaseEntity_cl *pOwnerEntity = vstatic_cast<VisBaseEntity_cl*>(m_pOwner);
  VisAnimConfig_cl* pAnimConfig = pOwnerEntity->GetAnimConfig();
  if (pAnimConfig == NULL || pAnimConfig->GetFinalResult() == NULL ||
    pAnimConfig->GetFinalResult()->GetSkeletalAnimInput() == NULL)
  {
    return false;
  }
  
  // Make sure to use the same motion delta setting.
  SetSkeletalAnimRootNode(pOwnerEntity->GetAnimConfig()->GetFinalResult()->GetSkeletalAnimInput(),
    (pOwnerEntity->GetAnimConfig()->GetFlags() & APPLY_MOTION_DELTA) != 0);
  return true;
}

void VEntityLODComponent::SetSkeletalAnimRootNode(IVisAnimResultGenerator_cl *pRoot, bool bAppyMotionDelta)
{
  for (int i = 0; i <= LOD_LevelCount; i++)
  {
    // Set motion delta
    VisAnimConfig_cl* pAnimConfig = m_pLevels[i].m_spAnimConfig;
    if (pAnimConfig != NULL)
    {
      if (bAppyMotionDelta)
        pAnimConfig->SetFlags(pAnimConfig->GetFlags() | APPLY_MOTION_DELTA);
      else
        pAnimConfig->SetFlags(pAnimConfig->GetFlags() & ~APPLY_MOTION_DELTA);
    }

    // Set anim control
    if (m_pLevels[i].m_spFinalSkeletalResult != NULL)
    {
      m_pLevels[i].m_spFinalSkeletalResult->SetSkeletalAnimInput(pRoot);
    }
  }
}

#ifdef SUPPORTS_LOD_HYSTERESIS_THRESHOLDING
int VEntityLODComponent::GetLODLevel() const
{
  return GetCurrentLODLevel();
}
#endif //SUPPORTS_LOD_HYSTERESIS_THRESHOLDING

#if defined(WIN32) || defined(_VISION_DOC)

void VEntityLODComponent::GetVariableAttributes(VisVariable_cl *pVariable, VVariableAttributeInfo &destInfo)
{
  // Check whether we need to display the LOD medium model property
  if (!strcmp(pVariable->GetName(), "Level_Medium_Mesh") ||
    !strcmp(pVariable->GetName(), "Level_Medium_Distance"))
  {
    if (LOD_LevelCount == 0)
      destInfo.m_bHidden = true;
  }
  // Check whether we need to display the LOD low model property
  else if (!strcmp(pVariable->GetName(), "Level_Low_Mesh") ||
    !strcmp(pVariable->GetName(), "Level_Low_Distance"))
  {
    if (LOD_LevelCount <= 1)
      destInfo.m_bHidden = true;
  }
  // Check whether we need to display the LOD ultralow model property
  else if (!strcmp(pVariable->GetName(), "Level_UltraLow_Mesh") ||
    !strcmp(pVariable->GetName(), "Level_UltraLow_Distance"))
  {
    if (LOD_LevelCount <= 2)
      destInfo.m_bHidden = true;
  }
}

#endif

void VEntityLODComponent::MessageFunction(int iID, INT_PTR iParamA, INT_PTR iParamB)
{
  if (iID == VIS_MSG_ENTITY_MESHCHANGED)
  {
    // If the mesh and animation config are the same as the one used in the current LOD,
    // it must have been set by this component.
    VDynamicMesh* pMesh = reinterpret_cast<VDynamicMesh*>(iParamA);
    VisAnimConfig_cl* pAnimConfig = reinterpret_cast<VisAnimConfig_cl*>(iParamB);

    if (pMesh == m_pLevels[m_iCurrentLevel].m_spMesh &&
      pAnimConfig == m_pLevels[m_iCurrentLevel].m_spAnimConfig)
    {
      return;
    }
    
    if (pMesh != NULL)
    {
      // Get new high level mesh.
      Level_High_Mesh = pMesh->GetFilename();
    }

    CommonInit();
  }
#ifdef WIN32
  else if (iID == VIS_MSG_EDITOR_PROPERTYCHANGED)
  {
    const char *szPropertyName = reinterpret_cast<const char*>(iParamA);
    // If ModelFile was modified, we have already handled the VIS_MSG_ENTITY_MESHCHANGED event.
    if (strcmp(szPropertyName, "ModelFile") != 0)
      CommonInit();
  }
#endif

  IVObjectComponent::MessageFunction(iID, iParamA, iParamB);
}


/// =========================================================================== ///
/// IVObjectComponent Overrides                                                 ///
/// =========================================================================== ///

BOOL VEntityLODComponent::CanAttachToObject(VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut)
{
  if (!IVObjectComponent::CanAttachToObject(pObject, sErrorMsgOut))
    return false;

  if (!pObject->IsOfType(V_RUNTIME_CLASS(VisBaseEntity_cl)))
  {
    sErrorMsgOut = "Component can only be added to instances of VisBaseEntity_cl or derived classes.";
    return FALSE;
  }

  return TRUE;
}

void VEntityLODComponent::SetOwner(VisTypedEngineObject_cl *pOwner)
{
  if (pOwner != NULL)
  {
    // mark entity so that visibility collector can quickly check whether this component is attached
    VisBaseEntity_cl* pEntity = (VisBaseEntity_cl*)pOwner;
    pEntity->m_iEntityFlags |= VISENTFLAG_LOD_COMPONENT_ATTACHED;
  }
  else if (GetOwner() != NULL && !GetOwner()->IsDisposing())
  {
    // remove mark (see above)
    VisBaseEntity_cl* pEntity = (VisBaseEntity_cl*)GetOwner();
    pEntity->m_iEntityFlags &= ~VISENTFLAG_LOD_COMPONENT_ATTACHED;
  }

  IVObjectComponent::SetOwner(pOwner);

  // Insert code here to respond to attaching this component to an object
  // This function is called with pOwner==NULL when de-attaching.
  if (pOwner != NULL)
  {
    VASSERT(VEntityLODComponentManager::GlobalManager().Instances().Find(this) == -1);
    VEntityLODComponentManager::GlobalManager().Instances().Add(this);
    CommonInit();
  }
  else
  {
    VASSERT(VEntityLODComponentManager::GlobalManager().Instances().Find(this) != -1);
    VEntityLODComponentManager::GlobalManager().Instances().Remove(this);
  }
}

void VEntityLODComponent::OnVariableValueChanged(VisVariable_cl *pVar, const char * value)
{
  // Don't perform any update/invalidation code in case we don't have an owner yet.
  // SetOwner will perform an initialization once it gets called.
  if (GetOwner() == NULL)
    return;

  CommonInit();
}

void VEntityLODComponent::Serialize( VArchive &ar )
{
  char iLocalVersion = ENTITYLODCOMPONENT_VERSION_CURRENT;
  IVObjectComponent::Serialize(ar);
  if (ar.IsLoading())
  {
    ar >> iLocalVersion;
    VASSERT_MSG(iLocalVersion >= ENTITYLODCOMPONENT_VERSION_0 &&
      iLocalVersion <= ENTITYLODCOMPONENT_VERSION_CURRENT, "Invalid local version. Please re-export");

    ar >> LOD_LevelMode;
    ar >> LOD_LevelCount;
    if (iLocalVersion < ENTITYLODCOMPONENT_VERSION_1)
      LOD_LevelCount += 1; // Enum values started at "1" (instead of "0") in old version.

    ar >> Level_Medium_Mesh;
    ar >> Level_Low_Mesh;
    ar >> Level_UltraLow_Mesh;
    ar >> Level_Medium_Distance;
    ar >> Level_Low_Distance;
    ar >> Level_UltraLow_Distance;

    CommonInit();
  } 
  else
  {
    ar << iLocalVersion;

    ar << LOD_LevelMode;
    ar << LOD_LevelCount;

    ar << Level_Medium_Mesh;
    ar << Level_Low_Mesh;
    ar << Level_UltraLow_Mesh;
    ar << Level_Medium_Distance;
    ar << Level_Low_Distance;
    ar << Level_UltraLow_Distance;
  }
}


/// =========================================================================== ///
/// VEntityLODComponentManager IVisCallbackHandler_cl Overrides                 ///
/// =========================================================================== ///

void VEntityLODComponentManager::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
  if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneFinished)
  {
    // call update function on every component
    const int iCount = m_Components.Count();
    for (int i = 0; i < iCount; i++)
    {
      m_Components.GetAt(i)->PerFrameUpdate();
    }
  }
  else if (pData->m_pSender == &Vision::Callbacks.OnAfterSceneLoaded)
  {
    // call update function on every component
    const int iCount = m_Components.Count();
    for (int i = 0; i < iCount; i++)
    {
      m_Components.GetAt(i)->ConnectToExistingAnimConfig();
    }
  }
}

/// =========================================================================== ///
/// VEntityLODComponent Variable Table                                          ///
/// =========================================================================== ///

START_VAR_TABLE(VEntityLODComponent,IVObjectComponent, "Entity LOD Component. Can be attached to any entity so it can have multiple LOD levels each represented by a different model. The highest level is defined by the model of the owner entity.", VVARIABLELIST_FLAGS_NONE, "Entity LOD" )
  DEFINE_VAR_ENUM     (VEntityLODComponent, LOD_LevelMode,            "LOD Mode - Activate a specific LOD Level or let the component set the LOD dynamically depending on the distance to the camera.", "LOD_AUTO", "LOD_HIGH,LOD_MEDIUM,LOD_LOW,LOD_ULTRALOW,LOD_DISABLED,LOD_AUTO", 0, 0);
  DEFINE_VAR_ENUM     (VEntityLODComponent, LOD_LevelCount,           "LOD Count - Specify the number of LOD Levels.", "1","0,1,2,3", 0, 0);
  DEFINE_VAR_VSTRING  (VEntityLODComponent, Level_Medium_Mesh,        "Modelfile for the Medium Level.", "", 0, 0, "assetpicker(Model)");
  DEFINE_VAR_VSTRING  (VEntityLODComponent, Level_Low_Mesh,           "Modelfile for the Low Level.", "", 0, 0, "assetpicker(Model)");
  DEFINE_VAR_VSTRING  (VEntityLODComponent, Level_UltraLow_Mesh,      "Modelfile for the Ultralow Level.", "", 0, 0, "assetpicker(Model)");
  DEFINE_VAR_FLOAT    (VEntityLODComponent, Level_Medium_Distance,    "Distance for the Medium Level.", "500.0", DISPLAY_HINT_GLOBALUNITSCALED, 0);
  DEFINE_VAR_FLOAT    (VEntityLODComponent, Level_Low_Distance,       "Distance for the Low Level.", "1000.0", DISPLAY_HINT_GLOBALUNITSCALED, 0);  
  DEFINE_VAR_FLOAT    (VEntityLODComponent, Level_UltraLow_Distance,  "Distance for the Ultralow Level.", "1500.0", DISPLAY_HINT_GLOBALUNITSCALED, 0);
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
