/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/HavokPhysicsEnginePluginPCH.h>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokRigidBody.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokUserData.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokShapeFactory.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokContactListener.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokConversionUtils.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokFileStreamAccess.hpp>

#include <Physics2012/Utilities/Dynamics/KeyFrame/hkpKeyFrameUtility.h>
// Serialization Support
#include <Common/Serialize/Util/hkSerializeUtil.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>
#include <Physics2012/Utilities/Serialize/hkpPhysicsData.h>
#include <Common/Serialize/Packfile/hkPackfileData.h>
#include <Common/Base/System/Io/Reader/Buffered/hkBufferedStreamReader.h>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/Effects/Mirror.hpp>
#include <Common/Base/Math/Matrix/hkMatrix3Util.h>

// -------------------------------------------------------------------------- //
// Constructor / Destructor                                                   //
// -------------------------------------------------------------------------- //

V_IMPLEMENT_SERIAL(vHavokRigidBody,IVObjectComponent,0,&g_vHavokModule);


vHavokRigidBody::vHavokRigidBody(int iComponentFlags)
  : IVObjectComponent(0, iComponentFlags)
  , m_pModule(NULL)
  , m_bAddedToWorld(false)
  , m_szShapeCacheId(NULL)
{
  m_pModule = vHavokPhysicsModule::GetInstance();

  Shape_Type = ShapeType_CONVEX;
  Shape_Radius = 0.0f;
  Shape_BoxSize.setZero();
  Shape_Height = 0.0f;

  Shape_CenterOfMass = hkvVec3::ZeroVector();   
  Shape_PivotOffset = hkvVec3::ZeroVector();
  Shape_PivotOffsetAutoAlign = TRUE;

  Havok_NoDeactivate = FALSE;

  Havok_Mass = 10.f;
  Havok_InertiaTensorComputeMode = InertiaTensorComputeMode_NONE;
  Havok_SurfaceThickness = 0.1f;
  Havok_InertiaTensorAxisScaling.set (1.0f);
  Havok_InertiaTensorScaling = 1.0f;

  Havok_Friction = 0.5f;
  Havok_Restitution = 0.4f;
  Havok_LinearDamping = 0.0f;
  Havok_AngularDamping = 0.0f;

  Havok_CollisionLayer = 1;
  Havok_CollisionGroup = 0;
  Havok_SubSystemId = 0;
  Havok_SubSystemDontCollideWith = 0;

  Debug_Render = FALSE;
  Debug_Color = VColorRef(50, 255, 50, 255);

  Havok_TakeRbDataFromFile = FALSE;
  Havok_MotionType = MotionType_DYNAMIC;
  Havok_QualityType = QualityType_MOVING;
  Havok_WeldingType = VIS_WELDING_TYPE_NONE;

  Havok_MaxKinematicVelocity = 600.0f;

  Havok_AlwaysInitAsDynamic = FALSE;
  Havok_Active = TRUE;
  Havok_TightFit = FALSE; // false so back compat. Better as true though.
}

vHavokRigidBody::~vHavokRigidBody()
{
  CommonDeinit();
}

void vHavokRigidBody::DisposeObject()
{
  CommonDeinit();
  IVObjectComponent::DisposeObject();
}

vHavokRigidBody *vHavokRigidBody::FromHkRigidBody(const hkpRigidBody *pHkRigidBody)
{
  if (!pHkRigidBody)
    return NULL;

  vHavokUserDataType_e eType = V_USERDATA_UNDEFINED;
  void *pDataPtr = vHavokUserDataPointerPair_t::ExtractTypeAndPointer(
    reinterpret_cast<void*>(pHkRigidBody->getUserData()), eType);

  return (eType == V_USERDATA_OBJECT) ? (vHavokRigidBody*)pDataPtr : NULL;
}


// -------------------------------------------------------------------------- //
// Common Init                                                                //
// -------------------------------------------------------------------------- //

void vHavokRigidBody::CommonInit()
{
  // Do not initialize the component in case our module is not active
  if (!m_pModule)
  {
    Vision::Error.Warning("Failed to initialize vHavokRigidBody since a non Havok physics module is currently active");
    return;
  }

  VisObject3D_cl *pOwnerObject = GetOwner3D();
  VVERIFY_OR_RET(pOwnerObject!=NULL);

  VBool bIsEntity = pOwnerObject->IsOfType(V_RUNTIME_CLASS(VisBaseEntity_cl));

  VVERIFY_OR_RET(bIsEntity || pOwnerObject->IsOfType(V_RUNTIME_CLASS(VisMirror_cl)));

  VDynamicMesh *pMesh = NULL; 
  hkVector4 vScale = hkVector4::getConstant<HK_QUADREAL_1>();
  if (bIsEntity)
  {
    // Get the model from owner entity
    VisBaseEntity_cl *pOwnerEntity = (VisBaseEntity_cl *)pOwnerObject;
    pMesh = pOwnerEntity->GetMesh();
    if (pMesh)
    {
      // Check whether the model is really there
      pMesh->EnsureLoaded();
      if (!pMesh->IsLoaded())
        pMesh = NULL;
    }

    // Get scaling vector of owner entity
    vHavokConversionUtils::VisVecToPhysVec_noscale(pOwnerEntity->GetScaling(), vScale);

    // Check if pMesh is valid. We need a valid pMesh if the type is Mesh or Convex
    if (Shape_Type==ShapeType_MESH || Shape_Type==ShapeType_CONVEX)
    {
      if (pMesh)
      {
		  const hkvAlignedBBox& bbox = pMesh->GetBoundingBox();
		  hkVector4 bbox_min; vHavokConversionUtils::VisVecToPhysVec_noscale(bbox.m_vMin, bbox_min);
		  hkVector4 bbox_max; vHavokConversionUtils::VisVecToPhysVec_noscale(bbox.m_vMax, bbox_max);
		  hkVector4 bbox_extent; bbox_extent.setSub(bbox_max,bbox_min); bbox_extent.mul(vScale); bbox_extent.mul(vHavokConversionUtils::GetVision2HavokScaleSIMD());

		  hkVector4 meshTol; meshTol.setAll(hkReal((Shape_Type==ShapeType_CONVEX) ? HKVIS_CONVEX_SHAPE_TOLERANCE : HKVIS_MESH_SHAPE_TOLERANCE));
		  hkVector4Comparison::Mask largeEnough = bbox_extent.greaterEqual(meshTol).getMask<hkVector4ComparisonMask::MASK_XYZ>();
		  if (hkMath::countBitsSet(largeEnough) < 2)
        {
          const char *szMeshFilename = (pMesh->GetFilename()!=NULL) ? pMesh->GetFilename() : "Unnamed";
          Vision::Error.Warning("Initializing vHavokRigidBody with a mesh [%s] with undersized extents (%.4f, %4f, %.4f), when using a shape type Mesh or Convex", 
                                szMeshFilename, bbox_extent(0), bbox_extent(1), bbox_extent(2));
          return;
        }
      }
      else
      {
        Vision::Error.Warning("Initializing vHavokRigidBody with a NULL mesh, when using a shape type Mesh or Convex");
        return;
      }
    }
  }
  else // VisMirror_cl
  {
    Shape_Type = ShapeType_BOX;
    Havok_MotionType = MotionType_FIXED;
    Havok_QualityType = QualityType_FIXED; 
    Havok_AlwaysInitAsDynamic = FALSE;  
    VisMirror_cl* pMirror = ((VisMirror_cl*)pOwnerObject);
    float fThickness = 10.f*Vision::World.GetGlobalUnitScaling(); 
    Shape_PivotOffsetAutoAlign = false; 
    Shape_PivotOffset.set(0.0f,0.0f,fThickness*-0.5f);
    Shape_BoxSize.set(pMirror->GetSizeX(),pMirror->GetSizeY(),fThickness);
  }

  // Get uniform scale
  float fScale;
  (vScale.horizontalAdd<3>() * hkSimdReal_Inv3).store<1>(&fScale); 

  // Set init template
  vHavokRigidBody::InitTemplate initTempl;

  // Set motion type
  switch (Havok_MotionType)
  {
    case MotionType_DYNAMIC: 
    case MotionType_SPHERE_INERTIA:
    case MotionType_BOX_INERTIA:
    case MotionType_THIN_BOX_INERTIA:
      {
        initTempl.m_motionType = hkpMotion::MOTION_DYNAMIC; 
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
      }
      break;
    case MotionType_KEYFRAMED: 
      {
        initTempl.m_motionType = hkpMotion::MOTION_KEYFRAMED; 
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_KEYFRAMED;  
      }
      break;
    case MotionType_FIXED:
      {
        initTempl.m_motionType = hkpMotion::MOTION_FIXED; 
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;  
      }
      break;
    case MotionType_CHARACTER:
      {
        initTempl.m_motionType = hkpMotion::MOTION_CHARACTER;
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_CHARACTER;
      }
      break;
    default:
      {
        VASSERT_MSG(false, "Unsupported MotionType!");
        initTempl.m_motionType = hkpMotion::MOTION_DYNAMIC; 
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
      }
      break;
  }

  // OVERWRITES motion type (See quality type settings done in the paragraph before this line)
  // Only do that for non motion fixed objects
  if(Havok_MotionType != MotionType_FIXED)
  {
    switch (Havok_QualityType)
    {
    case QualityType_AUTO:
      {
        // Nothing, as this property only overwrites automatic done settings one paragraph earlier 
      }
      break;
    case QualityType_FIXED:
      {
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;	
      }
      break;
    case QualityType_KEYFRAMED:
      {
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_KEYFRAMED;
      }
      break;
    case QualityType_DEBRIS:
      {
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_DEBRIS;
      }
      break;
    case QualityType_DEBRIS_SIMPLE_TOI:
      {
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_DEBRIS_SIMPLE_TOI;
      }
      break;
    case QualityType_MOVING:
      {
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
      }
      break;
    case QualityType_CRITICAL:
      {
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
      }
      break;
    case QualityType_BULLET:
      {
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_BULLET;
      }
      break;
    case QualityType_CHARACTER:
      {
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_CHARACTER;
      }
      break;
    case QualityType_KEYFRAMED_REPORTING:
      {
        initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_KEYFRAMED_REPORTING;
      }
      break;
    }
  }

  // Overwrite settings if dynamic init is desired  
  hkpMotion::MotionType savedMotionType = initTempl.m_motionType;
  hkpCollidableQualityType savedQualityType = initTempl.m_qualityType;
  if(Havok_AlwaysInitAsDynamic == TRUE)
  {
    initTempl.m_motionType = hkpMotion::MOTION_DYNAMIC;
    initTempl.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
  }

  // Reset the center of mass offset and pivot offset on each recreation. The various
  // Init* methods below may set a value again.
  m_vCenterOfMassOffset.setZero();
  Shape_PivotOffset.setZero();

  hkvVec3 visionObjScale;
  vHavokConversionUtils::PhysVecToVisVec_noscale(vScale, visionObjScale);

  // Set shape body type (box, sphere, convex, mesh)
  switch (Shape_Type)
  {
    case ShapeType_BOX:
      {
        hkVector4 shapeBoxSize; vHavokConversionUtils::VisVecToPhysVec_noscale(Shape_BoxSize, shapeBoxSize);

        // Get size from model bounding box when size < 0
        if (shapeBoxSize.lessEqualZero().anyIsSet<hkVector4ComparisonMask::MASK_XYZ>() && pMesh!=NULL)
        {
          InitBoxRb(pMesh, visionObjScale, initTempl);
        }
        else
        {
          VASSERT_MSG( shapeBoxSize.greaterZero().allAreSet<hkVector4ComparisonMask::MASK_XYZ>(),
                       "Shape_BoxSize is invalid. You need to use positive size." );      
          InitBoxRb( Shape_BoxSize, visionObjScale, initTempl);
        }
      }
      break;
    
    case ShapeType_SPHERE:
      {
        // Get radius from model bounding box when radius <= 0
        if (Shape_Radius <= 0.0f && pMesh != NULL)
        {
          InitSphereRb(pMesh, fScale, initTempl);
        }
        else
        {
          VASSERT_MSG( Shape_Radius > 0.0f, "Shape_Radius is invalid. You need to use a positive radius." );
          InitSphereRb(Shape_Radius, fScale, initTempl);
        }
      }
      break;

    case ShapeType_CONVEX:
      {
        // if we're here, pMesh is != NULL for sure, since we checked it at the beginning of method
        InitConvexRb(pMesh, visionObjScale, initTempl); 
      }
      break;

    case ShapeType_FILE:
      {
        if (!InitRbFromFile(Havok_FileResourceName.AsChar(), Havok_TakeRbDataFromFile, fScale, &initTempl))
        {
          // Use box collision shape instead
          Vision::Error.Warning("Havok collision file not found. Using box shape instead.");

          Shape_Type = ShapeType_BOX;
          CommonInit();
        }
      }
      break;
    case ShapeType_CAPSULE:
      {
        if((Shape_Height <= 0.0f || Shape_Radius <= 0.0f) && pMesh != NULL)
        {
          InitCapsuleRb(pMesh, fScale, initTempl);
        }
        else
        {
          VASSERT_MSG(Shape_Height > 0.0f && Shape_Radius > 0.0f, "Shape_Height or Shape_Radius are invalid. Use positive numbers" );
          InitCapsuleRb(hkvVec3::ZeroVector (), hkvVec3(0.0f, 0.0f, Shape_Height), Shape_Radius, fScale, initTempl);
        }
      }
      break;
    case ShapeType_CYLINDER:
      if ((Shape_Height <= 0.0f || Shape_Radius <= 0.0f) && pMesh != NULL)
      {
        InitCylinderRb(pMesh, fScale, initTempl);
      }
      else
      {
        VASSERT_MSG( Shape_Height > 0.0f && Shape_Radius > 0.0f, "Shape_Height or Shape_Radius are invalid. Use positive numbers" );
        InitCylinderRb(hkvVec3::ZeroVector (), hkvVec3(0.0f, 0.0f, Shape_Height), Shape_Radius, fScale, initTempl);
      }
      break;
    case ShapeType_MESH:
      {
        // if we're here, pMesh is != NULL for sure since we checked it at the beginning of method
        InitMeshRb(pMesh, visionObjScale, initTempl);
      }
      break;
    default: 
      {
        VASSERT_MSG(false, "Unsupported shape type!");
        return;
      }
  }

  // Reset to desired motion type, if dynamic init was chosen
  if(Havok_AlwaysInitAsDynamic == TRUE)
  {
    SetMotionType(savedMotionType);
    //we also have to set the quality type here, since "... SetMotionType updates hkpCollidableQualityType of this rigid body when 
    //changing to/from fixed or keyframed state."
    SetQualityType(savedQualityType); 
  }

  // Set debug visibility 
  SetDebugRendering(Debug_Render);
}

