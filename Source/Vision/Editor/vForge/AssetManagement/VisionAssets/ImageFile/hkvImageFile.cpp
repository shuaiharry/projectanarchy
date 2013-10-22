/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Editor/vForge/AssetManagement/VisionAssets/VisionAssetsPCH.h>

#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Base/hkvBase.hpp>

#include <Vision/Editor/vForge/AssetManagement/VisionAssets/ImageFile/hkvDds.hpp>
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/ImageFile/hkvImageFile.hpp>

#include <Common/Base/System/Io/FileSystem/hkFileSystem.h>
#include <Common/Base/System/Io/IArchive/hkIArchive.h>
#include <Common/Base/System/Io/Reader/hkStreamReader.h>
#include <Common/Base/System/Io/Reader/Buffered/hkBufferedStreamReader.h>


const char* hkvImageFileFormatNames[HKV_IMAGE_FILE_FORMAT_COUNT] =
{
  "(Invalid)",
  "BMP",
  "DDS",
  "JPG",
  "PNG",
  "TGA"
};
HK_COMPILE_TIME_ASSERT((sizeof(hkvImageFileFormatNames) / sizeof(char*)) == HKV_IMAGE_FILE_FORMAT_COUNT);

const hkvEnumDefinition& getImageFileFormatDefinition()
{
  static hkvEnumDefinition def(HKV_IMAGE_FILE_FORMAT_COUNT, hkvImageFileFormatNames);
  return def;
}


//-----------------------------------------------------------------------------
// hkvMipImage
//-----------------------------------------------------------------------------
hkvImage::hkvImage(hkUint32 width, hkUint32 height, hkUint32 depth)
: m_width(width), m_height(height), m_depth(depth)
{
  m_sliceLength = m_width * m_height * 4;
  hkUint32 dataSize = m_sliceLength * m_depth;
  VASSERT_MSG(dataSize > 0, "hkvMipImage: width, height and depth must all be greater than 0");
  m_data.setSize(dataSize);
}

const hkUint8* hkvImage::getData(hkUint32 slice) const
{
  return const_cast<hkvImage*>(this)->getData(slice);
}

hkUint8* hkvImage::getData(hkUint32 slice)
{
  if (slice >= m_depth)
  {
    return NULL;
  }

  return &m_data[slice * m_sliceLength];
}


//-----------------------------------------------------------------------------
// hkvImageFileDds
//-----------------------------------------------------------------------------
class hkvImageFileDds : public hkvImageFile
{
public:
  HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_TOOLS);

public:
  hkvImageFileDds();
private:
  hkvImageFileDds(const hkvImageFileDds&);
  hkvImageFileDds& operator=(const hkvImageFileDds&);

public:
  virtual hkvImageFileFormat getFileFormat() const HKV_OVERRIDE;

  virtual hkUint32 getWidth() const HKV_OVERRIDE;
  virtual hkUint32 getHeight() const HKV_OVERRIDE;
  virtual hkUint32 getDepth() const HKV_OVERRIDE;

  virtual hkUint32 getNumImages() const HKV_OVERRIDE;
  virtual hkUint32 getNumFaces() const HKV_OVERRIDE;
  virtual hkUint32 getNumMipLevels() const HKV_OVERRIDE;

  virtual bool hasAlpha() const HKV_OVERRIDE;

protected:
  virtual hkResult readHeader(hkStreamReader& reader) HKV_OVERRIDE;
  virtual hkResult readImageData(hkStreamReader& reader) HKV_OVERRIDE;

private:
  hkResult readImageBlock(hkStreamReader& reader, hkUint8* target, hkUint32 width, hkUint32 height);

private:
  DDS_HEADER m_header;
  DDS_HEADER_DXT10 m_dx10Header;
  bool m_isDx10Format;
};


//-----------------------------------------------------------------------------
// hkvImageFileTga
//-----------------------------------------------------------------------------
class hkvImageFileTga : public hkvImageFile
{
public:
  HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_TOOLS);

