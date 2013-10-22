/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

// ***********************************************************************************************
// FileServe sample application
// Copyright (C) Havok.com Inc. All rights reserved.
// ***********************************************************************************************
// Shows how to set up an application to consume data using a file serving client.
// ***********************************************************************************************
#include <Vision/Samples/Engine/FileServe/FileServePCH.h>
#include <Vision/Runtime/EnginePlugins/EnginePluginsImport.hpp>

VisSampleAppPtr spApp;

VISION_INIT
{
  // Deactivate automatic probing for alternative files (i.e., only use the asset system to determine
  // potential replacement resources)
  AssetSettings::SetUseAlternativeFiles(false);

  // Set up directories, does nothing on platforms other than iOS, 
  // pass true if you want load from the documents directory
  VISION_SET_DIRECTORIES(false);

#if defined( SUPPORTS_FILESERVE_CLIENT )
  VisSampleApp::SetupFileServeClient();
#endif
  
  // Create an application
  spApp = new VisSampleApp(); 
  spApp->LoadVisionEnginePlugin();
  
  // Init the application
  if (!spApp->InitSample("FileServe" /*DataDir*/, "Scenes/FileServeSample" /*SampleScene*/ ))
    return false;

  Vision::Renderer.SetDefaultTextureFilterMode(FILTER_MIN_MAG_MIP_LINEAR);
  
  return true;
}

VISION_SAMPLEAPP_AFTER_LOADING
{
  spApp->SetShowFrameRate(true);
  // Create a mouse controlled camera (optionally with gravity)
  VisBaseEntity_cl *pCamera = spApp->EnableMouseCamera();
  pCamera->SetPosition(-150.0f,-150.0f,150.0f);
  pCamera->SetDirection(hkvVec3(1.5f,1.f,-0.7f));
}

VISION_SAMPLEAPP_RUN
{
  return spApp->Run();
}

VISION_DEINIT
{
  // Deinit the application
  spApp->DeInitSample();
  spApp = NULL;
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