void vHavokRigidBody::CommonDeinit()
{
  RemoveHkRigidBody();
}


// -------------------------------------------------------------------------- //
// Shape Initialization                                                       //
// -------------------------------------------------------------------------- //

void vHavokRigidBody::InitCustomRb(hkpRigidBodyCinfo &cInfo, 
  hkpMassProperties const& massProperties)
{
  // Create the rigid body
  bool bResult = CreateHkRigidBody(cInfo, massProperties);
  VASSERT(bResult);
}

void vHavokRigidBody::InitBoxRb(const hkvVec3& vBoxSize, const hkvVec3& vScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Compute the box geometry for Havok
  HkBoxGeometry boxGeo;

  hkVector4 vScaleFinal; vHavokConversionUtils::VisVecToPhysVecLocal(vScale, vScaleFinal);

  hkVector4 vScaledBoxSize; vHavokConversionUtils::VisVecToPhysVec_noscale(vBoxSize, vScaledBoxSize);
  vScaledBoxSize.mul(vScaleFinal);
  
  boxGeo.m_HalfExtents.setMul(vScaledBoxSize, hkSimdReal_Half);

  // Initialize the rigid body
  InitBoxRb(boxGeo, initTempl);
}

void vHavokRigidBody::InitBoxRb(const VDynamicMesh *pMesh, const hkvVec3& vScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Compute the box geometry for Havok
  HkBoxGeometry boxGeo;

  hkVector4 vScaleFinal; vHavokConversionUtils::VisVecToPhysVecLocal(vScale, vScaleFinal);

  const hkvAlignedBBox& bbox = pMesh->GetCollisionBoundingBox();
  hkVector4 bbox_min; vHavokConversionUtils::VisVecToPhysVec_noscale(bbox.m_vMin, bbox_min);
  hkVector4 bbox_max; vHavokConversionUtils::VisVecToPhysVec_noscale(bbox.m_vMax, bbox_max);

  hkVector4 vScaledBoxSize; 
  vScaledBoxSize.setSub(bbox_max,bbox_min);
  vScaledBoxSize.mul(vScaleFinal);

  boxGeo.m_HalfExtents.setMul(vScaledBoxSize,hkSimdReal_Half);

  // Get automatic offset
  Shape_PivotOffset = Shape_PivotOffsetAutoAlign 
    ? vHavokShapeFactory::GetPivotOffset(pMesh, vScale) 
    : Shape_PivotOffset;

  // Initialize the rigid body
  InitBoxRb(boxGeo, initTempl);
}

void vHavokRigidBody::InitBoxRb(HkBoxGeometry const& geometry, vHavokRigidBody::InitTemplate &initTempl)
{
  // Fill the construction info based on the passed template
  hkpRigidBodyCinfo cInfo;
  FillConstructionInfo(cInfo, initTempl);

  HkBoxGeometry geometry0 = geometry;

  if (Havok_TightFit)
  {
	  hkVector4 r; r.set(hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius);
	  geometry0.m_HalfExtents.sub4(r);
  }

  // ensure valid box size
  const hkSimdReal fMinExtent = geometry0.m_HalfExtents.horizontalMin<3>();
  if (fMinExtent < hkSimdReal::fromFloat(HKVMATH_LARGE_EPSILON))
  {
    hkVector4 epsilon; epsilon.setAll(HKVMATH_LARGE_EPSILON);
    geometry0.m_HalfExtents.setMax(geometry0.m_HalfExtents, epsilon);
    Vision::Error.Warning("vHavokRigidBody: Box-size below minimum tolerance, clamped to %.4f", HKVMATH_LARGE_EPSILON);
  }

   // Create the box shape
  cInfo.m_shape = new hkpBoxShape(geometry0.m_HalfExtents);

 
  // Compute the box inertia tensor (if required)
  hkpMassProperties massProperties;
  switch (Havok_InertiaTensorComputeMode)
  {
  case InertiaTensorComputeMode_NONE:
    break;

  case InertiaTensorComputeMode_SURFACE:
    {
      float fAdjustedThickness = Havok_SurfaceThickness;
      hkSimdReal minHalfExtent = geometry0.m_HalfExtents.horizontalMin<3>();
      if(fAdjustedThickness >= minHalfExtent)
      {
        fAdjustedThickness = float(minHalfExtent) * 0.99f;
        Vision::Error.Warning("vHavokRigidBody: Surface thickness %f is greater than or equal to smallest box half extent %f, clamped to %f.", Havok_SurfaceThickness, float(minHalfExtent), fAdjustedThickness);
      }
      hkpInertiaTensorComputer::computeBoxSurfaceMassProperties(
        geometry0.m_HalfExtents, Havok_Mass, fAdjustedThickness,
        massProperties);
    }

    break;

  case InertiaTensorComputeMode_VOLUME:
    hkpInertiaTensorComputer::computeBoxVolumeMassProperties(
      geometry0.m_HalfExtents, Havok_Mass, massProperties);
    break;
  }

  // Create the rigid body
  InitCustomRb(cInfo, massProperties);
  // Remove reference
  cInfo.m_shape->removeReference();
  //Set shape type
  Shape_Type = ShapeType_BOX;
}

void vHavokRigidBody::InitSphereRb(float fRadius, float fScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Compute the sphere geometry for Havok
  hkReal rfScale = fScale * vHavokConversionUtils::GetVision2HavokScale();

  HkSphereGeometry sphereGeo;
  sphereGeo.m_Radius = fRadius * rfScale;

  // Initialize the rigid body
  InitSphereRb(sphereGeo, initTempl);
}

void vHavokRigidBody::InitSphereRb(const VDynamicMesh *pMesh, float fScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Compute the sphere geometry for Havok
  hkReal rfScale = fScale * vHavokConversionUtils::GetVision2HavokScale();

  hkvAlignedBBox bbox;
  pMesh->GetCollisionBoundingBox(bbox);

  HkSphereGeometry sphereGeo;
  sphereGeo.m_Radius = (bbox.getSizeX()+bbox.getSizeY()+bbox.getSizeZ())/(3.f*2.f) * rfScale;

  // Get automatic offset
  Shape_PivotOffset = Shape_PivotOffsetAutoAlign 
    ? vHavokShapeFactory::GetPivotOffset(pMesh, fScale) 
    : Shape_PivotOffset;

  // Initialize the rigid body
  InitSphereRb(sphereGeo, initTempl);
}

void vHavokRigidBody::InitSphereRb(HkSphereGeometry const& geometry, vHavokRigidBody::InitTemplate &initTempl)
{
  // Fill the construction info based on the passed template
  hkpRigidBodyCinfo cInfo;
  FillConstructionInfo(cInfo, initTempl);

  HkSphereGeometry geometry0 = geometry;

  // ensure valid radius
  if (geometry0.m_Radius < HKVMATH_LARGE_EPSILON)
  {
    geometry0.m_Radius = HKVMATH_LARGE_EPSILON;
    Vision::Error.Warning("vHavokRigidBody: Sphere-radius below minimum tolerance, clamped to %.4f", HKVMATH_LARGE_EPSILON);
  }

  // Create the sphere shape
  cInfo.m_shape = new hkpSphereShape(geometry0.m_Radius);

  // Compute the sphere inertia tensor (if required)
  hkpMassProperties massProperties;
  switch (Havok_InertiaTensorComputeMode)
  {
  case InertiaTensorComputeMode_NONE:
    break;

  case InertiaTensorComputeMode_SURFACE:
    {
      float fAdjustedThickness = Havok_SurfaceThickness;
      if(fAdjustedThickness >= geometry0.m_Radius)
      {
        fAdjustedThickness = float(geometry0.m_Radius) * 0.99f;
        Vision::Error.Warning("vHavokRigidBody: Surface thickness %f is greater than or equal to radius %f, clamped to %f.", Havok_SurfaceThickness, float(geometry0.m_Radius), fAdjustedThickness);
      }
      hkpInertiaTensorComputer::computeSphereSurfaceMassProperties(
        geometry0.m_Radius, Havok_Mass, fAdjustedThickness,
        massProperties);
      break;
    }

  case InertiaTensorComputeMode_VOLUME:
    hkpInertiaTensorComputer::computeSphereVolumeMassProperties(
      geometry0.m_Radius, Havok_Mass, massProperties);
    break;
  }

  // Create the rigid body
  InitCustomRb(cInfo, massProperties);
  // Remove reference
  cInfo.m_shape->removeReference();
  //Set shape type
  Shape_Type = ShapeType_SPHERE;
}

