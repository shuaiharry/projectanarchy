/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Samples/Engine/Scaleform/ScaleformPCH.h>         // precompiled header
#include <Vision/Runtime/Common/VisSampleApp.hpp> // Vision headers

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformManager.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformMovie.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformUtil.hpp>

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scene/VSceneLoader.hpp>
#include <Vision/Runtime/EnginePlugins/EnginePluginsImport.hpp>
#include <Vision/Runtime/Base/String/VStringTokenizerInPlace.hpp>

#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/VFmodManager.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Input/VVirtualThumbStick.hpp>

VIMPORT IVisPlugin_cl* GetEnginePlugin_vFmodEnginePlugin();

// ***********************************************************************************************
// ScaleformSample : Sample binding for Scaleform middleware
// Copyright (C) Havok.com Inc. All rights reserved.
// ***********************************************************************************************
// This sample shows a binding for the Scaleform middleware to playback flash movies
// or UIs  (.swf files). Key features of the binding are:
//    - Loading .swf files through the Vision filemanager (e.g. loading from ZIP)
//    - Hooking into existing renderloops so no modifications to existing renderloops have to be done
//    - Mouse control support
//    - Support for FSCommand and ExternalInterfaceCall listening
//    - movie rendering can be bound to any render contexts, e.g. monitor textures
//    - movie::Advance function runs multithreaded using the engine's task system
//
// On PC the command line argument '-rtt' shows the render-to-texture functionality. This functionality
// can be activated on other platforms by changing the default value of the 'g_bShowOnTexture' variable
// to true.
// ***********************************************************************************************

// switches for this demo
bool g_bShowOnTexture = false; // On PC: command line '-rtt', On other platforms set to true : render scaleform video into a texture rather than as a main HUD overlay
VString g_sMovieName;
VisSampleAppPtr g_spApp;

VScaleformMovieInstancePtr g_spMovie;
VScaleformValue* g_pMainMenuVar = NULL;

bool g_bMute = false;
#ifdef _VISION_MOBILE
  VisScreenMask_cl **g_ppTouchPoints;
#else
  VisScreenMaskPtr g_spMouse;
#endif

VisContextCameraPtr g_spCamera;
VisRenderContextPtr g_spContext;

// this listener is registered to the callback that receives the command callbacks and external interface calls
// alternatively a class can be derived from VScaleformMovieInstance and override the OnCommand / OnExternalInterfaceCall function
class ScaleformListener : public IVisCallbackHandler_cl
{
public:
  void OnHandleCallback(IVisCallbackDataObject_cl *pData)
  {
    if(pData->m_pSender == &VOnFSCommand::OnFSCallback)
    { 
      //this call back is marked as deprecated by scaleform
      VOnFSCommand *pCommand = (VOnFSCommand *)pData;
      
      // output to screen and log
      VString sCommand("FS COMMAND: ", 128);
      sCommand += pCommand->m_sCommand + " ARGS:" + pCommand->m_sArgs;
      Vision::Error.SystemMessage(sCommand.AsChar());
      Vision::Message.Add(1, sCommand.AsChar());
    }
    else if(pData->m_pSender == &VOnExternalInterfaceCall::OnExternalInterfaceCallback)
    {  
      VOnExternalInterfaceCall *pExternalCall = (VOnExternalInterfaceCall *)pData;

      //handle volume change
      if(pExternalCall->m_sMethodName == "SetVolume" && pExternalCall->m_uiArgCount==1)
      {
        VFmodManager::GlobalManager().SetVolumeAll(
          static_cast<float>(pExternalCall->m_ppArgs[0]->GetNumber()));
        return;
      }
      if(pExternalCall->m_sMethodName == "Mute")
      {
        g_bMute = !g_bMute;
        if(g_bMute) Vision::Message.Add(1, "Mute on");
        else Vision::Message.Add(1, "Mute off");
        VFmodManager::GlobalManager().SetMuteAll(g_bMute);
        return;
      }

      // output to screen and log
      VString sCommand("EXTERNAL CALL: ", 128);
      sCommand += pExternalCall->m_sMethodName + "(";

      for(int i=0;i<pExternalCall->m_uiArgCount;i++)
      {
        if(i!=0) sCommand += " ";
        sCommand += pExternalCall->m_ppArgs[i]->ToString();
        sCommand += ",";
      }
      sCommand[sCommand.GetLen()-1] = ')';

      Vision::Error.SystemMessage(sCommand.AsChar());
      Vision::Message.Add(1, sCommand.AsChar());
      return;
    }
  }
};

