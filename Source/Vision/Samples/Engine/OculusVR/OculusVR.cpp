/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

// ************************************************************************************************
// OculusVR Sample
// Copyright (C) Havok.com Inc. All rights reserved.
// ************************************************************************************************
// This sample shows how to implement two concurrent render contexts to achieve stereoscopic 3D 
// rendering on an Oculus Rift HMD.
// In the sample code, two rendering contexts are created with their own context camera each.
//
// Both contexts render to a render target texture. These two textures are distorted and combined in 
// the main context's render loop in order to generate a stereoscopic image in the HMD.
//
// HMD sensor information is read and a virtual head is positioned accordingly on a user-controllable
// base.
//
// To build this sample, install the Oculus SDK v0.2.3 and create an environment variable
// OCULUS_SDK_ROOT, which points to the OculusSDK directory.
// 
// ************************************************************************************************

#include <Vision/Samples/Engine/OculusVR/OculusVRPCH.h>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Entities/PathCameraEntity.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/Postprocessing/ToneMapping.hpp>

#if defined( HK_ANARCHY )
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/MobileForwardRenderer/VMobileForwardRenderer.hpp>
#else
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/ForwardRenderer/ForwardRenderer.hpp>
#endif

#include <OVR.h>

#include "HmdRenderLoop.hpp"
#include "HmdManager.hpp"


#define DATA_DIR    "Maps\\SciFi"
#define SCENE_FILE  "SciFi_Small_v01e"
//#define SCENE_FILE  "Crossing"
#define STARTING_POSITION hkvVec3( -300.0f, 0.0f, 180.0f )

//#define DATA_DIR    "FantasyMobile"
//#define SCENE_FILE  "DemoAdvanced"
//#define STARTING_POSITION hkvVec3( 0.0f, 0.0f, 200.0f )

//#define DATA_DIR    "Maps\\VillageMap"
//#define SCENE_FILE  "Scenes\\VillageMap_Mobile"
//#define STARTING_POSITION hkvVec3( 0.0f, 0.0f, 500.0f )


VisRenderContextPtr g_spContextP1;
VisRenderContextPtr g_spContextP2;


enum VISION_CONTROL 
{
  VISION_INC_EYE_SEPARATION = VISION_INPUT_CONTROL_LAST_ELEMENT+1,
  VISION_DEC_EYE_SEPARATION,
  VISION_RESET_EYE_SEPARATION,
  VISION_POST_PROCESS_NOWARP,
  VISION_POST_PROCESS_WARP,
  VISION_POST_PROCESS_WARP_CHROMATICABERRATION,
  VISION_RESET_SENSOR,
  VISION_TOGGLE_MOVEMODE,
};


// Some sample scenes create an automatic path camera, which doesn't work with
// our multi-context approach to rendering stereoscopic images, so we'll have to
// find and deactivate them.
void DeactivatePathCameraEntities()
{
  for (unsigned int i=0;i<VisBaseEntity_cl::ElementManagerGetSize();i++)
  {
    VisBaseEntity_cl *pEnt = VisBaseEntity_cl::ElementManagerGet(i);
    if ( pEnt && pEnt->GetTypeId() == PathCameraEntity::GetClassTypeId() )
    {
      PathCameraEntity* pPathCam = ( PathCameraEntity* ) pEnt;
      pPathCam->Stop();
    }
  }
}