void vHavokRigidBody::AdjustVertexesForCapsuleSemispheres(hkvVec3& vA, hkvVec3& vB, float fSphereRadius)
{
  hkvVec3 vDir = (vA - vB);
  float fHeight = vDir.getLength();
  vDir.normalizeIfNotZero();

  float fAdjustment = hkvMath::Min(fSphereRadius, (fHeight * 0.5f) - 0.0001f);
  vA -= vDir * fAdjustment;
  vB += vDir * fAdjustment;
}

void vHavokRigidBody::InitCapsuleRb(const hkvVec3& vVertexA, const hkvVec3& vVertexB, float fRadius, float fScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Compute the capsule geometry for Havok
  hkvVec3 vScaledVertexA(vVertexA);
  hkvVec3 vScaledVertexB(vVertexB);

  AdjustVertexesForCapsuleSemispheres(vScaledVertexA, vScaledVertexB, fRadius);

  HkCapsuleGeometry capsuleGeo;
  vHavokConversionUtils::VisVecToPhysVecLocal((vScaledVertexA * fScale),capsuleGeo.m_StartAxis);
  vHavokConversionUtils::VisVecToPhysVecLocal((vScaledVertexB * fScale),capsuleGeo.m_EndAxis);
  capsuleGeo.m_Radius = VIS2HK_FLOAT_SCALED((fRadius * fScale));

  // Initialize the rigid body
  InitCapsuleRb(capsuleGeo, initTempl);
}

void vHavokRigidBody::InitCapsuleRb(const VDynamicMesh *pMesh, float fScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Compute the capsule geometry for Havok
  VASSERT(pMesh);
  hkvAlignedBBox bbox;
  pMesh->GetCollisionBoundingBox(bbox);

  hkvVec3 vVertexA = hkvVec3(bbox.getCenter() + hkvVec3(0.0f, 0.0f, bbox.getSizeZ() * 0.5f));
  hkvVec3 vVertexB = hkvVec3(bbox.getCenter() - hkvVec3(0.0f, 0.0f, bbox.getSizeZ() * 0.5f));
  float fRadius = ((bbox.getSizeX()+bbox.getSizeY()) * 0.25f);

  AdjustVertexesForCapsuleSemispheres(vVertexA, vVertexB, fRadius);

  HkCapsuleGeometry capsuleGeo;
  vHavokConversionUtils::VisVecToPhysVecLocal((vVertexA * fScale),capsuleGeo.m_StartAxis);
  vHavokConversionUtils::VisVecToPhysVecLocal((vVertexB * fScale),capsuleGeo.m_EndAxis);
  capsuleGeo.m_Radius = VIS2HK_FLOAT_SCALED((fRadius * fScale));

  // Get center of mass from objects center not pivot
  m_vCenterOfMassOffset = vHavokShapeFactory::GetPivotOffset(pMesh, fScale);
  // Get automatic offset
  Shape_PivotOffset.setZero();

  // Initialize the rigid body
  InitCapsuleRb(capsuleGeo, initTempl);
}

void vHavokRigidBody::InitCapsuleRb(HkCapsuleGeometry const& geometry, vHavokRigidBody::InitTemplate &initTempl)
{
  // Fill the construction info based on the passed template
  hkpRigidBodyCinfo cInfo;
  FillConstructionInfo(cInfo, initTempl);

  HkCapsuleGeometry geometry0 = geometry;

  // ensure valid height and radius
  hkVector4 vd; 
  vd.setSub(geometry0.m_StartAxis, geometry0.m_EndAxis);
  if (vd.lengthSquared<3>() < hkSimdReal::fromFloat(HKVMATH_LARGE_EPSILON*HKVMATH_LARGE_EPSILON))
  {
	  geometry0.m_StartAxis.setAddMul(geometry0.m_EndAxis, hkVector4::getConstant<HK_QUADREAL_0010>(), hkSimdReal::fromFloat(HKVMATH_LARGE_EPSILON));
    Vision::Error.Warning("vHavokRigidBody: Capsule-height below tolerance, clamped to %.4f", HKVMATH_LARGE_EPSILON);
  } 
  if (geometry0.m_Radius < HKVMATH_LARGE_EPSILON)
  {
    geometry0.m_Radius = HKVMATH_LARGE_EPSILON;
    Vision::Error.Warning("vHavokRigidBody: Capsule-radius below tolerance, clamped to %.4f", HKVMATH_LARGE_EPSILON);
  }

  // Create the capsule shape
  cInfo.m_shape = new hkpCapsuleShape(
    geometry0.m_StartAxis, geometry0.m_EndAxis, geometry0.m_Radius);

  // Compute the capsule inertia tensor (if required)
  hkpMassProperties massProperties;
  switch (Havok_InertiaTensorComputeMode)
  {
  case InertiaTensorComputeMode_NONE:
    break;

  case InertiaTensorComputeMode_SURFACE:
    Vision::Error.Warning("Surface inertia tensor computation is not supported for capsule shapes!");
    Havok_InertiaTensorComputeMode = InertiaTensorComputeMode_NONE;
    break;

  case InertiaTensorComputeMode_VOLUME:
    hkpInertiaTensorComputer::computeCapsuleVolumeMassProperties(
      geometry0.m_StartAxis, geometry0.m_EndAxis, geometry0.m_Radius,
      Havok_Mass, massProperties);
    break;
  }

  // Create the rigid body
  InitCustomRb(cInfo, massProperties);
  // Remove reference
  cInfo.m_shape->removeReference();
  //Set shape type
  Shape_Type = ShapeType_CAPSULE;
}


void vHavokRigidBody::InitCylinderRb(const hkvVec3& vVertexA, const hkvVec3& vVertexB, float fRadius, float fScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Compute the cylinder geometry for Havok
  HkCylinderGeometry cylinderGeo;
  vHavokConversionUtils::VisVecToPhysVecLocal((vVertexA * fScale),cylinderGeo.m_StartAxis);
  vHavokConversionUtils::VisVecToPhysVecLocal((vVertexB * fScale),cylinderGeo.m_EndAxis);
  cylinderGeo.m_Radius = VIS2HK_FLOAT_SCALED((fRadius * fScale));

  // Initialize the rigid body
  InitCylinderRb(cylinderGeo, initTempl);
}

void vHavokRigidBody::InitCylinderRb(const VDynamicMesh *pMesh, float fScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Compute the cylinder geometry for Havok
  VASSERT(pMesh);
  hkvAlignedBBox bbox;
  pMesh->GetCollisionBoundingBox(bbox);

  float fRadius = (bbox.getSizeX()+bbox.getSizeY()) * 0.25f;
  const hkvVec3 vExtentZ = hkvVec3(0.0f, 0.0f, bbox.getSizeZ() * 0.5f);

  HkCylinderGeometry cylinderGeo;
  vHavokConversionUtils::VisVecToPhysVecLocal((hkvVec3(bbox.getCenter() - vExtentZ) * fScale),cylinderGeo.m_StartAxis);
  vHavokConversionUtils::VisVecToPhysVecLocal((hkvVec3(bbox.getCenter() + vExtentZ) * fScale),cylinderGeo.m_EndAxis);
  
  cylinderGeo.m_Radius = VIS2HK_FLOAT_SCALED(fRadius * fScale);

  // Get center of mass from objects center not pivot
  m_vCenterOfMassOffset = vHavokShapeFactory::GetPivotOffset(pMesh, fScale);
  // Get automatic offset
  Shape_PivotOffset.setZero();

  // Initialize the rigid body
  InitCylinderRb(cylinderGeo, initTempl);
}

void vHavokRigidBody::InitCylinderRb(HkCylinderGeometry const& geometry, vHavokRigidBody::InitTemplate &initTempl)
{
  // Fill the construction info based on the passed template
  hkpRigidBodyCinfo cInfo;
  FillConstructionInfo(cInfo, initTempl);

  HkCylinderGeometry geometry0 = geometry;

  if (Havok_TightFit)
  {
	  hkVector4 r; r.set(hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius);
	  hkVector4 a; a.setSub4( geometry0.m_EndAxis, geometry0.m_StartAxis ); a.normalize3();
	  a.mul(r);
	  geometry0.m_StartAxis.add4(a);
	  geometry0.m_EndAxis.sub4(a);
  }

  // ensure valid height and radius
  hkVector4 vd; 
  vd.setSub(geometry0.m_StartAxis, geometry0.m_EndAxis);
  if (vd.lengthSquared<3>() < hkSimdReal::fromFloat(HKVMATH_LARGE_EPSILON*HKVMATH_LARGE_EPSILON))
  {
	  geometry0.m_StartAxis.setAddMul(geometry0.m_EndAxis, hkVector4::getConstant<HK_QUADREAL_0010>(), hkSimdReal::fromFloat(HKVMATH_LARGE_EPSILON));
    Vision::Error.Warning("vHavokRigidBody: Cylinder-height below tolerance, clamped to %.4f", HKVMATH_LARGE_EPSILON);
  }
  if (geometry0.m_Radius < HKVMATH_LARGE_EPSILON)
  {
    geometry0.m_Radius = HKVMATH_LARGE_EPSILON;
    Vision::Error.Warning("vHavokRigidBody: Cylinder-radius below tolerance, clamped to %.4f", HKVMATH_LARGE_EPSILON);
  }



  // Create the cylinder shape
  cInfo.m_shape = new hkpCylinderShape(
    geometry0.m_StartAxis, geometry0.m_EndAxis, geometry0.m_Radius);

  // Compute the cylinder inertia tensor (if required)
  hkpMassProperties massProperties;
  switch (Havok_InertiaTensorComputeMode)
  {
  case InertiaTensorComputeMode_NONE:
    break;

  case InertiaTensorComputeMode_SURFACE:
    Vision::Error.Warning("Surface inertia tensor computation is not supported for cylinder shapes!");
    Havok_InertiaTensorComputeMode = InertiaTensorComputeMode_NONE;
    break;

  case InertiaTensorComputeMode_VOLUME:
    hkpInertiaTensorComputer::computeCylinderVolumeMassProperties(
      geometry0.m_StartAxis, geometry0.m_EndAxis, geometry0.m_Radius,
      Havok_Mass, massProperties);
    break;
  }

  // Create the rigid body
  InitCustomRb(cInfo, massProperties);
  // Remove reference
  cInfo.m_shape->removeReference();
  //Set shape type
  Shape_Type = ShapeType_CYLINDER;
}

void vHavokRigidBody::InitConvexRb(VDynamicMesh *pMesh, const hkvVec3& vScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Fill the construction info based on the passed template
  hkpRigidBodyCinfo cInfo;
  FillConstructionInfo(cInfo, initTempl);

  // Create convex shape
  int flags = vHavokShapeFactory::VShapeCreationFlags_CACHE_SHAPE | vHavokShapeFactory::VShapeCreationFlags_USE_VCOLMESH;
  if (Havok_TightFit)
  {
	  flags |= vHavokShapeFactory::VShapeCreationFlags_SHRINK;
  }
  
  hkRefPtr<hkpShape> spShape = vHavokShapeFactory::CreateConvexHullShapeFromMesh(pMesh, vScale, &m_szShapeCacheId, flags);
  cInfo.m_shape = spShape;

  // Compute the shape's inertia tensor (if required)
  hkpMassProperties massProperties;
  switch (Havok_InertiaTensorComputeMode)
  {
  case InertiaTensorComputeMode_NONE:
    break;

  case InertiaTensorComputeMode_SURFACE:
    Vision::Error.Warning("Surface inertia tensor computation is not supported for convex shapes!");
    Havok_InertiaTensorComputeMode = InertiaTensorComputeMode_NONE;
    break;

  case InertiaTensorComputeMode_VOLUME:
    hkpInertiaTensorComputer::computeShapeVolumeMassProperties(
      cInfo.m_shape, Havok_Mass, massProperties);
    break;
  }

  // Get center of mass from objects center not pivot
  m_vCenterOfMassOffset = vHavokShapeFactory::GetPivotOffset(pMesh, vScale);
  // Get automatic offset
  Shape_PivotOffset.setZero();

  // Create the rigid body
  InitCustomRb(cInfo, massProperties);
  //Set shape type
  Shape_Type = ShapeType_CONVEX;
}

