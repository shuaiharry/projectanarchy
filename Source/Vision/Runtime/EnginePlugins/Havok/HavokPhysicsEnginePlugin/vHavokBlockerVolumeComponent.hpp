/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef VHAVOKPHYSICSBLOCKERVOLUMECOMPONENT_HPP_INCLUDED
#define VHAVOKPHYSICSBLOCKERVOLUMECOMPONENT_HPP_INCLUDED

//------------------------------------------------------------------------------

class hkpRigidBody;
class VCustomVolumeObject;

//------------------------------------------------------------------------------

///
/// \brief
///   Defines the shape type of a blocker volume.
///
enum VHavokBlockerVolumeShapeType_e
{
  VHavokBlockerVolumeShapeType_CONVEX = 0,
  VHavokBlockerVolumeShapeType_MESH
};

///
/// \brief
///   Can be attached to a custom volume object for defining a blocker volume.
///
/// Blocker Volumes are fixed rigid bodies.
///
/// \note
///   If the blocker volume component should be turned on and off at runtime,
///   it is advised to use the collision filter functionality instead of 
///   modifying the enabled state.
///
class vHavokBlockerVolumeComponent : public IVObjectComponent
{
public:
  V_DECLARE_SERIAL_DLLEXP(vHavokBlockerVolumeComponent, VHAVOK_IMPEXP);
  V_DECLARE_VARTABLE(vHavokBlockerVolumeComponent, VHAVOK_IMPEXP);

  ///
  /// @name Constructor / Destructor
  /// @{
  ///

  /// \brief
  ///   Default constructor.
  VHAVOK_IMPEXP vHavokBlockerVolumeComponent();

  /// \brief
  ///   Destructor
  VHAVOK_IMPEXP virtual ~vHavokBlockerVolumeComponent();

  ///
  /// @}
  ///

  ///
  /// @name Component System Overrides
  /// @{
  ///

  VHAVOK_IMPEXP virtual void SetOwner(VisTypedEngineObject_cl *pOwner) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual BOOL CanAttachToObject(
    VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual void Serialize(VArchive &ar) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name Properties
  /// @{
  ///

  ///
  /// \brief
  ///   Sets the enabled state of the component.
  ///
  VHAVOK_IMPEXP void SetEnabled(bool bEnabled);

  ///
  /// \brief
  ///   Returns whether the component is enabled.
  ///
  inline bool IsEnabled() const
  {
    return (m_bEnabled == TRUE);
  }

  ///
  /// \brief
  ///   Sets the restitution of the collision object.
  ///
  VHAVOK_IMPEXP void SetRestitution(float fRestitution);

  ///
  /// \brief
  ///   Returns the restitution of the collision object.
  ///
  inline float GetRestitution() const
  {
    return m_fRestitution;
  }

  ///
  /// \brief
  ///   Sets the friction of the collision object.
  ///
  VHAVOK_IMPEXP void SetFriction(float fFriction);

  ///
  /// \brief
  ///   Returns the friction of the collision object.
  ///
  inline float GetFriction() const
  {
    return m_fFriction;
  }

  ///
  /// \brief
  ///   Sets the collision filter info the collision object.
  ///
  VHAVOK_IMPEXP void SetCollisionFilterInfo(int iCollisionLayer, int iCollisionGroup, 
    int iSubSystemId, int iSubSystemDontCollideWith);

  ///
  /// \brief
  ///   Retrieves the collision filter info.
  ///
  inline void GetCollisionFilterInfo(int& iCollisionLayerOut, int& iCollisionGroupOut,
    int& iSubSystemIdOut, int& iSubSystemDontCollideWithOut) const
  {
    iCollisionLayerOut = m_iCollisionLayer;
    iCollisionGroupOut = m_iCollisionGroup;
    iSubSystemIdOut = m_iSubSystemId;
    iSubSystemDontCollideWithOut = m_iSubSystemDontCollideWith;
  }

  ///
  /// \brief
  ///   Sets the shape type of the blocker volume.
  ///
  VHAVOK_IMPEXP void SetShapeType(VHavokBlockerVolumeShapeType_e eShapeType);

  ///
  /// \brief
  ///   Returns the shape type of the blocker volume.
  ///
  inline VHavokBlockerVolumeShapeType_e GetShapeType() const
  {
    return static_cast<VHavokBlockerVolumeShapeType_e>(m_iShapeType);
  }

  ///
  /// \brief
  ///   Returns the owner custom volume object.
  ///
  inline VCustomVolumeObject* GetOwnerVolume()
  {
    return vstatic_cast<VCustomVolumeObject*>(GetOwner());
  }

  ///
  /// @}
  ///

  ///
  /// @name Debug Rendering
  /// @{
  ///

  ///
  /// \brief
  ///   Enable / Disable debug rendering for this blocker volume's shape.
  ///
  VHAVOK_IMPEXP void SetDebugRendering(BOOL bEnable);

  ///
  /// \brief
  ///   Set the color of the debug rendering representation of this blocker volume's shape.
  ///
  VHAVOK_IMPEXP void SetDebugColor(VColorRef color);

  ///
  /// \brief 
  ///   Returns whether debug rendering is enabled.
  ///
  inline bool GetDebugRenderEnabled() const 
  { 
    return (m_bDebugRenderingEnabled == TRUE); 
  }

  ///
  /// @}
  ///

  ///
  /// @name Overridden functions
  /// @{
  ///

  VHAVOK_IMPEXP virtual void OnVariableValueChanged(VisVariable_cl* pVar, const char* value) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual void MessageFunction(int iID, INT_PTR iParamA, INT_PTR iParamB) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual void OnDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual VBool WantsDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE
  { 
    return (context.m_eType != VSerializationContext::VSERIALIZATION_EDITOR); 
  }

  ///
  /// @}
  ///

private:
  // Private functions
  void CreatePhysicsObject();
  void DestroyPhysicsObject();

  void AddToPhysicsWorld();
  void RemoveFromPhysicsWorld();

  // Member variables
  BOOL m_bEnabled;
  BOOL m_bDebugRenderingEnabled;
  VColorRef m_debugColor;

  int m_iShapeType;

  float m_fRestitution;
  float m_fFriction;

  int m_iCollisionLayer, m_iCollisionGroup;
  int m_iSubSystemId, m_iSubSystemDontCollideWith;

  hkpRigidBody* m_pRigidBody;
};

//------------------------------------------------------------------------------

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
