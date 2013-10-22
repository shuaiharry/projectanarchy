/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file vHavokTriggerVolume.hpp

#ifndef VHAVOKTRIGGERVOLUME_HPP_INCLUDED
#define VHAVOKTRIGGERVOLUME_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Entities/VCustomVolumeObject.hpp>

/// Serialization versions
#define VHAVOKTRIGGERVOLUME_VERSION_0        0                              // Initial version
#define VHAVOKTRIGGERVOLUME_VERSION_CURRENT  VHAVOKTRIGGERVOLUME_VERSION_0  // Current version

/// Trigger Component Source names
#define VHAVOKTRIGGERVOLUME_ONOBJECTENTER       "OnObjectEnter"
#define VHAVOKTRIGGERVOLUME_ONOBJECTLEAVE       "OnObjectLeave"
#define VHAVOKTRIGGERVOLUME_ONCHARACTERENTER    "OnCharacterEnter"
#define VHAVOKTRIGGERVOLUME_ONCHARACTERLEAVE    "OnCharacterLeave"

///
/// \brief
///   Enumeration defining the shape type of a trigger volume.
///
enum VHavokTriggerVolumeShapeType_e
{
  VHavokTriggerVolumeShapeType_CONVEX = 0,                ///< Convex shape
  VHavokTriggerVolumeShapeType_MESH = 1                   ///< Polygon Mesh
};

///
/// \brief
///   Enumeration defining the motion type of a trigger volume.
///
enum VHavokTriggerVolumeMotionType_e
{
  VHavokTriggerVolumeMotionType_FIXED = 0,                ///< Collides but doesn't move.
  VHavokTriggerVolumeMotionType_KEYFRAMED = 1             ///< Can only be moved manually via code.
};

///
/// \brief
///   Enumeration defining the quality type of a trigger volume.
enum VHavokTriggerVolumeQualityType_e
{
  VHavokTriggerVolumeQualityType_AUTO = 0,                ///< Automatic assignment
  VHavokTriggerVolumeQualityType_FIXED = 1,               ///< Use this for fixed trigger volumes
  VHavokTriggerVolumeQualityType_KEYFRAMED = 2,           ///< Use this for keyframed trigger volumes, 
                                                          ///< if you only want to pick up collisions with dynamic bodies.
  VHavokTriggerVolumeQualityType_KEYFRAMED_REPORTING = 3  ///< Use this for keyframed trigger volumes, 
                                                          ///< if you want to pick up collisions also with other keyframed or fixed bodies
};

// forward declaration
class vHavokPhysicsModule;
class vHavokTriggerVolume;
class vHavokTriggerVolumeInternal;
class hkpTriggerVolume;

/// \brief
///   Structure that holds information about a trigger event, fired by a vHavokTriggerVolume.
///
struct vHavokTriggerInfo
{
  vHavokTriggerInfo()
    : m_pTriggerVolume(NULL)
    , m_pTriggerSourceComponent(NULL)
    , m_pObject(NULL)
  {
  }

  vHavokTriggerVolume *m_pTriggerVolume;
  VisTriggerSourceComponent_cl *m_pTriggerSourceComponent;
  hkpWorldObject *m_pObject;
};

/// 
/// \brief
///   This class represents a trigger volume object. It is a component that can be attached to 
///   instances of the class VCustomVolumeObject.
/// 
/// The vHavokTriggerVolume object is the Vision representation of the hkpTriggerVolume in Havok Physics, 
/// which are rigid bodies, that record collision events, but have no physical effect. It can be 
/// used for notifications, when rigid bodies / character controllers enter / leave the volume. 
///
class vHavokTriggerVolume : public IVObjectComponent
{
public:
  friend class vHavokTriggerVolumeInternal;

  ///
  /// @name Constructor / Destructor
  /// @{
  ///