void vHavokRigidBody::InitMeshRb(VDynamicMesh* pMesh, const hkvVec3& vScale, vHavokRigidBody::InitTemplate &initTempl)
{
  // Fill the construction info based on the passed template
  hkpRigidBodyCinfo cInfo;
  FillConstructionInfo(cInfo, initTempl);

  // Create mesh shape
  const int iCreationFlags = vHavokShapeFactory::VShapeCreationFlags_CACHE_SHAPE | vHavokShapeFactory::VShapeCreationFlags_USE_VCOLMESH;
  hkRefPtr<hkpShape> spShape = vHavokShapeFactory::CreateShapeFromMesh(pMesh, vScale, &m_szShapeCacheId, iCreationFlags, (VisWeldingType_e)Havok_WeldingType);
  cInfo.m_shape = spShape;

  // Compute the shape's inertia tensor (if required)
  hkpMassProperties massProperties;
  switch (Havok_InertiaTensorComputeMode)
  {
  case InertiaTensorComputeMode_NONE:
    break;

  case InertiaTensorComputeMode_SURFACE:
    Vision::Error.Warning("Surface inertia tensor computation is not supported for mesh shapes!");
    Havok_InertiaTensorComputeMode = InertiaTensorComputeMode_NONE;
    break;

  case InertiaTensorComputeMode_VOLUME:
    hkpInertiaTensorComputer::computeShapeVolumeMassProperties(
      cInfo.m_shape, Havok_Mass, massProperties);
    break;
  }

  // Get center of mass from objects center not pivot
  m_vCenterOfMassOffset = vHavokShapeFactory::GetPivotOffset(pMesh, vScale);
  // Get automatic offset
  Shape_PivotOffset.setZero();

  // Create the rigid body
  InitCustomRb(cInfo, massProperties);
  //Set shape type
  Shape_Type = ShapeType_MESH;
}

bool vHavokRigidBody::InitRbFromFile(const char* filename, BOOL takeRbDataFromFile, float scale, 
                                     vHavokRigidBody::InitTemplate *initTempl)
{
  VVERIFY_OR_RET_VAL(m_pModule != NULL, false);

  // Remove the old instance, if available
  if (m_spRigidBody != NULL)
  {
    ASSERT(m_pOwner);       /// If there's no owner then the refcount will get 0 => crash
    RemoveHkRigidBody();
  }

  if (filename == NULL || *filename == '\0')
    return false;

  hkRefPtr<hkpShape> shape;

  char szShapeId[512];
  sprintf(szShapeId, "%s_FromFile_%.2f", filename, scale);

  shape = vHavokShapeFactory::FindShape(szShapeId, &m_szShapeCacheId);

  hkRefPtr<hkpRigidBody> spRbData;

  // We're not currently caching rb properties, so we always need to load the file if they are requested
  if(takeRbDataFromFile || !shape)
  {
    // TODO: implement this in vHavokStreamReader class
    hkRefPtr<vHavokStreamReader> spReader = hkRefNew<vHavokStreamReader>(new vHavokStreamReader(filename));
    if (!spReader->isOk())
    {
      Vision::Error.Warning("Could not open file: [%s]", filename);
      return false;
    }

    hkRefPtr<hkBufferedStreamReader> spHavokReader = hkRefNew<hkBufferedStreamReader>(new hkBufferedStreamReader(spReader));

    // Load on heap. Destruction holds extra references to rigid bodies, we don't want those to go away!
    hkSerializeUtil::ErrorDetails errRes;
    hkRefPtr<hkObjectResource> spResource = hkRefNew<hkObjectResource>(hkSerializeUtil::loadOnHeap(spHavokReader, &errRes));
    hkRootLevelContainer* root = spResource->getContents<hkRootLevelContainer>();

    if (!root)
    {
      Vision::Error.Warning("Could not load Havok data from file: [%s]: %s", filename, errRes.defaultMessage.cString());
      return false;
    }

    hkpPhysicsData* physicsData = static_cast<hkpPhysicsData*>( root->findObjectByType( hkpPhysicsDataClass.getName() ) );
    spRbData = static_cast<hkpRigidBody*>( root->findObjectByType( hkpRigidBodyClass.getName() ) );

    // If no rigid body is found directly, try to get it via PhysicsData
    if (physicsData && !spRbData)
    {
      const hkArray<hkpPhysicsSystem*>& systems = physicsData->getPhysicsSystems();
      if ((systems.getSize() > 0) && (systems[0]->getRigidBodies().getSize() > 0))
      {
        spRbData = systems[0]->getRigidBodies()[0];
        //XX could use Live obj load here to avoid keeping rest of file data around (Havok 7.0 serialization feature)
      }
    }

    if (!spRbData)
    {
      Vision::Error.Warning("Could not find a usable Havok rigid body in file: [%s]", filename);
      return false;
    }

    if(!shape)
    {
      hkpShape* fileShape = const_cast<hkpShape*>( spRbData->getCollidable()->getShape() ); 
      if ( hkMath::fabs( 1.0f - scale ) > HK_REAL_EPSILON )
      {
        shape = hkpShapeScalingUtility::scaleShape( fileShape, scale );
      }
      else
      {
        shape = fileShape;
      }

      m_szShapeCacheId = vHavokShapeFactory::AddShape(szShapeId, shape);
    }
  }

  // Get automatic offset
  Shape_PivotOffset.setZero();

  if (takeRbDataFromFile)
  {
    VASSERT(m_spRigidBody == NULL);
    m_spRigidBody = spRbData;
    m_spRigidBody->setShape(shape);

    // Set user data to identify this Havok component during collision detection
    m_spRigidBody->setUserData((hkUlong)vHavokUserDataPointerPair_t::CombineTypeAndPointer(this, V_USERDATA_OBJECT));

    // If we already have an owner, then add the rigid body to the Havok world
    if (GetOwner() != NULL)
    {
      UpdateVision2Havok();
      if (Havok_Active && !m_bAddedToWorld)
      {
        m_pModule->AddRigidBody(this);
        m_bAddedToWorld = true;
      }
    }
  }
  else
  {
    // XX live load here would mean that we could just keep the shape in mem.
    //    As it is here, we will still have the rb etc in mem, just unused (but the shape would be most of the mem anyway)
    hkpRigidBodyCinfo cInfo;
    if (initTempl)
    {
      FillConstructionInfo(cInfo, *initTempl);
    }
    cInfo.m_shape = shape;

    // Compute the shape's inertia tensor (if required)
    hkpMassProperties massProperties;
    switch (Havok_InertiaTensorComputeMode)
    {
    case InertiaTensorComputeMode_NONE:
      break;

    case InertiaTensorComputeMode_SURFACE:
      Vision::Error.Warning("Surface inertia tensor computation is not supported for file-based shapes!");
      Havok_InertiaTensorComputeMode = InertiaTensorComputeMode_NONE;
      break;

    case InertiaTensorComputeMode_VOLUME:
      hkpInertiaTensorComputer::computeShapeVolumeMassProperties(
        cInfo.m_shape, Havok_Mass, massProperties);
      break;
    }

    InitCustomRb(cInfo, massProperties);
  }


  // Set shape type
  Shape_Type = ShapeType_FILE;
  return true;
}

void vHavokRigidBody::FillConstructionInfo(hkpRigidBodyCinfo &cInfo, const vHavokRigidBody::InitTemplate &initTempl)
{
  // Fill the construction info based on the passed template
  cInfo.m_motionType = initTempl.m_motionType;
  cInfo.m_qualityType = initTempl.m_qualityType;
  if (!hkvMath::isFloatEqual (initTempl.m_restitution, -1.0f))
    cInfo.m_restitution = initTempl.m_restitution;
}

bool vHavokRigidBody::CreateHkRigidBody(hkpRigidBodyCinfo &cInfo,
  hkpMassProperties const& massProperties)
{
  VVERIFY_OR_RET_VAL(m_pModule != NULL, false);

  // Remove the old instance, if available
  if (m_spRigidBody != NULL)
  {
    ASSERT(m_pOwner);       /// If there's no owner then the refcount will get 0 => crash
    RemoveHkRigidBody();
  }

  // Set remaining construction properties
  vHavokConversionUtils::VisVecToPhysVecLocal(Shape_PivotOffset,cInfo.m_position);
  hkvVec3 finalCenterOfMass = Shape_CenterOfMass + m_vCenterOfMassOffset;
  vHavokConversionUtils::VisVecToPhysVecLocal(finalCenterOfMass,cInfo.m_centerOfMass);

  if (Havok_NoDeactivate)
  {
    // Never deactivate the solver
    cInfo.m_solverDeactivation = hkpRigidBodyCinfo::SOLVER_DEACTIVATION_OFF;

    // And, most important, never deactivate the rigid body itself.
    cInfo.m_enableDeactivation = false;
  }

  // Set the mass properties if they have been computed earlier. Otherwise just
  // set the mass.
  if (Havok_InertiaTensorComputeMode != InertiaTensorComputeMode_NONE)
  {
    cInfo.m_inertiaTensor = massProperties.m_inertiaTensor;
    cInfo.m_mass = massProperties.m_mass;
  }
  else
  {
    cInfo.m_mass = Havok_Mass;
  }

  // Scale the inertia tensor with the given values
  cInfo.m_inertiaTensor.mul(hkSimdReal::fromFloat(Havok_InertiaTensorScaling));
  hkVector4 diag;
  hkMatrix3Util::_getDiagonal(cInfo.m_inertiaTensor, diag);
  hkVector4 diagScale; diagScale.load<3,HK_IO_NATIVE_ALIGNED>(&Havok_InertiaTensorAxisScaling[0]);
  diag.mul(diagScale);
  hkMatrix3Util::_setDiagonalOnly(diag, cInfo.m_inertiaTensor);

  cInfo.m_friction = Havok_Friction;
  cInfo.m_restitution = Havok_Restitution;

  cInfo.m_linearDamping = Havok_LinearDamping;
  cInfo.m_angularDamping = Havok_AngularDamping;

  cInfo.m_collisionFilterInfo = hkpGroupFilter::calcFilterInfo(
    Havok_CollisionLayer, Havok_CollisionGroup, 
    Havok_SubSystemId, Havok_SubSystemDontCollideWith);

  // Compute the rigid body inertia.
  //cInfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;
  //hkpInertiaTensorComputer::setShapeVolumeMassProperties( cInfo.m_shape, 100.0f, cInfo );

  // Create the rigid body (with ref count of 1)
  m_spRigidBody = hkRefNew<hkpRigidBody>(new hkpRigidBody(cInfo));

  // Set user data to identify this Havok component during collision detection
  m_spRigidBody->setUserData((hkUlong)vHavokUserDataPointerPair_t::CombineTypeAndPointer(this, V_USERDATA_OBJECT));

  // If we already have an owner, then add the rigid body to the Havok world
  if (GetOwner() != NULL)
  {
    UpdateVision2Havok();
    if (Havok_Active && !m_bAddedToWorld)
    {
      m_pModule->AddRigidBody(this);
      m_bAddedToWorld = true;
    }
  }

  return true;
}

void vHavokRigidBody::RemoveHkRigidBody()
{
  if (!m_spRigidBody)
    return;

  // If we still have an owner, then remove the rigid body from the Havok world
  if (GetOwner() != NULL && m_bAddedToWorld)
  {
    m_pModule->RemoveRigidBody(this);
    m_bAddedToWorld = false;
  }

  m_spRigidBody = NULL;

  // Remove shape from cache
  vHavokShapeFactory::RemoveShape(m_szShapeCacheId);
  m_szShapeCacheId = NULL;
}