VScaleformMovieInstance* LoadMovie(const char* szMovieName)
{
  VScaleformMovieInstance* pMovie = VScaleformManager::GlobalManager().LoadMovie(szMovieName);
  if (pMovie==NULL)
  {
    Vision::Error.FatalError("Could not load movie: %s", szMovieName);
  }
  return pMovie;
}

VISION_INIT
{
  VISION_SET_DIRECTORIES(false);

  // include the vision engine plugin
  VisionAppHelpers::MakeEXEDirCurrent();
  VisSampleApp::LoadVisionEnginePlugin();

  VISION_PLUGIN_ENSURE_LOADED(vFmodEnginePlugin);

  // load the scaleform plugin:
  VISION_PLUGIN_ENSURE_LOADED(vScaleformPlugin);
  
  //the default movie
#ifdef _VISION_MOBILE
  g_sMovieName = "as3_havok_multitouch.swf";
#else
  g_sMovieName = "as2_havok.swf";
#endif
  
#ifdef WIN32
  VStaticString<1024> copyString = GetCommandLine();
  if (!copyString.IsEmpty())
  {
    // works not for spaces in the path
    VStringTokenizerInPlace tokenizer(copyString, ' ', true);
    for (unsigned int i=0;i<tokenizer.GetTokenCount();i++)
    {
      VString sArgument = tokenizer.Next();
      if(!sArgument.IsEmpty())
      {
        if (sArgument == "-rtt")
        {
          g_bShowOnTexture = true;
        }
        else if (sArgument.GetLen()>4 && (sArgument.EndsWith(".gfx")||sArgument.EndsWith(".swf")) )
        {
          g_sMovieName = sArgument;
        }
      }
    }
  }
#endif

  if (g_bShowOnTexture)
  {
    g_sMovieName = "GFx_GDC_06.swf";
  }

  // Create and init an application
  g_spApp = new VisSampleApp();

  uint64 iSampleInitFlags = VSampleFlags::VSAMPLE_INIT_DEFAULTS & ~VSampleFlags::VSAMPLE_HAVOKLOGO;
  int iEngineInitFlags = VAPP_INIT_DEFAULTS;

#ifdef _VISION_MOBILE
  iSampleInitFlags |= VSampleFlags::VSAMPLE_FORCEMOBILEMODE;
#endif

  // simple scene when rendering as a HUD overlay
  if (!g_spApp->InitSample("Maps\\ViewerMap" /*DataDir*/, "ViewerMap" /*SampleScene*/, iSampleInitFlags, VVIDEO_DEFAULTWIDTH, VVIDEO_DEFAULTHEIGHT, iEngineInitFlags))
    return false;

  return true;
}

VisBaseEntity_cl* pEntity = NULL;
VisMouseCamera_cl* pMouseCamera = NULL;

const int TOGGLE_MOUSE_MODE = 1+VISION_INPUT_CONTROL_LAST_ELEMENT;
const int TOGGLE_BORDERLESS_FULLSCREEN_MODE = 2+VISION_INPUT_CONTROL_LAST_ELEMENT;

bool bMouseModeChanged = true;
bool bPseudoFullScreen = false;

ScaleformListener *pListener = NULL;

bool bEditable = false;

int iCurrentXRes = VVIDEO_DEFAULTWIDTH;
int iCurrentYRes = VVIDEO_DEFAULTHEIGHT;

