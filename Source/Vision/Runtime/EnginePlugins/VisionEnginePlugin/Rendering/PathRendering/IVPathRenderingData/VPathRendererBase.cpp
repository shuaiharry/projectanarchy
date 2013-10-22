/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/VisionEnginePluginPCH.h>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/VPathRendererBase.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/IVPathRenderingData.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/ShadowMapping/VShadowMapGenerator.hpp>
#include <Vision/Runtime/Engine/System/VisApiSerialization.hpp>

#define PATHRENDERER_SERIALIZATION_VERSION_0        0   // initial version
#define PATHRENDERER_SERIALIZATION_VERSION_CURRENT  PATHRENDERER_SERIALIZATION_VERSION_0


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// GENERIC FACTORY CLASS - TBD: SHOULD THIS GO INTO VISION DLL?
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/// \brief
///   Generic object factory that manages class types derived by a given base class.
///   \example
///     \code
///       // MyClass.hpp:
///       V_DECLARE_TYPED_OBJECT_FACTORY(MyBaseClass, MyBaseClassFactory)
///
///       // MyClass.cpp:
///       V_IMPLEMENT_TYPED_OBJECT_FACTORY(MyBaseClass)
///
///       void EnumerateClassTypes()
///       {
///         int iNumTypes = MyBaseClassFactory::GlobalManager().GetTypeCount();
///         for (int i = 0; i < iNumTypes; ++i)
///         {
///           MyBaseClass* pInstance = (MyBaseClass*)MyBaseClassFactory::GlobalManager().GetType(i)->CreateInstance();
///
///           // do something..
///
///           delete pInstance;
///         }
///       }
///     \endcode
template <class TypedObjectBaseClass = VTypedObject, bool IgnoreBaseClass = true>
class VTypedObjectFactory
{
public:
  /// \brief 
  ///   Constructor
  inline VTypedObjectFactory() :
    m_iOverallTypeCount(0),
    m_iTypeCount(0),
    m_ppTypes(NULL)
  {}

  /// \brief 
  ///   Destructor
  inline virtual ~VTypedObjectFactory()
  {
    V_SAFE_DELETE_ARRAY(m_ppTypes);
  }

public:
  /// \brief 
  ///   Returns the number of registered classes that are derived by TypedObjectBaseClass
  inline int GetTypeCount()
  {
    UpdateTypeCache();
    return m_iTypeCount;
  }

  /// \brief 
  ///   Returns n-th type of registered classes that are derived by TypedObjectBaseClass
  inline VType* GetType(int iIndex)
  {
    UpdateTypeCache();
    VASSERT(iIndex >= 0 && iIndex < m_iTypeCount);

    return m_ppTypes[iIndex];
  }

  /// \brief 
  ///   Static function to access the global instance of the manager
  inline static VTypedObjectFactory<TypedObjectBaseClass, IgnoreBaseClass>& GlobalManager()
  {
    return g_GlobalManager;
  }

private:
  inline void UpdateTypeCache()
  {
    if (Vision::GetTypeManager()->GetTypeCount() == m_iOverallTypeCount)
      return; // cached types are up to date

    m_iOverallTypeCount = Vision::GetTypeManager()->GetTypeCount();
    m_iTypeCount = 0;
    V_SAFE_DELETE_ARRAY(m_ppTypes);

    // count relevant types
    VPOSITION pos = Vision::GetTypeManager()->GetStartType();
    while (pos != NULL)
    {
      VType* pType = Vision::GetTypeManager()->GetNextType(pos);
      if (IsRelevantType(pType))
        m_iTypeCount++;
    }

    // cache relevant types
    if (m_iTypeCount > 0)
    {
      m_ppTypes = new VType* [m_iTypeCount];

      pos = Vision::GetTypeManager()->GetStartType();
      int iNum = 0;
      while (pos != NULL)
      {
        VType* pType = Vision::GetTypeManager()->GetNextType(pos);
        if (IsRelevantType(pType))
          m_ppTypes[iNum++] = pType;
      }
    }
  }

  inline bool IsRelevantType(const VType* pType) const
  {
    VType* pBaseType = TypedObjectBaseClass::GetClassTypeId();
    if (pType->m_pfnCreateObject == NULL || ((IgnoreBaseClass || pType != pBaseType) && !pType->IsDerivedFrom(pBaseType)))
      return false;

    return true;
  }

protected:
  int m_iOverallTypeCount;
  int m_iTypeCount;
  VType** m_ppTypes;
  static VTypedObjectFactory<TypedObjectBaseClass, IgnoreBaseClass> g_GlobalManager;
};

template<class TypedObjectBaseClass, bool IgnoreBaseClass> 
VTypedObjectFactory<TypedObjectBaseClass, IgnoreBaseClass> VTypedObjectFactory<TypedObjectBaseClass, IgnoreBaseClass>::g_GlobalManager;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// GENERIC FACTORY CLASS -end-
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

typedef VTypedObjectFactory<IVPathRenderingData> VPathRenderingDataFactory;

