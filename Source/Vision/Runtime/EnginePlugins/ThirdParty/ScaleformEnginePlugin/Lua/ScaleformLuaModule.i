// ===========================================================
// Scaleform Lua Module - SWIG Interface
// ===========================================================
// This interface file generates ScaleformLuaModule_wrapper.cpp
// and is represented in Lua as the "GFx" (Scaleform) module.

#ifndef VLUA_APIDOC

//get access to all vision specific things...
%include <windows.i>
%include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scripting/Lua/vision_types.i>

//custom headers for generated source file
%module GFx
%{
  // GFx includes
  #include "GFx/GFx_Player.h"
  #include "GFx/GFx_Loader.h"
  #include "GFx/GFx_Log.h"
  #include "Render/Renderer2D.h"
  #include "Kernel/SF_Threads.h"

  #if defined (WIN32) && defined(_VR_DX9)
    #include "GFx_Renderer_D3D9.h"
  #elif defined(WIN32) && defined(_VR_DX11)
    #include "GFx_Renderer_D3D1x.h"
  #elif defined (_VISION_XENON)
    #include "Render/X360/X360_HAL.h"
  #elif defined (_VISION_PS3)
    #include "GFx_Renderer_PS3.h"
  #elif defined (_VISION_MOBILE)
    #include "GFx_Renderer_GL.h"
  #endif

  #ifdef SF_AMP_SERVER
    #include "GFx_AMP.h"
  #endif

  //vision includes
  #include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformAlloc.hpp>
  #include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformManager.hpp>
  #include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformMovie.hpp>
  #include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformInternal.hpp>
  #include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformUtil.hpp>
%}
  
%include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/Lua/vScaleformManager.i>;
%include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/Lua/vScaleformMovie.i>;
%include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/Lua/VScaleformUtil.i>;

#endif