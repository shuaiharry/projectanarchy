/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Samples/Engine/RPG/RPGPCH.h>
#include <Vision/Samples/Engine/RPGPlugin/Precompiled.h>
#include <Vision/Samples/Engine/RPGPlugin/GameManager.h>

IVisPlugin_cl* GetEnginePlugin_RPGPlugin();

enum GUI_DEMO_CONTROL 
{
  GUI_DEMO_CONTINUE = VISION_INPUT_CONTROL_LAST_ELEMENT+1
};

enum GUIState
{
  GS_INIT,
  GS_GAME
};

GUIState g_state = GS_INIT;

VisSampleAppPtr g_spApp;
VisScreenMaskPtr g_spSplashScreen = NULL;
VisScreenMaskPtr g_spBackground = NULL;
VisScreenMaskPtr g_spLogoHavok = NULL;
VisScreenMaskPtr g_spLogoWMD = NULL;

#if defined(_VISION_MOBILE)
  VTouchAreaPtr g_spDemoTapArea = NULL;
#endif

#define LOGOHAVOK_FILENAME "GUI\\Textures\\Logo_Havok_128x64.tga"
#define LOGOWMD_FILENAME "GUI\\Textures\\Logo_WMD_128x256.tga"
#define INTROSCREEN_FILENAME "GUI\\Textures\\Anarchy_Intro_512x512.tga"
#define GAMEOVERSCREEN_FILENAME "GUI\\Textures\\Anarchy_GameOver_512x512.tga"
#define LOADINGSCREEN_FILENAME "GUI\\Textures\\Anarchy_Splash_512x512.tga"
#define SCENE_FILENAME "Scenes\\Temple\\Temple.vscene"

void LoadSplashScreen(const char* filename, const bool showLogo)
{
  const float fWidth = (float) Vision::Video.GetXRes();
  const float fHeight = (float) Vision::Video.GetYRes();

  g_spBackground = new VisScreenMask_cl("plainwhite.dds");
  g_spBackground->SetTargetSize(fWidth, fHeight);
  g_spBackground->SetPos(0.f, 0.f, 0.f);
  g_spBackground->SetColor(V_RGBA_BLACK);
  g_spBackground->SetOrder(500);

  g_spSplashScreen = new VisScreenMask_cl(filename);
  g_spSplashScreen->SetDepthWrite(FALSE);
  g_spSplashScreen->SetTransparency (VIS_TRANSP_ALPHA);
  g_spSplashScreen->SetOrder(200);

  if (fWidth > fHeight)
  {
    g_spSplashScreen->SetTargetSize(fHeight, fHeight);
    g_spSplashScreen->SetPos((fWidth - fHeight) * 0.5f, 0.f);
  }
  else
  {
    g_spSplashScreen->SetTargetSize(fWidth, fWidth);
    g_spSplashScreen->SetPos(0.f, (fHeight - fWidth) * 0.5f);
  }

  // Display logos
  if (showLogo)
  {
    int xSize, ySize;
    g_spLogoHavok = new VisScreenMask_cl(LOGOHAVOK_FILENAME);
    g_spLogoHavok->SetDepthWrite(FALSE);
    g_spLogoHavok->GetTextureSize(xSize, ySize);
    g_spLogoHavok->SetTargetSize((float) xSize,(float)  ySize);
    g_spLogoHavok->SetPos(fWidth - xSize - 20, fHeight - ySize - 10);

    g_spLogoWMD = new VisScreenMask_cl(LOGOWMD_FILENAME);
    g_spLogoWMD->SetDepthWrite(FALSE);
    g_spLogoWMD->GetTextureSize(xSize, ySize);
    g_spLogoWMD->SetTargetSize((float) xSize,(float)  ySize);
    g_spLogoWMD->SetPos(20, fHeight - ySize - 10);
  }
}

