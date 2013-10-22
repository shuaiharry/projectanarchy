/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VPathRendererBase.hpp

#ifndef VPATHRENDERERBASE_H_INCLUDED
#define VPATHRENDERERBASE_H_INCLUDED

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/VPathRenderingModule.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/IVPathRenderingData.hpp>

/// \brief
///   Base class for all path renderers. This base class provides functions common to all renderers, such as registration of
///   relevant callbacks, and common component tasks.
class VPathRendererBase : public IVObjectComponent, public IVisCallbackHandler_cl
{
public:
  /// \brief
  ///   Constructor for this path renderer component.
  /// \param iComponentFlags
  ///   Component bit flags that define the behavior of the component.
  /// \see
  ///   IVObjectComponent::IVObjectComponent
  PATHRND_IMPEXP VPathRendererBase(int iComponentFlags = VIS_OBJECTCOMPONENTFLAG_NONE);

  /// \brief
  ///   Destructor; removes the renderer if it's still active.
  PATHRND_IMPEXP virtual ~VPathRendererBase();

public:
  /// \brief
  ///   Removes the rigid body from the simulation without necessarily deleting 
  ///   this instance.
  PATHRND_IMPEXP virtual  void DisposeObject() HKV_OVERRIDE;

  /// \brief
  ///   Flushes the path rendering data; called whenever data change.
  virtual void OnDataChanged();

protected:
  /// \brief
  ///   Initializes this component after it has been added to an owner.
  /// \note
  ///   Override the method DoInit() to perform initialization tasks in subclasses!
  void CommonInit();
  
  /// \brief
  ///   Overridable initialization method.
  /// \return
  ///   \c true if the initialization succeeded; \c false if it failed
  virtual bool DoInit();

  /// \brief
  ///   Deinitialization that is used both on DisposeObject and on destruction.
  virtual void CommonDeinit();

public:
  // Base Class Overrides
  PATHRND_IMPEXP virtual BOOL CanAttachToObject(VisTypedEngineObject_cl* pObject, VString& sErrorMsgOut) HKV_OVERRIDE;
  PATHRND_IMPEXP virtual void OnHandleCallback(IVisCallbackDataObject_cl* pData) HKV_OVERRIDE;
  PATHRND_IMPEXP virtual void SetOwner(VisTypedEngineObject_cl* pOwner) HKV_OVERRIDE;

  /// Serialization
  V_DECLARE_SERIAL_DLLEXP(VPathRendererBase, PATHRND_IMPEXP)
  V_DECLARE_VARTABLE(VPathRendererBase, PATHRND_IMPEXP)
  PATHRND_IMPEXP virtual void Serialize(VArchive& ar) HKV_OVERRIDE;
  PATHRND_IMPEXP virtual void OnDeserializationCallback(const VSerializationContext& context) HKV_OVERRIDE;
  virtual VBool WantsDeserializationCallback(const VSerializationContext& context) HKV_OVERRIDE { return TRUE; }

protected:  
  /// \brief
  ///   Performs the rendering of the constraint chain.
  ///
  /// \param pCallbackData
  ///   When performing shadow pass, pointer to shadow data, otherwise NULL
  virtual void OnRender(IVisCallbackDataObject_cl* pCallbackData);

  /// \brief
  ///   Performs any updating necessary before the chain is rendered in this frame
  virtual void OnUpdate();

private:  
  /// \brief
  ///   Factory function that tries to create an compatible rendering data adapter
  bool TryCreateRenderingData(VisTypedEngineObject_cl* pObject);

protected:
  IVPathRenderingDataPtr m_spPathRenderingData; ///< Adapter that provides path rendering data
  bool m_bIsInitialized;

};


#endif //VPATHRENDERERBASE_H_INCLUDED

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
