/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/HavokPhysicsEnginePluginPCH.h>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokBlockerVolumeComponent.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokShapeFactory.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokPhysicsModule.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokConversionUtils.hpp>

//------------------------------------------------------------------------------

V_IMPLEMENT_SERIAL(vHavokBlockerVolumeComponent, IVObjectComponent, 0, &g_vHavokModule);

//------------------------------------------------------------------------------

vHavokBlockerVolumeComponent::vHavokBlockerVolumeComponent()
  : IVObjectComponent()
  , m_bEnabled(TRUE)
  , m_bDebugRenderingEnabled(FALSE)
  , m_debugColor(50, 50, 255)
  , m_iShapeType(VHavokBlockerVolumeShapeType_CONVEX)
  , m_fRestitution(0.4f)
  , m_fFriction(0.5f)
  , m_iCollisionLayer(vHavokPhysicsModule::HK_LAYER_COLLIDABLE_STATIC)
  , m_iCollisionGroup(0)
  , m_iSubSystemId(0)
  , m_iSubSystemDontCollideWith(0)
  , m_pRigidBody(NULL)
{
}

vHavokBlockerVolumeComponent::~vHavokBlockerVolumeComponent()
{
}

//------------------------------------------------------------------------------

void vHavokBlockerVolumeComponent::SetOwner(VisTypedEngineObject_cl *pOwner)
{
  IVObjectComponent::SetOwner(pOwner);

  if (pOwner != NULL)
  {
    CreatePhysicsObject();
  }
  else
  {
    DestroyPhysicsObject();
  }
}

//-----------------------------------------------------------------------------------
// Properties

void vHavokBlockerVolumeComponent::SetEnabled(bool bEnabled)
{
  m_bEnabled = bEnabled ? TRUE : FALSE;

  // Add / Remove from world.
  if (m_pRigidBody != NULL)
  {
    if (bEnabled && m_pRigidBody->getWorld() == NULL)
    {
      AddToPhysicsWorld();
    }
    else if (!bEnabled && m_pRigidBody->getWorld() != NULL)
    {
      RemoveFromPhysicsWorld();
    }
  }
}

void vHavokBlockerVolumeComponent::SetRestitution(float fRestitution)
{
  m_fRestitution = fRestitution;

  if (m_pRigidBody != NULL)
  {
    vHavokPhysicsModule* pModule = vHavokPhysicsModule::GetInstance();
    VASSERT(pModule != NULL);

    pModule->MarkForWrite();
    m_pRigidBody->setRestitution(fRestitution);
    pModule->UnmarkForWrite();
  }
}

void vHavokBlockerVolumeComponent::SetFriction(float fFriction)
{
  m_fFriction = fFriction;

  if (m_pRigidBody != NULL)
  {
    vHavokPhysicsModule* pModule = vHavokPhysicsModule::GetInstance();
    VASSERT(pModule != NULL);

    pModule->MarkForWrite();
    m_pRigidBody->setFriction(fFriction);
    pModule->UnmarkForWrite();
  }
}

void vHavokBlockerVolumeComponent::SetCollisionFilterInfo(int iCollisionLayer, int iCollisionGroup, 
  int iSubSystemId, int iSubSystemDontCollideWith )
{
  m_iCollisionLayer = iCollisionLayer;
  m_iCollisionGroup = iCollisionGroup;
  m_iSubSystemId = iSubSystemId;
  m_iSubSystemDontCollideWith = iSubSystemDontCollideWith;

  if (m_pRigidBody != NULL)
  {
    vHavokPhysicsModule* pModule = vHavokPhysicsModule::GetInstance();
    VASSERT(pModule != NULL);

    const int iFilterInfo = hkpGroupFilter::calcFilterInfo(
      m_iCollisionLayer, m_iCollisionGroup, 
      m_iSubSystemId, m_iSubSystemDontCollideWith);

    pModule->MarkForWrite();

    m_pRigidBody->setCollisionFilterInfo(iFilterInfo);

    hkpWorld* pWorld = m_pRigidBody->getWorld();
    if (pWorld != NULL)
    {
      pWorld->updateCollisionFilterOnEntity(m_pRigidBody, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, 
        HK_UPDATE_COLLECTION_FILTER_IGNORE_SHAPE_COLLECTIONS);
    }

    pModule->UnmarkForWrite();
  }
}

void vHavokBlockerVolumeComponent::SetShapeType(VHavokBlockerVolumeShapeType_e eShapeType)
{
  m_iShapeType = static_cast<int>(eShapeType);
  if (GetOwner() != NULL)
    CreatePhysicsObject();
}

