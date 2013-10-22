/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/FmodEnginePlugin.hpp>
#include <Vision/Runtime/Engine/SceneElements/VisApiTriggerComponent.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scripting/VScriptManager.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/VFmodManager.hpp>

#include <Vision/Runtime/Base/System/Memory/VMemDbg.hpp>


// static variables
VFmodManager VFmodManager::g_GlobalManager;
int VFmodManager::PROFILING_FMOD_OVERALL = 0;
int VFmodManager::PROFILING_FMOD_PUREUPDATE = 0;
int VFmodManager::PROFILING_FMOD_PURGE = 0;


// extern variables
extern "C" int luaopen_FireLight(lua_State *);


#define _DEBUG_OUTPUT
#undef _DEBUG_OUTPUT


// -------------------------------------------------------------------------- //
// Constructor/ Destructor                                                 
// -------------------------------------------------------------------------- //
VFmodManager::VFmodManager() 
{
  m_bAnyStopped = false;
  m_pListenerObject = NULL;

  m_pSoundResourceManager = NULL;
  m_pEventGroupManager = NULL;

  m_pEventSystem = NULL;
  m_pSystem = NULL;
  pMemoryPool = NULL;
  m_pMasterGroup = NULL;
  m_pMusicGroup = NULL;

  m_fTimeLeftOver = 0.0f;
}

VFmodManager::~VFmodManager()
{
}


// -------------------------------------------------------------------------- //
// Init/ Deinit                                                 
// -------------------------------------------------------------------------- //
VFmodManager& VFmodManager::GlobalManager() 
{
  return g_GlobalManager;
}

void VFmodManager::OneTimeInit()
{
  if (!PROFILING_FMOD_OVERALL)
  {
    PROFILING_FMOD_OVERALL = Vision::Profiling.GetFreeElementID();
    PROFILING_FMOD_PUREUPDATE = Vision::Profiling.GetFreeElementID();
    PROFILING_FMOD_PURGE = Vision::Profiling.GetFreeElementID();

    Vision::Profiling.AddGroup("Fmod");
    VProfilingNode *pOverall = Vision::Profiling.AddElement(PROFILING_FMOD_OVERALL, "Overall", TRUE);
      VProfilingNode *pUpdate = Vision::Profiling.AddElement(PROFILING_FMOD_PUREUPDATE, "  Pure update", TRUE, pOverall);  
        Vision::Profiling.AddElement(PROFILING_FMOD_PURGE, "  Purge instances", TRUE, pOverall);
  }

  Vision::RegisterModule(&g_FmodModule);

  m_pSoundResourceManager = new VFmodSoundResourceManager();
  Vision::ResourceSystem.RegisterResourceManager(m_pSoundResourceManager, VColorRef(0,255,100));
  m_pEventGroupManager = new VFmodEventGroupManager();
  Vision::ResourceSystem.RegisterResourceManager(m_pEventGroupManager, VColorRef(255,255,100));

  Vision::Callbacks.OnEngineInit += this;
  Vision::Callbacks.OnEngineDeInit += this;
  Vision::Callbacks.OnWorldInit += this;
  Vision::Callbacks.OnWorldDeInit += this;
  Vision::Callbacks.OnUpdateSceneFinished += this;
 
  IVScriptManager::OnRegisterScriptFunctions += this;
  IVScriptManager::OnScriptProxyCreation += this;

  // give the application a chance to modify the m_config
  VFmodConfigCallbackData config(&OnBeforeInitializeFmod, m_config);
  OnBeforeInitializeFmod.TriggerCallbacks(&config);
}