// -------------------------------------------------------------------------- //
// Access Members                                                             //
// -------------------------------------------------------------------------- //

const hkpShape *vHavokRigidBody::GetHkShape() const
{
  if (m_spRigidBody == NULL) 
    return NULL;

  hkpRigidBodyCinfo cInfo;
  m_spRigidBody->getCinfo(cInfo);

  return cInfo.m_shape;
}

VisObject3D_cl *vHavokRigidBody::GetOwner3D()
{
  VisTypedEngineObject_cl *pOwner = GetOwner();
  if (pOwner == NULL)
    return NULL;
  
  VASSERT(pOwner->IsOfType(V_RUNTIME_CLASS(VisObject3D_cl)));
  return (VisObject3D_cl *) pOwner; 
}

VisBaseEntity_cl *vHavokRigidBody::GetOwnerEntity()
{
  VisTypedEngineObject_cl *pOwner = GetOwner();
  if (!pOwner || !pOwner->IsOfType(V_RUNTIME_CLASS(VisBaseEntity_cl)))
    return NULL;

  return static_cast<VisBaseEntity_cl*>(pOwner); 
}


// -------------------------------------------------------------------------- //
// Synchronization                                                            //
// -------------------------------------------------------------------------- //

void vHavokRigidBody::Step(float dt)
{
  // Only keyframed objects need to be processed here.
  if ((Havok_MotionType != MotionType_KEYFRAMED) ||
    (!GetOwner()) || (!vHavokPhysicsModule::GetInstance()) ||
    (!vHavokPhysicsModule::GetInstance()->GetPhysicsWorld()) ||
    (!m_spRigidBody))
  {
    return;
  }

  // Return, if owner is not an entity (in case of mirrors).
  if(!GetOwner()->IsOfType(V_RUNTIME_CLASS(VisBaseEntity_cl)))
    return;

  VisBaseEntity_cl *pOwnerEntity = static_cast<VisBaseEntity_cl*>(GetOwner());

  // Store motion delta state before clearing the motion delta on the entity.
  const bool bHasLocalPositionDelta = !pOwnerEntity->GetMotionDeltaLocalSpace().isZero();
  const bool bHasWorldPositionDelta = !pOwnerEntity->GetMotionDeltaWorldSpace().isZero();
  
  // Do rotation delta before offset delta so we can use it to transform the local motion delta.
  hkQuaternion hkTargetOrientation; 
  
  // Process rotation delta.
  // Retrieve rotation from entity in order to also apply delta for modifications that were directly set on the entity.
  hkvVec3 vEntityOrientation = pOwnerEntity->GetOrientation() + pOwnerEntity->GetRotationDelta();
  hkvQuat qEntityOrientation;
  qEntityOrientation.setFromEulerAngles(vEntityOrientation.z, vEntityOrientation.y, vEntityOrientation.x);

  vHavokConversionUtils::VisQuatToHkQuat(qEntityOrientation, hkTargetOrientation);

  // Clear the entity's rotation delta.
  pOwnerEntity->ResetRotationDelta();

  // Motion delta
  hkVector4 hkMotionDelta; 
  hkMotionDelta.setZero();

  // Local motion delta
  if (bHasLocalPositionDelta)
  {
	  // Assign the animation MotionDelta 
	  vHavokConversionUtils::VisVecToPhysVecLocal(pOwnerEntity->GetMotionDeltaLocalSpace(), hkMotionDelta);

	  // Get rotation matrix for world space transformation and transform 
	  // the motion delta
	  hkMotionDelta._setRotatedDir(hkTargetOrientation, hkMotionDelta);  
  }
  
  // World motion delta. Needs to be processed after the local delta.
  if (bHasWorldPositionDelta)
  {
	  hkVector4 hkWorldDelta;
	  vHavokConversionUtils::VisVecToPhysVecLocal(pOwnerEntity->GetMotionDeltaWorldSpace(), hkWorldDelta);
	  hkMotionDelta.add(hkWorldDelta);
  }

  // Clear the entity's motion delta.
  pOwnerEntity->ResetMotionDelta();

  // Lock the world and apply new position and rotation.
  vHavokPhysicsModule* pModule = vHavokPhysicsModule::GetInstance();
  hkpWorld* pWorld = pModule->GetPhysicsWorld();

  pWorld->lock();

  // This can cause precision loss for far away objects. 
  // Retrieve position from entity (instead of the rigid body) in order to also apply delta 
  // for modifications that were directly set on the object3d.
  hkVector4 hkTargetPos;
  vHavokConversionUtils::VisVecToPhysVecWorld(GetOwner3D()->GetPosition(), hkTargetPos);
  hkTargetPos.add(hkMotionDelta);
	  
  if (hkTargetPos.distanceTo(m_spRigidBody->getPosition()) > hkSimdReal::fromFloat(Havok_MaxKinematicVelocity * dt))
  {
    m_spRigidBody->setRotation(hkTargetOrientation);
    m_spRigidBody->setPosition(hkTargetPos);
  }
  else
  {
    // Use velocity here to get stable collisions.
    hkReal invDeltaTime = 1.0f / hkvMath::Max(dt, 0.0001f);

    hkpKeyFrameUtility::applyHardKeyFrame(hkTargetPos, hkTargetOrientation, invDeltaTime, m_spRigidBody);
  }

  pWorld->unlock();
}

void vHavokRigidBody::UpdateOwner()
{
  // No need to update inactive objects. This is *very* important for performance, since it avoids unnecessary state updates
  // inside the Vision Engine.
  if (m_bAddedToWorld && (m_spRigidBody->isActive()))
    UpdateHavok2Vision();
}

void vHavokRigidBody::UpdateHavok2Vision()
{
  VisObject3D_cl *pOwner3d = GetOwner3D();
  VVERIFY_OR_RET(m_spRigidBody && GetOwner());

  // Get the transformation from Havok
  hkTransform hkTf = m_spRigidBody->getTransform();

  hkVector4 vOffset; 
  vHavokConversionUtils::VisVecToPhysVecLocal(Shape_PivotOffset, vOffset);
  vOffset._setRotatedDir(hkTf.getRotation(), vOffset);
  hkTf.getTranslation().sub(vOffset);

  hkvVec3 vNewPos;
  hkvMat3 vNewRot;
  vHavokConversionUtils::PhysTransformToVisMatVecWorld(hkTf, vNewRot, vNewPos);

  // Set the transformation in Vision
  pOwner3d->SetPosition(vNewPos);
  pOwner3d->SetRotationMatrix(vNewRot);
}

void vHavokRigidBody::UpdateVision2Havok()
{
  hkpWorld* world = vHavokPhysicsModule::GetInstance()? vHavokPhysicsModule::GetInstance()->GetPhysicsWorld() : HK_NULL;
  if (!world)
    return;

  VisObject3D_cl *pOwner3d = GetOwner3D();
  VVERIFY_OR_RET(m_spRigidBody && pOwner3d);

  // Get the transformation from Vision
  const hkvVec3& vPos = pOwner3d->GetPosition();
  const hkvMat3& vRot = pOwner3d->GetRotationMatrix();
  hkTransform hkTfOut;
  vHavokConversionUtils::VisMatVecToPhysTransformWorld(vRot, vPos, hkTfOut);

  hkVector4 vOffset; vHavokConversionUtils::VisVecToPhysVecLocal(Shape_PivotOffset, vOffset);
  vOffset._setRotatedDir(hkTfOut.getRotation(), vOffset);
  hkTfOut.getTranslation().add(vOffset);

  // Set the transformation in Havok
  m_pModule->MarkForWrite();
  m_spRigidBody->setTransform(hkTfOut);
  m_pModule->UnmarkForWrite();
}

// -------------------------------------------------------------------------- //
// Velocity & Property Wrapper Functions                                      //
// -------------------------------------------------------------------------- //

void vHavokRigidBody::SetLinearDamping(float fDamping)
{
  VVERIFY_OR_RET(m_spRigidBody);

  m_pModule->MarkForWrite();
  m_spRigidBody->setLinearDamping(fDamping);
  m_pModule->UnmarkForWrite();

  Havok_LinearDamping = fDamping;
}

hkReal vHavokRigidBody::GetLinearDamping() const
{
  VVERIFY_OR_RET_VAL(m_spRigidBody, 0.f);
  return m_spRigidBody->getLinearDamping();
}

void vHavokRigidBody::SetDamageMultiplier(float fMultiplier)
{
	VVERIFY_OR_RET(m_spRigidBody);

	m_pModule->MarkForWrite();
	m_spRigidBody->m_damageMultiplier = fMultiplier;
	m_pModule->UnmarkForWrite();
}

hkReal vHavokRigidBody::GetDamageMultiplier() const
{
	VVERIFY_OR_RET_VAL(m_spRigidBody, 1.0f);
	return m_spRigidBody->m_damageMultiplier;
}

void vHavokRigidBody::SetAngularDamping(float fDamping)
{
  VVERIFY_OR_RET(m_spRigidBody);

  m_pModule->MarkForWrite();
  m_spRigidBody->setAngularDamping(fDamping);
  m_pModule->UnmarkForWrite();

  Havok_AngularDamping = fDamping;
}

hkReal vHavokRigidBody::GetAngularDamping() const
{
  VVERIFY_OR_RET_VAL(m_spRigidBody, 0.f);
  return m_spRigidBody->getAngularDamping();
}

void vHavokRigidBody::SetPosition(const hkvVec3& value)
{
  VVERIFY_OR_RET(m_spRigidBody);

  hkVector4 pos; vHavokConversionUtils::VisVecToPhysVecWorld(value,pos);
  m_pModule->MarkForWrite();
  m_spRigidBody->setPosition(pos);
  m_pModule->UnmarkForWrite();
}

void vHavokRigidBody::SetOrientation(const hkvVec3& value)
{
  VVERIFY_OR_RET(m_spRigidBody);

  hkvQuat quat;
  quat.setFromEulerAngles( value.x, value.y, value.z );

  hkQuaternion hkquat; vHavokConversionUtils::VisQuatToHkQuat( quat, hkquat );
  m_pModule->MarkForWrite();
  m_spRigidBody->setRotation(hkquat);
  m_pModule->UnmarkForWrite();
}

hkvVec3 vHavokRigidBody::GetPosition() const
{
  VVERIFY_OR_RET_VAL(m_spRigidBody, hkvVec3(hkvNoInitialization));

  hkvVec3 v; 
  m_pModule->MarkForRead();
  const hkVector4& pos = m_spRigidBody->getPosition();
  vHavokConversionUtils::PhysVecToVisVecWorld(pos,v);
  m_pModule->UnmarkForRead();
  return v;
}

void vHavokRigidBody::SetLinearVelocity(const hkvVec3& value)
{
  VVERIFY_OR_RET(m_spRigidBody);

  hkVector4 linearVelocity; vHavokConversionUtils::VisVecToPhysVecLocal(value,linearVelocity);
  m_pModule->MarkForWrite();
  m_spRigidBody->setLinearVelocity(linearVelocity);
  m_pModule->UnmarkForWrite();
}

hkvVec3 vHavokRigidBody::GetLinearVelocity() const
{
  VVERIFY_OR_RET_VAL(m_spRigidBody, hkvVec3(hkvNoInitialization));

  hkvVec3 v; 
  m_pModule->MarkForRead();
  const hkVector4& linearVelocity = m_spRigidBody->getLinearVelocity();
  vHavokConversionUtils::PhysVecToVisVecLocal(linearVelocity,v);
  m_pModule->UnmarkForRead();
  return v;
}