VISION_SAMPLEAPP_AFTER_LOADING
{  
  g_spApp->AddSampleDataDir("Scaleform");

  // Create a mouse controlled camera (optionally with gravity)
  g_spApp->EnableMouseCamera();

  // register a listener to the command callback (optional)
  pListener = new ScaleformListener();
  VOnFSCommand::OnFSCallback += *pListener;
  VOnExternalInterfaceCall::OnExternalInterfaceCallback += *pListener;

  pMouseCamera = (VisMouseCamera_cl*)g_spApp->GetMouseCamera();

#if defined(SUPPORTS_MULTITOUCH)
  if (pMouseCamera->GetVirtualThumbStick() != NULL)
    pMouseCamera->GetVirtualThumbStick()->Hide();
#endif

#ifdef _VISION_MOBILE

  if (!g_bShowOnTexture)
  {
    int iMaxTouchPoints = VScaleformManager::GlobalManager().GetMaxTouchPoints();
    g_ppTouchPoints = new VisScreenMask_cl*[iMaxTouchPoints];

    for(int i=0;i<iMaxTouchPoints;i++)
    {
      g_ppTouchPoints[i] = new VisScreenMask_cl("GUI/circle.dds");
      VSimpleRenderState_t renderState(VIS_TRANSP_ALPHA);
      renderState.SetFlags(RENDERSTATEFLAG_FRONTFACE|RENDERSTATEFLAG_FILTERING);
      g_ppTouchPoints[i]->SetRenderState(renderState);
      g_ppTouchPoints[i]->SetVisible(true);
      g_ppTouchPoints[i]->SetPos(-100,-100); //start ouside the visible area
      g_ppTouchPoints[i]->SetColor(VColorRef(255/((i%4)+1), (25*i)%255, 255/(i/3+1))); //randomize colors
    
      //rescale the texture a little bit, so that we get some smoother edges through the filtering
      int iWidth, iHeight;
      g_ppTouchPoints[i]->GetTextureSize(iWidth, iHeight);
      g_ppTouchPoints[i]->SetTargetSize(iWidth-2, iHeight-2);
    }
  }
#else
  // display our own mouse cursor (optional)
  g_spMouse = new VisScreenMask_cl("mouseNorm.TGA");
  g_spMouse->SetTransparency(VIS_TRANSP_ALPHA);
  g_spMouse->SetVisible(true);
#endif

  pMouseCamera->SetThinkFunctionStatus(false);

  /*
  //Sample code for associating a subset of an input map with scaleform's key events:
  VInputMap *pInputMap = spApp->GetInputMap();
  const int iTriggerID[] = {FOV_INCREASE,FOV_DECREASE};
  const int iGFxKey[] = {VScaleformManager::SF_KEY_UP, VScaleformManager::SF_KEY_DOWN};
  const int iTriggerCount = sizeof(iTriggerID)/sizeof(iTriggerID[0]); // 2 elements
  // FOV_INCREASE/DECREASE (KEYPAD +/-) will be mapped to VScaleformManager::SF_KEY_UP/DOWN (GFxKey::Up and GFxKey::Down)
  VScaleformManager::GlobalManager().SetKeyMapping(pInputMap, iTriggerCount, iTriggerID, iGFxKey);
  */

  //optional code for rendering the GUI into a texture that is used as a monitor texture:
  if (g_bShowOnTexture)
  {
    // apply to simple rotating entity
    pEntity = Vision::Game.CreateEntity("VisBaseEntity_cl",hkvVec3(30.f,0,-10.f),"\\Models\\MissingModel.MODEL");
    pEntity->SetScaling(1.6f);
    VDynamicMesh *pMesh = pEntity->GetMesh();

    VisSurface_cl* pSurface = pMesh->GetSurfaceByName("blinn1");
    VASSERT(pSurface);
    
    pSurface->m_spDiffuseTexture = Vision::TextureManager.Load2DTexture(g_sMovieName);
  }
  else
  {
    // create a movie instance to playback
    g_spMovie = LoadMovie(g_sMovieName.AsChar());

    if(g_sMovieName=="havok.swf")
    {
      g_spApp->AddHelpText ("");
      g_spApp->AddHelpText ("Adjust the sliders and watch the result");
      g_spApp->AddHelpText ("");
    }

    g_pMainMenuVar = g_spMovie->GetVariable("_root.mainMenu");
  }
  

#if defined(SUPPORTS_KEYBOARD)
  g_spApp->GetInputMap()->MapTrigger(TOGGLE_MOUSE_MODE,                 V_KEYBOARD, CT_KB_PAUSE, VInputOptions::Once());
  g_spApp->GetInputMap()->MapTrigger(TOGGLE_BORDERLESS_FULLSCREEN_MODE, V_KEYBOARD, CT_KB_PGDN,  VInputOptions::Once());
  g_spApp->GetInputMap()->MapTrigger(TOGGLE_BORDERLESS_FULLSCREEN_MODE, V_KEYBOARD, CT_KB_PGUP,  VInputOptions::Once());
  g_spApp->AddHelpText ("");
  g_spApp->AddHelpText ("PAUSE - Toggle Mouse Cursor / Mouse Look");
#ifdef WIN32
  g_spApp->AddHelpText ("Page Up/Down - FullScreen/Window mode");
  g_spApp->AddHelpText ("");
  g_spApp->AddHelpText ("Try to start the sample with command line parameter -rtt or -scifi");
  #endif
#endif
#if defined(_VISION_PS3) || defined(_VISION_XENON)
  g_spApp->GetInputMap()->MapTrigger(TOGGLE_MOUSE_MODE, VInputManager::GetPad(0), CT_PAD_LEFT_SHOULDER, VInputOptions::Once());
  g_spApp->AddHelpText ("PAD1 - Left Trigger : Show/Hide Mouse Cursor");
#endif
}

