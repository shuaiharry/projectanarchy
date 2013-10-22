/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file hkvAssetDecl.hpp

#ifndef HKV_ASSETDECL_HPP_INCLUDED
#define HKV_ASSETDECL_HPP_INCLUDED

#include <Vision/Editor/vForge/AssetManagement/AssetFramework/AssetFramework.hpp>

#define HKV_INVALID_INDEX ULONG_MAX

/// \brief
///   Possible sorting criteria for lists of assets.
enum hkvAssetSortingCriterion
{
  HKV_ASC_NAME,     ///< Sort by asset name
  HKV_ASC_TYPE,     ///< Sort by asset type name
  HKV_ASC_TAG,      ///< Sort by number of tags set
  HKV_ASC_VARIANT,  ///< Sort by number of variants set, then by variant names
  HKV_ASC_FOLDER,   ///< Sort by folder path
};


/// \brief
///   Return values for loading asset libraries.
enum hkvAssetLibraryLoadResult
{
  HKV_ALLR_SUCCESS,                 ///< The library has been loaded successfully
  HKV_ALLR_PATH_NOT_FOUND,          ///< The asset library path is not a valid path
  HKV_ALLR_LIBRARY_ALREADY_PRESENT, ///< A new library should have been created, but an asset library file already exists
  HKV_ALLR_LIBRARY_NOT_FOUND,       ///< An existing asset library should be loaded, but no asset library file was found
  HKV_ALLR_LIBRARY_LOAD_FAILED      ///< Loading a library failed for another reason (i.e., inaccessible or corrupt file)
};


/// \brief
///   Generic return values for asset operations.
enum hkvAssetOperationResult
{
  HKV_AOR_SUCCESS,   ///< The operation succeeded
  HKV_AOR_FAILURE,   ///< The operation failed
  HKV_AOR_MAY_RETRY  ///< The operation failed, but there is a chance of it succeeding when retried.
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