void vHavokRigidBody::SetAngularVelocity(const hkvVec3& value)
{
  VVERIFY_OR_RET(m_spRigidBody);

  hkVector4 angularVelocity; vHavokConversionUtils::VisVecToPhysVecLocal(value,angularVelocity);
  m_pModule->MarkForWrite();
  m_spRigidBody->setAngularVelocity(angularVelocity);
  m_pModule->UnmarkForWrite();
}

hkvVec3 vHavokRigidBody::GetAngularVelocity() const
{
  VVERIFY_OR_RET_VAL(m_spRigidBody, hkvVec3(hkvNoInitialization));

  hkvVec3 v; 
  m_pModule->MarkForRead();
  const hkVector4& angularVelocity = m_spRigidBody->getAngularVelocity();
  vHavokConversionUtils::PhysVecToVisVecLocal(angularVelocity,v);
  m_pModule->UnmarkForRead();
  return v;
}

void vHavokRigidBody::ApplyForce(const hkvVec3& value, float deltaT)
{
  VVERIFY_OR_RET(m_spRigidBody);

  hkVector4 force; vHavokConversionUtils::VisVecToPhysVecLocal(value,force);
  m_pModule->MarkForWrite();
  m_spRigidBody->applyForce(deltaT, force);
  m_pModule->UnmarkForWrite();
}

void vHavokRigidBody::ApplyLinearImpulse(const hkvVec3& value)
{
  VVERIFY_OR_RET(m_spRigidBody);

  hkVector4 impulse; vHavokConversionUtils::VisVecToPhysVecLocal(value,impulse);
  m_pModule->MarkForWrite();
  m_spRigidBody->applyLinearImpulse(impulse);
  m_pModule->UnmarkForWrite();
}

void vHavokRigidBody::SetMass(float fMass)
{
  VVERIFY_OR_RET(m_spRigidBody);

  // clamp mass to 1 gram
  if (fMass < HKVMATH_HUGE_EPSILON)
  {
    fMass = HKVMATH_HUGE_EPSILON;
    Vision::Error.Warning("vHavokRigidBody: Mass below minimum tolerance, clamped to %.4f", HKVMATH_HUGE_EPSILON);
  }

  m_pModule->MarkForWrite();
  m_spRigidBody->setMass(fMass);
  m_pModule->UnmarkForWrite();

  Havok_Mass = fMass;
}

void vHavokRigidBody::SetMotionType(hkpMotion::MotionType type)
{
  VVERIFY_OR_RET(m_spRigidBody);

  m_pModule->MarkForWrite();
  m_spRigidBody->setMotionType(type);
  m_pModule->UnmarkForWrite();

  MotionType_e visionMotionType = MotionType_DYNAMIC; //initialize with default value
  switch (type)
  {
    case hkpMotion::MOTION_DYNAMIC: 
      visionMotionType = MotionType_DYNAMIC;
      break;
    case hkpMotion::MOTION_SPHERE_INERTIA: 
      visionMotionType = MotionType_SPHERE_INERTIA;
      break;
    case hkpMotion::MOTION_BOX_INERTIA: 
      visionMotionType = MotionType_BOX_INERTIA;
      break;
    case hkpMotion::MOTION_KEYFRAMED: 
      visionMotionType = MotionType_KEYFRAMED;
      break;
    case hkpMotion::MOTION_FIXED: 
      visionMotionType = MotionType_FIXED;
      break;
    case hkpMotion::MOTION_THIN_BOX_INERTIA: 
      visionMotionType = MotionType_THIN_BOX_INERTIA;
      break;
    case hkpMotion::MOTION_CHARACTER:
      visionMotionType = MotionType_CHARACTER;
      break;
  }
  
  Havok_MotionType = visionMotionType;
}

void vHavokRigidBody::SetQualityType(hkpCollidableQualityType type)
{
  VVERIFY_OR_RET(m_spRigidBody);

  m_pModule->MarkForWrite();
  m_spRigidBody->setQualityType(type);
  m_pModule->UnmarkForWrite();

  QualityType_e visionQualityType = QualityType_AUTO; //initialize with default value
  switch (type)
  {
  case HK_COLLIDABLE_QUALITY_FIXED: 
    visionQualityType = QualityType_FIXED;
    break;
  case HK_COLLIDABLE_QUALITY_KEYFRAMED: 
    visionQualityType = QualityType_KEYFRAMED;
    break;
  case HK_COLLIDABLE_QUALITY_DEBRIS: 
    visionQualityType = QualityType_DEBRIS;
    break;
  case HK_COLLIDABLE_QUALITY_DEBRIS_SIMPLE_TOI: 
    visionQualityType = QualityType_DEBRIS_SIMPLE_TOI;
    break;
  case HK_COLLIDABLE_QUALITY_MOVING: 
    visionQualityType = QualityType_MOVING;
    break;
  case HK_COLLIDABLE_QUALITY_CRITICAL: 
    visionQualityType = QualityType_CRITICAL;
    break;
  case HK_COLLIDABLE_QUALITY_BULLET:
    visionQualityType = QualityType_BULLET;
    break;
  case HK_COLLIDABLE_QUALITY_CHARACTER:
    visionQualityType = QualityType_CHARACTER;
    break;
  case HK_COLLIDABLE_QUALITY_KEYFRAMED_REPORTING:
    visionQualityType = QualityType_KEYFRAMED_REPORTING;
    break;
  }

  Havok_QualityType = visionQualityType;
}

void vHavokRigidBody::SetRestitution(float fRestitution)
{
  VVERIFY_OR_RET(m_spRigidBody);

  m_pModule->MarkForWrite();
  m_spRigidBody->setRestitution(fRestitution);
  m_pModule->UnmarkForWrite();

  Havok_Restitution = fRestitution;
}

void vHavokRigidBody::SetFriction(float fFriction)
{
  VVERIFY_OR_RET(m_spRigidBody);

  m_pModule->MarkForWrite();
  m_spRigidBody->setFriction(fFriction);
  m_pModule->UnmarkForWrite();

  Havok_Friction = fFriction;
}

void vHavokRigidBody::SetCollisionInfo(int iLayer, int iGroup,
  int iSubsystem, int iSubsystemDontCollideWith)
{
  VVERIFY_OR_RET(m_spRigidBody);

  int iFilterInfo = hkpGroupFilter::calcFilterInfo(
    iLayer, iGroup, iSubsystem, iSubsystemDontCollideWith);

  m_pModule->MarkForWrite();
  m_spRigidBody->setCollisionFilterInfo(iFilterInfo);
  hkpWorld* w = m_spRigidBody->getWorld();
  if (w)
  {
    w->updateCollisionFilterOnEntity(m_spRigidBody, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_IGNORE_SHAPE_COLLECTIONS);
  }
  m_pModule->UnmarkForWrite();

  Havok_CollisionLayer = iLayer;
  Havok_CollisionGroup = iGroup;
  Havok_SubSystemId = iSubsystem;
  Havok_SubSystemDontCollideWith = iSubsystemDontCollideWith;
}

void vHavokRigidBody::SetAlwaysInitAsDynamic(bool bStatus)
{
  Havok_AlwaysInitAsDynamic = bStatus;
}

void vHavokRigidBody::SetActive(bool bStatus)
{
  Havok_Active = bStatus;

  if (Havok_Active && !m_bAddedToWorld)
  {
    // Add rigid body to world when it is active and currently not added
    VASSERT(m_pModule != NULL);
    m_pModule->AddRigidBody(this);
    m_bAddedToWorld = true;
  }
  else if (!Havok_Active && m_bAddedToWorld)
  {
    // Remove rigid body from world when it is not active and currently added
    VASSERT(m_pModule != NULL);
    m_pModule->RemoveRigidBody(this);
    m_bAddedToWorld = false;
  }
}

bool vHavokRigidBody::GetActive() const
{
  return Havok_Active;
}


// -------------------------------------------------------------------------- //
// Debug Rendering                                                            //
// -------------------------------------------------------------------------- //

void vHavokRigidBody::SetDebugRendering(BOOL bEnable)
{
  Debug_Render = bEnable;

  if (m_spRigidBody == NULL)
    return;

  vHavokPhysicsModule* pInstance = vHavokPhysicsModule::GetInstance();

  // Get ID (cast from collidable pointer as its is used for display geometry ID)
  hkpWorld* world = pInstance ? pInstance->GetPhysicsWorld() : HK_NULL;
  if (world) world->markForRead();

  const bool bEnabledDebug = Debug_Render || (pInstance ? pInstance->m_bDebugRenderRigidBodies : false);

  hkUlong id = (hkUlong)m_spRigidBody->getCollidable();

  if (world) world->unmarkForRead();

  // Ensure debug display handler is created
  if (bEnabledDebug)
    ((vHavokPhysicsModule*) Vision::GetApplication()->GetPhysicsModule())->SetEnabledDebug(true);

  // Set display properties
  vHavokDisplayHandler* pDisplay = ((vHavokPhysicsModule*) Vision::GetApplication()->GetPhysicsModule())->GetHavokDisplayHandler();
  if (!pDisplay)
    return;

  // Set debug color and visibility state
  pDisplay->SetVisible(id, bEnabledDebug);
  pDisplay->SetColor(id, Debug_Color);

  // In Editor attach the Havok Debug Shape to the owner object
  if (bEnabledDebug)
    pDisplay->SetOwner(id, GetOwner3D());
}

void vHavokRigidBody::SetDebugColor(VColorRef color)
{
  Debug_Color = color;

  if (m_spRigidBody == NULL)
    return;

  hkpWorld* world = vHavokPhysicsModule::GetInstance()? vHavokPhysicsModule::GetInstance()->GetPhysicsWorld() : HK_NULL;
  if (world) world->markForRead();

  // Get ID (cast from collidable pointer as its is used for display geometry ID)
  hkUlong id = (hkUlong)m_spRigidBody->getCollidable();

  if (world) world->unmarkForRead();

  vHavokDisplayHandler* pDisplay = ((vHavokPhysicsModule*) Vision::GetApplication()->GetPhysicsModule())->GetHavokDisplayHandler();
  if (!pDisplay)
    return;

  // Set debug color
  pDisplay->SetColor(id, Debug_Color);
}


// -------------------------------------------------------------------------- //
// IVObjectComponent Virtual Overrides                                        //
// -------------------------------------------------------------------------- //

BOOL vHavokRigidBody::CanAttachToObject(VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut)
{
  if (!IVObjectComponent::CanAttachToObject(pObject, sErrorMsgOut))
  {
    return FALSE;
  }

  // Object has to be derived from VisBaseEntity_cl or from VisMirror_cl
  bool bIsValidType = false;
  bIsValidType = (pObject->IsOfType(V_RUNTIME_CLASS(VisBaseEntity_cl)) == TRUE) || (pObject->IsOfType(V_RUNTIME_CLASS(VisMirror_cl)) == TRUE);

  if (!bIsValidType)
  {
    sErrorMsgOut = "Component can only be added to instances of VisBaseEntity_cl, VisMirror_cl or derived classes.";
    return FALSE;
  }

  // vHavokRigidBody component can not be attached to objects, that already have a vHavokCharacterController component attached
  if (pObject->Components().GetComponentOfType<vHavokCharacterController>())
    return FALSE;

  return TRUE;
}

