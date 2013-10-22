/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Samples/Engine/OculusVR/OculusVRPCH.h>

/// \brief 
///   Render loop class for the main renderloop. This render loop does not do much except for displaying
///   the two resulting render target textures.
class HmdRenderLoop : public IVisRenderLoop_cl
{
public:
  /// \brief
  ///   The constructor takes the two render target textures.
  HmdRenderLoop(VTextureObject *pTarget1, VTextureObject *pTarget2);

  virtual void OnDoRenderLoop(void *pUserData) HKV_OVERRIDE;

  enum HMDPostProcess
  {
    HP_NoWarp = 0,                  ///< Simple pass-through, i.e., directly render the non-distorted (but shrunk) eye render targets.
    HP_Warp,                        ///< Warp the per-eye images to cater for distortion caused by the HMD lenses.
    HP_WarpChromaticAberration,     ///< Warp the images and perform color correction to counter chromatic aberration caused by the display.

    HP_Count
  };

  /// \brief
  ///   Retrieve the current post-processing mode.
  inline unsigned int GetCurrentPostProcess() const
  {
    return m_eCurrentPostProcess;
  }

  /// /brief
  ///   Set the current post-processing mode - see HMDPostProcess.
  void SetCurrentPostProcess( unsigned int uiPostProcess );

  /// \brief
  ///   Get a string representation for the given post-processing mode.
  static const char* GetPostProcessString( unsigned int uiPostProcess );

  /// \brief
  ///   Update post-processing parameters (based on the current HMD setup).
  void SetPostProcessParameters( const hkvVec4& v4LensCenter_ScreenCenter, 
    const hkvVec4& v4Scale_ScaleIn, 
    const hkvVec4& v4HmdWarpParameters, 
    const hkvVec4& v4ChromaticAberration );

protected:
  /// \brief Initialize the given post-processing mode.
  bool InitPostProcess( HMDPostProcess ePostProcess );

private:
  VTextureObjectPtr       m_spSplitTexture[2];
  VShaderEffectLibPtr     m_spOculusVRShaderLib;
  VCompiledShaderPassPtr  m_spPostProcess[ HP_Count ];
  HMDPostProcess          m_eCurrentPostProcess;

  ///< Helper struct for keeping track of shader registers.
  struct ShaderRegisterSet
  {
    int m_iLensCenter_ScreenCenter;
    int m_iScale_ScaleIn;
    int m_iHmdWarpParameters;
    int m_iChromaticAberration;
  };
  ShaderRegisterSet m_shaderRegisters[ HP_Count ];

  ///< Current set of shader parameters.
  hkvVec4 m_v4LensCenter_ScreenCenter_LeftEye;
  hkvVec4 m_v4LensCenter_ScreenCenter_RightEye;
  hkvVec4 m_v4Scale_ScaleIn;
  hkvVec4 m_v4HmdWarpParameters;
  hkvVec4 m_v4ChromaticAberration;
};

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
