/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef VLINEFOLLOWERCOMPONENT_HPP_INCLUDED
#define VLINEFOLLOWERCOMPONENT_HPP_INCLUDED

#define RESET_ACTION    "resetpath"
#define SET_ACTION      "setpath"

#define ENTITY_PROPERTY "entity"
#define TO_PROPERTY     "to"
#define ACTION_PROPERTY "action"

#define SHARED_IMPEXP VHAVOK_IMPEXP
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokCharacterController.hpp>

///
/// \brief
///   Component which follows a predefined path, using a character controller for physics.
/// 
/// Component that can be attached to an entity that will play an animation and follow 
/// a path using physics. Make sure the entity has an animation and that the animation 
/// has  motion delta
///
class VLineFollowerComponent : public IVObjectComponent, public IVisCallbackHandler_cl
{
public:
  ///
  /// @name Constructor / Initialization / Destructor
  /// @{
  ///

  /// \brief
  ///   Default constructor. Sets up the input map.
  ///
  /// The actual initialisation of the component happens in the ::SetOwner function
  ///
  SHARED_IMPEXP VLineFollowerComponent();

  /// \brief
  ///   Destructor
  SHARED_IMPEXP virtual ~VLineFollowerComponent();

  /// \brief
  ///   Init Function
  SHARED_IMPEXP void CommonInit();

  /// \brief
  ///   DeInit Function
  SHARED_IMPEXP void CommonDeInit();

  ///
  /// @}
  ///

  ///
  /// @name IVObjectComponent Overrides
  /// @{
  ///

  /// \brief
  ///   Overridden function to respond to owner changes
  SHARED_IMPEXP virtual void SetOwner(VisTypedEngineObject_cl *pOwner) HKV_OVERRIDE;

  /// \brief
  ///   Overridden function. Blob shadows can be attached to VisObject3D_cl instances
  SHARED_IMPEXP virtual BOOL CanAttachToObject(VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut) HKV_OVERRIDE;

  /// \brief
  ///   Overridden function to respond to variable changes
  SHARED_IMPEXP virtual void OnVariableValueChanged(VisVariable_cl *pVar, const char * value) HKV_OVERRIDE;

  /// \brief
  ///   Overridden function to process incoming messages, such as collision events and property changes
  SHARED_IMPEXP virtual void MessageFunction(int iID, INT_PTR iParamA, INT_PTR iParamB) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name VTypedObject Overrides
  /// @{
  ///

  /// \brief
  ///   Virtual overridable that indicates whether OnDeserializationCallback 
  ///   should be called for this object
  SHARED_IMPEXP virtual VBool WantsDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE
  {
    return (context.m_eType != VSerializationContext::VSERIALIZATION_EDITOR);
  }

  /// \brief
  ///   Virtual overridable that gets called when a loading archive closes
  /// 
  SHARED_IMPEXP virtual void OnDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name Serialization
  /// @{
  ///

  /// \brief
  ///   RTTI macro
  V_DECLARE_SERIAL_DLLEXP(VLineFollowerComponent, SHARED_IMPEXP)

  /// \brief
  ///   RTTI macro to add a variable table
  V_DECLARE_VARTABLE(VLineFollowerComponent, SHARED_IMPEXP)

  /// \brief
  ///   Serialization function
  ///
  /// \param ar
  ///   binary archive
  ///
  SHARED_IMPEXP virtual void Serialize(VArchive &ar) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name IVisCallbackHandler_cl Overrides
  /// @{
  ///

  /// 
  /// \brief
  ///   Callback handler implementation.
  ///
  SHARED_IMPEXP virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name Update Method
  /// @{
  ///

  ///
  /// \brief
  ///   Updates the the component for the current frame.
  ///
  void PerFrameUpdate();

  ///
  /// @}
  ///

  ///
  /// @name Helpers
  /// @{
  ///

  SHARED_IMPEXP void InitPhysics(float fPathPos);
  SHARED_IMPEXP BOOL StartAnimation(const char *szAnimName);

  ///
  /// @}
  ///

  vHavokCharacterController *m_pPhys;

  // entity parameters
  char Model_AnimationName[128];
  char Path_Key[128];
  float Path_NumberSteps;
  float Path_TriggerDistance;
  VisPath_cl *m_pFollowPath;
  float Path_InitialOffset;
  float m_fCurrentPathPos;
  BOOL Debug_DisplayBoxes;
  bool m_bPlayingAnim;
  
  float Model_DeltaRotation;
  hkvMat3 m_mDeltaRotation;
  float Model_GroundOffset;
  float Model_CapsuleHeight;
  float Model_CapsuleRadius;

  BOOL Debug_RenderMesh;
};

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
