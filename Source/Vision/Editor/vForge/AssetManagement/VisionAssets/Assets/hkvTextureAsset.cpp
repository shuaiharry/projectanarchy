/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/VisionAssetsPCH.h>
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/Assets/hkvTextureAsset.hpp>
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/hkvTextureDefinitions.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvAssetManager.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Assets/hkvAssetTypeManager.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Profiles/hkvProfileManager.hpp>

#include <Vision/Runtime/Engine/System/Vision.hpp>
#include <Vision/Runtime/Engine/System/Resource/VisApiResource.hpp>

hkUint32 hkvTextureAsset::s_iAssetTypeIndex = HKV_INVALID_INDEX;
hkvEnumDefinition hkvTextureAsset::s_imageFormatsDefinition(HKV_IMAGE_FILE_FORMAT_COUNT, hkvImageFileFormatNames);
hkvEnumDefinition hkvTextureAsset::s_usageDefinition(HKV_TEXTURE_USAGE_COUNT, hkvTextureUsageNames);


/////////////////////////////////////////////////////////////////////////////
// hkvTextureAssetType static functions
/////////////////////////////////////////////////////////////////////////////

void hkvTextureAsset::StaticInit()
{
  hkvAssetTypeInfo ti;
  ti.m_name = "Texture";
  ti.m_createFunc = &CreateAsset;
  ti.m_supportedFileExtensions.pushBack("bmp");
  ti.m_supportedFileExtensions.pushBack("dds");
  ti.m_supportedFileExtensions.pushBack("jpg");
  ti.m_supportedFileExtensions.pushBack("png");
  ti.m_supportedFileExtensions.pushBack("tga");
  ti.m_szTypeIconQt = ":/Icons/Icons/TextureAsset.png";

  ti.m_resourceManagerName = VIS_RESOURCEMANAGER_TEXTURES;
  ti.m_useEngineForDependencies = true;
  ti.m_useEngineForThumbnails = true;

  // register at the hkvAssetTypeManager and store the asset type index in static variable.
  s_iAssetTypeIndex = hkvAssetTypeManager::getGlobalInstance()->addAssetType(ti);
}


void hkvTextureAsset::StaticDeInit()
{
  // de-register at the hkvAssetTypeManager
  hkvAssetTypeManager::getGlobalInstance()->removeAssetType(s_iAssetTypeIndex);
  s_iAssetTypeIndex = HKV_INVALID_INDEX;
}


hkvAsset* hkvTextureAsset::CreateAsset()
{
  hkvAsset* pAsset = new hkvTextureAsset;
  VASSERT(pAsset)

  return pAsset;
}



/////////////////////////////////////////////////////////////////////////////
// hkvTextureAssetType public functions
/////////////////////////////////////////////////////////////////////////////

hkvTextureAsset::hkvTextureAsset() 
: m_usageInstance(s_usageDefinition), m_sRgb(false)
{

}


hkvTextureAsset::~hkvTextureAsset()
{

}


/////////////////////////////////////////////////////////////////////////////
// hkvTextureAssetType public override functions
/////////////////////////////////////////////////////////////////////////////

unsigned int hkvTextureAsset::getTypeIndex() const
{
  return s_iAssetTypeIndex;
}


const char* hkvTextureAsset::getTypeName() const
{
  return "Texture";
}


void hkvTextureAsset::getSpecificProperties(hkvPropertyList& properties, hkvProperty::Purpose purpose) const
{
  int flags = (purpose == hkvProperty::PURPOSE_USER_INTERFACE_READ_ONLY) ? hkvProperty::FLAG_READ_ONLY : 0;

  properties.reserve(properties.size() + 10);

  properties.push_back(hkvProperty::groupStart("Texture"));
  properties.back().setDescription("Asset specific properties and settings.");

  hkvPropertyList imageProps;
  m_imageProperties.getProperties(imageProps, purpose);
  for (hkvPropertyList::iterator iter = imageProps.begin(); iter != imageProps.end(); ++iter)
  {
    iter->setFlags(iter->getFlags() | hkvProperty::FLAG_INFORMATION_ONLY);
    properties.push_back(*iter);
  }

  properties.push_back(hkvProperty("sRGB", m_sRgb, flags, "If enabled, the texture will be gamma-corrected at runtime. Use this for all textures that contain painted or photographed images. Do not set this on Normal Maps or Lookup Tables."));
  properties.push_back(hkvProperty::groupEnd());
}

