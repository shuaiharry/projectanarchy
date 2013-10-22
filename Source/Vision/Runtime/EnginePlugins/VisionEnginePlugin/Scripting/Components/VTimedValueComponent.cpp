/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/VisionEnginePluginPCH.h>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scripting/VScriptIncludes.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scripting/Components/VTimedValueComponent.hpp>

//for every callback in the VTimedValueComponent we create a VTimedValueComponent
// to remember the callback and the time when to call it.
class VTimedValueComponent::VTimedLuaCallback
{
  friend class VTimedValueComponent;

public:
  VTimedLuaCallback(VTimedValueComponent *pParent, VString sCallbackName, float fTimeFrom0To1) :
    m_sCallbackName(sCallbackName)
      , m_fInvokationTime(fTimeFrom0To1)
      , m_bExecuted(false)
      , m_pParent(pParent)
    {};

    //default constructor for dyn array
    VTimedLuaCallback() :
    m_sCallbackName()
      , m_fInvokationTime(1)
      , m_bExecuted(false)
      , m_pParent(NULL)
    {};

    inline void ExecuteOnDemand(float fCurrentTimeFrom0To1)
    {
      if(!m_bExecuted && fCurrentTimeFrom0To1>m_fInvokationTime)
        Execute();
    };

    inline void Reset()
    {
      m_bExecuted = false;
    };

protected:

  inline void Execute()
  {
    //re-scan script script components to find out if a new one has been attached
    if(!m_pParent->EvaluateAttachedScripts(true))
      return;

    const unsigned int numParentScripts = m_pParent->m_parentScripts.GetValidSize();

    for(unsigned int i=0; i<numParentScripts; i++)
    {
      if(m_pParent->m_parentScripts[i]!=NULL)
      {
        VScriptInstance *pInst = m_pParent->m_parentScripts[i]->GetScriptInstance();
        if(pInst!=NULL)
          pInst->ExecuteFunctionArg(m_sCallbackName, "*");
      }
    }
    m_bExecuted = true;
  };

  VString m_sCallbackName;
  float m_fInvokationTime;
  bool m_bExecuted;
  VTimedValueComponent * m_pParent;
};

VTimedValueComponent::VTimedValueComponent(int iComponentFlags /* = VIS_OBJECTCOMPONENTFLAG_NOSERIALIZE */) :
    IVObjectComponent(0, iComponentFlags)
  , m_fFromValue(0)
  , m_fToValue(10)
  , m_fCurrentTime(0)
  , m_fCurrentTimeFrom0To1(0)
  , m_fDuration(10)
  , m_bPaused(false)
  , m_bSceneRunning(false)
  , m_iCachedComponentCount(0)
  , m_luaCallbacks(2, NULL)
  , m_parentScripts(2, NULL)
{
  Vision::Callbacks.OnAfterSceneLoaded += this;
  Vision::Callbacks.OnBeforeSceneUnloaded += this;
  Vision::Callbacks.OnScriptThink += this;
}

VTimedValueComponent::~VTimedValueComponent()
{
  Vision::Callbacks.OnScriptThink -= this;
  Vision::Callbacks.OnBeforeSceneUnloaded -= this;
  Vision::Callbacks.OnAfterSceneLoaded -= this;
  
  m_bPaused  = true;
  m_bSceneRunning = false;

  const unsigned int numCallbacks = m_luaCallbacks.GetValidSize();
  for(unsigned int i=0; i<numCallbacks; i++)
  {
    V_SAFE_DELETE(m_luaCallbacks[i]);
  }
  m_luaCallbacks.Reset();
  m_parentScripts.Reset();
}

void VTimedValueComponent::Resume()
{
  m_bPaused = false;
  m_bSceneRunning = true;
}

void VTimedValueComponent::Stop()
{
  m_bPaused = true;
  m_fCurrentTime = 0;
  m_fCurrentTimeFrom0To1 = 0;
}

void VTimedValueComponent::Pause()
{
  m_bPaused = false;
}

void VTimedValueComponent::Reset()
{
  m_fCurrentTime = 0;
  m_fCurrentTimeFrom0To1 = 0;
}

