/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Editor/vForge/AssetManagement/VisionAssets/VisionAssetsPCH.h>

#include <Vision/Editor/vForge/AssetManagement/VisionAssets/ImageFile/hkvDds.hpp>
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/ImageFile/hkvImageFile.hpp>
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/TransformationRules/hkvTextureTransformationSettings.hpp>
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/TransformationRules/hkvTransformationStepImageToDds.hpp>

#include <Common/Base/System/Io/OArchive/hkOArchive.h>

hkvTransformationStepImageToDds::hkvTransformationStepImageToDds(const hkvTextureTransformationSettings& settings,
  const char* sourceFile, const char* targetFile)
: hkvFileTransformationStep(sourceFile, targetFile), m_rgbaBits(0), m_rWidth(0), m_rShift(0), m_gWidth(0), m_gShift(0),
  m_bWidth(0), m_bShift(0), m_aWidth(0), m_aShift(0)
{
  switch (settings.getTargetDataFormat())
  {
  case HKV_TEXTURE_DATA_FORMAT_A4R4G4B4:
    {
      m_rgbaBits = 16;
      m_rWidth = m_gWidth = m_bWidth = m_aWidth = 4;
      m_rShift = 8;
      m_gShift = 4;
      m_bShift = 0;
      m_aShift = 12;
      break;
    }
  case HKV_TEXTURE_DATA_FORMAT_R4G4B4A4_GL:
    {
      m_rgbaBits = 16;
      m_rWidth = m_gWidth = m_bWidth = m_aWidth = 4;
      m_rShift = 12;
      m_gShift = 8;
      m_bShift = 4;
      m_aShift = 0;
      break;
    }
  case HKV_TEXTURE_DATA_FORMAT_A1R5G5B5:
    {
      m_rgbaBits = 16;
      m_rWidth = 5;
      m_gWidth = 5;
      m_bWidth = 5;
      m_aWidth = 1;
      m_rShift = 10;
      m_gShift = 5;
      m_bShift = 0;
      m_aShift = 15;
      break;
    }
  case HKV_TEXTURE_DATA_FORMAT_R5G6B5:
    {
      m_rgbaBits = 16;
      m_rWidth = 5;
      m_gWidth = 6;
      m_bWidth = 5;
      m_aWidth = 0;
      m_rShift = 11;
      m_gShift = 5;
      m_bShift = 0;
      m_aShift = 0;
      break;
    }
  case HKV_TEXTURE_DATA_FORMAT_A8R8G8B8:
    {
      m_rgbaBits = 32;
      m_rWidth = m_gWidth = m_bWidth = m_aWidth = 8;
      m_rShift = 16;
      m_gShift = 8;
      m_bShift = 0;
      m_aShift = 24;
      break;
    }
  case HKV_TEXTURE_DATA_FORMAT_X8R8G8B8:
    {
      m_rgbaBits = 32;
      m_rWidth = m_gWidth = m_bWidth = 8;
      m_aWidth = 0;
      m_rShift = 16;
      m_gShift = 8;
      m_bShift = 0;
      m_aShift = 0;
      break;
    }
  case HKV_TEXTURE_DATA_FORMAT_R8G8B8:
    {
      m_rgbaBits = 24;
      m_rWidth = m_gWidth = m_bWidth = 8;
      m_aWidth = 0;
      m_rShift = 16;
      m_gShift = 8;
      m_bShift = 0;
      m_aShift = 0;
      break;
    }
  }
}


hkResult hkvTransformationStepImageToDds::writeImage(hkStreamWriter& writer, const DDS_HEADER& ddsHeader, hkvImage* image)
{
  hkArray<hkUint8> scanLine;

  const hkUint32 width = image->getWidth();
  const hkUint32 height = image->getHeight();

  const DWORD stride = ddsHeader.ddspf.dwRGBBitCount / 8;
  scanLine.setSize(width * stride);

  const hkUint32 numSlices = image->getDepth();
  for (hkUint32 sliceIdx = 0; sliceIdx < numSlices; ++sliceIdx)
  {
    for (DWORD y = 0; y < height; ++y)
    {
      const hkUint8* imageData = image->getData(sliceIdx) + ((height - y - 1) * width * 4);

      int scanLinePos = 0;
      for (DWORD x = 0; x < width; ++x)
      {
        DWORD b = *imageData++;
        DWORD g = *imageData++;
        DWORD r = *imageData++;
        DWORD a = *imageData++;

        b = (b >> (8 - m_bWidth)) << m_bShift;
        g = (g >> (8 - m_gWidth)) << m_gShift;
        r = (r >> (8 - m_rWidth)) << m_rShift;
        a = (a >> (8 - m_aWidth)) << m_aShift;

        DWORD val = b | g | r | a;

        for (DWORD part = 0; part < stride; ++part)
        {
          scanLine[scanLinePos++] = (hkUint8)(val & 0xff);
          val >>= 8;
        }
      }

      if (writer.write(scanLine.begin(), scanLine.getSize()) != scanLine.getSize())
      {
        return HK_FAILURE;
      }
    }
  }

  return HK_SUCCESS;
}


