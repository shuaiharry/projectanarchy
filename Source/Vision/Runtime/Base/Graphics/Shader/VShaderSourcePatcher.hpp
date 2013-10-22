/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VShaderSourcePatcher.hpp

#ifndef VSHADERSOURCEPATCHER_HPP_INCLUDED
#define VSHADERSOURCEPATCHER_HPP_INCLUDED

class IVFileStreamManager;

/// \brief
///   Helper class for patching shader source code based on a rule set given in a JSON file.
class VShaderSourcePatcher
{
public:

  /// \brief Helper enumeration for specifying a shader type in a replacement rule.
  enum ShaderType
  {
    ST_VERTEX     = V_BIT( 0 ),
    ST_FRAGMENT   = V_BIT( 1 ),
    ST_ALL        = ( ST_VERTEX | ST_FRAGMENT )
  };

  /// \brief Helper struct for defining a single replacement rule.
  struct ReplacementInfo
  {
    VString     sOriginal;
    VString     sSubstitute;
    ShaderType  eShaderType;
  };

  /// \brief Initialize the shader patcher with the rules defined in Shaders/ShaderSourcePatcher.json.
  static VBASE_IMPEXP bool Initialize( IVFileStreamManager* pFileStreamManager );

  /// \brief Delete all replacement rules.
  static VBASE_IMPEXP void DeInitialize();

  /// \brief Check, whether the shader source patcher has been initialized successfully.
  static VBASE_IMPEXP bool IsInitialized();

  /// \brief Patch the given shader source code by applying replacement rules.
  static VBASE_IMPEXP bool PatchShader( char* pShaderSource, unsigned int uiShaderSourceLength, ShaderType eShaderType );

  /// \brief Check, whether we can patch shaders in place or whether we have to allocate memory for every single shader instance.
  /// Note: For now, only in-place patching is supported.
  static VBASE_IMPEXP bool CanPatchInPlace();


private:
  static VArray< ReplacementInfo > m_aReplacements;

  static bool m_bInitialized;   
  static bool m_bPatchInPlace;  

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
