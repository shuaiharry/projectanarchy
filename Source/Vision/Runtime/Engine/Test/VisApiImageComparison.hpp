/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef VISAPIIMAGECOMPARISON_HPP_INCLUDED
#define VISAPIIMAGECOMPARISON_HPP_INCLUDED

/// \brief
/// TODO: image comparison not finished yet, part of 2013.3 roadmap (see HS ID#1322)
class VisImageComparison_cl  
{
public:
  VISION_APIFUNC VisImageComparison_cl();
  VISION_APIFUNC virtual ~VisImageComparison_cl();

  /// \brief
  ///   Save a reference image of the current framebuffer content.
  /// 
  /// \param szFileName
  ///   The filename the reference should be saved at.
  /// 
  /// \return
  ///   Returns true if successful.
  VISION_APIFUNC bool SaveReferenceImage(const char* szFileName);

private:
  void ValidateScreenBuffer();
  void GrabScreenBuffer();

  unsigned int m_uiScreenBufferWidth;
  unsigned int m_uiScreenBufferHeight;

  UBYTE* m_pScreenBuffer;
};

#endif

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
