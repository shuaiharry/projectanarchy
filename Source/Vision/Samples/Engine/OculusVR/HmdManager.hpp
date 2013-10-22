/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#if !defined( _HMDMANAGER_HPP )
#define _HMDMANAGER_HPP

#include <OVR.h>


/// \brief
///   A small manager class for encapsulating HMD data.
class HMDManager : public OVR::MessageHandler
{
public:
  HMDManager();
  virtual ~HMDManager();

  /// \brief
  ///   Initialize the HMD manager and retrieve some info about the current setup.
  bool Init( int& iWidth_out, int& iHeight_out, float &fResolutionMultiplier_out );

  /// \brief
  ///   Retrieve the HMD sensor.
  inline OVR::SensorDevice* GetSensor() const
  {
    return m_pSensor.GetPtr();
  }

  /// \brief
  ///   Retrieve the current stereo configuration.
  inline OVR::Util::Render::StereoConfig* GetStereoConfig()
  {
    return &m_stereoConfig;
  }

  /// \brief
  ///   Retrieve the sensor fusion object.
  inline OVR::SensorFusion* GetSensorFusion()
  {
    return &m_sensorFusion;
  }

  /// \brief 
  ///   Process OculusVR messages.
  virtual void OnMessage( const OVR::Message& msg ) HKV_OVERRIDE;

private:
  OVR::Ptr<OVR::DeviceManager> m_pManager;
  OVR::Ptr<OVR::SensorDevice> m_pSensor;
  OVR::Ptr<OVR::HMDDevice> m_pHMD;
  OVR::HMDInfo m_hmdInfo;
  OVR::Util::Render::StereoConfig m_stereoConfig;
  OVR::SensorFusion m_sensorFusion;
};

#endif // defined( _HMDMANAGER_HPP )

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
