/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VFmodNull.hpp

#ifndef VISION_FMOD_NULL_INCLUDED_HPP
#define VISION_FMOD_NULL_INCLUDED_HPP

// This file contains minimal enums and class declarations to have compiling header files when compiling
// in an environment where there is no FMod support

namespace FMOD
{
  /// \brief See FMOD documentation.
  class System
  {
  };

  /// \brief See FMOD documentation.
  class Sound
  {
  };
  
  /// \brief See FMOD documentation.
  class Channel
  {
  };
  
  /// \brief See FMOD documentation.
  class ChannelGroup
  {
  };
  
  /// \brief See FMOD documentation.
  class Geometry
  {
  };
  
  /// \brief See FMOD documentation.
  class Event
  {
  };
  
  /// \brief See FMOD documentation.
  class EventParameter
  {
  };
  
  /// \brief See FMOD documentation.
  class EventGroup
  {
  };
  
  /// \brief See FMOD documentation.
  class EventProject
  {
  };
  
  /// \brief See FMOD documentation.
  class EventReverb
  {
  };
  
  /// \brief See FMOD documentation.
  class EventSystem
  {
  };
}

// Minimal typedefs and enums so the header file is usable in an environment where there is no FMod support
typedef unsigned int FMOD_RESULT;
typedef unsigned int FMOD_DEBUGLEVEL;

/// \brief See FMOD documentation.
enum FMOD_SOUND_FORMAT
{
  FMOD_SOUND_FORMAT_PCM16
};

/// \brief See FMOD documentation.
enum FMOD_DSP_RESAMPLER
{
  FMOD_DSP_RESAMPLER_LINEAR
};

/// \brief See FMOD documentation.
enum FMOD_CHANNEL_CALLBACKTYPE
{
  FMOD_DUMMY_CHANNEL_CALLBACKTYPE
};

/// \brief See FMOD documentation.
enum FMOD_EVENT_CALLBACKTYPE
{
  FMOD_DUMMY_EVENT_CALLBACKTYPE
};

/// \brief See FMOD documentation.
struct FMOD_EVENT_INFO
{
};

/// \brief See FMOD documentation.
struct FMOD_VECTOR
{
  float x; float y; float z;
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
