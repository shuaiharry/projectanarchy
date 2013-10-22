/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/ScaleformEnginePlugin.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scripting/VScriptIncludes.hpp>

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformManager.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformTexture.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformAlloc.hpp>

#include "Kernel/SF_System.h"
#include <Vision/Runtime/Base/System/Memory/VMemDbg.hpp>

#if defined(GFX_ENABLE_SOUND)
  // needed for VISION_PLUGIN_ENSURE_LOADED(vFmodEnginePlugin);
  VIMPORT IVisPlugin_cl* GetEnginePlugin_vFmodEnginePlugin();
#endif

// extern variables
extern "C" int luaopen_GFx(lua_State *);

//helper method
static void EnsureScaleformScriptRegistration()
{
  if (Vision::IsInitialized() || Vision::IsInitializing())
  {
    IVScriptManager* pScriptManager = Vision::GetScriptManager();
    if(pScriptManager!=NULL)
    {
      lua_State* pLuaState = ((VScriptResourceManager*)pScriptManager)->GetMasterState();
      if(pLuaState!=NULL)
      {
        lua_getglobal(pLuaState, "Scaleform");
        int iType = lua_type(pLuaState, -1);
        lua_pop(pLuaState, 1);

        if(iType!=LUA_TUSERDATA)
        {
          luaopen_GFx(pLuaState);
          int iRetParams = LUA_CallStaticFunction(pLuaState,"GFx","VScaleformManager","Cast","v>v", 
            &VScaleformManager::GlobalManager());
          if (iRetParams>0)
          {
            if(lua_isnil(pLuaState, -1))
            {
              lua_pop(pLuaState, iRetParams);
            }
            else
            {
              lua_setglobal(pLuaState, "Scaleform");
              return;
            }
          }
        }
        else
        {
          return; //already loaded
        }
      }

      Vision::Error.Warning("Unable to create Lua Scaleform Module, lua_State is NULL");
    }
  }
}

//the plugin class
class vScaleformPlugin_cl : public IVisPlugin_cl, public IVisCallbackHandler_cl
{
public:
  virtual void OnInitEnginePlugin() HKV_OVERRIDE;
  virtual void OnDeInitEnginePlugin() HKV_OVERRIDE;
  
  virtual VExportReferenceBehavior GetExportBehavior() HKV_OVERRIDE
  {
    if (VScaleformTexture::GetResourceCount() > 0)
      return VEXPORTREFERENCE_ALWAYS;

    // don't use default "when used" because then it does not work when used in plugins
    return VEXPORTREFERENCE_USER_SELECT; 
  }

  virtual const char *GetPluginName() HKV_OVERRIDE
  {
    return "vScaleformPlugin";
  }

  virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

private:
  VScaleformSysAlloc m_ScaleformAllocator;
};

//globals
vScaleformPlugin_cl g_ScaleformPlugin;
VScaleformTextureFormatProvider g_ScaleformFormatProvider;

// plugin module
DECLARE_THIS_MODULE( g_ScaleformModule, MAKE_VERSION(1, 0), "Scaleform", "Havok", "Scaleform engine binding", &g_ScaleformPlugin );

// Use this to get and initialize the plugin when you link statically
VEXPORT IVisPlugin_cl* GetEnginePlugin_vScaleformPlugin()
{
  return &g_ScaleformPlugin;
}



#if ((defined _DLL) || (defined _WINDLL)) && !defined(VBASE_LIB)
//  The engine uses this to get and initialize the plugin dynamically
VEXPORT IVisPlugin_cl* GetEnginePlugin()
{
  return &g_ScaleformPlugin;
}

#endif // _DLL or _WINDLL

// initialize the plugin
void vScaleformPlugin_cl::OnInitEnginePlugin()
{
#if defined(GFX_ENABLE_SOUND)
  VISION_PLUGIN_ENSURE_LOADED(vFmodEnginePlugin);
#endif

  Scaleform::System::Init(&m_ScaleformAllocator);

  VScaleformManager::GlobalManager();

  Vision::RegisterModule(&g_ScaleformModule);
  Vision::TextureManager.GetManager().RegisterFormatProvider(&g_ScaleformFormatProvider);

  // Register for the script registration callback.
  IVScriptManager::OnRegisterScriptFunctions += this;

  // Execute script registration function in case the plugin was
  // added too late and the registration callback already occurred.
  EnsureScaleformScriptRegistration();
}

// de-initialize the plugin
void vScaleformPlugin_cl::OnDeInitEnginePlugin()
{
  IVScriptManager::OnRegisterScriptFunctions -= this;

  Vision::TextureManager.GetManager().DeRegisterFormatProvider(&g_ScaleformFormatProvider);
  Vision::UnregisterModule(&g_ScaleformModule);
  
  VScaleformManager::GlobalManager().DeInit();
  
  Scaleform::System::Destroy();
}

void vScaleformPlugin_cl::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
  if (pData->m_pSender == &IVScriptManager::OnRegisterScriptFunctions)
  {
    EnsureScaleformScriptRegistration();
    return;
  }
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
