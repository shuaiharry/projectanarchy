/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file vHavokPhysicsImportExport.h

#ifndef VHAVOKPHYSICSIMPORTEXPORT_H_INCLUDED
#define VHAVOKPHYSICSIMPORTEXPORT_H_INCLUDED

#if defined(__HAVOK_PARSER__)
  #define VHAVOK_IMPEXP
#else

  #ifdef WIN32
    #ifdef VHAVOKMODULE_EXPORTS
      #define VHAVOK_IMPEXP __declspec(dllexport)
    #elif defined VHAVOKMODULE_IMPORTS
      #define VHAVOK_IMPEXP __declspec(dllimport)
    #else
      #define VHAVOK_IMPEXP
    #endif
  #elif defined (_VISION_XENON)
    #define VHAVOK_IMPEXP 
  #elif defined (_VISION_PS3)
    #define VHAVOK_IMPEXP
  #elif defined (_VISION_IOS) || defined(_VISION_ANDROID) || defined(_VISION_TIZEN)
    #define VHAVOK_IMPEXP
  #elif defined(_VISION_PSP2)
    #define VHAVOK_IMPEXP 
  #elif defined(_VISION_WIIU)
    #define VHAVOK_IMPEXP 
  #else
    #error Undefined platform!
  #endif

#endif // __HAVOK_PARSER__

#endif // VHAVOKPHYSICSIMPORTEXPORT_H_INCLUDED

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