VISION_INIT
{
  g_state = GS_INIT;

  // setup directories, does nothing on platforms other than iOS, 
  // pass true if you want load from the documents directory
  VISION_SET_DIRECTORIES(false);

  // Create an application
  g_spApp = new VisSampleApp(); 
  g_spApp->LoadVisionEnginePlugin();
  
  VISION_PLUGIN_ENSURE_LOADED(RPGPlugin);

  // Init the application
  if (!g_spApp->InitSample("RPG" /*DataDir*/, NULL, VSampleFlags::VSAMPLE_INIT_DEFAULTS & ~VSampleFlags::VSAMPLE_HAVOKLOGO))
    return false;
  
  // Disable automatic use of 2X assets on smaller screens even the density is pretty high. 
  // For instance, on the HTC One X (720x1280) the buttons are just too big when using the 
  // 2x version, even when the dpi is relatively high.
  if (Vision::Video.GetYRes() < 1000)
	Vision::Video.SetAllowAutomaticUseOf2xAssets(false);

  return true;
}

VISION_SAMPLEAPP_AFTER_LOADING
{
  if (g_state == GS_INIT)
  {
    Vision::InitWorld();

    // Intro screen
    LoadSplashScreen(INTROSCREEN_FILENAME, true);

#if defined(SUPPORTS_KEYBOARD)
    VisSampleApp::GetInputMap()->MapTrigger(GUI_DEMO_CONTINUE, V_KEYBOARD, CT_KB_ANYKEY);
    
    // Help text
    g_spApp->AddHelpText("");
    g_spApp->AddHelpText("Controls for this demo (PC-Version):");
    g_spApp->AddHelpText("");
    g_spApp->AddHelpText("[LEFT MOUSE BUTTON] : Move character or do a simple attack");
    g_spApp->AddHelpText("[LEFT MOUSE BUTTON] + [SHIFT] : Throw a magic axe");
    g_spApp->AddHelpText("[RIGHT MOUSE BUTTON] : Hit the axe to the ground (consumes Mana)");
    g_spApp->AddHelpText("[MIDDLE MOUSE BUTTON] : Swing the axe around character (consumes Mana)");
#elif defined(_VISION_MOBILE)
    g_spDemoTapArea = new VTouchArea(VInputManager::GetTouchScreen(), VRectanglef(0.0f, 0.0f, Vision::Video.GetXRes(), Vision::Video.GetYRes()));
    g_spApp->GetInputMap()->MapTrigger(GUI_DEMO_CONTINUE, g_spDemoTapArea, CT_TOUCH_ANY);
#endif
  }
}

VISION_SAMPLEAPP_RUN
{
  if  (g_state == GS_INIT)
  {
    if (g_spApp->GetInputMap()->GetTrigger(GUI_DEMO_CONTINUE))
    {
      // Reset intro screen
      g_spSplashScreen = NULL;
      g_spBackground = NULL;
      g_spLogoHavok = NULL;
      g_spLogoWMD = NULL;

      // Custom loading screen for RPG demo
      VisSampleApp::LoadingScreenSettings settings;
      settings.fFadeOutTime = 2.0f;
      settings.backgroundColor = VColorRef(35, 31, 32);
      settings.eAspectRatioAlignment = VisSampleApp::LoadingScreenSettings::ALIGN_VERTICAL;
      settings.bForceFullScreenTexture = true;
      settings.sBackgroundImagePath = LOADINGSCREEN_FILENAME;
      g_spApp->SetupLoadingScreen(true, settings);

      // Load scene
      g_spApp->LoadScene(SCENE_FILENAME);
      g_state = GS_GAME;
    }
  }
  else if (g_state == GS_GAME)
  {
    if (RPG_GameManager::s_instance.IsGameOver())
    {
      // Unload scene
      g_spApp->ClearScene();

      // Show end load screen  
      LoadSplashScreen(GAMEOVERSCREEN_FILENAME, true);
      g_state = GS_INIT;
    }
  }

  return g_spApp->Run();
}

VISION_DEINIT
{
  g_spSplashScreen = NULL;
  g_spBackground = NULL;
  g_spLogoHavok = NULL;
  g_spLogoWMD = NULL;
#if defined(_VISION_MOBILE)
  g_spDemoTapArea = NULL;
#endif

  // Deinit the application
  g_spApp->UnloadScene();
  g_spApp->DeInitSample();
  g_spApp = NULL;
  return true;
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