void VTimedValueComponent::SetRange(float fFromValue, float fToValue, bool bReset)
{
  if(bReset)
    Reset();

  m_fFromValue = fFromValue;
  m_fToValue = fToValue;
}

void VTimedValueComponent::SetDuration(float fDuration)
{
  m_fDuration = fDuration;
}

float VTimedValueComponent::GetValue()
{
  return hkvMath::interpolate(m_fFromValue, m_fToValue, m_fCurrentTimeFrom0To1);
}

unsigned int VTimedValueComponent::GetNumCallbacks()
{
  return m_luaCallbacks.GetValidSize();
}

bool VTimedValueComponent::EvaluateAttachedScripts(bool bUseCachingHeuristic)
{
  if(m_pOwner==NULL)
    return false;

  int iCurrentComponentCount = m_pOwner->Components().Count();

  //in case the number of components stayed the same we assume that no new script component has been removed or attached
  if(bUseCachingHeuristic && m_iCachedComponentCount==iCurrentComponentCount)
    return true;

  m_iCachedComponentCount = iCurrentComponentCount;

  //maybe there are multiple scripts attached...
  m_parentScripts.Reset();
  for(int i=0;i<iCurrentComponentCount; i++)
  {
    VScriptComponent *pComp = vdynamic_cast<VScriptComponent*>(m_pOwner->Components().GetAt(i));
    if( pComp!=NULL )
    {
      m_parentScripts[m_parentScripts.GetFreePos()] = pComp;
    } 
  }

  return true;
}

void VTimedValueComponent::AddCallback(const char * szCallbackName, float fCallTime /*= -1 */)
{
  const float fRelativeTime = fCallTime < 0 ? 1 : fCallTime / m_fDuration;
  m_luaCallbacks[m_luaCallbacks.GetFreePos()] = new VTimedLuaCallback(this, szCallbackName, fRelativeTime);
}

void VTimedValueComponent::Serialize( VArchive &ar )
{
  char iLocalVersion = 0;
  IVObjectComponent::Serialize(ar);
  Vision::Error.Warning("VTimedValueComponent is not intended to be (de-)serialized. It should only be created by Lua script.");
  if (ar.IsLoading())
  {
    ar >> iLocalVersion;
    VASSERT_MSG(iLocalVersion == 0, "Invalid local version. Please re-export");
  } 
  else
  {
    ar << iLocalVersion;
  }
}

void VTimedValueComponent::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
  if(pData->m_pSender==&Vision::Callbacks.OnScriptThink)
  {
    if(!m_bSceneRunning||m_bPaused)
      return;

    //update timer
    m_fCurrentTime += Vision::GetTimer()->GetTimeDifference();
    m_fCurrentTimeFrom0To1 = m_fCurrentTime/m_fDuration;

    const unsigned int numCallbacks = m_luaCallbacks.GetValidSize();

    bool bReset = m_fCurrentTime>m_fDuration;
    
    for(unsigned int i=0;i<numCallbacks;i++)
    {
      if(m_luaCallbacks[i]!=NULL)
      {
        m_luaCallbacks[i]->ExecuteOnDemand(m_fCurrentTimeFrom0To1);
        if(bReset)
          m_luaCallbacks[i]->Reset();
      }
    }

    if(bReset)
      m_fCurrentTime -= m_fDuration;
    
    return;
  }
  else if(pData->m_pSender==&Vision::Callbacks.OnAfterSceneLoaded)
  {   
    Reset();
    m_bSceneRunning = true;
    return;
  }
  else if(pData->m_pSender==&Vision::Callbacks.OnBeforeSceneUnloaded)
  {
    m_bSceneRunning = false;
    return;
  }
}

V_IMPLEMENT_SERIAL(VTimedValueComponent, IVObjectComponent, 0, Vision::GetEngineModule() );

START_VAR_TABLE(VTimedValueComponent, IVObjectComponent, "Lua TimedValue component. Let a timer call arbitrary Lua callbacks.", VFORGE_HIDECLASS|VCOMPONENT_ALLOW_MULTIPLE, "Lua TimedValue Component" )
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