void SyncHMDSettings( HmdRenderLoop* pRenderLoop, HMDManager* pHMDManager, VisContextCamera_cl* pLeftEyeCam, VisContextCamera_cl* pRightEyeCam )
{
  OVR::Util::Render::StereoConfig* pStereoConfig = pHMDManager->GetStereoConfig();

  if ( pStereoConfig->GetStereoMode() == OVR::Util::Render::Stereo_LeftRight_Multipass )
  {
    float fNear = g_spContextP1->GetViewProperties()->getNear();
    float fFar = g_spContextP1->GetViewProperties()->getFar();

    // Retrieve eye parameters for the left eye.
    // (As the whole things is rather symmetric, we can use this info, invert values where appropriate, and use them for the right eye as well.)

    // Retrieve the projection matrix and copy it into a hkvMat4.
    const OVR::Util::Render::StereoEyeParams& eyeParamsLeft = pStereoConfig->GetEyeRenderParams( OVR::Util::Render::StereoEye_Left );
    hkvMat4 m4Projection( hkvNoInitialization );
    m4Projection.set( (float*)&eyeParamsLeft.Projection.M, hkvMat4::RowMajor );

    // Adjust the OculusVR projection matrix to work with Vision:
    // - Invert the third column.
    // - Adjust near and far ranges. (As of LibOVR 0.2.3 they are hardwired to 0.01 and 1000.0, respectively.)
    m4Projection.m_Column[2][0] = -m4Projection.m_Column[2][0];
    m4Projection.m_Column[2][2] = fFar / ( fFar - fNear ); // -m4Projection.m_Column[2][2];
    m4Projection.m_Column[2][3] = 1.0f;
    m4Projection.m_Column[3][2] = ( fFar * fNear ) / ( fNear - fFar );
    g_spContextP1->SetCustomProjectionMatrix( &m4Projection );

    // Invert the element at [2][0] again to get the projection matrix for the right eye.
    m4Projection.m_Column[2][0] = -m4Projection.m_Column[2][0];
    g_spContextP2->SetCustomProjectionMatrix( &m4Projection );

    // Retrieve information for adjusting the eye position based on eye separation.
    float fHalfEyeSeparation = eyeParamsLeft.ViewAdjust.M[ 0 ][ 3 ];   // Note: LibOVR stores matrices in row-major order.

    // This is the minimal head model that's used in the OculusVR samples as well.
    const float fEyeProtrusion = 9.0f;                        // That would be 9cm.
    const float fEyeHeight = 15.0f;                           // That's 15cm.
    const float fEyeOffset = fHalfEyeSeparation * 100.0f;     // Converting LibOVRs data from meters to centimeters. Also: Note that local y points to the user's left.
    pLeftEyeCam->SetLocalPosition( fEyeProtrusion, fEyeOffset, fEyeHeight );
    pRightEyeCam->SetLocalPosition( fEyeProtrusion, -fEyeOffset, fEyeHeight );

    // Determine post-processing parameters.
    const OVR::Util::Render::DistortionConfig* pDistortion = eyeParamsLeft.pDistortion;
    hkvVec4 v4LensCenter_ScreenCenter( 0.5f + pDistortion->XCenterOffset * 0.5f, 0.5f, 0.5f, 0.5f );
    float fScaleFactor = 1.0f / pDistortion->Scale;
    float fAspectRatio = 0.5f * Vision::Video.GetXRes() / float( Vision::Video.GetYRes() );
    hkvVec4 v4Scale_ScaleIn( 0.5f * fScaleFactor, 0.5f * fScaleFactor * fAspectRatio, 2.0f, 2.0f / fAspectRatio );
    hkvVec4 v4HmdWarpParameters( pDistortion->K[ 0 ], pDistortion->K[ 1 ], pDistortion->K[ 2 ], pDistortion->K[ 3 ] );
    hkvVec4 v4ChromaticAberration( pDistortion->ChromaticAberration[ 0 ], pDistortion->ChromaticAberration[ 1 ], pDistortion->ChromaticAberration[ 2 ], pDistortion->ChromaticAberration[ 3 ] );
    pRenderLoop->SetPostProcessParameters( v4LensCenter_ScreenCenter, v4Scale_ScaleIn, v4HmdWarpParameters, v4ChromaticAberration );
  }
}

