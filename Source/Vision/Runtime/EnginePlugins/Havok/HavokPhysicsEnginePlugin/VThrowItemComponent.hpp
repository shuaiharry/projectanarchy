/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef VTHROWITEMCOMPONENT_HPP_INCLUDED
#define VTHROWITEMCOMPONENT_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokPhysicsModule.hpp>

/// \brief
///   Adds the possibility to throw Havok Physics rigid bodies with the "Q" key.
///
class VThrowItemComponent : public IVObjectComponent, public IVisCallbackHandler_cl
{
public:
  ///
  /// @name Constructor / Destructor
  /// @{
  ///

  /// \brief
  ///   Default constructor. Sets up the input map.
  ///
  /// The actual initialisation of the component happens in the SetOwner function
  ///
  VHAVOK_IMPEXP VThrowItemComponent();
  
   /// \brief
  ///   Destructor
  VHAVOK_IMPEXP virtual ~VThrowItemComponent();

  /// \brief
  ///   Init Function
  VHAVOK_IMPEXP void CommonInit();

  /// \brief
  ///   DeInit Function
  VHAVOK_IMPEXP void CommonDeInit();

  ///
  /// @}
  ///

  ///
  /// @name IVObjectComponent Overrides
  /// @{
  ///

  /// \brief
  ///   Overridden function to respond to owner changes
  ///
  /// \param pOwner
  ///   The owner of this component.
  VHAVOK_IMPEXP virtual void SetOwner(VisTypedEngineObject_cl *pOwner) HKV_OVERRIDE;

  /// \brief
  ///   Overridden function. This component can be attached to VisBaseEntity_cl instances
  ///
  /// \param pObject
  ///   Potential owner.
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
  ///   Pointer to the variable object.
  /// 
  /// \param value
  ///   New value of the variable.
  /// 
  VHAVOK_IMPEXP VOVERRIDE void OnVariableValueChanged(VisVariable_cl *pVar, const char * value);

  ///
  /// @}
  ///

  ///
  /// @name Serialization
  /// @{
  ///

  /// \brief
  ///   RTTI macro
  V_DECLARE_SERIAL_DLLEXP(VThrowItemComponent, VHAVOK_IMPEXP)

  /// \brief
  ///   RTTI macro to add a variable table
  V_DECLARE_VARTABLE(VThrowItemComponent, VHAVOK_IMPEXP)

  /// \brief
  ///   Serialization function
  ///
  /// \param ar
  ///   binary archive
  VHAVOK_IMPEXP virtual void Serialize(VArchive &ar) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name IVisCallbackHandler_cl Overrides
  /// @{
  ///

  /// 
  /// \brief
  ///   Overridden function that gets called by all callbacks that this handler is registered to.
  /// 
  /// \param pData
  ///   See IVisCallbackHandler_cl
  ///
  /// \see
  ///   IVisCallbackHandler_cl
  /// 
  VHAVOK_IMPEXP virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name Update Method
  /// @{
  ///

  ///
  /// \brief
  ///   Updates the the state machine for the current frame.
  /// 
  /// The PerFrameUpdate() function takes care of processing the input map (keyboard events)
  /// and updating the throw-item component.
  ///
  /// \note
  ///   You do not have to call this function manually, since the VThrowItemComponentManager
  ///   class will take care of this.
  ///
  VHAVOK_IMPEXP void PerFrameUpdate();

  ///
  /// @}
  ///
  
  /// \brief
  ///   Creates a physics object and throws using the specified direction.
  VHAVOK_IMPEXP void ThrowItem(const hkvVec3& vThrowDir);
  
  /// \brief
  ///   Creates a physics object and throws it in the facing direction of its owner.
  VHAVOK_IMPEXP void ThrowItem();  
  
  inline void SetModelFile(VString ModelFile) 
  { 
    this->ModelFile = ModelFile; 
  }

  inline const VString& GetModelFile() const 
  { 
    return ModelFile; 
  }
  
  inline void SetVelocity(float Velocity) 
  { 
    this->Velocity = Velocity; 
  }

  inline float GetVelocity() const 
  { 
    return Velocity; 
  }

private:
  /// \brief
  ///   Removes all physics object.
  void RemoveAllItems();

  /* Exposed to vForge */
  VString ModelFile;                    ///< Model filename
  float Velocity;                       ///< Velocity that is applied to the item

  /* Not Exposed to vForge */
  VInputMap *m_pInputMap;               ///< Input map for the character's keyboard/gamepad controls
};

#endif // VTHROWITEMCOMPONENT_HPP_INCLUDED

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