//------------------------------------------------------------------------------

BOOL vHavokBlockerVolumeComponent::CanAttachToObject(
  VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut)
{
  if (!IVObjectComponent::CanAttachToObject(pObject, sErrorMsgOut))
    return FALSE;

  if (!pObject->IsOfType(V_RUNTIME_CLASS(VCustomVolumeObject)))
  {
    sErrorMsgOut += "Component can only be added to instances of VCustomVolumeObject or derived classes.";
    return FALSE;
  }

  return TRUE;
}

//------------------------------------------------------------------------------

void vHavokBlockerVolumeComponent::CreatePhysicsObject()
{
  DestroyPhysicsObject();

  // Find static mesh of owner.
  VCustomVolumeObject* pCustomVolume = GetOwnerVolume();

  VisStaticMesh_cl* pMesh = pCustomVolume->GetStaticMesh();
  if (pMesh == NULL)
    return;
  pMesh->EnsureLoaded();

  if (!pMesh->IsLoaded() || pMesh->GetNumOfTriangles() < 1)
    return;

  // Check scale is valid.
  const hkvVec3 vScale = pCustomVolume->GetScale();
  const hkvAlignedBBox& bbox = pMesh->GetBoundingBox();

  hkVector4 bbox_min; 
  vHavokConversionUtils::VisVecToPhysVec_noscale(bbox.m_vMin, bbox_min);
  hkVector4 bbox_max; 
  vHavokConversionUtils::VisVecToPhysVec_noscale(bbox.m_vMax, bbox_max);

  hkVector4 bbox_extent; 
  bbox_extent.setSub(bbox_max, bbox_min); 
  bbox_extent.mul(vHavokConversionUtils::GetVision2HavokScaleSIMD());
  hkVector4 hkScale;
  vHavokConversionUtils::VisVecToPhysVec_noscale(vScale, hkScale);
  bbox_extent.mul(hkScale);

  hkVector4 meshTol; 
  meshTol.setAll(hkReal((m_iShapeType == VHavokBlockerVolumeShapeType_CONVEX) ? 
    HKVIS_CONVEX_SHAPE_TOLERANCE : HKVIS_MESH_SHAPE_TOLERANCE));

  hkVector4Comparison notLargeEnough = bbox_extent.less(meshTol);
  if (notLargeEnough.anyIsSet<hkVector4ComparisonMask::MASK_XYZ>())
  {
    const char *szMeshFilename = (pMesh->GetFilename() != NULL) ? pMesh->GetFilename() : "UnnamedMesh";
    Vision::Error.Warning(
      "Havok Physics Blocker Volume: Static mesh [%s] has undersized extents (%.4f, %4f, %.4f)", 
      szMeshFilename, bbox_extent(0), bbox_extent(1), bbox_extent(2));
    return;
  }
  
  // Create the shape.
  hkRefPtr<hkpShape> shape;
  switch(m_iShapeType)
  {
  case VHavokBlockerVolumeShapeType_CONVEX:
    shape = vHavokShapeFactory::CreateConvexHullShapeFromMesh(pMesh, pCustomVolume->GetScale(), NULL, 0);
    break;
  case VHavokBlockerVolumeShapeType_MESH:
    shape = vHavokShapeFactory::CreateShapeFromMesh(pMesh, pCustomVolume->GetScale(), 
      NULL, 0 /* Do not cache shape for the time being*/);
    break;
  default:
    VASSERT_MSG(false, "Unknown shape type.");
    break;
  }

  if (shape.val() == NULL)
    return;

  // Create the rigid body.
  hkpRigidBodyCinfo cInfo;
  cInfo.m_motionType = hkpMotion::MOTION_FIXED;
  cInfo.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;
  cInfo.m_restitution = m_fRestitution;
  cInfo.m_friction = m_fFriction;
  cInfo.m_shape = shape.val();

  vHavokConversionUtils::VisVecToPhysVecWorld(pCustomVolume->GetPosition(), cInfo.m_position);
  vHavokConversionUtils::VisMatrixToHkQuat(pCustomVolume->GetRotationMatrix(), cInfo.m_rotation);

  cInfo.m_collisionFilterInfo = hkpGroupFilter::calcFilterInfo(
    m_iCollisionLayer, m_iCollisionGroup, m_iSubSystemId, m_iSubSystemDontCollideWith);
  
  // Create the rigid body.
  m_pRigidBody = new hkpRigidBody(cInfo);

  // Add to world.
  if (IsEnabled())
    AddToPhysicsWorld();
}

