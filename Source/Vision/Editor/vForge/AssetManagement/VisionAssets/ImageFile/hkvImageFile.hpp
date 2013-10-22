/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HKV_IMAGE_FILE_HPP_INCLUDED
#define HKV_IMAGE_FILE_HPP_INCLUDED

class hkvEnumDefinition;

enum hkvImageFileFormat
{
  HKV_IMAGE_FILE_FORMAT_INVALID,
  HKV_IMAGE_FILE_FORMAT_BMP,
  HKV_IMAGE_FILE_FORMAT_DDS,
  HKV_IMAGE_FILE_FORMAT_JPG,
  HKV_IMAGE_FILE_FORMAT_PNG,
  HKV_IMAGE_FILE_FORMAT_TGA,
  HKV_IMAGE_FILE_FORMAT_COUNT
};

extern const char* hkvImageFileFormatNames[HKV_IMAGE_FILE_FORMAT_COUNT];
const hkvEnumDefinition& getImageFileFormatDefinition();

class hkStreamReader;

class hkvImage
{
public:
  hkvImage(hkUint32 width, hkUint32 height, hkUint32 depth);
private:
  hkvImage(const hkvImage&);
  hkvImage& operator=(const hkvImage&);

public:
  hkUint32 getWidth() const { return m_width; }
  hkUint32 getHeight() const { return m_height; }
  hkUint32 getDepth() const { return m_depth; }

  hkUint32 getDataSize() const { return m_sliceLength; }

  const hkUint8* getData(hkUint32 slice = 0) const;
  hkUint8* getData(hkUint32 slice = 0);

private:
  hkUint32 m_width;
  hkUint32 m_height;
  hkUint32 m_depth;
  hkUint32 m_sliceLength;
  hkArray<hkUint8> m_data;
};

class hkvImageFile : public hkReferencedObject
{
public:
  HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_TOOLS);

public:
  typedef hkRefPtr<hkvImageFile> RefPtr;
  typedef hkRefPtr<const hkvImageFile> RefCPtr;
  typedef hkRefNew<hkvImageFile> RefNew;

protected:
  hkvImageFile();
private:
  hkvImageFile(const hkvImageFile&);
  hkvImageFile& operator=(const hkvImageFile&);
public:
  virtual ~hkvImageFile();

public:
  static RefPtr open(const char* fileName, bool readData);
  static RefPtr open(hkStreamReader& reader, hkvImageFileFormat format, bool readData);

  static hkvImageFileFormat guessFormatFromFileName(const char* fileName);

public:
  virtual hkvImageFileFormat getFileFormat() const = 0;

  // If false, the file could not be opened or the header format
  // was not valid.
  bool isHeaderValid() const { return m_headerValid; }
  
  virtual hkUint32 getWidth() const = 0;
  virtual hkUint32 getHeight() const = 0;
  virtual hkUint32 getDepth() const { return 1; }

  virtual hkUint32 getNumImages() const { return 1; }
  virtual hkUint32 getNumFaces() const { return 1; }
  virtual hkUint32 getNumMipLevels() const { return 1; }

  // True, if the source file had a valid alpha channel. If false, the alpha
  // values of the image data array are set to 255.
  virtual bool hasAlpha() const = 0;

  bool isDataValid() const { return !m_images.isEmpty(); }

  hkvImage* getImage(hkUint32 index, hkUint32 face, hkUint32 mipLevel) const;

protected:
  virtual hkResult readHeader(hkStreamReader& reader) = 0;
  virtual hkResult readImageData(hkStreamReader& reader) = 0;

  void clearData();
  void setImage(hkUint32 index, hkUint32 face, hkUint32 mipLevel, hkvImage* image);

private:
  bool m_headerValid;
  hkArray<hkvImage*> m_images;
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