void VFmodManager::OneTimeDeInit()
{
  Vision::UnregisterModule(&g_FmodModule);

  Vision::ResourceSystem.UnregisterResourceManager(m_pSoundResourceManager);
  V_SAFE_DELETE(m_pSoundResourceManager);
  Vision::ResourceSystem.UnregisterResourceManager(m_pEventGroupManager);
  V_SAFE_DELETE(m_pEventGroupManager);

  Vision::Callbacks.OnEngineInit -= this;
  Vision::Callbacks.OnEngineDeInit -= this;
  Vision::Callbacks.OnWorldInit -= this;
  Vision::Callbacks.OnWorldDeInit -= this;
  Vision::Callbacks.OnUpdateSceneFinished -= this;

  IVScriptManager::OnRegisterScriptFunctions -= this;
  IVScriptManager::OnScriptProxyCreation -= this;
}

bool VFmodManager::IsInitialized() const
{
  return true;
}

void VFmodManager::InitFmodSystem()
{
  IVisCallbackDataObject_cl dataAfter(&OnAfterDeinitializeFmod);
  OnAfterDeinitializeFmod.TriggerCallbacks(&dataAfter);
}

// -------------------------------------------------------------------------- //
// Resources                                                
// -------------------------------------------------------------------------- //
VFmodSoundResource* VFmodManager::LoadSoundResource(const char *szFilename, int iUsageFlags)
{
  VASSERT(szFilename!=NULL);

  // create a new resource (unique for streaming resources)
  VFmodSoundResource* pRes = new VFmodSoundResource(m_pSoundResourceManager, iUsageFlags);
  pRes->SetFilename(szFilename);
  pRes->EnsureLoaded();
  return pRes;
}

FMOD::EventProject* VFmodManager::LoadEventProject(const char *szEventProjectPath)
{
  VASSERT(szEventProjectPath!=NULL/* && m_pEventSystem!=NULL*/);

  return (FMOD::EventProject*)0;
}

VFmodEventGroup* VFmodManager::LoadEventGroup(const char *szEventProjectPath, const char *szEventGroupName)
{
  VASSERT(szEventProjectPath!=NULL && szEventGroupName!=NULL);

  VFmodEventGroup *pEventGroup = NULL;
 
  // create a new event group
  pEventGroup = new VFmodEventGroup(m_pEventGroupManager);
  pEventGroup->SetFilename("NullEventGroup");
  pEventGroup->EnsureLoaded();

  return pEventGroup;
}


// -------------------------------------------------------------------------- //
// Instances                                                
// -------------------------------------------------------------------------- //
VFmodSoundObject* VFmodManager::CreateSoundInstance(const char *szFilename, int iResourceUsageFlags, int iInstanceFlags, int iPriority)
{
  VFmodSoundResource *pRes = LoadSoundResource(szFilename, iResourceUsageFlags);
  if (!pRes)
    return NULL;

  // Note: Create all sound objects here as paused as we need to set AND update the
  // correct distance and attenuation first! Call Play() afterwards it set.
  VFmodSoundObject* pObj = pRes->CreateInstance(hkvVec3(0.0f,0.0f,0.0f), iInstanceFlags | VFMOD_FLAG_PAUSED, iPriority);
  return pObj;
}

VFmodEvent* VFmodManager::CreateEvent(const char *szEventProjectPath, const char *szEventName, int iFlags)
{
  VFmodEvent* pEvent = new VFmodEvent();
  return pEvent;
}


// -------------------------------------------------------------------------- //
// Channel groups                                               
// -------------------------------------------------------------------------- //
void VFmodManager::SetVolumeAll(float fVol)
{
}

void VFmodManager::SetMuteAll(bool bStatus)
{
}

void VFmodManager::SetPauseAll(bool bStatus)
{
}


