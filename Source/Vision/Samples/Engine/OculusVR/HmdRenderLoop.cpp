/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Samples/Engine/OculusVR/OculusVRPCH.h>

#include "HmdRenderLoop.hpp"


// Render loop class for the main renderloop. This render loop does not do much except for displaying
// the two resulting render target textures.
HmdRenderLoop::HmdRenderLoop(VTextureObject *pTarget1, VTextureObject *pTarget2)
  : m_eCurrentPostProcess( HP_Count )
{
  m_spSplitTexture[0] = pTarget1;
  m_spSplitTexture[1] = pTarget2;

  // Load the shader library for HMD post-processing...
  m_spOculusVRShaderLib = Vision::Shaders.LoadShaderLibrary( "Shaders/OculusVR.ShaderLib" );
  VASSERT_MSG( m_spOculusVRShaderLib != NULL, "Unable to load OculusVR.ShaderLib" );

  // ...and initialize the corresponding shaders.
  for ( int i = 0; i < HP_Count; ++i )
  {
    InitPostProcess( ( HMDPostProcess ) i );
  }

  // Start with the fancy (i.e., proper) one.
  m_eCurrentPostProcess = HP_WarpChromaticAberration;
}

void HmdRenderLoop::OnDoRenderLoop(void *pUserData)
{
  INSERT_PERF_MARKER_SCOPE("VCombine3DScreenRenderLoop::OnDoRenderLoop");

  Vision::RenderLoopHelper.ClearScreen();

  const float fWidth = float( Vision::Video.GetXRes() );
  const float fHeight = float( Vision::Video.GetYRes() );
  const hkvVec2 v2LeftEye1( 0.0f, 0.0f );
  const hkvVec2 v2LeftEye2( 0.5f * fWidth, fHeight );
  const hkvVec2 v2RightEye1( 0.5f * fWidth, 0.0f );
  const hkvVec2 v2RightEye2( fWidth, fHeight );
  const hkvVec2 v2UV0( 0.0f, 0.0f );
  const hkvVec2 v2UV1( 1.0f, 1.0f );

  IVRender2DInterface *pRI = Vision::RenderLoopHelper.BeginOverlayRendering();

  // Set common shader parameters.
  VShaderConstantBuffer* pConstantBuffer = m_spPostProcess[ m_eCurrentPostProcess ]->GetConstantBuffer( VSS_PixelShader );
  const ShaderRegisterSet& registerSet = m_shaderRegisters[ m_eCurrentPostProcess ];
  if ( registerSet.m_iScale_ScaleIn >= 0 )
    pConstantBuffer->SetSingleRegisterF( registerSet.m_iScale_ScaleIn, m_v4Scale_ScaleIn.data );
  if ( registerSet.m_iHmdWarpParameters >= 0)
    pConstantBuffer->SetSingleRegisterF( registerSet.m_iHmdWarpParameters, m_v4HmdWarpParameters.data );
  if ( registerSet.m_iChromaticAberration >= 0 )
    pConstantBuffer->SetSingleRegisterF( registerSet.m_iChromaticAberration, m_v4ChromaticAberration.data );

  // Set eye-specific shader parameters and render quads for the left and right eyes.
  if ( registerSet.m_iLensCenter_ScreenCenter >= 0 )
    pConstantBuffer->SetSingleRegisterF( registerSet.m_iLensCenter_ScreenCenter, m_v4LensCenter_ScreenCenter_LeftEye.data );
  pRI->DrawTexturedQuadWithShader( v2LeftEye1, v2LeftEye2, m_spSplitTexture[ 0 ], v2UV0, v2UV1, V_RGBA_WHITE, *m_spPostProcess[ m_eCurrentPostProcess ] );


  if ( registerSet.m_iLensCenter_ScreenCenter >= 0 )
    pConstantBuffer->SetSingleRegisterF( registerSet.m_iLensCenter_ScreenCenter, m_v4LensCenter_ScreenCenter_RightEye.data );
  pRI->DrawTexturedQuadWithShader( v2RightEye1, v2RightEye2, m_spSplitTexture[ 1 ], v2UV0, v2UV1, V_RGBA_WHITE, *m_spPostProcess[ m_eCurrentPostProcess ] );

  Vision::RenderLoopHelper.EndOverlayRendering();

  // Explicitly trigger message rendering.
  Vision::Message.HandleMessages();
}