void vHavokBlockerVolumeComponent::DestroyPhysicsObject()
{
  if (m_pRigidBody == NULL)
    return;

  // Remove from world
  if (IsEnabled())
    RemoveFromPhysicsWorld();

  m_pRigidBody->removeReference();
  m_pRigidBody = NULL;
}

//------------------------------------------------------------------------------

void vHavokBlockerVolumeComponent::AddToPhysicsWorld()
{
  VASSERT(m_pRigidBody != NULL);
  VASSERT_MSG(m_pRigidBody->getWorld() == NULL, "Blocker Volume already added to physics world.");

  hkpWorld* pPhysicsWorld = vHavokPhysicsModule::GetInstance()->GetPhysicsWorld();
  if (pPhysicsWorld != NULL)
  {
    pPhysicsWorld->markForWrite();
    pPhysicsWorld->addEntity(m_pRigidBody);
    pPhysicsWorld->unmarkForWrite();

    vHavokPhysicsModule::GetInstance()->AddBlockerVolume(this);
  }
}

void vHavokBlockerVolumeComponent::RemoveFromPhysicsWorld()
{
  VASSERT(m_pRigidBody != NULL);
  VASSERT_MSG(m_pRigidBody->getWorld() != NULL, "Blocker Volume not added to physics world.");

  hkpWorld* pPhysicsWorld = vHavokPhysicsModule::GetInstance()->GetPhysicsWorld();
  if (pPhysicsWorld != NULL)
  {
    pPhysicsWorld->markForWrite();
    pPhysicsWorld->removeEntity(m_pRigidBody);
    pPhysicsWorld->unmarkForWrite();

    vHavokPhysicsModule::GetInstance()->RemoveBlockerVolume(this);
  }
}

//------------------------------------------------------------------------------

void vHavokBlockerVolumeComponent::OnVariableValueChanged(
  VisVariable_cl* pVar, const char* value) 
{
  if (vHavokPhysicsModule::GetInstance() == NULL || m_pRigidBody == NULL ||
    GetOwner() == NULL)
  {
    return;
  }

  // Ensure rigid body gets added / removed from world.
  else if (strcmp(pVar->GetName(), "m_bEnabled") == 0)
  {
    SetEnabled(IsEnabled());
  }
  else if (strcmp(pVar->GetName(), "m_iShapeType") == 0)
  {
    // Needs re-creation
    CreatePhysicsObject();
  }
  else if (strcmp(pVar->GetName(), "m_fRestitution") == 0)
  {
    SetRestitution(m_fRestitution);
  }
  else if (strcmp(pVar->GetName(), "m_fFriction") == 0)
  {
    SetFriction(m_fFriction);
  }
  else if (strcmp(pVar->GetName(), "m_iCollisionLayer") == 0 ||
    strcmp(pVar->GetName(), "m_iCollisionGroup") == 0 ||
    strcmp(pVar->GetName(), "m_iSubSystemId") == 0 ||
    strcmp(pVar->GetName(), "m_iSubSystemDontCollideWith") == 0)
  {
    SetCollisionFilterInfo(m_iCollisionLayer, m_iCollisionGroup,
      m_iSubSystemId, m_iSubSystemDontCollideWith);
  }
  // Debug Rendering
  else if (strcmp(pVar->GetName(), "m_bDebugRenderingEnabled") == 0)
  {
    SetDebugRendering(m_bDebugRenderingEnabled);
  }
  else if (strcmp(pVar->GetName(), "m_debugColor") == 0)
  {
    SetDebugColor(m_debugColor);
  }
}

