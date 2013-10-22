/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/VisionAssetsPCH.h>
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/Assets/hkvPrefabAsset.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Assets/hkvAssetTypeManager.hpp>

#include <Vision/Runtime/Base/ThirdParty/tinyXML/TinyXMLHelper.hpp>

#include <Common/Base/System/Io/IStream/hkIStream.h>
#include <Common/Serialize/Serialize/Xml/hkXmlObjectReader.h>


unsigned int hkvPrefabAsset::s_iAssetTypeIndex = HKV_INVALID_INDEX;

/////////////////////////////////////////////////////////////////////////////
// hkvPrefabAsset static functions
/////////////////////////////////////////////////////////////////////////////

void hkvPrefabAsset::StaticInit()
{
  hkvAssetTypeInfo ti;
  ti.m_name = "Prefab";
  ti.m_createFunc = &CreateAsset;
  ti.m_generateThumbnailFunc = &GenerateThumbnail;
  ti.m_supportedFileExtensions.pushBack("prefab");
  ti.m_useEngineForThumbnails = false;
  ti.m_szTypeIconQt = ":/Icons/Icons/PrefabAsset.png";

  // register at the hkvAssetTypeManager and store the asset type index in static variable.
  s_iAssetTypeIndex = hkvAssetTypeManager::getGlobalInstance()->addAssetType(ti);
}


void hkvPrefabAsset::StaticDeInit()
{
  // de-register at the hkvAssetTypeManager
  hkvAssetTypeManager::getGlobalInstance()->removeAssetType(s_iAssetTypeIndex);
  s_iAssetTypeIndex = HKV_INVALID_INDEX;
}


hkvAsset* hkvPrefabAsset::CreateAsset()
{
  hkvAsset* pAsset = new hkvPrefabAsset;
  VASSERT(pAsset)

    return pAsset;
}


bool hkvPrefabAsset::GenerateThumbnail(const char* libraryPath, const char* assetPath, const char* thumbnailPath)
{
  hkStringBuf sFilename(libraryPath);
  sFilename.pathAppend(assetPath);

  TiXmlDocument doc;
  if (!doc.LoadFile(sFilename.cString()))
  {
    return true;
  }

  if (doc.RootElement() == NULL)
    return true;

  // Proceed if any only if there is actually a thumbnail node
  TiXmlElement* pThumbnailNode = doc.RootElement()->FirstChildElement("thumbnail");
  if (pThumbnailNode == NULL)
    return true;

  IVFileOutStream* pFile = VBase_GetFileStreamManager()->Create(thumbnailPath);
  if (pFile == NULL)
    return true;

  int iSize = 0;
  pThumbnailNode->QueryIntAttribute("size", &iSize);

  // Convert base64 encoded data back to binary form
  char* imageData = new char[iSize];
  hkXmlObjectReader::base64read(hkIstream(pThumbnailNode->GetText(), strlen(pThumbnailNode->GetText())).getStreamReader(), imageData, iSize);

  // Write image to disk
  pFile->Write(imageData, iSize);
  pFile->Close();

  delete[] imageData;

  return true;
}


/////////////////////////////////////////////////////////////////////////////
// hkvPrefabAsset public functions
/////////////////////////////////////////////////////////////////////////////

hkvPrefabAsset::hkvPrefabAsset()
{

}


hkvPrefabAsset::~hkvPrefabAsset()
{

}


/////////////////////////////////////////////////////////////////////////////
// hkvPrefabAsset public override functions
/////////////////////////////////////////////////////////////////////////////

unsigned int hkvPrefabAsset::getTypeIndex() const
{
  return s_iAssetTypeIndex;
}


const char* hkvPrefabAsset::getTypeName() const
{
  return "Prefab";
}


void hkvPrefabAsset::getSpecificProperties(hkvPropertyList& properties, hkvProperty::Purpose purpose) const
{
  // TODO
}

void hkvPrefabAsset::setSpecificProperty(const hkvProperty& prop, const hkArray<hkStringPtr>& path, unsigned int iStackIndex, hkvProperty::Purpose purpose)
{
  // TODO
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
