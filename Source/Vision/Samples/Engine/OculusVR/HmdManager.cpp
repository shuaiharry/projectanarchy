/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Samples/Engine/OculusVR/OculusVRPCH.h>
#include "HmdManager.hpp"

#include <OVR.h>


HMDManager::HMDManager()
{
}

HMDManager::~HMDManager()
{
  RemoveHandlerFromDevices();

  // Clear references to LibOVR objects.
  m_pSensor.Clear();
  m_pHMD.Clear();
}

bool HMDManager::Init( int& iWidth_out, int& iHeight_out, float &fResolutionMultiplier_out )
{
  // Initialize the Oculus Device Manager and register for handling its messages.
  m_pManager = *OVR::DeviceManager::Create();
  m_pManager->SetMessageHandler( this );

  m_pHMD = *m_pManager->EnumerateDevices< OVR::HMDDevice >().CreateDevice();
  if ( m_pHMD )
  {
    // Retrieve the sensor from the HMD to make sure that it's the correct one.
    m_pSensor = *m_pHMD->GetSensor();

    if ( m_pHMD->GetDeviceInfo( &m_hmdInfo ) )
    {
      // Provide the stereo configuration with information about our HMD device.
      m_stereoConfig.SetHMDInfo( m_hmdInfo );
    }
  }
  else
  {
    // If we can't find a HMD, try to pick up the sensor only.
    m_pSensor = *m_pManager->EnumerateDevices< OVR::SensorDevice >().CreateDevice();

    // Provide a fake HMD resolution
    m_hmdInfo.HResolution = 1280;
    m_hmdInfo.VResolution = 800;
  }

  if ( m_pSensor )
  {
    // Attach the sensor to our sensorFusion object in order to receive
    // body frame messages and update orientation.
    m_sensorFusion.AttachToSensor( m_pSensor );
    m_sensorFusion.SetDelegateMessageHandler( this );
    m_sensorFusion.SetPredictionEnabled( true );
  }

  int iWidth = m_hmdInfo.HResolution;
  int iHeight = m_hmdInfo.VResolution;

  // Configure stereo settings
  m_stereoConfig.SetFullViewport( OVR::Util::Render::Viewport( 0, 0, iWidth, iHeight ) );
  m_stereoConfig.SetStereoMode( OVR::Util::Render::Stereo_LeftRight_Multipass );
  m_stereoConfig.Set2DAreaFov( OVR::DegreeToRad( 85.0f ) );

  // Configure distortion fit.
  // For 7" screens, fit to touch the left side of the view, leaving a bit of invisible screen on the top (to save some rendering cost).
  // For smaller screens, fit to the top.
  if ( m_hmdInfo.HScreenSize > 0.0f )
  {
    if ( m_hmdInfo.HScreenSize > 0.14f ) // 7"
    {
      m_stereoConfig.SetDistortionFitPointVP( -1.0f, 0.0f );
    }
    else
    {
      m_stereoConfig.SetDistortionFitPointVP( 0.0f, 1.0f );
    }
  }


  // Return some information about our setup.
  iWidth_out = iWidth;
  iHeight_out = iHeight;
  fResolutionMultiplier_out = m_stereoConfig.GetDistortionScale();  // This defaults to 1.0 if no stereo config is set up.

  return true;
}


// Process OculusVR messages.
void HMDManager::OnMessage( const OVR::Message& msg )
{
  using namespace OVR;

  if ( msg.Type == Message_DeviceAdded && msg.pDevice == m_pManager )
  {
    OVR::LogText( "DeviceManager reported device added.\n" );
  }
  else if ( msg.Type == Message_DeviceRemoved && msg.pDevice == m_pManager )
  {
    OVR::LogText( "DeviceManager reported device removed.\n" );
  }
  else if ( msg.Type == Message_DeviceAdded && msg.pDevice == m_pSensor )
  {
    LogText( "Sensor reported device added.\n" );
  }
  else if ( msg.Type == Message_DeviceRemoved && msg.pDevice == m_pSensor )
  {
    LogText( "Sensor reported device removed.\n" );
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