void vHavokBlockerVolumeComponent::MessageFunction(
  int iID, INT_PTR iParamA, INT_PTR iParamB)
{
  IVObjectComponent::MessageFunction(iID, iParamA, iParamB);

  if (GetOwner() == NULL)
    return;

  if (iID == VIS_MSG_EDITOR_PROPERTYCHANGED)
  {
    const char* szPropertyName = reinterpret_cast<const char*>(iParamA);

    // Re-transform the rigid body if one of the following properties changed.
    if (strcmp(szPropertyName, "Position") == 0 ||
      strcmp(szPropertyName, "Orientation") == 0)
    {
      // Rigid body may not have been created due to invalid mesh.
      if (m_pRigidBody == NULL)
        return;

      hkpWorld* pPhysicsWorld = vHavokPhysicsModule::GetInstance()->GetPhysicsWorld();
      if (pPhysicsWorld != NULL)
      {
        VCustomVolumeObject* pCustomVolume = vstatic_cast<VCustomVolumeObject*>(GetOwner());

        hkVector4 position;
        vHavokConversionUtils::VisVecToPhysVecWorld(pCustomVolume->GetPosition(), position);

        hkQuaternion rotation;
        vHavokConversionUtils::VisMatrixToHkQuat(pCustomVolume->GetRotationMatrix(), rotation);

        pPhysicsWorld->markForWrite();
        m_pRigidBody->setPositionAndRotation(position, rotation);
        pPhysicsWorld->unmarkForWrite();
      }
    }
    // Re-create the rigid body if one of the following properties changed.
    else if (strcmp(szPropertyName, "Scaling") == 0 ||
      strcmp(szPropertyName, "Height") == 0 ||
      strcmp(szPropertyName, "VolumeGeometry") == 0 ||
      strcmp(szPropertyName, "CustomStaticMesh") == 0 ||
      strcmp(szPropertyName, "StaticMeshPath") == 0)
    {
      CreatePhysicsObject();
    }
  }
}

//------------------------------------------------------------------------------

void vHavokBlockerVolumeComponent::OnDeserializationCallback(
  const VSerializationContext &context)
{
  // Mesh should be available now.
  if (m_pRigidBody == NULL)
    CreatePhysicsObject();
}

//------------------------------------------------------------------------------

#define VHAVOKBLOCKERVOLUMECOMPONENT_VERSION_1        1 // Initial version
#define VHAVOKBLOCKERVOLUMECOMPONENT_VERSION_CURRENT  VHAVOKBLOCKERVOLUMECOMPONENT_VERSION_1

void vHavokBlockerVolumeComponent::Serialize(VArchive &ar)
{
  IVObjectComponent::Serialize(ar);

  if (ar.IsLoading())
  {
    int iLocalVersion;
    ar >> iLocalVersion;
    VASSERT_MSG(iLocalVersion >= VHAVOKBLOCKERVOLUMECOMPONENT_VERSION_1 &&
      iLocalVersion <= VHAVOKBLOCKERVOLUMECOMPONENT_VERSION_CURRENT, 
      "vHavokBlockerVolumeComponent: Invalid local version. Please re-export");

    ar >> m_bEnabled;
    ar >> m_bDebugRenderingEnabled;
    ar >> m_debugColor;

    ar >> m_iShapeType;

    ar >> m_fRestitution;
    ar >> m_fFriction;

    ar >> m_iCollisionLayer;
    ar >> m_iCollisionGroup;
    ar >> m_iSubSystemId;
    ar >> m_iSubSystemDontCollideWith;
  }
  else
  {
    const int iLocalVersion = VHAVOKBLOCKERVOLUMECOMPONENT_VERSION_CURRENT;
    ar << iLocalVersion;

    ar << m_bEnabled;
    ar << m_bDebugRenderingEnabled;
    ar << m_debugColor;

    ar << m_iShapeType;

    ar << m_fRestitution;
    ar << m_fFriction;

    ar << m_iCollisionLayer;
    ar << m_iCollisionGroup;
    ar << m_iSubSystemId;
    ar << m_iSubSystemDontCollideWith;
  }
}

//------------------------------------------------------------------------------
// Debug Rendering

void vHavokBlockerVolumeComponent::SetDebugRendering(BOOL bEnable)
{
  m_bDebugRenderingEnabled = bEnable;

  if (m_pRigidBody == NULL || m_pRigidBody->getWorld() == NULL)
    return;

  vHavokPhysicsModule* pPhysicsModule = vHavokPhysicsModule::GetInstance();
  if (pPhysicsModule == NULL)
    return;

  const bool bEnableDebug = (m_bDebugRenderingEnabled || pPhysicsModule->m_bDebugRenderBlockerVolumes);

  // Ensure debug display handler is created
  if (bEnableDebug)
    pPhysicsModule->SetEnabledDebug(true);

  // Get display handler
  vHavokDisplayHandler* pDisplay = pPhysicsModule->GetHavokDisplayHandler();
  if (pDisplay == NULL)
    return;

  hkpWorld* pWorld = pPhysicsModule->GetPhysicsWorld();
  VASSERT(pWorld != NULL);
  pWorld->markForRead();

  // Get ID (cast from collidable pointer as its is used for display geometry ID)
  const hkUlong id = reinterpret_cast<hkUlong>(m_pRigidBody->getCollidable());

  pWorld->unmarkForRead();

  // Set display properties
  // Set debug color and visibility state
  pDisplay->SetVisible(id, bEnableDebug);
  pDisplay->SetColor(id, m_debugColor);

  // Attach the Havok Debug Shape to the owner object
  if (bEnableDebug)
    pDisplay->SetOwner(id, vstatic_cast<VisObject3D_cl*>(GetOwner()));
}