void vHavokRigidBody::SetOwner(VisTypedEngineObject_cl *pOwner)
{
  IVObjectComponent::SetOwner(pOwner);

  // Do not initialize the component in case our module is not active
  if (!m_pModule)
  {
    Vision::Error.Warning("Failed to initialize vHavokRigidBody since a non Havok physics module is currently active");
    return;
  }

  // Add or remove from manager according to whether we have an owner or not
  if (pOwner)
  {
    if (Havok_MotionType == MotionType_KEYFRAMED)
    {
      // Euler angles need to be enabled for keyframed rotations.
      GetOwner3D()->SetUseEulerAngles(true);
    }
    else
    {
      // Use matrix directly instead of euler angles when doing real physics, as Euler
      // angles are slower and less accurate. Before, however, ensure that the object's
      // rotation matrix is current.
      GetOwner3D()->EnsureCachedRotationMatrixValid();
      GetOwner3D()->SetUseEulerAngles(false); 
    }

    // Add the rigid body to the Havok physics world
    if (m_spRigidBody)
    {
      // Set initial object position in Havok
      UpdateVision2Havok();

      if (Havok_Active && !m_bAddedToWorld)
      {
        m_pModule->AddRigidBody(this);
        m_bAddedToWorld = true;
      }
    }
    else
    {
      CommonInit();
    }
  }
  else
  {
    // Remove the rigid body from the Havok physics world
    if (m_spRigidBody && m_bAddedToWorld)
    {
      m_pModule->RemoveRigidBody(this);
      m_bAddedToWorld = false;
    }

    // Remove shape from cache
    vHavokShapeFactory::RemoveShape(m_szShapeCacheId);
    m_szShapeCacheId = NULL;
  }
}

void vHavokRigidBody::OnVariableValueChanged(VisVariable_cl *pVar, const char * value)
{
  // Do not touch the rigid body in case our module is not active
  if (!m_pModule)
    return;

  if (m_spRigidBody == NULL)
    return;

  CommonInit();
}

bool vHavokRigidBody::IsResourceRelevant(const VManagedResource* pResource) 
{
  if (pResource == NULL)
    return false;
  const VisBaseEntity_cl *pOwnerEntity = GetOwnerEntity();
  if (pOwnerEntity == NULL)
    return false;
  const VDynamicMesh *pMesh = pOwnerEntity->GetMesh();
  if (pMesh == NULL)
    return false;
  if (pMesh == pResource)
    return true;
  if (Shape_Type==ShapeType_MESH || Shape_Type==ShapeType_CONVEX)
  {
    if (pMesh->GetFilename() != NULL)
    {
      char szColMeshFile[FS_MAX_PATH];
      VFileHelper::AddExtension(szColMeshFile, pMesh->GetFilename(), "vcolmesh");
      if (strcmp(pResource->GetFilename(), szColMeshFile)==0) 
        return true;
    }
  }
  return false;
}

void vHavokRigidBody::MessageFunction( int iID, INT_PTR iParamA, INT_PTR iParamB )
{
  IVObjectComponent::MessageFunction(iID, iParamA, iParamB);

  // Do not touch the rigid body in case our module is not active
  if (!m_pModule)
    return;

  if ( iID == VIS_MSG_EDITOR_PROPERTYCHANGED )
  {
    const char *szPropertyName = (const char *) iParamA;

    // Recreate the Havok actor in case the model file gets changed within vForge
    if ( _stricmp(szPropertyName, "ModelFile") == 0 || _stricmp(szPropertyName, "Scaling") == 0 )
      CommonInit();

    if(m_spRigidBody)
      SetDebugRendering(Debug_Render);   

    return;
  }
  else if ( iID == VIS_MSG_HAVOK_ONCOLLISION )
  {
    vHavokPhysicsModule::TriggerCollisionScriptFunction(this, (vHavokCollisionInfo_t *)iParamA);
    return;
  }
  else if ( iID == VIS_MSG_ACTIVATE )
  {
    SetActive(iParamA != 0);
    return;
  }
  else if (iID == VIS_MSG_RESOURCE_BEFORE_FILEMODIFIED)
  {
    const VManagedResource* pResource = reinterpret_cast<VManagedResource*>(iParamA);
    if (IsResourceRelevant(pResource))
      CommonDeinit();
    return;
  }
  else if (iID == VIS_MSG_RESOURCE_AFTER_FILEMODIFIED)
  {
    const VManagedResource* pResource = reinterpret_cast<VManagedResource*>(iParamA);
    if (IsResourceRelevant(pResource))
    {
      CommonInit();
      if (m_spRigidBody)
        SetDebugRendering(Debug_Render);   
    }
    return;
  }
}

// -------------------------------------------------------------------------- //
// Serialization                                                              //
// -------------------------------------------------------------------------- //

void vHavokRigidBody::Serialize( VArchive &ar )
{
  char iLocalVersion = VHAVOKRIGIDBODY_VERSION_CURRENT;
  IVObjectComponent::Serialize(ar);
  if (ar.IsLoading())
  {
    char iFileVersion = 0;
    ar >> iFileVersion;
    VASSERT_MSG(iFileVersion <= iLocalVersion, "Invalid file version. Please re-export");

    // VHAVOKRIGIDBODY_VERSION_0
    ar >> Shape_Type;
    Shape_BoxSize.SerializeAsVisVector (ar);
    ar >> Shape_Radius;
    Shape_PivotOffset.SerializeAsVisVector (ar);
    Shape_CenterOfMass.SerializeAsVisVector (ar);
    ar >> Havok_Mass;
    ar >> Havok_Friction;
    ar >> Havok_Restitution;
    ar >> Havok_LinearDamping;
    ar >> Havok_AngularDamping;
    ar >> Debug_Color; 

    // VHAVOKRIGIDBODY_VERSION_1
    if (iFileVersion > 0)
    {
      ar >> Havok_FileResourceName;
    }

    // VHAVOKRIGIDBODY_VERSION_2
    if (iFileVersion > 1)
    {
      ar >> Havok_TakeRbDataFromFile;
      ar >> Shape_PivotOffsetAutoAlign;
      ar >> Havok_MotionType;
    }

    // VHAVOKRIGIDBODY_VERSION_3
    if (iFileVersion >= VHAVOKRIGIDBODY_VERSION_3)
    {
      ar >> Havok_CollisionLayer;
      ar >> Havok_CollisionGroup;
      ar >> Havok_SubSystemId;
      ar >> Havok_SubSystemDontCollideWith;

      m_vTempDeserializedLinearVel.SerializeAsVisVector (ar);
      m_vTempDeserializedAngularVel.SerializeAsVisVector (ar);
    }

    // VHAVOKRIGIDBODY_VERSION_4
    if (iFileVersion >= VHAVOKRIGIDBODY_VERSION_4)
    {
      ar >> Shape_Height;
    }

    // VHAVOKRIGIDBODY_VERSION_5
    if (iFileVersion >= VHAVOKRIGIDBODY_VERSION_5)
    {
      ar >> Havok_NoDeactivate;
      ar >> Havok_InertiaTensorComputeMode;
      ar >> Havok_SurfaceThickness;
      Havok_InertiaTensorAxisScaling.SerializeAsVisVector (ar);
      ar >> Havok_InertiaTensorScaling;
    }

    // VHAVOKRIGIDBODY_VERSION_6
    if (iFileVersion >= VHAVOKRIGIDBODY_VERSION_6)
    {
      ar >> Havok_QualityType;
    }

    // VHAVOKRIGIDBODY_VERSION_7
    if (iFileVersion >= VHAVOKRIGIDBODY_VERSION_7)
    {
      ar >> Havok_MaxKinematicVelocity;
    }

    // VHAVOKRIGIDBODY_VERSION_8
    if (iFileVersion >= VHAVOKRIGIDBODY_VERSION_8)
    {
      ar >> Havok_AlwaysInitAsDynamic;
    }

    // VHAVOKRIGIDBODY_VERSION_9
    if (iFileVersion >= VHAVOKRIGIDBODY_VERSION_9)
    {
      ar >> Havok_Active;
    }

    // VHAVOKRIGIDBODY_VERSION_10
    if (iFileVersion >= VHAVOKRIGIDBODY_VERSION_10)
    {
      ar >> Havok_WeldingType;
    }

	 // VHAVOKRIGIDBODY_VERSION_11
    if (iFileVersion >= VHAVOKRIGIDBODY_VERSION_11)
    {
      ar >> Havok_TightFit;
    }

    //For save games: Additional physics state like current velocity
  } 
  else
  {
    ar << iLocalVersion;

    // VHAVOKRIGIDBODY_VERSION_0
    ar << Shape_Type;
    Shape_BoxSize.SerializeAsVisVector (ar);
    ar << Shape_Radius;
    Shape_PivotOffset.SerializeAsVisVector (ar);
    Shape_CenterOfMass.SerializeAsVisVector (ar);
    ar << Havok_Mass;
    ar << Havok_Friction;
    ar << Havok_Restitution;
    ar << Havok_LinearDamping;
    ar << Havok_AngularDamping;
    ar << Debug_Color;
    
    // VHAVOKRIGIDBODY_VERSION_1
    ar << Havok_FileResourceName;

    // VHAVOKRIGIDBODY_VERSION_2
    ar << Havok_TakeRbDataFromFile;
    ar << Shape_PivotOffsetAutoAlign;
    ar << Havok_MotionType;

    // VHAVOKRIGIDBODY_VERSION_3
    ar << Havok_CollisionLayer;
    ar << Havok_CollisionGroup;
    ar << Havok_SubSystemId;
    ar << Havok_SubSystemDontCollideWith;

    if (m_spRigidBody != NULL)
    {
      GetLinearVelocity().SerializeAsVisVector(ar);
      GetAngularVelocity().SerializeAsVisVector(ar);
    }
    else
    {
      // serialize zero vectors
      hkvVec3 zeroVector(0.0f, 0.0f, 0.0f);
      zeroVector.SerializeAsVisVector(ar);
      zeroVector.SerializeAsVisVector(ar);
    }

    // VHAVOKRIGIDBODY_VERSION_4
    ar << Shape_Height;

    // VHAVOKRIGIDBODY_VERSION_5
    ar << Havok_NoDeactivate;
    ar << Havok_InertiaTensorComputeMode;
    ar << Havok_SurfaceThickness;
    Havok_InertiaTensorAxisScaling.SerializeAsVisVector (ar);
    ar << Havok_InertiaTensorScaling;

    // VHAVOKRIGIDBODY_VERSION_6
    ar << Havok_QualityType;

    // VHAVOKRIGIDBODY_VERSION_7
    ar << Havok_MaxKinematicVelocity;

    // VHAVOKRIGIDBODY_VERSION_8
    ar << Havok_AlwaysInitAsDynamic;

    // VHAVOKRIGIDBODY_VERSION_9
    ar << Havok_Active;

    // VHAVOKRIGIDBODY_VERSION_10
    ar << Havok_WeldingType;

    // VHAVOKRIGIDBODY_VERSION_11
    ar << Havok_TightFit;
  }
}


///-------------------------------------------------------------------------///
/// VTypedObject Override                                                   ///
///-------------------------------------------------------------------------///

void vHavokRigidBody::OnDeserializationCallback(const VSerializationContext &context)
{
  // We can finally create the rigid body now, since the owner entity is guaranteed
  // to be deserialized completely.
  if (!m_spRigidBody)
    CommonInit();

  if (Havok_MotionType != MotionType_FIXED)
  {
    SetLinearVelocity(m_vTempDeserializedLinearVel);
    SetAngularVelocity(m_vTempDeserializedAngularVel);
  }
}


// -------------------------------------------------------------------------- //
// Variable Attributes                                                        //
// -------------------------------------------------------------------------- //

#ifdef WIN32