  /// 
  /// \brief
  ///   Constructor.
  /// 
  /// \param eShapeType
  ///   Shape type of this trigger volume.
  /// 
  /// \sa
  ///   IVObjectComponent::IVObjectComponent
  ///
  VHAVOK_IMPEXP vHavokTriggerVolume(VHavokTriggerVolumeShapeType_e eShapeType=VHavokTriggerVolumeShapeType_CONVEX);

  /// 
  /// \brief
  ///   Destructor of the Havok Trigger Volume Component. Deletes the Havok Physics trigger volume, if still alive.
  ///
  VHAVOK_IMPEXP virtual ~vHavokTriggerVolume();

  ///
  /// \brief
  ///   Removes the trigger volume from the simulation without necessarily deleting this instance.
  ///
  VHAVOK_IMPEXP VOVERRIDE void DisposeObject();

  ///
  /// @}
  ///

  ///
  /// @name IVObjectComponent Virtual Overrides
  /// @{
  ///

  ///
  /// \brief
  ///   Overridden function handling owner changes.
  ///
  /// By setting the owner of this component the vHavokTriggerVolume object will be 
  /// added to the Havok Physics World and gets registered in the Havok Physics module. 
  ///
  /// \param pOwner
  ///   The owner of this component.
  /// 
  /// \remarks
  ///   SetOwner(NULL) removes the vHavokTriggeredVolume object from the Havok Physics World and
  ///   unregisters it in the Havok Physics module.
  ///
  VHAVOK_IMPEXP virtual void SetOwner(VisTypedEngineObject_cl *pOwner) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Overridden function determining if this component can be attached to a given object.
  ///
  /// The vHavokTriggerVolume component can be attached to VCustomVolumeObject instances.
  /// 
  /// \param pObject
  ///   Potential owner of this component.
  /// 
  /// \param sErrorMsgOut
  ///   Reference to error message string.
  /// 
  /// \returns
  ///   TRUE if this component can be attached to the given object.
  ///
  VHAVOK_IMPEXP virtual BOOL CanAttachToObject(VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Overridden function handling variable changes.
  /// 
  /// \param pVar
  ///   Pointer to the variable object to identify the variable.
  /// 
  /// \param value
  ///   New value of the variable
  ///
  VHAVOK_IMPEXP virtual void OnVariableValueChanged(VisVariable_cl *pVar, const char* value) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Overridden function processing incoming messages, such as collision events and property changes.
  ///
  VHAVOK_IMPEXP virtual void MessageFunction(int iID, INT_PTR iParamA, INT_PTR iParamB) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name Serialization
  /// @{
  ///

  /// \brief
  ///   RTTI macro
  V_DECLARE_SERIAL_DLLEXP(vHavokTriggerVolume, VHAVOK_IMPEXP)

  /// \brief
  ///   RTTI macro to add a variable table
  V_DECLARE_VARTABLE(vHavokTriggerVolume, VHAVOK_IMPEXP)

  /// \brief
  ///   Serialization function
  /// \param ar
  ///   Binary archive
  VHAVOK_IMPEXP virtual void Serialize(VArchive &ar) HKV_OVERRIDE;

  /// \brief
  ///   Overridden function finalizing the Havok Physics trigger volume once deserialization has finished.
  VHAVOK_IMPEXP virtual void OnDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE;

  /// \brief
  ///   Overridden function indicating whether the component needs a deserialization callback
  VHAVOK_IMPEXP virtual VBool WantsDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE
  {
    return (context.m_eType != VSerializationContext::VSERIALIZATION_EDITOR);
  }

  ///
  /// @}
  ///
  
  ///
  /// @name Property Functions
  /// @{
  ///

  ///
  /// \brief
  ///   Returns the owner of this trigger volume as a VisObject3d_cl.
  ///
  VHAVOK_IMPEXP VisObject3D_cl *GetOwner3D();

  ///
  /// \brief
  ///   Gets the owner custom volume object of this trigger volume as a VCustomVolumeObject.
  ///
  VHAVOK_IMPEXP VCustomVolumeObject *GetOwnerCustomVolume();

  ///
  /// \brief
  ///   Sets the position of this trigger volume, in world space.
  ///
  /// \param value
  ///   Position vector.
  ///
  VHAVOK_IMPEXP void SetPosition(const hkvVec3& value);

  ///
  /// \brief
  ///   Returns the world position of this trigger volume.
  ///
  /// \return
  ///   Position vector.
  ///
  VHAVOK_IMPEXP hkvVec3 GetPosition() const;

  ///
  /// \brief
  ///   Sets the rotation of this trigger volume, in world space.
  ///
  /// \param value
  ///   Rotation matrix.
  ///
  VHAVOK_IMPEXP void SetRotation(const hkvMat3& value);

  ///
  /// \brief
  ///   Returns the world space rotation of this trigger volume.
  ///
  /// \return
  ///   Rotation matrix.
  ///
  VHAVOK_IMPEXP hkvMat3 GetRotation() const;

  /// \brief
  ///   Sets the motion type of this trigger volume. 
  /// 
  /// \param eMotionType
  ///   The new motion type.
  VHAVOK_IMPEXP void SetMotionType(VHavokTriggerVolumeMotionType_e eMotionType);

  /// \brief
  ///   Sets the quality type of this trigger volume. 
  /// 
  /// \param eQualityType
  ///   The new quality type.
  VHAVOK_IMPEXP void SetQualityType(VHavokTriggerVolumeQualityType_e eQualityType);

  /// \brief
  ///   Sets the collision parameters of this trigger volume.
  ///
  /// \param iLayer
  ///   the collision layer
  ///
  /// \param iGroup
  ///   the collision group
  ///
  /// \param iSubsystem
  ///   the collision subsystem
  ///
  /// \param iSubsystemDontCollideWith
  ///   the collision subsystem this body shouldn't collide with
  ///
  /// \note
  ///   See the Havok Physics documentation on rigid body collisions for more 
  ///   information about what values to specify for these parameters.
  ///
  VHAVOK_IMPEXP void SetCollisionInfo(int iLayer, int iGroup, int iSubsystem, int iSubsystemDontCollideWith);

  ///
  /// @}
  ///

  ///
  /// @name Access to Havok Physics Internals
  /// @{
  ///

  ///
  /// \brief
  ///   Gets the internal Havok Physics trigger volume instance (NULL if not initialized).
  ///
  /// \return
  ///   hkpTriggerVolume: Pointer to the Havok Physics trigger volume (NULL if not initialized).
  ///
  inline hkpTriggerVolume* GetHkTriggerVolume() const 
  { 
    return (hkpTriggerVolume*)m_pTriggerVolume; 
  }

  ///
  /// \brief
  ///   Gets the Havok Physics internal rigid body instance, which represents this trigger volume 
  ///   (NULL if not initialized).
  ///
  /// \return
  ///   hkpRigidBody: Pointer to the Havok Physics rigid body (NULL if not initialized).
  ///
  VHAVOK_IMPEXP hkpRigidBody* GetHkTriggerBody() const; 

  ///
  /// @}
  ///

  ///
  /// @name Debug Rendering
  /// @{
  ///

  ///
  /// \brief
  ///   Enable / Disable debug rendering for this trigger volume's shape.
  ///
  /// \param bEnable
  ///   If TRUE, the shape of this trigger volume will be displayed in Vision.
  ///
  VHAVOK_IMPEXP void SetDebugRendering(BOOL bEnable);

  ///
  /// \brief
  ///   Set the color of the debug rendering representation of this trigger volume's shape.
  ///
  /// \param color
  ///   New Color of the debug rendering representation.
  ///
  VHAVOK_IMPEXP void SetDebugColor(VColorRef color);

  ///
  /// \brief 
  ///   Returns whether debug rendering is enabled.
  ///
  inline bool GetDebugRenderEnabled () const 
  { 
    return (Debug_Render == TRUE); 
  }

  ///
  /// @}
  ///

  ///
  /// @name Trigger Components
  /// @{
  ///

  ///
  /// \brief
  ///   Returns the OnObjectEnter trigger source component.
  ///
  inline VisTriggerSourceComponent_cl* GetOnObjectEnterTrigger() const 
  { 
    return m_spOnObjectEnter; 
  } 

  ///
  /// \brief
  ///   Returns the OnObjectLeave trigger source component.
  ///
  inline VisTriggerSourceComponent_cl* GetOnObjectLeaveTrigger() const 
  { 
    return m_spOnObjectLeave; 
  }

  ///
  /// \brief
  ///   Returns the OnCharacterEnter trigger source component.
  ///
  inline VisTriggerSourceComponent_cl* GetOnCharacterEnterTrigger() const 
  { 
    return m_spOnCharacterEnter; 
  }

  ///
  /// \brief
  ///   Returns the OnCharacterLeave trigger source component.
  ///
  inline VisTriggerSourceComponent_cl* GetOnCharacterLeaveTrigger() const 
  { 
    return m_spOnCharacterLeave; 
  }

  ///
  /// @}
  ///

private:
  ///
  /// \brief
  ///   Initializes the trigger volume component after it has been added to an owner.
  void CommonInit();

  ///
  /// \brief
  ///   Deinitialization that is used both for DisposeObject and on destruction.
  void CommonDeinit();

  ///
  /// \brief
  ///   Creates the actual Havok Physics trigger volume and adds it to the simulation.
  bool CreateHkTriggerVolume(VisStaticMesh_cl* pMesh, const hkvVec3& vScale, int iCreationFlags);

  ///
  /// \brief
  ///   Removes trigger volume from the simulation and releases the trigger volume reference.
  void RemoveHkTriggerVolume();

  /// \brief
  ///   Retrieves the transformation of the owner object from Vision and applies them to the Havok Physics representation.
  void UpdateVision2Havok();

 
  vHavokPhysicsModule *m_pModule;                               ///< Reference to Havok Physics Module.
  vHavokTriggerVolumeInternal *m_pTriggerVolume;                ///< The actual Havok Physics Trigger Volume.
  const char *m_szShapeCacheId;                                 ///< ID of collision shape in runtime cache table (points to memory in cache table).

  VSmartPtr<VisTriggerSourceComponent_cl> m_spOnObjectEnter;    ///< Component that triggers the OnObjectEnter event
  VSmartPtr<VisTriggerSourceComponent_cl> m_spOnObjectLeave;    ///< Component that triggers the OnObjectLeave event
  VSmartPtr<VisTriggerSourceComponent_cl> m_spOnCharacterEnter; ///< Component that triggers the OnCharacterEnter event
  VSmartPtr<VisTriggerSourceComponent_cl> m_spOnCharacterLeave; ///< Component that triggers the OnCharacterLeave event

  //
  // Members exposed to vForge:
  //
  int Havok_ShapeType;                                          ///< Shape type of trigger volume's RB
  int Havok_MotionType;                                         ///< Physics Motion Type: Fixed or Keyframed
  int Havok_QualityType;                                        ///< Quality type: Fixed, Keyframed or Keyframed-Reporting
  int Havok_CollisionLayer;                                     ///< Defines the collision layer this trigger volume's RB is assigned to.
  int Havok_CollisionGroup;                                     ///< Defines the collision group this trigger volume's RB is assigned to.
  int Havok_SubSystemId;                                        ///< Defines the sub system ID of this trigger volume's RB.
  int Havok_SubSystemDontCollideWith;                           ///< Defines the sub system ID this trigger volume's RB should not collide with.

  BOOL Debug_Render;                                            ///< Indicates whether debug rendering is active or not.
  VColorRef Debug_Color;                                        ///< Color for debug rendering

};

#endif // VHAVOKTRIGGERVOLUME_HPP_INCLUDED

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