VISION_MAIN
{
  // Initialize LibOVR.
  OVR::System::Init( OVR::Log::ConfigureDefaultLog( OVR::LogMask_All ) );

  { // We use a new scope to make sure that the HMD manager is destroyed before OVR is deinitialized.

    // Initialize the HMD manager and retrieve information about our current setup.
    HMDManager hmdManager;
    int iWidth = 0, iHeight = 0;
    float fResolutionMultiplier = 1.0f;
    if ( !hmdManager.Init( iWidth, iHeight, fResolutionMultiplier ) )
    {
      return -1;
    }

    // Include the vision engine plugin.
    VisionAppHelpers::MakeEXEDirCurrent();
    VisSampleApp::LoadVisionEnginePlugin();

    VISION_PLUGIN_ENSURE_LOADED( vHavok );
    VISION_PLUGIN_ENSURE_LOADED( vFmodEnginePlugin );

    // Create and init an application.
    VisSampleAppPtr spApp = new VisSampleApp();

    uint64 uiSampleFlags = VSampleFlags::VSAMPLE_INIT_DEFAULTS & ~(VSampleFlags::VSAMPLE_HAVOKLOGO);

    uiSampleFlags |= VSampleFlags::VSAMPLE_FORCEFULLSCREEN; 
    uiSampleFlags &= ~VSampleFlags::VSAMPLE_ASKFULLSCREEN;    // Rendering for the HMD requires fullscreen rendering - however, for debugging it might be useful to comment this line out.

    if (!spApp->InitSample( DATA_DIR, SCENE_FILE, uiSampleFlags, iWidth, iHeight))
    {
      return -1;
    }

    // Deactivate all path camera entities, if present.
    DeactivatePathCameraEntities();

    // Setup some triggers for controlling eye separation, post-processing mode, etc.
    VisSampleApp::GetInputMap()->MapTrigger(VISION_TOGGLE_MOVEMODE, V_PC_KEYBOARD, CT_KB_F, VInputOptions::Once() );
    VisSampleApp::GetInputMap()->MapTrigger(VISION_RESET_SENSOR, V_PC_KEYBOARD, CT_KB_R, VInputOptions::Once() );
    VisSampleApp::GetInputMap()->MapTrigger(VISION_POST_PROCESS_NOWARP, V_PC_KEYBOARD, CT_KB_1, VInputOptions::Once() );
    VisSampleApp::GetInputMap()->MapTrigger(VISION_POST_PROCESS_WARP, V_PC_KEYBOARD, CT_KB_2, VInputOptions::Once() );
    VisSampleApp::GetInputMap()->MapTrigger(VISION_POST_PROCESS_WARP_CHROMATICABERRATION, V_PC_KEYBOARD, CT_KB_3, VInputOptions::Once() );
    VisSampleApp::GetInputMap()->MapTrigger(VISION_INC_EYE_SEPARATION, V_PC_KEYBOARD, CT_KB_C);
    VisSampleApp::GetInputMap()->MapTrigger(VISION_DEC_EYE_SEPARATION, V_PC_KEYBOARD, CT_KB_X);
    VisSampleApp::GetInputMap()->MapTrigger(VISION_RESET_EYE_SEPARATION, V_PC_KEYBOARD, CT_KB_V);

    // Unmap debug shading mode key - we need the key and it doesn't work in this context anyway.
    VisSampleApp::GetInputMap()->UnmapInput(SHOW_NEXT_DEBUG_SHADING_MODE);

    // define help text
    spApp->AddHelpText("");
    spApp->AddHelpText("How to use this demo :");
    spApp->AddHelpText("");
    spApp->AddHelpText("Move around using the Mouse and the Arrow Keys or WASD,");
    spApp->AddHelpText("or use a game pad (where the triggers control camera elevation).");
    spApp->AddHelpText("Camera elevation can be modified via 'Q' and 'E'.");
    spApp->AddHelpText("Press 'F' to toggle the movement mode (i.e., camera movement relative to HMD orientation)." );
    spApp->AddHelpText("Press 'R' to reset the HMD motion sensor. (Looking straight forward is a good idea here.)" );
    spApp->AddHelpText("Press '1' to switch to no-warp HMD post processing mode." );
    spApp->AddHelpText("Press '2' to switch to warp HMD post processing mode." );
    spApp->AddHelpText("Press '3' to switch to warp HMD post processing mode with correction for chromatic aberration." );
    spApp->AddHelpText("Press 'X' to decrease the eye-distance.");
    spApp->AddHelpText("Press 'C' to increase the eye-distance.");
    spApp->AddHelpText("Press 'V' to reset the eye distance." );

    // Determine render target size (taking HMD optics warp into account).
    int iSizeX = int( ceil( 0.5f * fResolutionMultiplier * Vision::Video.GetXRes() ) );
    int iSizeY = int( ceil(        fResolutionMultiplier * Vision::Video.GetYRes() ) );

    // Setup a render contexts and targets for the left and right eye each.
    VisRenderableTextureConfig_t renderTargetConfig, depthStencilConfig;

    renderTargetConfig.m_iWidth  = depthStencilConfig.m_iWidth  = iSizeX;
    renderTargetConfig.m_iHeight = depthStencilConfig.m_iHeight = iSizeY;

    renderTargetConfig.m_eFormat = VTextureLoader::DEFAULT_RENDERTARGET_FORMAT_32BPP;
    depthStencilConfig.m_eFormat = VTextureLoader::D24S8;
    depthStencilConfig.m_bIsDepthStencilTarget = true;
    depthStencilConfig.m_bRenderTargetOnly = true;

    // Note: These FOV settings are preliminary. Proper synchronization between context cameras and HMD settings is done later on.
    const float fFovX = 90.f;
    const float fFovY = hkvMathHelpers::getFovY( fFovX, float( iSizeX ) / float( iSizeY ) );

    VisContextCameraPtr spCameraLeft;
    VisContextCameraPtr spCameraRight;
    VSmartPtr< VisBaseEntity_cl > spCameraBase;
    VisRenderableTexturePtr m_spRenderTarget1;
    VisRenderableTexturePtr m_spRenderTarget2;
    VisRenderableTexturePtr m_spDepthStencilTarget1;
    VisRenderableTexturePtr m_spDepthStencilTarget2;
    VisMouseCamera_cl *pMouseCamera = NULL;;
    HmdRenderLoop *pCombineRenderLoop = NULL;

    // Use visible bitmask of Vision::Message to restrict text rendering to the main context (mainly for FPS display).
    const unsigned int uiMessageVisibilityBitmask = 0x00000010;
    Vision::Message.SetVisibleBitmask( uiMessageVisibilityBitmask );
    spApp->SetShowFrameRate( true );

    {
      // Left eye setup.
      spCameraLeft = new VisContextCamera_cl();
      g_spContextP1 = new VisRenderContext_cl(spCameraLeft, fFovX, fFovY, iSizeX, iSizeY, 5.f, 30000.f, VIS_RENDERCONTEXT_FLAGS_SECONDARYCONTEXT);
      g_spContextP1->SetName( "LeftEyeContext" );
      g_spContextP1->SetRenderFilterMask( g_spContextP1->GetRenderFilterMask() & ~uiMessageVisibilityBitmask );

      m_spRenderTarget1 = Vision::TextureManager.CreateRenderableTexture("<LeftEyeTarget>", renderTargetConfig);
      m_spDepthStencilTarget1 = Vision::TextureManager.CreateRenderableTexture("<LeftEyeDS>", depthStencilConfig);

      g_spContextP1->SetRenderTarget(0, m_spRenderTarget1);
      g_spContextP1->SetDepthStencilTarget(m_spDepthStencilTarget1);

#if defined( HK_ANARCHY )
      VMobileForwardRenderingSystem *pRenderer1 = new VMobileForwardRenderingSystem( g_spContextP1 );
#else
      VForwardRenderingSystem *pRenderer1 = new VForwardRenderingSystem(g_spContextP1, false, false);
#endif

      VPostProcessToneMapping *pToneMapper1 = new VPostProcessToneMapping(TONEMAP_SCALE, 1.2f, 1.2f, 0.0f);
      pRenderer1->AddComponent(pToneMapper1);
      pRenderer1->InitializeRenderer();
      Vision::Renderer.SetRendererNode(0, pRenderer1);
    }

    {
      // Right eye setup.
      spCameraRight = new VisContextCamera_cl();
      g_spContextP2 = new VisRenderContext_cl(spCameraRight, fFovX, fFovY, iSizeX, iSizeY, 5.f, 30000.f, VIS_RENDERCONTEXT_FLAGS_SECONDARYCONTEXT);
      g_spContextP2->SetName( "RightEyeContext" );
      g_spContextP2->SetRenderFilterMask( g_spContextP2->GetRenderFilterMask() & ~uiMessageVisibilityBitmask );

      m_spRenderTarget2 = Vision::TextureManager.CreateRenderableTexture("<RightEyeTarget>", renderTargetConfig);
      m_spDepthStencilTarget2 = Vision::TextureManager.CreateRenderableTexture("<RightEyeDS>", depthStencilConfig);

      g_spContextP2->SetRenderTarget(0, m_spRenderTarget2);
      g_spContextP2->SetDepthStencilTarget(m_spDepthStencilTarget2); 

#if defined( HK_ANARCHY )
      VMobileForwardRenderingSystem *pRenderer2 = new VMobileForwardRenderingSystem( g_spContextP2 );
#else
      VForwardRenderingSystem *pRenderer2 = new VForwardRenderingSystem(g_spContextP2, false, false);
#endif

      VPostProcessToneMapping *pToneMapper2 = new VPostProcessToneMapping(TONEMAP_SCALE, 1.2f, 1.2f, 0.0f);
      pRenderer2->AddComponent(pToneMapper2);
      pRenderer2->InitializeRenderer();
      Vision::Renderer.SetRendererNode(1, pRenderer2);
    }

    {
      // Main context setup.

      // Avoid processing scene elements for the main context (e.g. visibility, mirrors, ...)
      int iFlags = Vision::Contexts.GetMainRenderContext()->GetRenderFlags();
      iFlags &= ~(VIS_RENDERCONTEXT_FLAG_NO_SCENEELEMENTS|VIS_RENDERCONTEXT_FLAG_VIEWCONTEXT);
      Vision::Contexts.GetMainRenderContext()->SetRenderFlags(iFlags);
      Vision::Contexts.GetMainRenderContext()->SetVisibilityCollector(NULL); // we don't need visibility in the main context at all
      Vision::Contexts.GetMainRenderContext()->SetPriority(VIS_RENDERCONTEXTPRIORITY_DISPLAY);
      Vision::Contexts.GetMainRenderContext()->SetRenderFilterMask( Vision::Contexts.GetMainRenderContext()->GetRenderFilterMask() | uiMessageVisibilityBitmask );

      pCombineRenderLoop = new HmdRenderLoop( m_spRenderTarget1, m_spRenderTarget2 );
      Vision::Contexts.GetMainRenderContext()->SetRenderLoop( pCombineRenderLoop );

      Vision::Contexts.AddContext( Vision::Contexts.GetMainRenderContext() );
    }  

    // Create a player object (in the form of a mouse camera) to allow for moving throught the scene.
    {
      pMouseCamera = (VisMouseCamera_cl *) spApp->EnableMouseCamera();
      VASSERT(pMouseCamera);
      pMouseCamera->SetPosition( STARTING_POSITION );

      // Create a camera base entity and attach it to the player object for some very basic head modeling.
      spCameraBase = Vision::Game.CreateEntity( "VisBaseEntity_cl", hkvVec3::ZeroVector() );
      spCameraBase->AttachToParent( pMouseCamera );
      spCameraBase->SetLocalPosition( hkvVec3::ZeroVector() );
      spCameraBase->SetLocalOrientation( hkvVec3::ZeroVector() );

      // Make the camera controllable via WASD as well
      VInputMap* pInputMap = pMouseCamera->GetInputMap();
      pInputMap->MapTrigger(API_CAMERA_MOVE_FORWARD,  V_KEYBOARD, CT_KB_W);
      pInputMap->MapTrigger(API_CAMERA_MOVE_BACKWARD, V_KEYBOARD, CT_KB_S);
      pInputMap->MapTrigger(API_CAMERA_MOVE_LEFT,     V_KEYBOARD, CT_KB_A);
      pInputMap->MapTrigger(API_CAMERA_MOVE_RIGHT,    V_KEYBOARD, CT_KB_D);
      pInputMap->MapTrigger(API_CAMERA_MOVE_UP,       V_KEYBOARD, CT_KB_E);
      pInputMap->MapTrigger(API_CAMERA_MOVE_DOWN,     V_KEYBOARD, CT_KB_Q);

      // Remove the vertical look if we have a valid HMD sensor.
      if ( hmdManager.GetSensor() != NULL )
      {
        pInputMap->UnmapInput( API_CAMERA_VERTICAL_LOOK );
      }

      // Also, hook up the game pad
      pInputMap->MapTrigger(API_CAMERA_MOVE_FORWARD,  VInputManagerPC::GetPad(0), CT_PAD_LEFT_THUMB_STICK_UP,    VInputOptions::DeadZone(0.25f));
      pInputMap->MapTrigger(API_CAMERA_MOVE_BACKWARD, VInputManagerPC::GetPad(0), CT_PAD_LEFT_THUMB_STICK_DOWN,  VInputOptions::DeadZone(0.25f));
      pInputMap->MapTrigger(API_CAMERA_MOVE_LEFT,     VInputManagerPC::GetPad(0), CT_PAD_LEFT_THUMB_STICK_LEFT,  VInputOptions::DeadZone(0.25f));
      pInputMap->MapTrigger(API_CAMERA_MOVE_RIGHT,    VInputManagerPC::GetPad(0), CT_PAD_LEFT_THUMB_STICK_RIGHT, VInputOptions::DeadZone(0.25f));
      pInputMap->MapTrigger(API_CAMERA_MOVE_UP,       VInputManagerPC::GetPad(0), CT_PAD_RIGHT_TRIGGER,          VInputOptions::DeadZone(0.25f));
      pInputMap->MapTrigger(API_CAMERA_MOVE_DOWN,     VInputManagerPC::GetPad(0), CT_PAD_LEFT_TRIGGER,           VInputOptions::DeadZone(0.25f));

      pInputMap->MapTriggerAxis(API_CAMERA_HORIZONTAL_LOOK, VInputManagerPC::GetPad(0), CT_PAD_RIGHT_THUMB_STICK_LEFT, CT_PAD_RIGHT_THUMB_STICK_RIGHT, VInputOptions::DeadZone(0.15f, true));
      if ( hmdManager.GetSensor() == NULL )
      {
        // Again, only allow vertical look control if we don't have a valid HMD sensor.
        pInputMap->MapTriggerAxis(API_CAMERA_VERTICAL_LOOK,   VInputManagerPC::GetPad(0), CT_PAD_RIGHT_THUMB_STICK_UP,   CT_PAD_RIGHT_THUMB_STICK_DOWN,  VInputOptions::DeadZone(0.15f, true));
      }

      pInputMap->MapTrigger(API_CAMERA_ANY_ACTION,   VInputManagerPC::GetPad(0), CT_PAD_ANY_KEY);
      pInputMap->MapTrigger(API_CAMERA_ANY_ACTION,   VInputManagerPC::GetPad(0), CT_PAD_LEFT_THUMB_STICK_CHANGED);
      pInputMap->MapTrigger(API_CAMERA_LOOK_CHANGED, VInputManagerPC::GetPad(0), CT_PAD_RIGHT_THUMB_STICK_CHANGED);

      pInputMap->MapTrigger(VISION_RESET_SENSOR, VInputManagerPC::GetPad(0), CT_PAD_Y, VInputOptions::Once());
      pInputMap->MapTrigger(VISION_TOGGLE_MOVEMODE, VInputManagerPC::GetPad(0), CT_PAD_X, VInputOptions::Once());

      // Attach the two cameras (left eye, right eye) to this player object, with a relative offset to the left and right
      spCameraLeft->AttachToEntity(spCameraBase, hkvVec3 (0,0,0));
      spCameraRight->AttachToEntity(spCameraBase, hkvVec3 (0,0,0));
    }

    // Keep track of original settings.
    const float fOriginalInterpupillaryDistance = hmdManager.GetStereoConfig()->GetIPD();
    float fLastYaw = 0.0f;
    {
      // Retrieve HMD motion sensor data and apply it to the head model.
      OVR::SensorFusion* pFusion = hmdManager.GetSensorFusion();
      OVR::Quatf hmdOrientation = pFusion->GetOrientation();
      float pitch, roll;
      hmdOrientation.GetEulerAngles< OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z >( &fLastYaw, &pitch, &roll );
    }

    bool bMoveRelativeToViewDirection = true;

    // Sync HMD settings to the cameras and the final render loop.
    SyncHMDSettings( pCombineRenderLoop, &hmdManager, spCameraLeft, spCameraRight );

    // Enter the application loop.
    while ( spApp->Run() ) 
    {
      {
        // Retrieve HMD motion sensor data and apply it to the head model.
        OVR::SensorFusion* pFusion = hmdManager.GetSensorFusion();
        OVR::Quatf hmdOrientation = pFusion->GetOrientation();
        float yaw, pitch, roll;
        hmdOrientation.GetEulerAngles< OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z >( &yaw, &pitch, &roll );
        float fDeltaYaw = yaw - fLastYaw;
        fLastYaw = yaw;
        pitch = hkvMath::Rad2Deg( pitch );
        roll = hkvMath::Rad2Deg( roll );

        if ( bMoveRelativeToViewDirection )
        {
          pMouseCamera->IncOrientation( hkvMath::Rad2Deg( fDeltaYaw ), 0.0f, 0.0f );
          spCameraBase->SetLocalOrientation( 0.0f, -pitch, -roll );
        }
        else
        {
          yaw = hkvMath::Rad2Deg( yaw );
          spCameraBase->SetLocalOrientation( yaw, -pitch, -roll );
        }
      }

      // Process user input.
      if (VisSampleApp::GetInputMap()->GetTrigger(VISION_INC_EYE_SEPARATION))
      {
        float fIPD = hmdManager.GetStereoConfig()->GetIPD();
        fIPD += 0.025f * Vision::GetTimer()->GetTimeDifference();
        hmdManager.GetStereoConfig()->SetIPD( fIPD );
        SyncHMDSettings( pCombineRenderLoop, &hmdManager, spCameraLeft, spCameraRight );
        Vision::Message.Add( 1, "New IPD: %f", fIPD );
      }
      if (VisSampleApp::GetInputMap()->GetTrigger(VISION_DEC_EYE_SEPARATION))
      {
        float fIPD = hmdManager.GetStereoConfig()->GetIPD();
        fIPD -= 0.025f * Vision::GetTimer()->GetTimeDifference();
        hmdManager.GetStereoConfig()->SetIPD( fIPD );
        SyncHMDSettings( pCombineRenderLoop, &hmdManager, spCameraLeft, spCameraRight );
        Vision::Message.Add( 1, "New IPD: %f", fIPD );
      }
      if ( VisSampleApp::GetInputMap()->GetTrigger( VISION_POST_PROCESS_NOWARP ) )
      {
        HmdRenderLoop::HMDPostProcess ePostProcess = HmdRenderLoop::HP_NoWarp;
        pCombineRenderLoop->SetCurrentPostProcess( ePostProcess );
        Vision::Message.Add( 1, "Changed HMD Post-Processing to '%s'", HmdRenderLoop::GetPostProcessString( ePostProcess ) );
      }
      if ( VisSampleApp::GetInputMap()->GetTrigger( VISION_POST_PROCESS_WARP ) )
      {
        HmdRenderLoop::HMDPostProcess ePostProcess = HmdRenderLoop::HP_Warp;
        pCombineRenderLoop->SetCurrentPostProcess( ePostProcess );
        Vision::Message.Add( 1, "Changed HMD Post-Processing to '%s'", HmdRenderLoop::GetPostProcessString( ePostProcess ) );
      }
      if ( VisSampleApp::GetInputMap()->GetTrigger( VISION_POST_PROCESS_WARP_CHROMATICABERRATION ) )
      {
        HmdRenderLoop::HMDPostProcess ePostProcess = HmdRenderLoop::HP_WarpChromaticAberration;
        pCombineRenderLoop->SetCurrentPostProcess( ePostProcess );
        Vision::Message.Add( 1, "Changed HMD Post-Processing to '%s'", HmdRenderLoop::GetPostProcessString( ePostProcess ) );
      }
      if ( VisSampleApp::GetInputMap()->GetTrigger( VISION_RESET_SENSOR ) )
      {
        Vision::Message.Add( 1, "Resetting HMD motion sensor" );

        hmdManager.GetSensorFusion()->Reset();
        hkvVec3 v3CamYPR = spCameraBase->GetLocalOrientation();
        pMouseCamera->IncOrientation( v3CamYPR.x, 0.0f, 0.0f ); // keep the view direction the same
      }
      if ( VisSampleApp::GetInputMap()->GetTrigger( VISION_TOGGLE_MOVEMODE ) )
      {
        bMoveRelativeToViewDirection = !bMoveRelativeToViewDirection;
        Vision::Message.Add( 1, "Movement relative to view direction: '%s'", bMoveRelativeToViewDirection ? "on" : "off" );

        // Reset the camera base and the sensor.
        hmdManager.GetSensorFusion()->Reset();
        hkvVec3 v3CamYPR = spCameraBase->GetLocalOrientation();
        spCameraBase->SetLocalOrientation( 0.0f, v3CamYPR.y, v3CamYPR.z );
        pMouseCamera->IncOrientation( v3CamYPR.x, 0.0f, 0.0f );

      }
      if ( VisSampleApp::GetInputMap()->GetTrigger( VISION_RESET_EYE_SEPARATION ) )
      {
        hmdManager.GetStereoConfig()->SetIPD( fOriginalInterpupillaryDistance );
        SyncHMDSettings( pCombineRenderLoop, &hmdManager, spCameraLeft, spCameraRight );
        Vision::Message.Add( 1, "Reset IPD to %f", fOriginalInterpupillaryDistance );
      }
    }

    // clean-up, destroy all objects
    g_spContextP1 = NULL;
    g_spContextP2 = NULL;
    spCameraLeft = NULL;
    spCameraRight = NULL;
    spCameraBase = NULL;
    Vision::Contexts.GetMainRenderContext()->SetRenderLoop(NULL); // destroy VCombine3DScreenRenderLoop, which has references to render targets

    m_spRenderTarget1 = NULL;
    m_spDepthStencilTarget1 = NULL;
    m_spRenderTarget2 = NULL;
    m_spDepthStencilTarget2 = NULL;

    Vision::Renderer.SetRendererNode(0, NULL);
    Vision::Renderer.SetRendererNode(1, NULL);

    // deinit the App
    spApp->DeInitSample();
    Vision::Plugins.UnloadAllEnginePlugins();

  }  // We use a new scope to make sure that the HMD manager is destroyed before OVR is deinitialized.

  // Deinit LibOVR.
  OVR::System::Destroy();

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
