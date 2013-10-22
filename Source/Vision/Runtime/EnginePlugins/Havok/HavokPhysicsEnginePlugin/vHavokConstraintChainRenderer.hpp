/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef V_HAVOK_CONSTRAINT_CHAIN_RENDERER_HPP_INCLUDED
#define V_HAVOK_CONSTRAINT_CHAIN_RENDERER_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokPhysicsIncludes.hpp>

class vHavokConstraintChain;

/// \brief
///   Base class for all vHavokPhysics constraint chain renderers.
///
/// This base class provides functions common to all renderers, such as 
/// registration of relevant callbacks, and common component tasks.
///
class vHavokConstraintChainRendererBase
  : public IVObjectComponent
  , public IVisCallbackHandler_cl
{
public:
  ///
  /// @name Constructor / Destructor
  /// @{
  ///

  /// \brief
  ///   Constructor for this constraint chain renderer component.
  ///
  /// \param iComponentFlags
  ///   Component bit flags that define the behavior of the component.
  /// \see
  ///   IVObjectComponent::IVObjectComponent
  ///
  VHAVOK_IMPEXP vHavokConstraintChainRendererBase(
    int iComponentFlags=VIS_OBJECTCOMPONENTFLAG_NONE);

  /// \brief
  ///   Destructor; removes the renderer if it's still active.
  ///
  VHAVOK_IMPEXP virtual ~vHavokConstraintChainRendererBase();

  ///
  /// @}
  ///

  ///
  /// @name Initialization / Deinitialization
  /// @{
  ///

  /// \brief
  ///   Removes the rigid body from the simulation without necessarily deleting 
  ///   this instance.
  VHAVOK_IMPEXP virtual void DisposeObject() HKV_OVERRIDE;

protected:
  /// \brief
  ///   Initializes this component after it has been added to an owner.
  ///
  /// \note
  ///   Override the method DoInit() to perform initialization tasks in subclasses.
  ///
  VHAVOK_IMPEXP void CommonInit();
  
  /// \brief
  ///   Overridable initialization method.
  ///
  /// \return
  ///   \c true if the initialization succeeded; \c false if it failed.
  ///
  VHAVOK_IMPEXP virtual bool DoInit();

  /// \brief
  ///   Deinitialization that is used both on DisposeObject and on destruction.
  ///
  VHAVOK_IMPEXP virtual void CommonDeinit();  

  ///
  /// @}
  ///

public:
  ///
  /// @name Base Class Overrides
  /// @{
  ///

  VHAVOK_IMPEXP virtual void SetOwner(VisTypedEngineObject_cl *pOwner) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual BOOL CanAttachToObject(VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name Serialization
  /// @{
  ///

  V_DECLARE_SERIAL_DLLEXP(vHavokConstraintChainRendererBase, VHAVOK_IMPEXP)
  V_DECLARE_VARTABLE(vHavokConstraintChainRendererBase, VHAVOK_IMPEXP)

  VHAVOK_IMPEXP virtual void Serialize(VArchive &ar) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual void OnDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual VBool WantsDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE
  { 
    return (context.m_eType != VSerializationContext::VSERIALIZATION_EDITOR); 
  }
  
  ///
  /// @}
  ///

private:  
  ///
  /// @name Rendering
  /// @{
  ///

  /// \brief
  ///   Performs the rendering of the constraint chain.
  virtual void OnRender();

  /// \brief
  ///   Performs any updates necessary before the chain is rendered in this frame.
  virtual void OnUpdate();

  ///
  /// @}
  ///

  ///
  /// @name Internal Attributes
  /// @{
  ///

protected:
  vHavokConstraintChain *m_pConstraintChain;  ///< The constraint chain this renderer is attached to

private:
  static const unsigned int s_iSerialVersion; ///< The current serialization version for objects of this class.

  ///
  /// @}
  ///
};

#endif // V_HAVOK_CONSTRAINT_CHAIN_RENDERER_HPP_INCLUDED

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