VISION_SAMPLEAPP_RUN
{
  if(g_spApp->Run())
  {
    #if defined(_VISION_PS3) || defined(_VISION_XENON)
      Vision::Message.Print(1, 10, 20, "Press \"PAD1 - Left Shoulder\" to toggle camera / mouse cursor control");
    #endif
    
    if (pEntity)
    {
      pEntity->IncOrientation(Vision::GetTimer()->GetTimeDifference()*10.f,0,0);
    }
    
    #ifndef _VISION_MOBILE
      if(g_spApp->GetInputMap()->GetTrigger(TOGGLE_MOUSE_MODE))
      {
        g_spMouse->SetVisible(! g_spMouse->IsVisible() );
        bool bMouseInput = g_spMouse->IsVisible()==TRUE;
        VScaleformManager::GlobalManager().SetHandleCursorInput(bMouseInput);
        pMouseCamera->SetThinkFunctionStatus(!bMouseInput);
      }
    #endif
    
    #ifdef WIN32
    if(g_spApp->GetInputMap()->GetTrigger(TOGGLE_BORDERLESS_FULLSCREEN_MODE))
    {
      if(bPseudoFullScreen)
      {
        //Gets the default resolution
        iCurrentXRes = VVIDEO_DEFAULTWIDTH;
        iCurrentYRes = VVIDEO_DEFAULTHEIGHT;
        bPseudoFullScreen = false;
      }
      else
      {
        //Gets the full screen resolution, note that we'll change resolution only.(It's not the real fullscreen mode)
        DEVMODEA deviceMode;
        deviceMode = Vision::Video.GetAdapterMode(g_spApp->m_appConfig.m_videoConfig.m_iAdapter);
        iCurrentXRes = deviceMode.dmPelsWidth;
        iCurrentYRes = deviceMode.dmPelsHeight;
        bPseudoFullScreen = true;
      }

      if(!g_bShowOnTexture)
      {
        int iOrigX=0, iOrigY=0;
        g_spMovie->GetAuthoredSize(iOrigX,iOrigY);
        VASSERT(iOrigX!=0&&iOrigY!=0);
        float fRatio = (float)iOrigX/(float)iOrigY;

        if(iCurrentXRes/fRatio>iCurrentYRes)
        {
          if(g_sMovieName=="havok.swf")
          { 
            //do not "over scale" for the havok movie, otherwise
            //controls are outside the reachable area
            g_spMovie->SetPosition((int)((iCurrentXRes-iCurrentYRes*fRatio)*0.5f),0);
            g_spMovie->SetSize((int)(iCurrentYRes*fRatio), iCurrentYRes);
          }
          else
          {
            //in this case the window is wider than the move
            //(avoid border by scaling the movie up)
            g_spMovie->SetPosition(0,(int)(((float)iCurrentXRes/fRatio-iCurrentYRes)*-0.5f));
            g_spMovie->SetSize(iCurrentXRes, (int)(iCurrentXRes/fRatio));
          }
        }
        else
        {
          g_spMovie->SetPosition(0,0);
          g_spMovie->SetSize(iCurrentXRes, iCurrentYRes);
        }
      }

      VisionWindowManager::GetActiveWindow()->GetConfig().SetBorderlessWindow(bPseudoFullScreen);       
      Vision::Video.ChangeScreenResolution(0,0, iCurrentXRes, iCurrentYRes, false, 0);
    }
    #endif

    #ifdef _VISION_MOBILE
    if (!g_bShowOnTexture)
    {
      for(int i=0;i<VScaleformManager::GlobalManager().GetMaxTouchPoints();i++)
      {
        float fX, fY;
        int iWidth, iHeight;
        int iTextX, iTextY;
      
        VScaleformManager::GlobalManager().GetCursorPos(fX,fY,i);
        g_ppTouchPoints[i]->GetTextureSize(iWidth, iHeight);
      
        if(VInputManager::GetTouchScreen().IsActiveTouch(i))
        {
          g_ppTouchPoints[i]->SetPos(fX-iWidth/2, fY-iHeight/2); //update the position when active
          iTextX = fX-iWidth*0.65f;
          iTextY = fY-iHeight*0.65f; //display the number outside for active touch points
        }
        else
        {
          iTextX = fX-iWidth/12;
          iTextY = fY-iHeight/8; //display the number inside for in-active touch points
        }
      
        Vision::Message.SetTextColor(V_RGBA_BLACK);
        Vision::Message.Print(1, iTextX+1, iTextY+1, "%d", i); 
        Vision::Message.SetTextColor(V_RGBA_WHITE);
        Vision::Message.Print(1, iTextX, iTextY, "%d", i);
      }
    }
    #else
      if(!pMouseCamera->GetThinkFunctionStatus())
      {
        float x,y;
        VScaleformManager::GlobalManager().GetCursorPos(x,y);
        g_spMouse->SetPos(x,y);

        if (g_pMainMenuVar!=NULL)
        {
          g_pMainMenuVar->Display_SetXYRotation((iCurrentYRes*0.5f-y)*-0.07f, (iCurrentXRes*0.5f-x)*0.07f);
        }
      }
    #endif
    

    return true;
  } 
  return false;
}

VISION_DEINIT
{
  // de-register
  VOnFSCommand::OnFSCallback -= *pListener;
  VOnExternalInterfaceCall::OnExternalInterfaceCallback -= *pListener;

  V_SAFE_DELETE(pListener);

  #ifdef _VISION_MOBILE
    V_SAFE_DELETE_ARRAY(g_ppTouchPoints);
  #else
    g_spMouse = NULL; 
  #endif
  
  // delete the movie instance  
  g_pMainMenuVar = NULL;
  g_spMovie = NULL; 

  // deinit (only relevant if bShowOnTexture was true)
  g_spCamera = NULL;
  g_spContext = NULL;

  // Deinit the application
  g_spApp->DeInitSample();
  g_spApp = NULL;
  Vision::Plugins.UnloadAllEnginePlugins();

  return 0;
}

VISION_MAIN_DEFAULT

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
