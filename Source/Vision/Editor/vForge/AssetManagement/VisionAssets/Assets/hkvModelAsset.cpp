/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Editor/vForge/AssetManagement/VisionAssets/VisionAssetsPCH.h>
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/Assets/hkvModelAsset.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvAssetManager.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Assets/hkvAssetTypeManager.hpp>

#include <Vision/Runtime/Engine/System/Vision.hpp>
#include <Vision/Runtime/Engine/System/Resource/VisApiResource.hpp>

unsigned int hkvModelAsset::s_iAssetTypeIndex = HKV_INVALID_INDEX;
const char* const hkvModelAsset::s_lodDistanceCustomType = "MeshLODDistances";

/////////////////////////////////////////////////////////////////////////////
// hkvModelAsset static functions
/////////////////////////////////////////////////////////////////////////////

void hkvModelAsset::StaticInit()
{
  hkvAssetTypeInfo ti;
  ti.m_name = "Model";
  ti.m_createFunc = &CreateAsset;
  ti.m_supportedFileExtensions.pushBack("model");
  ti.m_szTypeIconQt = ":/Icons/Icons/ModelAsset.png";

  ti.m_resourceManagerName = VIS_RESOURCEMANAGER_MESHES;
  ti.m_useEngineForDependencies = true;
  ti.m_useEngineForThumbnails = true;
  ti.m_useEngineForPropertyHint = true;

  // register at the hkvAssetTypeManager and store the asset type index in static variable.
  s_iAssetTypeIndex = hkvAssetTypeManager::getGlobalInstance()->addAssetType(ti);
}


void hkvModelAsset::StaticDeInit()
{
  // de-register at the hkvAssetTypeManager
  hkvAssetTypeManager::getGlobalInstance()->removeAssetType(s_iAssetTypeIndex);
  s_iAssetTypeIndex = HKV_INVALID_INDEX;
}


hkvAsset* hkvModelAsset::CreateAsset()
{
  hkvAsset* pAsset = new hkvModelAsset;
  VASSERT(pAsset)

  return pAsset;
}



/////////////////////////////////////////////////////////////////////////////
// hkvModelAsset public functions
/////////////////////////////////////////////////////////////////////////////

hkvModelAsset::hkvModelAsset()
{

}


hkvModelAsset::~hkvModelAsset()
{

}


/////////////////////////////////////////////////////////////////////////////
// hkvModelAsset public override functions
/////////////////////////////////////////////////////////////////////////////

unsigned int hkvModelAsset::getTypeIndex() const
{
  return s_iAssetTypeIndex;
}


const char* hkvModelAsset::getTypeName() const
{
  return "Model";
}


void hkvModelAsset::processPropertyHint(const char* propertyHint)
{
  VAssetMetadataParser hintParser(propertyHint, true);
  hkvProperty::VariantValueList newLodDistances;
  while (hintParser.next())
  {
    if (hkString::strCmp(hintParser.name(), "LODSwitchDistances") == 0)
    {
      VNameValueListParser<';', '~'> lodParser(hintParser.value(), true);
      while (lodParser.next())
      {
        float lodValue = -1;
        sscanf(lodParser.name(), "%f", &lodValue);
        newLodDistances.push_back(lodValue);
      }
    }
  }

  // Only overwrite LOD distances if the property hint has a different number of values
  // than we had before.
  if (newLodDistances.size() != m_lodDistances.size())
  {
    m_lodDistances = newLodDistances;
  }
}


void hkvModelAsset::getSpecificProperties(hkvPropertyList& properties, hkvProperty::Purpose purpose) const
{
  int flags = (purpose == hkvProperty::PURPOSE_USER_INTERFACE_READ_ONLY) ? hkvProperty::FLAG_READ_ONLY : 0;

  properties.reserve(properties.size() + 10);

  properties.push_back(hkvProperty::groupStart("Model"));
  properties.back().setDescription("Asset specific properties and settings.");

  properties.push_back(hkvProperty("LODSwitchDistances", m_lodDistances, s_lodDistanceCustomType, flags, "The viewing distances up to which each detail level of the model will be displayed. If this list has no entries, the mesh does not support multiple levels of detail. A distance value of -1 means 'infinity'."));

  properties.push_back(hkvProperty::groupEnd());
}

void hkvModelAsset::setSpecificProperty(const hkvProperty& prop, const hkArray<hkStringPtr>& path, unsigned int stackIndex, hkvProperty::Purpose purpose)
{
  int stackSize = (path.getSize() - stackIndex);

  if ((stackSize == 1) && (hkvStringHelper::safeCompare(path.back(), "Model") == 0))
  {
    if (hkvStringHelper::safeCompare(prop.getName(), "LODSwitchDistances") == 0)
    {
      m_lodDistances = prop.getArray();
    }
  }
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