// -------------------------------------------------------------------------- //
// Misc                                           
// -------------------------------------------------------------------------- //
void VFmodManager::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
  if (pData->m_pSender==&Vision::Callbacks.OnEngineInit)
  {
    return;
  }

  if(pData->m_pSender == &Vision::Callbacks.OnEngineDeInit)
  {
    // release all resources before the engine deinitializes
    m_soundInstances.Clear(); 
    m_events.Clear();
    m_collisionMeshes.Clear();
    m_reverbs.Clear();

    m_bAnyStopped = false;

    return;
  }

  if (pData->m_pSender==&Vision::Callbacks.OnUpdateSceneFinished)
  {
    RunTick(Vision::GetTimer()->GetTimeDifference());
    return;
  }

  if (pData->m_pSender==&Vision::Callbacks.OnWorldInit)
  {
    if (!IsInitialized())
      return;

    //VASSERT(m_pSystem!=NULL);
    return;
  }

  if (pData->m_pSender==&Vision::Callbacks.OnWorldDeInit)
  {
    return;
  }

  if (pData->m_pSender==&IVScriptManager::OnScriptProxyCreation)
  {
    VScriptCreateStackProxyObject * pScriptData = (VScriptCreateStackProxyObject *)pData;

    //process data only as far as not handled until now
    if (!pScriptData->m_bProcessed)
    {
      int iRetParams = 0;
      if (pScriptData->m_pInstance->IsOfType(V_RUNTIME_CLASS(VFmodSoundObject)))
        iRetParams = LUA_CallStaticFunction(pScriptData->m_pLuaState,"FireLight","VFmodSoundObject","Cast","O>O",pScriptData->m_pInstance);
      else if (pScriptData->m_pInstance->IsOfType(V_RUNTIME_CLASS(VFmodEvent)))
        iRetParams = LUA_CallStaticFunction(pScriptData->m_pLuaState,"FireLight","VFmodEvent","Cast","O>O",pScriptData->m_pInstance);

      if (iRetParams>0)
      {
        if(lua_isnil(pScriptData->m_pLuaState, -1))   
          lua_pop(pScriptData->m_pLuaState, iRetParams);
        else                                         
          pScriptData->m_bProcessed = true;
      }
    }
    return;
  }

  if(pData->m_pSender==&IVScriptManager::OnRegisterScriptFunctions)
  {
    IVScriptManager* pSM = Vision::GetScriptManager();
    if (pSM)
    {
      lua_State* pLuaState = ((VScriptResourceManager*)pSM)->GetMasterState();
      if(pLuaState)
      {
        lua_getglobal(pLuaState, "Fmod");
        int iType = lua_type(pLuaState, -1);
        lua_pop(pLuaState, 1);

        if(iType!=LUA_TUSERDATA)
        {
          luaopen_FireLight(pLuaState);
          int iRetParams = LUA_CallStaticFunction(pLuaState,"FireLight","VFmodManager","Cast","v>v", &VFmodManager::GlobalManager());
          if (iRetParams==1)
          {
            if(lua_isnil(pLuaState, -1))
            {
              lua_pop(pLuaState, iRetParams);
            }
            else
            {
              lua_setglobal(pLuaState, "Fmod");
              return;
            }
          }
        }
        else
        {
          return; //already loaded
        }
      }
      
      Vision::Error.Warning("Unable to create Lua Fmod Module, lua_State is NULL or cast failed!");
    }
    return;
  }
}

void VFmodManager::RunTick(float fTimeDelta)
{
  if (!IsInitialized())
    return;

  VISION_PROFILE_FUNCTION(PROFILING_FMOD_OVERALL);

  // profiling scope
  {
    VISION_PROFILE_FUNCTION(PROFILING_FMOD_PUREUPDATE);
  } 
  
  // do not purge sounds/ events in vForge, in order to allow toggling playback via hotspot button
  if (Vision::Editor.IsInEditor())
    return;  

  if (m_bAnyStopped)
  {
    VISION_PROFILE_FUNCTION(PROFILING_FMOD_PURGE);
  }
}

bool VFmodManager::GetMemoryStats(int *pCurrentMemSize, int *pMaxMemSize, bool bBlocking) const
{

  *pCurrentMemSize = 0;
  *pMaxMemSize = 42;
  
  return true;
}

bool VFmodManager::SetAmbientReverbProperties(VFmodReverbProps &properties)
{
  return true;
}

int VFmodManager::GetActiveSourceVoiceCount() const
{
 return 0;
}

int VFmodManager::GetExistingSourceVoiceCount() const
{ 
  return 0;
}

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