/////////////////////////////////////////////////////////////////////
// VPathRendererBase
/////////////////////////////////////////////////////////////////////
V_IMPLEMENT_SERIAL(VPathRendererBase, IVObjectComponent, 0, &g_VisionEngineModule);

VPathRendererBase::VPathRendererBase(int iComponentFlags) :
  IVObjectComponent(0, iComponentFlags), m_bIsInitialized(false)
{
}

VPathRendererBase::~VPathRendererBase()
{
  CommonDeinit();
}

BOOL VPathRendererBase::CanAttachToObject(VisTypedEngineObject_cl* pObject, VString& sErrorMsgOut)
{
  if (!IVObjectComponent::CanAttachToObject(pObject, sErrorMsgOut))
    return FALSE;

  if (!TryCreateRenderingData(pObject))
    return FALSE;

  return TRUE;
}

bool VPathRendererBase::TryCreateRenderingData(VisTypedEngineObject_cl* pObject)
{
  int iNumTypes = VPathRenderingDataFactory::GlobalManager().GetTypeCount();
  for (int i = 0; i < iNumTypes; i++)
  {
    IVPathRenderingData* pData = (IVPathRenderingData*)VPathRenderingDataFactory::GlobalManager().GetType(i)->CreateInstance();
    if (pData->SetPathObject(pObject))
    {
      m_spPathRenderingData = pData;
      return true;
    }

    delete pData;
  }

  return false;
}

void VPathRendererBase::CommonDeinit()
{
  if (!m_bIsInitialized)
    return;

  Vision::Callbacks.OnRenderHook -= this;
  Vision::Callbacks.OnUpdateSceneBegin -= this;
  VShadowMapGenerator::OnRenderShadowMap -= this;
  m_spPathRenderingData = NULL;
  m_bIsInitialized = false;
}

void VPathRendererBase::CommonInit()
{
  CommonDeinit();

  if (!DoInit())
    CommonDeinit();
}

void VPathRendererBase::DisposeObject()
{
  CommonDeinit();
  IVObjectComponent::DisposeObject();
}

bool VPathRendererBase::DoInit()
{
  if (m_bIsInitialized)
    return true;

  // Get the constraint chain that owns this component
  VisTypedEngineObject_cl* pOwner = GetOwner();

  if (pOwner != NULL)
  {
    if (m_spPathRenderingData == NULL)
      TryCreateRenderingData(pOwner);
    VASSERT(m_spPathRenderingData != NULL);

    if (m_spPathRenderingData != NULL)
      m_spPathRenderingData->Init(pOwner);
  }

  // Register as a rendering hook
  Vision::Callbacks.OnRenderHook += this;
  Vision::Callbacks.OnUpdateSceneBegin += this;
  VShadowMapGenerator::OnRenderShadowMap += this;

  m_bIsInitialized = true;

  return true;
}

void VPathRendererBase::OnDeserializationCallback(const VSerializationContext& context)
{
  // The owner constraint chain should now be established; we can init this 
  // renderer now.
  CommonInit();
}

void VPathRendererBase::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
  if (GetOwner() == NULL)
    return;

  if (pData->m_pSender == &Vision::Callbacks.OnRenderHook)
  {
    VisRenderHookDataObject_cl* pHookData = static_cast<VisRenderHookDataObject_cl*>(pData);
    if (pHookData->m_iEntryConst != VRH_PRE_OCCLUSION_TESTS)
      return;

    OnRender(pData);
  }
  else if (pData->m_pSender == &VShadowMapGenerator::OnRenderShadowMap)
  {
    VShadowRendererDataObject* pShadowData = (VShadowRendererDataObject*)pData;
    OnRender(pShadowData);
  }
  else if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
  {
    if (m_spPathRenderingData != NULL)
    {
      if (m_spPathRenderingData->HasDataChanged())
        OnDataChanged();
    }

    OnUpdate();
  }
}

void VPathRendererBase::OnRender(IVisCallbackDataObject_cl* pCallbackData)
{
}

void VPathRendererBase::OnUpdate()
{
}

void VPathRendererBase::OnDataChanged()
{
  if (GetOwner() != NULL && m_spPathRenderingData != NULL)
    m_spPathRenderingData->Init(GetOwner());
}

void VPathRendererBase::Serialize(VArchive& ar)
{
  IVObjectComponent::Serialize(ar);

  if (ar.IsLoading())
  {
    unsigned int iVersion = 0;
    ar >> iVersion;
    VASSERT_MSG(iVersion <= PATHRENDERER_SERIALIZATION_VERSION_CURRENT, "Invalid version of serialized data!");
  }
  else
  {
    ar << (unsigned int)PATHRENDERER_SERIALIZATION_VERSION_CURRENT;
  }
}

void VPathRendererBase::SetOwner(VisTypedEngineObject_cl* pOwner)
{
  IVObjectComponent::SetOwner(pOwner);
  CommonInit();
}

// ----------------------------------------------------------------------------
START_VAR_TABLE(VPathRendererBase, IVObjectComponent, "Base class for all path renderers", VFORGE_HIDECLASS, "Path Renderer (Base Class)" )
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