void vHavokRigidBody::GetVariableAttributes(VisVariable_cl *pVariable, VVariableAttributeInfo &destInfo)
{
  // Check whether we need to display the radius property
  if (!strcmp(pVariable->GetName(),"Shape_Radius"))
  {
    // Get shape type
    char pStr[128];
    GetVariable("Shape_Type")->GetValue(this, pStr, true);

    if (strcmp(pStr, "Sphere") && strcmp(pStr, "Capsule") && strcmp(pStr, "Cylinder"))
      destInfo.m_bHidden = true;
  }
  // Check whether we need to display the box size property
  else if (!strcmp(pVariable->GetName(),"Shape_BoxSize"))
  {
    // Get shape type
    char pStr[128];
    GetVariable("Shape_Type")->GetValue(this, pStr, true);

    if (strcmp(pStr, "Box"))
      destInfo.m_bHidden = true;
  }
  // Check whether we need to display the debug color property
  else if (!strcmp(pVariable->GetName(),"Debug_Color"))
  {
    // Get debug display state
    BOOL b;
    GetVariable("Debug_Render")->GetValueDirect(this, (void*) &b, true);

    if (b == false)
      destInfo.m_bHidden = true;
  }
  // Check whether we need to display the pivot offset property
  else if (!strcmp(pVariable->GetName(),"Shape_PivotOffset"))
  {
    // Get debug display state
    BOOL b;
    GetVariable("Shape_PivotOffsetAutoAlign")->GetValueDirect(this, (void*) &b, true);

    if (b == TRUE)
      destInfo.m_bHidden = true;
  }
  else if (!strcmp(pVariable->GetName(),"Shape_Height"))
  {
    // Get shape type
    char pStr[128];
    GetVariable("Shape_Type")->GetValue(this, pStr, true);

    if (strcmp(pStr, "Capsule") && strcmp(pStr, "Cylinder"))
      destInfo.m_bHidden = true;
  }
  else if (!strcmp(pVariable->GetName(),"Havok_SurfaceThickness"))
  {
    // Get shape type
    char pStr[128];
    GetVariable("Havok_InertiaTensorComputeMode")->GetValue(this, pStr, true);

    if (0 != strcmp(pStr, "Surface"))
      destInfo.m_bHidden = true;
  }
  else if (!strcmp(pVariable->GetName(), "Havok_MaxKinematicVelocity"))
  {
    // Get motion type
    char pStr[128];
    GetVariable("Havok_MotionType")->GetValue(this, pStr, true);

    if (strcmp(pStr, "Keyframed"))
    {
      destInfo.m_bHidden = true;
    }
  }
  else if (!strcmp(pVariable->GetName(),"Havok_WeldingType"))
  {
    // Get shape type
    char pStr[128];
    GetVariable("Shape_Type")->GetValue(this, pStr, true);
    if (strcmp(pStr, "Mesh")!=0)
      destInfo.m_bHidden = true;
  }
}

#endif

START_VAR_TABLE(vHavokRigidBody,IVObjectComponent,"Havok Rigid Body Component",VVARIABLELIST_FLAGS_NONE, "Havok Rigid Body" )
  DEFINE_VAR_ENUM_AND_NAME         (vHavokRigidBody, Shape_Type, "Shape Type", "Geometry shape type used for Physics. When possible, try to approximate objects by a simpler shape type (such as Box or Sphere).", "Convex Hull","Box/Sphere/Convex Hull/File/Capsule/Cylinder/Mesh", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Shape_Radius, "Radius", "Sphere/Capsule/Cylinder Radius", "0", 0, "Clamp(0,1e6)");
  DEFINE_VAR_VECTOR_FLOAT_AND_NAME (vHavokRigidBody, Shape_BoxSize, "Box Size", "XYZ Box Size", "0/0/0", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Shape_Height, "Height", "Capsule/Cylinder Height", "0", 0, "Clamp(0,1e6)");
  DEFINE_VAR_VECTOR_FLOAT_AND_NAME (vHavokRigidBody, Shape_PivotOffset, "Pivot Offset", "Offset between center of owner object and its RB.", "0/0/0", 0,0);
  DEFINE_VAR_VECTOR_FLOAT_AND_NAME (vHavokRigidBody, Shape_CenterOfMass, "Center of Mass", "The center of mass of this rigid body in its local space.", "0/0/0", 0,0);
  DEFINE_VAR_BOOL_AND_NAME         (vHavokRigidBody, Havok_NoDeactivate, "No Deactivate", "Prevents deactivation of the rigid body when it is moving very slowly. Usually this setting should be left as FALSE, which helps stabilize stacks of objects.", "FALSE", 0, NULL);
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Havok_Mass, "Mass", "The mass of the RB in kilograms.", "10.0", 0,"Clamp(1e-3,1e12)");
  DEFINE_VAR_ENUM_AND_NAME         (vHavokRigidBody, Havok_InertiaTensorComputeMode, "Inertia Tensor computation mode", "Inertia tensor computation mode. Use Volume for solid objects, and Surface for objects which consist of a hollow shell. If set to None, the inertia tensor needs to be specified manually via the InertiaTensorAxisScaling and InertiaTensorScaling properties. Surface is only supported for Box and Sphere type objects.", "Volume", "None/Surface/Volume", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Havok_SurfaceThickness, "Surface Thickness", "The thickness of the object's shell for surface inertia tensor computation.", "0.1", 0, 0);
  DEFINE_VAR_VECTOR_FLOAT_AND_NAME (vHavokRigidBody, Havok_InertiaTensorAxisScaling, "Inertia Tensor Axis Scaling", "Factors to scale the inertia tensor's axes with. Increasing the inertia for an axis will increase the RB's resistance to changes in angular momentum in that axis.", "1/1/1", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Havok_InertiaTensorScaling, "Inertia Tensor Scaling", "Factor to scale all components of the inertia tensor with. Increasing the inertia will increase the RB's resistance to changes in angular momentum.", "1", 0, "Clamp(1e-3, 1e3)");
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Havok_Friction, "Friction", "The coefficient of friction of the surface of this RB. Lower values will result in a smoother surface and more slipping, while higher values will result in a less smooth surface and less slipping.", "0.5", 0, "Clamp(0,1)");
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Havok_Restitution, "Restitution", "The restitution defines how much impulse is preserved during a collision. A restitution of 0 will result in the body losing its entire momentum, while higher values result in \"bouncier\" collisions. A value of 1 corresponds to a perfectly elastic collision where no energy is lost. You may use values higher than 1 to achieve a certain desired behavior, although this may result in unrealistic collisions.", "0.4", 0, "Clamp(0,1.98)");
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Havok_LinearDamping, "Linear Damping", "Additional damping to control the rigid body velocity. A value of 0.05 corresponds to losing 5% of the velocity every second.", "0.0", 0, "Clamp(0,1)");
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Havok_AngularDamping, "Angular Damping", "Additional damping to control the rigid body angular velocity. A value of 0.05 corresponds to losing 5% of the angular velocity every second.", "0.05", 0, "Clamp(0,1)");
  DEFINE_VAR_VSTRING_AND_NAME      (vHavokRigidBody, Havok_FileResourceName, "File Resource", "The .hkt file which contains the serialized Physics shape if the shape type is set to File.", "", 1024, 0, "filepicker(.hkt)");
  DEFINE_VAR_BOOL_AND_NAME         (vHavokRigidBody, Debug_Render, "Debug Render", "Enables/Disables Physics Debug Rendering.", "FALSE", 0, 0);
  DEFINE_VAR_COLORREF_AND_NAME     (vHavokRigidBody, Debug_Color, "Debug Render Color", "Color of this RB when Debug Rendering is active.", "50,255,50,255", 0, NULL);
  DEFINE_VAR_BOOL_AND_NAME         (vHavokRigidBody, Havok_TakeRbDataFromFile, "All RB data from file", "Defines whether to use the entire set of RB properties from the .hkt file or just the collision shape.", "FALSE", 0, 0);
  DEFINE_VAR_ENUM_AND_NAME         (vHavokRigidBody, Havok_MotionType, "Motion Type",
    "Type of Physics:\n"
    "Dynamic - Moves dynamically and collides with other objects.\n"
    "Keyframed - Can be moved through user code. It is not affected by collisions, but can push other objects. \n"
    "Fixed - Collides but doesn't move.\n"
    "Others - Should not be used",
    "Dynamic", "Dynamic/Keyframed/Fixed/Sphere Inertia/Box Inertia/Thin Box Inertia/Character", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME         (vHavokRigidBody, Shape_PivotOffsetAutoAlign, "Auto-Align Pivot Offset", "Automatic Alignment of the pivot offset.", "TRUE", 0,0);
  DEFINE_VAR_INT_AND_NAME          (vHavokRigidBody, Havok_CollisionLayer, "Collision Layer", "Defines the collision layer this RB is assigned to.\n" COLLISION_LAYER_NUMBER_VARTABLE_DESCRIPTION, "1", 0, "Clamp(0, 31)");
  DEFINE_VAR_INT_AND_NAME          (vHavokRigidBody, Havok_CollisionGroup, "Collision Group", "Defines the collision group this RB is assigned to.", "0", 0, "Clamp(0, 65535)");
  DEFINE_VAR_INT_AND_NAME          (vHavokRigidBody, Havok_SubSystemId, "SubSystem ID", "Defines the sub system ID of this RB.", "0", 0, "Clamp(0, 31)");
  DEFINE_VAR_INT_AND_NAME          (vHavokRigidBody, Havok_SubSystemDontCollideWith, "SubSystem to not collide with", "Defines the sub system ID this RB should not collide with.", "0", 0, "Clamp(0, 31)");
  DEFINE_VAR_ENUM_AND_NAME         (vHavokRigidBody, Havok_QualityType, "Quality Type",
    "Used to specify when to use continuous physics. Does not affect fixed motion rigid bodies!\n"
    "Auto - Default, automatic assignment depending on the motion type.\n"
    "Fixed - Use this for fixed bodies.\n"
    "Keyframed - Use this for keyframed bodies.\n"
    "Debris - Use this for debris objects which only require low quality collisions.\n"
    "Debris_Simple_Toi - Similar to Debris, but using simplified TOI collisions with fixed/landscape objects.\n"
    "Moving - Use this for standard moving bodies.\n"
    "Critical - Use this for objects which must never tunnel through the geometry.\n"
    "Bullet - Use this for very fast moving objects.\n"
    "Others - Should not be used",
    "Auto","Auto/Fixed/Keyframed/Debris/Debris_Simple_Toi/Moving/Critical/Bullet/Character/Keyframed_Reporting", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME        (vHavokRigidBody, Havok_MaxKinematicVelocity, "Max Kinematic Velocity", "If the current velocity exceeds this value the object will be teleported each frame rather than moving continuously.", "600.0", 0, "Clamp(1,1e12)");
  DEFINE_VAR_BOOL_AND_NAME         (vHavokRigidBody, Havok_AlwaysInitAsDynamic, "Always init as dynamic", "Enforce initialization as dynamic motion type and set configured motion type after initialization. Reason: only dynamicly initialized objects can be changed back from a fixed/keyframed state to become dynamic again (e.g. after loading a savegame)", "FALSE", 0,0);
  DEFINE_VAR_BOOL_AND_NAME         (vHavokRigidBody, Havok_Active, "Active", "Whether the rigid body is active or not. The object is removed from the Havok World on deactivation so its collision is ignored then.", "TRUE", 0,0);
  DEFINE_VAR_ENUM_AND_NAME         (vHavokRigidBody, Havok_WeldingType , "WeldingType", "Welding is used to reduce the problem of objects bouncing while transitioning from a collision with one shape to a collision with a neighboring shape. Only considered for Shape Type Mesh. When enabled, the orientation for meshes used in Vision is usually Anticlockwise.", "None", "None/Anticlockwise/Clockwise/Two-sided", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME         (vHavokRigidBody, Havok_TightFit, "TightFitShape", "If object is convex, this will shrink the collision geometry such that the surface, including the 'convex radius', is as close as possible to the graphical one. Only affects generated shapes, not shapes from HKT files.", "FALSE", 0,0);
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