void hkvTextureAsset::setSpecificProperty(const hkvProperty& prop, const hkArray<hkStringPtr>& path, unsigned int stackIndex, hkvProperty::Purpose purpose)
{
  int stackSize = (path.getSize() - stackIndex);

  if ((stackSize == 1) && (hkvStringHelper::safeCompare(path.back(), "Texture") == 0))
  {
    m_imageProperties.setProperty(prop, path, stackIndex + 1, purpose);
    
    // Usage is still present here for backwards compatibility to convert older asset libraries, but has now moved to the texture transform rule instead.
    if (hkvStringHelper::safeCompare(prop.getName(), "Usage") == 0)
    {
      if (purpose == hkvProperty::PURPOSE_SERIALIZATION)
      {
        if (m_usageInstance.setByString(prop.getString()) != HK_SUCCESS)
        {
          m_usageInstance.setByAvailableElementsId(0);
        }
      }
    }
    else if (hkvStringHelper::safeCompare(prop.getName(), "sRGB") == 0)
    {
      m_sRgb = prop.getBool();
    }
  }
}


void hkvTextureAsset::handleProjectLoaded()
{
  hkvAsset::handleProjectLoaded();

  hkvAssetManager* assetManager = hkvAssetManager::getGlobalInstance();
  hkvProfileManager* profileManager = hkvProfileManager::getGlobalInstance();
  hkvCriticalSectionLock lock(assetManager->acquireLock());
  // Migrate texture usage
  hkvTextureUsage usage = (hkvTextureUsage)m_usageInstance.getDefinitionId();

  if (usage == HKV_TEXTURE_USAGE_DIFFUSE_MAP || usage == HKV_TEXTURE_USAGE_NORMAL_MAP)
  {
    const hkUint32 iTemplateIdx = getTransformTemplate();

    hkvProperty usageProperty("Usage", m_usageInstance.getString(), hkvProperty::TYPE_STRING);

    touch();
    // Migrate usage setting to 'Custom' template.
    if (hasCustomTransformRule())
    {
      // Serialization path from the asset to the custom transform rule. If the asset has a different rule set, the setProperty call will simply be ignored.
      hkArray<hkStringPtr> path;
      path.pushBack("Transformations");
      path.pushBack("PROFILENAME");
      path.pushBack("TextureTransformationRule");

      hkUint32 iProfiles = profileManager->getNumberOfProfiles();
      for (hkUint32 iProfileIdx = 0; iProfileIdx < iProfiles; ++iProfileIdx)
      {
        hkvProfile::RefCPtr profile = profileManager->getProfileByIndex(iProfileIdx);
        path[1] = profile->getName();

        setProperty(usageProperty, path, 0, hkvProperty::PURPOSE_SERIALIZATION);
      }
    }
    // Migrate usage setting to template
    else
    {
      // Serialization path from the asset to the custom transform rule. If the asset has a different rule set, the setProperty call will simply be ignored.
      hkArray<hkStringPtr> path;
      path.pushBack("PROFILENAME");
      path.pushBack("TextureTransformationRule");

      const hkvTransformRuleTemplate* templateRule = profileManager->getTransformRuleTemplateByIndex(getTypeName(), iTemplateIdx);
      VASSERT_MSG(templateRule != NULL, "hkvTextureAsset::handleProjectLoaded: Could not resolve transform template via index!");
      // Build name of new template
      hkStringBuf newTemplateName(templateRule->getName(), (usage == HKV_TEXTURE_USAGE_DIFFUSE_MAP) ? "_DIFFUSE_USAGE" : "_NORMAL_USAGE");

      hkUint32 newTemplateIndex = profileManager->getTransformRuleTemplateIndexByName(getTypeName(), newTemplateName.cString());
      // If it does not exist yet, duplicate the current template and set the usage setting
      if (newTemplateIndex == HKV_INVALID_INDEX)
      {
        newTemplateIndex = profileManager->duplicateTransformRuleTemplate(getTypeName(), iTemplateIdx, newTemplateName.cString());
        hkvTransformRuleTemplate* newTemplateRule = profileManager->getTransformRuleTemplateByIndex(getTypeName(), newTemplateIndex);
        VASSERT_MSG(newTemplateRule != NULL, "hkvTextureAsset::handleProjectLoaded: Could not resolve transform template via index!");

        hkUint32 iProfiles = profileManager->getNumberOfProfiles();
        for (hkUint32 iProfileIdx = 0; iProfileIdx < iProfiles; ++iProfileIdx)
        {
          hkvProfile::RefCPtr profile = profileManager->getProfileByIndex(iProfileIdx);
          path[0] = profile->getName();

          newTemplateRule->setProperty(usageProperty, path, 0, hkvProperty::PURPOSE_SERIALIZATION);
        }
        
      }
      setTransformTemplate(newTemplateIndex);
    }
  }
}


void hkvTextureAsset::clearAssetSpecificData() const
{
  m_imageProperties.clear();
}

hkvAssetOperationResult hkvTextureAsset::updateAssetSpecificData(hkStreamReader& fileData) const
{
  clearAssetSpecificData();
  
  hkStringBuf pathBuf;
  hkvImageFileFormat imageFormat = hkvImageFile::guessFormatFromFileName(getRelativeFilePath(pathBuf));
  hkResult examineResult = m_imageProperties.examineStream(fileData, imageFormat);

  return (examineResult == HK_SUCCESS) ? HKV_AOR_SUCCESS : HKV_AOR_FAILURE;
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