hkResult hkvTransformationStepImageToDds::run()
{
  hkvImageFile::RefPtr imageFile = hkvImageFile::open(getSourceFile(), true);
  if ((imageFile == NULL) || !imageFile->isDataValid())
  {
    addMessage(hkvAssetLogMessage(HKV_MESSAGE_CATEGORY_ASSET_TRANSFORMATION, HKV_MESSAGE_SEVERITY_ERROR, "Source image is not valid."));
    return HK_FAILURE;
  }

  if (m_rgbaBits == 0 || (m_rgbaBits % 8) != 0)
  {
    addMessage(hkvAssetLogMessage(HKV_MESSAGE_CATEGORY_ASSET_TRANSFORMATION, HKV_MESSAGE_SEVERITY_ERROR, "Target format is not valid."));
    return HK_FAILURE;
  }

  if (imageFile->getNumImages() > 1)
  {
    addMessage(hkvAssetLogMessage(HKV_MESSAGE_CATEGORY_ASSET_TRANSFORMATION, HKV_MESSAGE_SEVERITY_ERROR, "Texture arrays are not yet supported."));
    return HK_FAILURE;
  }

  DDS_HEADER ddsHeader;
  memset(&ddsHeader, 0, sizeof(DDS_HEADER));
  ddsHeader.dwSize = sizeof(DDS_HEADER);
  ddsHeader.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT;
  ddsHeader.dwHeight = imageFile->getHeight();
  ddsHeader.dwWidth = imageFile->getWidth();
  ddsHeader.dwPitchOrLinearSize = imageFile->getWidth() * (m_rgbaBits / 8);
  if (imageFile->getDepth() > 1)
  {
    ddsHeader.dwFlags |= DDSD_DEPTH;
    ddsHeader.dwDepth = imageFile->getDepth();
  }
  if (imageFile->getNumMipLevels() > 1)
  {
    ddsHeader.dwFlags |= DDSD_MIPMAPCOUNT;
    ddsHeader.dwMipMapCount = imageFile->getNumMipLevels();
  }

  ddsHeader.ddspf.dwSize = sizeof(DDS_PIXELFORMAT);
  ddsHeader.ddspf.dwFlags = DDPF_RGB;
  if (m_aWidth > 0)
    ddsHeader.ddspf.dwFlags |= DDPF_ALPHAPIXELS;
  ddsHeader.ddspf.dwRGBBitCount = m_rgbaBits;

  ddsHeader.ddspf.dwRBitMask = makeBitMask(m_rWidth, m_rShift);
  ddsHeader.ddspf.dwGBitMask = makeBitMask(m_gWidth, m_gShift);
  ddsHeader.ddspf.dwBBitMask = makeBitMask(m_bWidth, m_bShift);
  ddsHeader.ddspf.dwABitMask = makeBitMask(m_aWidth, m_aShift);

  ddsHeader.dwCaps = DDSCAPS_TEXTURE;
  if (imageFile->getNumMipLevels() > 1 || imageFile->getNumFaces() > 1 || imageFile->getDepth() > 1)
  {
    ddsHeader.dwCaps |= DDSCAPS_COMPLEX;
  }

  if (imageFile->getNumFaces() > 1)
  {
    VASSERT(imageFile->getNumFaces() == 6);
    ddsHeader.dwCaps2 |= DDSCAPS2_CUBEMAP;
    ddsHeader.dwCaps2 |= DDSCAPS2_CUBEMAP_ALLFACES;
  }

  hkRefPtr<hkStreamWriter> writer = hkRefNew<hkStreamWriter>(hkFileSystem::getInstance().openWriter(getTargetFile()));
  if ((writer == NULL) || !writer->isOk())
    return HK_FAILURE;

  {
    hkOArchive archive(writer, hkBool(HK_ENDIAN_BIG));

    archive.write32u(DDS_MAGIC);
    archive.writeArray32u((hkUint32*)&ddsHeader, sizeof(ddsHeader) / sizeof(DWORD));
    if (!archive.isOk())
      return HK_FAILURE;
  }

  const hkUint32 numImages = imageFile->getNumImages();
  for (hkUint32 imageIdx = 0; imageIdx < numImages; ++imageIdx)
  {
    const hkUint32 numFaces = imageFile->getNumFaces();
    for (hkUint32 faceIdx = 0; faceIdx < numFaces; ++faceIdx)
    {
      const hkUint32 numMips = imageFile->getNumMipLevels();
      for (hkUint32 mipIdx = 0; mipIdx < numMips; ++mipIdx)
      {
        hkvImage* img = imageFile->getImage(imageIdx, faceIdx, mipIdx);
        if (writeImage(*writer, ddsHeader, img) != HK_SUCCESS)
        {
          return HK_FAILURE;
        }
      }
    }
  }

  return HK_SUCCESS;
}


void hkvTransformationStepImageToDds::cancel()
{
  // Not implemented yet.
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