void HmdRenderLoop::SetCurrentPostProcess( unsigned int uiPostProcess )
{
  VASSERT_MSG( HP_NoWarp <= uiPostProcess && uiPostProcess < HP_Count,
    "Invalid Post-Process Id %d", uiPostProcess );

  m_eCurrentPostProcess = ( HMDPostProcess )uiPostProcess;
}

// Get a string representation for the given post-processing mode.
const char* HmdRenderLoop::GetPostProcessString( unsigned int uiPostProcess )
{
  switch ( uiPostProcess )
  {
  case HP_NoWarp:
    return "NoWarp";
  case HP_Warp:
    return "Warp";
  case HP_WarpChromaticAberration:
    return "WarpChromaticAberration";
  default:
    return "(invalid)";
  }
}

void HmdRenderLoop::SetPostProcessParameters( const hkvVec4& v4LensCenter_ScreenCenter,
  const hkvVec4& v4Scale_ScaleIn, const hkvVec4& v4HmdWarpParameters, const hkvVec4& v4ChromaticAberration )
{
  m_v4LensCenter_ScreenCenter_LeftEye = v4LensCenter_ScreenCenter;
  m_v4LensCenter_ScreenCenter_RightEye = v4LensCenter_ScreenCenter;
  m_v4LensCenter_ScreenCenter_RightEye[ 0 ] = 1.0f - m_v4LensCenter_ScreenCenter_RightEye[ 0 ];
  m_v4Scale_ScaleIn = v4Scale_ScaleIn;
  m_v4HmdWarpParameters = v4HmdWarpParameters;
  m_v4ChromaticAberration = v4ChromaticAberration;
}

bool HmdRenderLoop::InitPostProcess( HMDPostProcess ePostProcess )
{
  const char* szPostProcessString = GetPostProcessString( ePostProcess );
  char szTechniqueName[ 256 ];
  sprintf_s( szTechniqueName, "HMDPostProcess_%s", szPostProcessString );
  VCompiledTechnique *pTech = Vision::Shaders.CreateTechnique( szTechniqueName, NULL, NULL, EFFECTCREATEFLAG_NONE, m_spOculusVRShaderLib );
  VASSERT_MSG( pTech != NULL, "Unable to create technique '%s'!", szTechniqueName );

  m_spPostProcess[ ePostProcess ] = pTech->GetShader( 0 );
  VASSERT_MSG( m_spPostProcess[ ePostProcess ] != NULL, "Unable to retrieve shader from HMDPostProcess technique '%s'!", szTechniqueName );

  VShaderConstantBuffer *pConstantBuffer = m_spPostProcess[ ePostProcess ]->GetConstantBuffer( VSS_PixelShader );
  VASSERT_MSG( pConstantBuffer != NULL, "Unable to retrieve pixel shader constant buffer from '%s'!", szTechniqueName );

  ShaderRegisterSet& regSet = m_shaderRegisters[ ePostProcess ];
  regSet.m_iLensCenter_ScreenCenter = pConstantBuffer->GetRegisterByName( "f4LensCenter_ScreenCenter" );
  regSet.m_iScale_ScaleIn = pConstantBuffer->GetRegisterByName( "f4Scale_ScaleIn" );
  regSet.m_iHmdWarpParameters = pConstantBuffer->GetRegisterByName( "f4HmdWarpParameters" );
  regSet.m_iChromaticAberration = pConstantBuffer->GetRegisterByName( "f4ChromaticAberration" );

  return true;
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
