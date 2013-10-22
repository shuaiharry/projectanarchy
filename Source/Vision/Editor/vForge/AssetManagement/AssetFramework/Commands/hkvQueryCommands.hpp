/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file hkvQueryCommands.hpp

#ifndef HKV_QUERY_COMMANDS_HPP_INCLUDED
#define HKV_QUERY_COMMANDS_HPP_INCLUDED

#pragma warning(push)
#pragma warning(disable: 6211 6001 6326 6011 6385 6031 6387 6246 6386)

#include <Common/Base/Types/hkBaseTypes.h>

#ifdef WIN32
#pragma managed(push, off)
#endif

#include <Common/Base/hkBase.h>

#ifdef WIN32
#pragma managed(pop)
#endif

#pragma warning(pop)
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvAssetDecl.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvAssetQuery.hpp>

/// \brief
///   Command for setting a property on all assets found by a query.
struct hkvQueryCommandSetProperty
{
  /// \brief
  ///   Input options
  struct Options
  {
    /// \brief
    ///   Initializes this options structure. See the member field documentation for the meaning
    ///   of the parameters.
    Options(const hkvAssetQuery& query, const hkvProperty& prop, const hkArray<hkStringPtr>& path, hkUint32 stackIndex)
      : m_query(query), m_prop(prop), m_path(path), m_stackIndex(stackIndex) {}

    const hkvAssetQuery& m_query; ///< The query on whose assets to set the property
    const hkvProperty& m_prop; ///< The property to set
    const hkArray<hkStringPtr>& m_path; ///< The path to the property
    hkUint32 m_stackIndex; ///< The index of the first relevant path element
  };

  /// \brief
  ///   Executes this command.
  /// \param opts
  ///   The input options for this execution
  /// \return
  ///   HK_SUCCESS if the command executed successfully; HK_FAILURE otherwise
  ASSETFRAMEWORK_IMPEXP static hkResult execute(const Options& opts);
};


/// \brief
///   Command for setting or removing an existing tag on all assets found by a query.
struct hkvQueryCommandSetTag
{
  /// \brief
  ///   Input options
  struct Options
  {
    /// \brief
    ///   Initializes this options structure. See the member field documentation for the meaning
    ///   of the parameters.
    Options(const hkvAssetQuery& query, bool state, hkUint32 tagIdx)
      : m_query(query), m_state(state), m_tagIdx(tagIdx) {}

    const hkvAssetQuery& m_query; ///< The query on whose assets to set/remove the tag
    hkBool m_state; ///< The new state of the tag (= whether it should be set)
    hkUint32 m_tagIdx; ///< Index of the tag to set/remove
  };

  /// \brief
  ///   Executes this command.
  /// \param opts
  ///   The input options for this execution
  /// \return
  ///   HK_SUCCESS if the command executed successfully; HK_FAILURE otherwise
  ASSETFRAMEWORK_IMPEXP static hkResult execute(const Options& opts);
};


/// \brief
///   Command for adding a named tag all assets found by a query. If the tag does not
///   yet exist, it is created.
struct hkvQueryCommandAddTag
{
  /// \brief
  ///   Input options
  struct Options
  {
    /// \brief
    ///   Initializes this options structure. See the member field documentation for the meaning
    ///   of the parameters.
    Options(const hkvAssetQuery& query, const char* tagName)
      : m_query(query), m_tagName(tagName) {}

    const hkvAssetQuery& m_query; ///< The query to whose assets to add the tag
    hkStringPtr m_tagName; ///< The tag to add
  };

  /// \brief
  ///   Executes this command.
  /// \param opts
  ///   The input options for this execution
  /// \return
  ///   HK_SUCCESS if the command executed successfully; HK_FAILURE otherwise
  ASSETFRAMEWORK_IMPEXP static hkResult execute(const Options& opts);
};


/// \brief
///   Command for deleting a tag from the asset system
struct hkvQueryCommandDeleteTag
{
  /// \brief
  ///   Input options
  struct Options
  {
    /// \brief
    ///   Initializes this options structure. See the member field documentation for the meaning
    ///   of the parameters.
    Options(hkUint32 tagIdx)
      : m_tagIdx(tagIdx) {}

    hkUint32 m_tagIdx; ///< Index of the tag to delete
  };

  /// \brief
  ///   Executes this command.
  /// \param opts
  ///   The input options for this execution
  /// \return
  ///   HK_SUCCESS if the command executed successfully; HK_FAILURE otherwise
  ASSETFRAMEWORK_IMPEXP static hkResult execute(const Options& opts);
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
