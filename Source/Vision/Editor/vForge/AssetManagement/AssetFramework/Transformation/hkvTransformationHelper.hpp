/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file hkvTransformationHelper.hpp

#ifndef HKV_TRANSFORMATION_HELPER_HPP_INCLUDED
#define HKV_TRANSFORMATION_HELPER_HPP_INCLUDED

#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvAssetStructs.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvIProperties.hpp>

#include <deque>

class hkvAsset;
class hkvAssetLibrary;
class hkvTransformationRule;

class hkvTransformationHelper
{
private:
  hkvTransformationHelper();
  hkvTransformationHelper(const hkvTransformationHelper&);
  hkvTransformationHelper& operator=(const hkvTransformationHelper&);

public:
  ASSETFRAMEWORK_IMPEXP static bool appendLookupTableEntries(const hkvAsset& asset, const hkvTransformationRule& rule, bool forEditor, bool assetPreviewMode, std::vector<hkvLookupTableEntry>& out_entries);
  ASSETFRAMEWORK_IMPEXP static bool queryOutputFiles(const hkvAsset& asset, const hkvTransformationRule& rule, hkArray<hkStringPtr>& out_files);
  ASSETFRAMEWORK_IMPEXP static hkvAssetOperationResult transformAsset(hkvAsset& asset, 
    const hkvTransformationRule& rule, hkArray<hkvAssetLogMessage>& messages, 
    hkvITransformationControlHost* controlHost = NULL, bool addTransformedToRCS = false);

public:
  ASSETFRAMEWORK_IMPEXP static void filterRelevantProperties(const hkvPropertyList& properties, hkvPropertyList& out_properties);

private:
  static bool assetNeedsTransformation(const hkvAsset& asset, const hkvTransformationRule& rule);
  static bool fillTargetFileInformation(const hkvAsset& asset, const hkvTransformationRule& rule, hkvTransformationInput& transformationInput);
  static bool performTransformation(const hkvAsset& asset, const hkvTransformationRule& rule, 
    hkArray<hkvAssetLogMessage>& messages, hkvITransformationControlHost* controlHost, bool addTransformedToRCS);
  static bool queryOutputFileSpecs(const hkvAsset& asset, const hkvTransformationRule& rule, bool includeEditorPreview, hkArray<hkvTransformationOutputFileSpec>& out_specs);
  static bool staticTargetExists(const hkvAsset& asset, const hkvTransformationRule& rule);
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