private:
#pragma pack(push, 1)
  struct Header
  {
    hkUint8 identSize;          // size of ID field that follows 18 byte header (0 usually)
    hkUint8 colorMapType;      // type of color map 0=none, 1=has palette
    hkUint8 imageType;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    hkUint16 colorMapStart;     // first color map entry in palette
    hkUint16 colorMapLength;    // number of colors in palette
    hkUint8 colorMapBits;      // number of bits per palette entry 15,16,24,32

    hkUint16 xStart;             // image x origin
    hkUint16 yStart;             // image y origin
    hkUint16 width;              // image width in pixels
    hkUint16 height;             // image height in pixels
    hkUint8 bits;               // image bits per pixel 8,16,24,32
    hkUint8 descriptor;         // image descriptor bits (vh flip bits)
  };
#pragma pack(pop)

public:
  hkvImageFileTga() {}
private:
  hkvImageFileTga(const hkvImageFileTga&);
  hkvImageFileTga& operator=(const hkvImageFileTga&);

public:
  virtual hkvImageFileFormat getFileFormat() const HKV_OVERRIDE;
  virtual hkUint32 getWidth() const HKV_OVERRIDE;
  virtual hkUint32 getHeight() const HKV_OVERRIDE;
  virtual bool hasAlpha() const HKV_OVERRIDE;

protected:
  virtual hkResult readHeader(hkStreamReader& reader) HKV_OVERRIDE;
  virtual hkResult readImageData(hkStreamReader& reader) HKV_OVERRIDE;

private:
  hkResult readRleData(hkStreamReader& reader);
  void postprocessData();

private:
  Header m_header;
};


//-----------------------------------------------------------------------------
// hkvImageFileDds
//-----------------------------------------------------------------------------
hkvImageFileDds::hkvImageFileDds()
:m_isDx10Format(false)
{
}

hkvImageFileFormat hkvImageFileDds::getFileFormat() const
{
  return HKV_IMAGE_FILE_FORMAT_DDS;
}

hkUint32 hkvImageFileDds::getWidth() const
{
  return isHeaderValid() ? m_header.dwWidth : 0;
}

hkUint32 hkvImageFileDds::getHeight() const
{
  return isHeaderValid() ? m_header.dwHeight : 0;
}

hkUint32 hkvImageFileDds::getDepth() const
{
  VASSERT(isHeaderValid());
  if ((m_header.dwFlags & DDSD_DEPTH) == 0)
  {
    return 1;
  }

  return m_header.dwDepth;
}

hkUint32 hkvImageFileDds::getNumImages() const
{
  VASSERT(isHeaderValid());
  if (!m_isDx10Format)
  {
    return 1;
  }

  return m_dx10Header.arraySize;
}

hkUint32 hkvImageFileDds::getNumFaces() const
{
  VASSERT(isHeaderValid());
  if ((m_header.dwCaps2 & DDSCAPS2_CUBEMAP) == 0)
  {
    return 1;
  }

  return 6;
}

hkUint32 hkvImageFileDds::getNumMipLevels() const
{
  VASSERT(isHeaderValid());
  if ((m_header.dwFlags & DDSD_MIPMAPCOUNT) != 0)
  {
    return hkvMath::Max(1, m_header.dwMipMapCount);
  }

  return 1;
}


bool hkvImageFileDds::hasAlpha() const
{
  VASSERT(isHeaderValid());

  if (((m_header.ddspf.dwFlags & DDPF_ALPHAPIXELS) != 0) || ((m_header.ddspf.dwFlags & DDPF_ALPHA) != 0))
    return true;

  // DXT formats have alpha as well
  if (((m_header.dwFlags & DDPF_FOURCC) != 0) && 
    (m_header.ddspf.dwFourCC == DDS_FOURCC_DXT1 || m_header.ddspf.dwFourCC == DDS_FOURCC_DXT3 || m_header.ddspf.dwFourCC == DDS_FOURCC_DXT5))
    return true;

  return false;
}