void vHavokBlockerVolumeComponent::SetDebugColor(VColorRef color)
{
  m_debugColor = color;

  if (m_pRigidBody == NULL || m_pRigidBody->getWorld() == NULL)
    return;

  vHavokPhysicsModule* pPhysicsModule = vHavokPhysicsModule::GetInstance();
  if (pPhysicsModule == NULL)
    return;

  // Get display handler
  vHavokDisplayHandler* pDisplay = pPhysicsModule->GetHavokDisplayHandler();
  if (pDisplay == NULL)
    return;

  hkpWorld* pWorld = pPhysicsModule->GetPhysicsWorld();
  VASSERT(pWorld != NULL);
  pWorld->markForRead();

  // Get ID (cast from collidable pointer as its is used for display geometry ID)
  const hkUlong id = reinterpret_cast<hkUlong>(m_pRigidBody->getCollidable());

  pWorld->unmarkForRead();

  // Set debug color
  pDisplay->SetColor(id, m_debugColor);
}

//------------------------------------------------------------------------------

START_VAR_TABLE(vHavokBlockerVolumeComponent, IVObjectComponent, "Turns custom volume object into an invisible physics blocker.", VVARIABLELIST_FLAGS_NONE, "Havok Physics Blocker Volume" )
  DEFINE_VAR_BOOL_AND_NAME(vHavokBlockerVolumeComponent, m_bEnabled, "Enabled", "Enable or disable component", "True", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(vHavokBlockerVolumeComponent, m_bDebugRenderingEnabled, "Debug", "Enables / Disables Physics Debug Rendering.", "False", 0, 0);
  DEFINE_VAR_COLORREF_AND_NAME(vHavokBlockerVolumeComponent, m_debugColor, "DebugColor", "Color of this blocker volume when Debug Rendering is active.", "50,50,255,255", 0, 0);
  DEFINE_VAR_ENUM_AND_NAME(vHavokBlockerVolumeComponent, m_iShapeType, "ShapeType", "Geometry used for Physics", "Convex Hull", "Convex Hull/Mesh", 0, 0);  
  DEFINE_VAR_FLOAT_AND_NAME(vHavokBlockerVolumeComponent, m_fRestitution, "Restitution", "The restitution defines how much impulse is preserved during a collision. A restitution of 0 will result in the body losing its entire momentum, while higher values result in \"bouncier\" collisions. A value of 1 corresponds to a perfectly elastic collision where no energy is lost. You may use values higher than one to achieve a certain desired collision behavior, although this may result in unrealistic collisions.", "0.4", 0, "Clamp(0,1.98)");
  DEFINE_VAR_FLOAT_AND_NAME(vHavokBlockerVolumeComponent, m_fFriction, "Friction", "The coefficient of friction of the surface of this collision object. Lower values will result in a smoother surface and more slipping, while higher values will result in a less smooth surface and less slipping.", "0.5", 0, "Clamp(0,1)");
  DEFINE_VAR_INT_AND_NAME(vHavokBlockerVolumeComponent, m_iCollisionLayer, "Collision Layer", "Defines the collision layer this collision object is assigned to.", "2", 0, "Clamp(0, 31)");
  DEFINE_VAR_INT_AND_NAME(vHavokBlockerVolumeComponent, m_iCollisionGroup, "Collision Group", "Defines the collision group this collision object is assigned to.", "0", 0, "Clamp(0, 65535)");
  DEFINE_VAR_INT_AND_NAME(vHavokBlockerVolumeComponent, m_iSubSystemId, "SubSystem ID", "Defines the sub system ID of this RB.", "0", 0, "Clamp(0, 31)");
  DEFINE_VAR_INT_AND_NAME(vHavokBlockerVolumeComponent, m_iSubSystemDontCollideWith, "SubSystem to not collide with", "Defines the sub system ID this collision object doesn't collide with.", "0", 0, "Clamp(0, 31)");
END_VAR_TABLE

//------------------------------------------------------------------------------

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