hkResult hkvImageFileDds::readHeader(hkStreamReader& reader)
{
  hkIArchive archive(&reader, hkBool(HK_ENDIAN_BIG));

  // DDS header consists of DWORDs entirely, so read it as one array
  DWORD dwMagic = archive.read32u();
  int numElements = sizeof(DDS_HEADER) / sizeof(DWORD);
  archive.readArray32u(reinterpret_cast<hkUint32*>(&m_header), numElements);

  if (!archive.isOk() || dwMagic != DDS_MAGIC)
    return HK_FAILURE;

  if (m_header.ddspf.dwFourCC == DDS_FOURCC_DX10)
  {
    int numElements = sizeof(DDS_HEADER_DXT10) / sizeof(DWORD);
    archive.readArray32u(reinterpret_cast<hkUint32*>(&m_dx10Header), numElements);

    if (!archive.isOk())
      return HK_FAILURE;
  }

  // Check cubemap settings. Only considered valid if all 6 faces are defined.
  if ((m_header.dwCaps2 & DDSCAPS2_CUBEMAP) != 0 && (m_header.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES) != DDSCAPS2_CUBEMAP_ALLFACES)
  {
    return HK_FAILURE;
  }

  return HK_SUCCESS;
}

hkResult hkvImageFileDds::readImageData(hkStreamReader& reader)
{
  if (!isHeaderValid())
    return HK_FAILURE;

  DWORD checkFlags = DDPF_ALPHAPIXELS | DDPF_RGB;
  if ((m_header.ddspf.dwFlags & checkFlags) != checkFlags)
    return HK_FAILURE;

  if (m_header.ddspf.dwRGBBitCount != 32 || m_header.ddspf.dwRBitMask != 0x00ff0000 || m_header.ddspf.dwGBitMask != 0x0000ff00
    || m_header.ddspf.dwBBitMask != 0x000000ff || m_header.ddspf.dwABitMask != 0xff000000)
    return HK_FAILURE;

  const hkUint32 numImages = getNumImages();
  for (hkUint32 imageIdx = 0; imageIdx < numImages; ++imageIdx)
  {
    const hkUint32 numFaces = getNumFaces();
    for (hkUint32 faceIdx = 0; faceIdx < numFaces; ++faceIdx)
    {
      const hkUint32 mipLevels = getNumMipLevels();
      hkUint32 mipWidth = m_header.dwWidth;
      hkUint32 mipHeight = m_header.dwHeight;
      hkUint32 mipDepth = m_header.dwDepth;
      for (hkUint32 mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
      {
        hkvImage* img = new hkvImage(mipWidth, mipHeight, mipDepth);
        if (img == NULL || img->getData() == NULL)
        {
          delete img;
          return HK_FAILURE;
        }

        setImage(imageIdx, faceIdx, mipLevel, img);

        for (hkUint32 sliceIdx = 0; sliceIdx < mipDepth; ++sliceIdx)
        {
          hkUint8* data = img->getData(sliceIdx);
          if (readImageBlock(reader, data, mipWidth, mipHeight) != HK_SUCCESS)
          {
            return HK_FAILURE;
          }
        }

        mipWidth = mipWidth > 1 ? mipWidth >> 1 : 1;
        mipHeight = mipHeight > 1 ? mipHeight >> 1 : 1;
        mipDepth = mipDepth > 1 ? mipDepth >> 1 : 1;
      }
    }
  }

  return HK_SUCCESS;
}

hkResult hkvImageFileDds::readImageBlock(hkStreamReader& reader, hkUint8* target, hkUint32 width, hkUint32 height)
{
  const int scanLineSize = width * 4;

  for (DWORD y = 0; y < height; ++y)
  {
    hkUint8* targetPos = target + ((height - y - 1) * scanLineSize);
    if (reader.read(targetPos, scanLineSize) != scanLineSize)
    {
      return HK_FAILURE;
    }
  }

  return HK_SUCCESS;
}


//-----------------------------------------------------------------------------
// hkvImageFileTga
//-----------------------------------------------------------------------------
hkvImageFileFormat hkvImageFileTga::getFileFormat() const
{
  return HKV_IMAGE_FILE_FORMAT_TGA;
}


hkUint32 hkvImageFileTga::getWidth() const
{
  VASSERT(isHeaderValid());
  return m_header.width;
}


hkUint32 hkvImageFileTga::getHeight() const
{
  VASSERT(isHeaderValid());
  return m_header.height;
}


bool hkvImageFileTga::hasAlpha() const
{
  VASSERT(isHeaderValid());
  return m_header.bits == 32;
}


hkResult hkvImageFileTga::readHeader(hkStreamReader& reader)
{
  hkIArchive archive(&reader, hkBool(HK_ENDIAN_BIG));

  archive.readArray8u(&m_header.identSize, 3);
  //hkUint8 identSize;
  //hkUint8 colorrMapType;
  //hkUint8 imageType;
  archive.readArray16u(&m_header.colorMapStart, 2);
  //hkUint16 colorMapStart;
  //hkUint16 colorMapLength;
  m_header.colorMapBits = archive.read8u();
  archive.readArray16u(&m_header.xStart, 4);
  //hkUint16 xStart;
  //hkUint16 yStart;
  //hkUint16 width;
  //hkUint16 height;
  archive.readArray8u(&m_header.bits, 2);
  //hkUint8 bits;
  //hkUint8 descriptor;

  if (!archive.isOk())
  {
    return HK_FAILURE;
  }

  // Check for plausible size
  if ((m_header.width < 1) || (m_header.height < 1))
  {
    return HK_FAILURE;
  }

  // Check for bit depth and supported image types
  switch (m_header.bits)
  {
  case 8:
    {
      // Image type must be grayscale
      if ((m_header.imageType != 3) && (m_header.imageType != 11)) // 3: grayscale; 11: grayscale RLE
      {
        return HK_FAILURE;
      }
      break;
    }
  case 24:
  case 32:
    {
      // Image type must be truecolor
      if ((m_header.imageType != 2) && (m_header.imageType != 10)) // 2: truecolor; 10: truecolor RLE
      {
        return HK_FAILURE;
      }
      break;
    }
  default:
    {
      // Unsupported bit depth
      return HK_FAILURE;
    }
  }

  return HK_SUCCESS;
}


hkResult hkvImageFileTga::readImageData(hkStreamReader& reader)
{
  if (!isHeaderValid())
  {
    return HK_FAILURE;
  }

  hkvImage* img = new hkvImage(m_header.width, m_header.height, 1);
  if (img == NULL || img->getData() == NULL)
  {
    delete img;
    return HK_FAILURE;
  }

  setImage(0, 0, 0, img);
  hkUint8* data = img->getData();

  switch (m_header.imageType)
  {
  case 2:
  case 3:
    {
      // Uncompressed data
      int toRead = m_header.width * m_header.height * m_header.bits / 8;
      if (reader.read(data, toRead) != toRead)
      {
        return HK_FAILURE;
      }
      break;
    }
  case 10:
  case 11:
    {
      // RLE-compressed data
      if (readRleData(reader) != HK_SUCCESS)
      {
        return HK_FAILURE;
      }
      break;
    }
  }

  postprocessData();

  return HK_SUCCESS;
}


hkResult hkvImageFileTga::readRleData(hkStreamReader& reader)
{
  hkUint32 numPixels = m_header.width * m_header.height;
  hkUint32 bytesPerPixel = m_header.bits / 8;

  hkUint32 pixelIdx = 0;

  hkvImage* img = getImage(0, 0, 0);
  hkUint8* data = img->getData();

  while (reader.isOk() && (pixelIdx < numPixels))
  {
    hkUint8 chunkInfo = 0;
    if (reader.read(&chunkInfo, 1) != 1)
    {
      return HK_FAILURE;
    }

    if (chunkInfo < 128)
    {
      // Raw values follow: chunkInfo + 1 values
      hkUint32 numRawValues = hkMath::min2((hkUint32)chunkInfo + 1, numPixels - pixelIdx);
      int toRead = numRawValues * bytesPerPixel;
      if (reader.read(&data[pixelIdx * bytesPerPixel], toRead) != toRead)
      {
        return HK_FAILURE;
      }
      pixelIdx += numRawValues;
    }
    else
    {
      // RLE: next value is repeated n times
      hkUint32 numRepeats = hkMath::min2((hkUint32)chunkInfo - 127, numPixels - pixelIdx);
      hkUint8 rlePixel[4];
      if (reader.read(rlePixel, bytesPerPixel) != bytesPerPixel)
      {
        return HK_FAILURE;
      }

      for (hkUint32 repeatIdx = 0; repeatIdx < numRepeats; ++repeatIdx)
      {
        for (hkUint8 byteIdx = 0; byteIdx < bytesPerPixel; ++byteIdx)
        {
          data[((pixelIdx + repeatIdx) * bytesPerPixel) + byteIdx] = rlePixel[byteIdx];
        }
      }
      pixelIdx += numRepeats;
    }
  }

  return HK_SUCCESS;
}


void hkvImageFileTga::postprocessData()
{
  hkUint32 numPixels = m_header.width * m_header.height;
  hkUint32 bytesPerPixel = m_header.bits / 8;
  hkUint32 srcOffset = (numPixels - 1) * bytesPerPixel;
  hkUint32 destOffset = (numPixels - 1) * 4;

  hkvImage* img = getImage(0, 0, 0);
  hkUint8* data = img->getData();

  hkUint8 srcData[4];
  for (hkInt32 pixelIdx = numPixels - 1; pixelIdx >= 0; --pixelIdx)
  {
    for (hkUint8 byteIdx = 0; byteIdx < bytesPerPixel; ++byteIdx)
    {
      srcData[byteIdx] = data[srcOffset + byteIdx];
    }
    
    switch (bytesPerPixel)
    {
    case 1:
      {
        data[destOffset    ] = data[destOffset + 1] = data[destOffset + 2] = srcData[0];
        data[destOffset + 3] = 255;
        break;
      }
    case 3:
      {
        data[destOffset    ] = srcData[0];
        data[destOffset + 1] = srcData[1];
        data[destOffset + 2] = srcData[2];
        data[destOffset + 3] = 255;
        break;
      }
    case 4:
      {
        data[destOffset    ] = srcData[0];
        data[destOffset + 1] = srcData[1];
        data[destOffset + 2] = srcData[2];
        data[destOffset + 3] = srcData[3];
        break;
      }
    }

    srcOffset -= bytesPerPixel;
    destOffset -= 4;
  }
}


//-----------------------------------------------------------------------------
// hkvImageFile
//-----------------------------------------------------------------------------
hkvImageFile::hkvImageFile()
: m_headerValid(false)
{
}


hkvImageFile::~hkvImageFile()
{
  clearData();
}


hkvImageFile::RefPtr hkvImageFile::open(const char* fileName, bool readData)
{
  if (fileName == NULL)
  {
    return NULL;
  }

  // Extract the file extension, and determine the image file type from it
  int lastPeriod = hkString::lastIndexOf(fileName, '.');
  if (lastPeriod < 0)
  {
    return NULL;
  }
  const char* extension = fileName + lastPeriod + 1;

  hkvImageFileFormat format = HKV_IMAGE_FILE_FORMAT_INVALID;
  /*
  if (hkString::strCasecmp(extension, "bmp") == 0)
  {
    format = HKV_IMAGE_FILE_FORMAT_BMP;
  }
  else*/ if (hkString::strCasecmp(extension, "dds") == 0)
  {
    format = HKV_IMAGE_FILE_FORMAT_DDS;
  }
  /*else if (hkString::strCasecmp(extension, "jpg") == 0) 
  {
    format = HKV_IMAGE_FILE_FORMAT_JPG;
  }
  else if (hkString::strCasecmp(extension, "png") == 0) 
  {
    format = HKV_IMAGE_FILE_FORMAT_PNG;
  }*/
  else if (hkString::strCasecmp(extension, "tga") == 0) 
  {
    format = HKV_IMAGE_FILE_FORMAT_TGA;
  }

  if (format == HKV_IMAGE_FILE_FORMAT_INVALID)
  {
    return NULL;
  }

  // If the file type is recognized, open a stream of its contents and
  // open the image with the appropriate reader class
  hkRefPtr<hkStreamReader> reader(hkFileSystem::getInstance().openReader(fileName));
  if (reader == NULL || !reader->isOk())
  {
    return NULL;
  }
  hkBufferedStreamReader bufferedReader(reader);

  return open(bufferedReader, format, readData);
}


hkvImageFile::RefPtr hkvImageFile::open(hkStreamReader& reader, hkvImageFileFormat format, bool readData)
{
  hkvImageFile::RefPtr imageFile;
  switch (format)
  {
  case HKV_IMAGE_FILE_FORMAT_DDS:
    {
      imageFile = hkvImageFile::RefNew(new hkvImageFileDds());
      break;
    }
  case HKV_IMAGE_FILE_FORMAT_TGA:
    {
      imageFile = hkvImageFile::RefNew(new hkvImageFileTga());
      break;
    }
  default:
    {
      VASSERT_MSG(FALSE, "A valid image file format must be specified!");
      return NULL;
    }
  }

  imageFile->m_headerValid = (imageFile->readHeader(reader) == HK_SUCCESS);
  if (!imageFile->m_headerValid)
  {
    return NULL;
  }

  if (readData)
  {
    const hkUint32 numImages = imageFile->getNumImages() * imageFile->getNumFaces() * imageFile->getNumMipLevels();
    imageFile->m_images.setSize(numImages);
    for (hkUint32 imageIdx = 0; imageIdx < numImages; ++imageIdx)
    {
      imageFile->m_images[imageIdx] = NULL;
    }

    if (imageFile->readImageData(reader) != HK_SUCCESS)
    {
      imageFile->clearData();
      return NULL;
    }
  }

  return imageFile;
}


hkvImageFileFormat hkvImageFile::guessFormatFromFileName(const char* fileName)
{
  if (fileName == NULL)
  {
    return HKV_IMAGE_FILE_FORMAT_INVALID;
  }

  // Extract the file extension, and determine the image file type from it
  int lastPeriod = hkString::lastIndexOf(fileName, '.');
  if (lastPeriod < 0)
  {
    return HKV_IMAGE_FILE_FORMAT_INVALID;
  }
  const char* extension = fileName + lastPeriod + 1;

  hkvImageFileFormat format = HKV_IMAGE_FILE_FORMAT_INVALID;
  if (hkString::strCasecmp(extension, "bmp") == 0)
  {
    format = HKV_IMAGE_FILE_FORMAT_BMP;
  }
  else if (hkString::strCasecmp(extension, "dds") == 0)
  {
    format = HKV_IMAGE_FILE_FORMAT_DDS;
  }
  else if (hkString::strCasecmp(extension, "jpg") == 0) 
  {
    format = HKV_IMAGE_FILE_FORMAT_JPG;
  }
  else if (hkString::strCasecmp(extension, "png") == 0) 
  {
    format = HKV_IMAGE_FILE_FORMAT_PNG;
  }
  else if (hkString::strCasecmp(extension, "tga") == 0) 
  {
    format = HKV_IMAGE_FILE_FORMAT_TGA;
  }

  return format;
}


hkvImage* hkvImageFile::getImage(hkUint32 index, hkUint32 face, hkUint32 mipLevel) const
{
  if (index >= getNumImages() || face >= getNumFaces() || mipLevel >= getNumMipLevels())
  {
    VASSERT_MSG(false, "Image index, face index or mip level is invalid");
    return NULL;
  }

  hkUint32 arrayIndex = (index * getNumFaces() * getNumMipLevels()) + (face * getNumMipLevels()) + mipLevel;
  VASSERT(arrayIndex <= (hkUint32)m_images.getSize());
  return m_images[arrayIndex];
}


void hkvImageFile::clearData()
{
  const int numMips = m_images.getSize();
  for (int mipIdx = 0; mipIdx < numMips; ++mipIdx)
  {
    delete m_images[mipIdx];
  }
  m_images.clear();
}


void hkvImageFile::setImage(hkUint32 index, hkUint32 face, hkUint32 mipLevel, hkvImage* image)
{
  if (index >= getNumImages() || face >= getNumFaces() || mipLevel >= getNumMipLevels())
  {
    VASSERT_MSG(false, "Image index, face index or mip level is invalid");
    return;
  }

  hkUint32 arrayIndex = (index * getNumFaces() * getNumMipLevels()) + (face * getNumMipLevels()) + mipLevel;
  VASSERT(arrayIndex <= (hkUint32)m_images.getSize());

  VASSERT(m_images[arrayIndex] == NULL);

  m_images[arrayIndex] = image;
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
