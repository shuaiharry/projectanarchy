/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file hkvAssetLibrary.hpp

#ifndef HKV_ASSET_LIBRARY_HPP_INCLUDED
#define HKV_ASSET_LIBRARY_HPP_INCLUDED

#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvAssetQuery.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Assets/hkvAsset.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Assets/hkvDummyAsset.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Base/hkvBase.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Base/hkvCriticalSectionLock.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Base/hkvCriticalSectionLock.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Base/hkvFlagFieldArray.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Profiles/hkvProfile.hpp>

#include <vector>
#include <map>
#include <set>
#include <deque>

#include <Common/Base/Thread/CriticalSection/hkCriticalSection.h>
#include <Common/Base/Thread/Semaphore/hkSemaphore.h>
#include <Common/Base/System/Io/FileSystem/hkFileSystem.h>
#include <Common/Base/Container/BitField/hkBitField.h>


class hkvAssetManager;
class hkvAssetTrackedDirectory;

/// \brief
///   Configuration properties of an asset library.
class hkvAssetLibraryConfig : public hkvIProperties
{
  friend class hkvAssetLibrary;

public:
  ASSETFRAMEWORK_IMPEXP hkvAssetLibraryConfig();
  ASSETFRAMEWORK_IMPEXP hkvAssetLibraryConfig(const hkvAssetLibraryConfig& config);

  /// \brief
  ///   Constructor; initializes this object and sets the specified library name.
  /// \param name
  ///   the name of the asset library
  ASSETFRAMEWORK_IMPEXP hkvAssetLibraryConfig(const char* name);

  ASSETFRAMEWORK_IMPEXP void operator=(const hkvAssetLibraryConfig& rhs);

  /// \brief
  ///   Returns the name of the asset library
  ASSETFRAMEWORK_IMPEXP const char* getName() const;

  /// \brief
  ///   Returns the format version of the asset library
  ASSETFRAMEWORK_IMPEXP hkUint32 getFormatVersion() const;

  /// \brief
  ///   Sets the format version of the asset library to the current format version.
  ASSETFRAMEWORK_IMPEXP void setFormatVersionCurrent();

  /// \brief
  ///   Returns the timestamp of the last modification to this object.
  ASSETFRAMEWORK_IMPEXP hkFileSystem::TimeStamp getLastEditTime() const;

  /// \brief
  ///   Sets the timestamp of the last modification to the current time.
  ASSETFRAMEWORK_IMPEXP void touch();

  ASSETFRAMEWORK_IMPEXP virtual const char* getTypeName() const HKV_OVERRIDE;
  ASSETFRAMEWORK_IMPEXP virtual void getProperties(hkvPropertyList& properties, hkvProperty::Purpose purpose) const HKV_OVERRIDE;
  ASSETFRAMEWORK_IMPEXP virtual void setProperty(const hkvProperty& prop, const hkArray<hkStringPtr>& path, hkUint32 stackIndex, hkvProperty::Purpose purpose) HKV_OVERRIDE;

public:
  /// \brief
  ///   Returns the current format version for serializing this object.
  ASSETFRAMEWORK_IMPEXP static hkUint32 getCurrentFormatVersion();

private:
  hkStringPtr m_name;
  hkUint32 m_formatVersion;
  hkFileSystem::TimeStamp m_lastEditTime;

private:
  static hkUint32 s_currentFormatVersion; // Increment this value for all internal changes that make an update of assets necessary!
};


/// \brief
///   Helper class to read in one asset library file into an intermediate internal structure for
///   further processing.
class hkvAssetLibraryReader : public hkReferencedObject, public hkvPropertyReader
{
public:
  HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_TOOLS);

public:
  typedef hkRefNew<hkvAssetLibraryReader> RefNew;
  typedef hkRefPtr<hkvAssetLibraryReader> RefPtr;
  typedef hkRefPtr<const hkvAssetLibraryReader> RefCPtr;

public:
  hkvAssetLibraryReader() {}
private:
  hkvAssetLibraryReader(const hkvAssetLibraryReader&);
  hkvAssetLibraryReader& operator=(const hkvAssetLibraryReader&);
public:
  ~hkvAssetLibraryReader() {}

public:
  /// \brief
  ///   Returns the asset library configuration that has been read by this reader.
  const hkvAssetLibraryConfig& getConfig() const { return m_config; }

  /// \brief
  ///   Reads and parses an asset library file.
  ///
  /// All objects resulting from parsing the asset library file are stored within this reader instance;
  /// no global state is modified.
  ///
  /// \param filePath
  ///   the path to the asset library file to read
  /// \return
  ///   \c true if reading the file succeeded; \c false if an error occurred.
  bool read(const char* filePath);

  virtual hkvJsonStreamReadHandler::ParsingResult ParseString(const unsigned char* pszValue, size_t len) HKV_OVERRIDE;
  virtual hkvJsonStreamReadHandler::ParsingResult ParseMapKey(const unsigned char* pszValue, size_t len) HKV_OVERRIDE;

  /// \brief
  ///   Retrieves the first asset available in the internal asset map, and removes it from
  ///   that map.
  /// \param out_key
  ///   if successful, receives the map key (= the relative path) of the asset
  /// \param out_asset
  ///   if successful, receives a pointer to the asset
  /// \return
  ///   \c true if an asset was retrieved and written to the output parameters; \c false
  ///   if the internal map was empty.
  bool popFirstAsset(hkStringBuf& out_key, hkvDummyAsset::RefPtr& out_asset);

  /// \brief
  ///   Retrieves a specific asset from the internal asset map, and removes it from
  ///   that map.
  /// \param key
  ///   the key identifying the asset to retrieve
  /// \param out_asset
  ///   if successful, receives a pointer to the asset
  /// \return
  ///   \c true if an asset was retrieved and written to the output parameter; \c false
  ///   if no matching asset was found in the internal map.
  bool popSpecificAsset(hkStringBuf& key, hkvDummyAsset::RefPtr& out_asset);

  /// \brief
  ///   Retrieves the first tracked file available in the internal map, and removes it from
  ///   that map.
  /// \param out_key
  ///   if successful, receives the map key (= the relative path) of the tracked file
  /// \param out_file
  ///   if successful, receives a pointer to the tracked file
  /// \return
  ///   \c true if a tracked file was retrieved and written to the output parameters; \c false
  ///   if the internal map was empty.
  bool popFirstTrackedFile(hkStringBuf& out_key, hkRefPtr<hkvTrackedFileDummy>& out_file);

  /// \brief
  ///   Retrieves a specific tracked file from the internal map, and removes it from
  ///   that map.
  /// \param key
  ///   the key identifying the tracked file to retrieve
  /// \param out_file
  ///   if successful, receives a pointer to the tracked file
  /// \return
  ///   \c true if a tracked file was retrieved and written to the output parameter; \c false
  ///   if no matching tracked file was found in the internal map.
  bool popSpecificTrackedFile(hkStringBuf& key, hkRefPtr<hkvTrackedFileDummy>& out_file);

private:
  typedef std::map<const hkStringPtr, hkvDummyAsset::RefPtr, hkvStringSafeLessI<hkStringPtr> > AssetMap;
  typedef std::map<const hkStringPtr, hkRefPtr<hkvTrackedFileDummy>, hkvStringSafeLessI<hkStringPtr> > TrackedFileDummyMap;

private:
  // Json parser stuff
  hkStringPtr m_loaderPath;

  hkvAssetLibraryConfig m_config;
  AssetMap m_assets;
  TrackedFileDummyMap m_trackedFiles;
};


/// \brief
///   The hkvAssetLibrary class represents a single asset library that was loaded by adding a data directory.
///   The library manages all assets below its base path.
///
/// Asset-related data is stored in a folder "AssetMgmt_data" below the base directory of the asset library.
/// The asset library also creates the lookup table for the engine that maps assets to actual files.
class hkvAssetLibrary : public hkReferencedObject
{
  friend class hkvAssetManager;
  friend class hkvAssetTrackedFile;

public:
  HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_TOOLS);

public:
  typedef hkRefNew<hkvAssetLibrary> RefNew;
  typedef hkRefPtr<hkvAssetLibrary> RefPtr;
  typedef hkRefPtr<const hkvAssetLibrary> RefCPtr;

public:
  /// \brief
  ///   Holds information about the current state or the results of a background or on-demand
  ///   asset processing operation.
  struct AssetStatistics
  {
    AssetStatistics()
    : m_totalAssets(0), m_numUpdatePending(0), m_numUpdateError(0), m_numUpToDate(0),
      m_numTransformPending(0), m_numTransformError(0), m_numTransformed(0)
    {
    }

    size_t m_totalAssets;         ///< The total number of assets covered by these statistics
    size_t m_numUpdatePending;    ///< The number of assets queued for update
    size_t m_numUpdateError;      ///< The number of assets for which the update has failed
    size_t m_numUpToDate;         ///< The number of assets successfully updated
    size_t m_numTransformPending; ///< The number of assets that are queued for transformation
    size_t m_numTransformError;   ///< The number of assets for which the transformation failed
    size_t m_numTransformed;      ///< The number of assets successfully transformed or found not needing to be transformed
  };

private:
  struct AssetDeletion
  {
    hkFileSystem::TimeStamp m_deleteTime;
    hkvPropertyList m_properties;
  };

public:
  /// \brief
  ///   Constructor; initializes this asset library object.
  /// \param libraryPath
  ///   the base path of the asset library this object manages
  /// \param create
  ///   whether a new library should be created or an existing library should be opened. At 
  ///   construction time, this merely sets internal state, which will later be used by
  ///   loadLibrary().
  /// \param isProjectRoot
  ///   whether this asset library is the main asset library of the project (i.e., this is the
  ///   project directory). While this constraint is not enforced, exactly one of the libraries 
  ///   opened at the same time should have that flag set.
  ASSETFRAMEWORK_IMPEXP hkvAssetLibrary(const char* libraryPath, bool create, bool isProjectRoot);

  /// \brief
  ///   Destructor.
  ASSETFRAMEWORK_IMPEXP ~hkvAssetLibrary();

  /// \brief
  ///   Acquires a lock under which thread-safe access to the library is possible.
  ///
  /// The lock is a light-weight object returned by value. The lock object is assignable, and
  /// the underlying lock is released when the last copy of the lock object is destroyed. This
  /// call blocks until the lock has been successfully acquired.
  ///
  /// \note
  ///   Currently, this is a global lock on all asset data. Don't perform lengthy operations under
  ///   a lock, and keep an eye on the locking order if you need to acquire other locks as well to
  ///   avoid deadlocks.
  ///
  /// \return
  ///   a lock object
  ASSETFRAMEWORK_IMPEXP hkvCriticalSectionLock acquireLock() const;

  /// \brief
  ///   Clears all data managed by this object.
  ASSETFRAMEWORK_IMPEXP void clear();

  /// \brief
  ///   Loads or creates the asset library based on the parameters passed when constructing this
  ///   object.
  ///
  /// The loading behavior can be influenced by the parameters specified. Normally, the \c AssetMgmt_data
  /// subdirectory is scanned for all \c .assetlib files, and the asset data of all files is merged
  /// during loading.
  ///
  /// By passing a local file hint, the merging still takes place, but the saved timestamp of the
  /// asset files is taken from that file if possible. This saves a lot of unnecessary asset updating
  /// if the project is under source control and multiple working copies with different file timestamps
  /// exist.
  ///
  /// When a specific input file should be used, only this file is loaded, and no merging takes place.
  /// This parameter is most useful in situations when a known good asset library should be processed
  /// by a tool, and you want to make sure that you load the library exactly as it was saved.
  ///
  /// \param localFileHint
  ///   if specified, the name of the asset library file that will be written specifically for this
  ///   user and machine. It's not an error if this file doesn't exist.
  /// \param inputFile
  ///   if specified, the .assetlib file to load. This file must exist.
  /// \return
  ///   the result of the load operation
  ASSETFRAMEWORK_IMPEXP hkvAssetLibraryLoadResult loadLibrary(const char* localFileHint = NULL, const char* inputFile = NULL);

  /// \brief
  ///   Saves this asset library to a file.
  ///
  /// As with loading, the saving behavior of this library can be influenced by the passed parameters.
  ///
  /// If a target file is specified, the library is saved to that file. Otherwise, a file name is
  /// automatically generated based on the current user and computer names.
  ///
  /// By default, a deletion record is saved for each asset that is removed from this asset libraries.
  /// Deletion records are retained as long as there is any asset library file for this asset library
  /// listing the respective asset as non-deleted. Purging of deletion records takes place when the
  /// library is loaded. If so desired, writing of deletion records can be suppressed. This should be 
  /// done for asset libraries that are to be distributed, where usually only one default \c .assetlib
  /// file exists.
  ///
  /// \param targetFile
  ///   the file to save the asset library to. If not specified, a default file name is generated based
  ///   on the current user and computer name.
  /// \param omitDeletions
  ///   if true, no deletion records are written to the asset library.
  ASSETFRAMEWORK_IMPEXP bool saveLibrary(const char* targetFile = NULL, bool omitDeletions = false);

  /// \brief
  ///   Returns the name of this asset library.
  ASSETFRAMEWORK_IMPEXP const char* getName() const;

  /// \brief
  ///   Returns the absolute path to the base folder of this asset library.
  ASSETFRAMEWORK_IMPEXP const char* getPath() const;

  /// \brief
  ///   Queries the relative path for transformed assets, and writes it to a string buffer.
  /// \param out_path
  ///   the buffer to receive the transformed asset path. This is a path relative to the
  ///   base directory of this asset library.
  ASSETFRAMEWORK_IMPEXP void getRelativeAssetCachePath(hkStringBuf& out_path) const;

  /// \brief
  ///   Queries the absolute path for transformed assets, and writes it to a string buffer.
  /// \param out_path
  ///   the buffer to receive the transformed asset path.
  ASSETFRAMEWORK_IMPEXP void getAbsoluteAssetCachePath(hkStringBuf& out_path) const;

  /// \brief
  ///   Queries the relative path for assets thumbnails, and writes it to a string buffer.
  /// \param out_path
  ///   the buffer to receive the asset thumbnails path. This is a path relative to the
  ///   base directory of this asset library.
  ASSETFRAMEWORK_IMPEXP void getRelativeThumbnailCachePath(hkStringBuf& out_path) const;

  /// \brief
  ///   Queries the absolute path for assets thumbnails, and writes it to a string buffer.
  /// \param out_path
  ///   the buffer to receive the asset thumbnails path.
  ASSETFRAMEWORK_IMPEXP void getAbsoluteThumbnailCachePath(hkStringBuf& out_path) const;

  /// \brief
  ///   Returns whether this asset library has pending modifications.
  /// \note
  ///   This indicator is not completely reliable currently. When in doubt, always assume 
  ///   that there are pending changes, and save the asset library.
  /// \return
  ///   \c true if there are pending changes; \c false otherwise
  ASSETFRAMEWORK_IMPEXP bool isDirty() const { return m_isDirty; }

  /// \brief
  ///   Returns whether this asset library is currently being loaded.
  ASSETFRAMEWORK_IMPEXP bool isLoading() const  { return m_isLoading; }

  /// \brief
  ///   Returns whether this asset library has been flagged as being the project root.
  ASSETFRAMEWORK_IMPEXP bool isProjectRoot() const  { return m_isProjectRoot; }

  /// \brief
  ///   Sets whether this asset library should be considered the project root.
  /// \param root
  ///   whether this asset library should be considered the project root
  ASSETFRAMEWORK_IMPEXP void setProjectRoot(bool root) { m_isProjectRoot = root; }

  /// \brief
  ///   Returns the number of assets managed by this asset library.
  ASSETFRAMEWORK_IMPEXP hkUint32 getNumberOfAssets() const;

  /// \brief
  ///   Returns an asset by index.
  /// \param assetIndex
  ///   index of the asset to return
  /// \return
  ///   the requested asset, or \c NULL, if \c assetIndex is not a valid index
  ASSETFRAMEWORK_IMPEXP hkvAsset::RefPtr getAssetByIndex(hkUint32 assetIndex) const;

  /// \brief 
  ///   Returns an asset by its relative file path.
  /// \param assetPath
  ///   the relative file path to search for
  /// \return
  ///   the requested asset, or \c NULL, if no asset could be found by this path
  ASSETFRAMEWORK_IMPEXP hkvAsset::RefPtr getAssetByPath(const char* assetPath) const;

  /// \brief 
  ///   Returns a tracked file its relative path.
  /// \param relativePath
  ///   the relative file path to search for
  /// \return
  ///   the requested tracked file, or \c NULL, if no file could be found by this path
  ASSETFRAMEWORK_IMPEXP hkvAssetTrackedFile* getTrackedFileByPath(const char* relativePath) const;

  /// \brief
  ///   Creates a tracked file entry for a missing dependency. This can be used to track
  ///   the dependencies of an asset and to update the asset once a missing dependency is
  ///   created on disk.
  /// \param relativePath
  ///   the relative path of the tracked file to create.
  /// \return
  ///   the newly created file, or, if a file already existed for \c relativePath, the
  ///   existing file. In both cases, the file is referenced and owned by the asset library's
  ///   directory structure.
  ASSETFRAMEWORK_IMPEXP hkvAssetTrackedFile* createMissingTrackedFile(const char* relativePath);

  /// \brief
  ///   Executes any actions necessary after the project (not just this asset library) has been loaded. This
  ///   method calls hkvAsset::handleProjectLoaded() on each asset in this library.
  ASSETFRAMEWORK_IMPEXP void handleProjectLoaded();

  /// \brief
  ///   Changes the modification time of this library and all assets contained therein by the specified number
  ///   of milliseconds.
  /// \param offsetMs
  ///   the offset in milliseconds.
  ASSETFRAMEWORK_IMPEXP void changeModificationTimes(int offsetMs);

  /// \brief
  ///   Runs an asset query on this asset library.
  /// \param query
  ///   the query to run. Results are collected within the query itself.
  ASSETFRAMEWORK_IMPEXP void query(hkvAssetQuery& query) const;

  /// \brief
  ///   Returns the root directory object of this asset library.
  ASSETFRAMEWORK_IMPEXP hkvAssetTrackedDirectory& getRootDirectory();

  /// \brief
  ///   Returns a reference to the list of ignored files and folders.
  ///
  /// Each entry in the list is a path component; wild cards are allowed. If, while processing, any one of
  /// these components is encountered in the path to a
  ASSETFRAMEWORK_IMPEXP const hkArray<hkStringPtr>& getIgnoreList() const;

  /// \brief
  ///   Invalidates all asset transforms; i.e., resets the transformation state of all assets to "Unknown".
  ASSETFRAMEWORK_IMPEXP void invalidateAssetTransforms();

  /// \brief
  ///   Returns the status flags for the specified asset.
  /// \param assetIndex
  ///   index of the asset to return the flags for
  /// \return
  ///   the flags of the specified asset
  ASSETFRAMEWORK_IMPEXP hkvAsset::FlagsType getAssetFlags(hkUint32 assetIndex);

  /// \brief
  ///   Changes the status flags of the specified asset
  /// \param assetIndex
  ///   index of the asset to change the flags of
  /// \param flags
  ///   the target state of the flags
  /// \param mask
  ///   a mask defining which flags may be changed
  ASSETFRAMEWORK_IMPEXP void setAssetFlags(hkUint32 assetIndex, hkvAsset::FlagsType flags, hkvAsset::FlagsType mask);

  /// \brief
  ///   Fills the passed statistics object with status information about this library's assets.
  /// \param stats
  ///   the statistics object to fill
  ASSETFRAMEWORK_IMPEXP void collectAssetStatistics(AssetStatistics& stats);

  /// \brief
  ///   Cleans up stale asset files (both transformed assets and thumbnails) in this library's 
  ///   asset cache.
  /// \return
  ///   the number of stale files deleted, or -1, if an error occurred
  ASSETFRAMEWORK_IMPEXP hkInt32 cleanUpAssetCache();

  /// \brief
  ///   Generates a profile-specific lookup table and returns a string of all entries.
  /// \param profile
  ///   the asset profile for which to generate the lookup table
  /// \param forEditor
  ///   whether the lookup table is created for a scene open in vForge (\c true) or for
  ///   an exported scene (\c false)
  /// \param assetPreviewMode
  ///   whether to generate entries for asset preview mode (only relevant if \c forEditor is \c true)
  /// \param out_buffer
  ///   the string buffer to receive the lookup table
  /// \return
  ///   a pointer to the string in \c out_buffer
  ASSETFRAMEWORK_IMPEXP const char* generateLookupTable(hkvProfile::RefCPtr profile, bool forEditor, bool assetPreviewMode, hkStringBuf& out_buffer) const;

  /// \brief
  ///   Generates a profile-specific lookup table and writes it to a file.
  /// \param profile
  ///   the asset profile for which to generate the lookup table
  /// \param forEditor
  ///   whether the lookup table is created for a scene open in vForge (\c true) or for
  ///   an exported scene (\c false)
  /// \param assetPreviewMode
  ///   whether to generate entries for asset preview mode (only relevant if \c forEditor is \c true)
  /// \param out_fileName
  ///   a hkStringPtr pointer to receive the lookup table. May be \c NULL if the output file name
  ///   is not needed.
  /// \return
  ///   \c true if writing the lookup table succeeded; \c false otherwise
  ASSETFRAMEWORK_IMPEXP bool writeLookupTable(hkvProfile::RefCPtr profile, bool forEditor, bool assetPreviewMode, hkStringPtr* out_fileName = NULL) const;

  /// \brief
  ///   Invalidates the cached lookup table entry for the specified asset.
  /// \param assetPtr
  ///   the asset for which to invalidate the lookup entry
  ASSETFRAMEWORK_IMPEXP void invalidateLookupTableEntry(hkvAsset* assetPtr) const;

  /// \brief
  ///   Generates a profile-specific lookup table and writes the entries to a vector.
  /// \param profile
  ///   the asset profile for which to generate the lookup table
  /// \param forEditor
  ///   whether the lookup table is created for a scene open in vForge (\c true) or for
  ///   an exported scene (\c false)
  /// \param assetPreviewMode
  ///   whether to generate entries for asset preview mode (only relevant if \c forEditor is \c true)
  /// \param out_entries
  ///   receives the entries, grouped by asset.
  ASSETFRAMEWORK_IMPEXP void getLookupTable(hkvProfile::RefCPtr profile, bool forEditor, bool assetPreviewMode, std::vector< std::vector<hkvLookupTableEntry> >& out_entries) const;

private:
  hkvAssetLibraryLoadResult loadLibraryInternal(const std::vector<hkStringPtr>& libraryFiles, const char* localFileHint);
  void useMostRecentConfiguration(const hkArray<hkvAssetLibraryReader::RefPtr>& libraryReaders);
  void useMostRecentAssets(const hkArray<hkvAssetLibraryReader::RefPtr>& libraryReaders, int localFile);
  void useMostRecentTrackedFiles(const hkArray<hkvAssetLibraryReader::RefPtr>& libraryReaders, int localFile);

  hkvAsset::RefPtr addAssetFromDummy(hkvDummyAsset& dummyAsset);
  void addAssetToLibrary(hkvAsset& asset);
  void removeAssetFromLibrary(hkvAsset& asset);
  void removeAssetFromLibrary(int index);

  void generateLookupTable(hkvProfile::RefCPtr profile, bool forEditor, bool assetPreviewMode) const;
  void clearLutEntry(int index);

  bool generateTransformedAssetFileNames(hkArray<hkStringPtr>& out_names);
  bool generateTransformedAssetFileNames(const hkvProfile& profile, hkArray<hkStringPtr>& out_names);
  bool generateAssetCacheStaleFilesList(std::vector<hkStringPtr>& out_files);

  bool generateThumbnailFileNames(hkArray<hkStringPtr>& out_names);
  bool generateThumbnailCacheStaleFilesList(std::vector<hkStringPtr>& out_files);

  bool generateExistingFilesList(const char* path, hkArray<hkStringPtr>& out_files);

  struct AssetMapComp {
    bool operator() (const char* const lhs, const char* const rhs) const
    {
      return hkvStringHelper::safeCompare(lhs, rhs, true) < 0;
    }
  };

  bool m_isProjectRoot;
  bool m_createLibrary;
  bool m_isLoaded;
  bool m_isDirty;
  bool m_isLoading;

  hkvAssetLibraryConfig m_config;

  hkStringPtr m_absolutePath;
  hkStringPtr m_localFileName;
  hkUint32 m_libraryIndex; // Set by the hkvAssetManager

  hkArray<hkvAsset::RefPtr> m_assets;
  hkvFlagFieldArray<hkvAsset::FlagsType> m_assetFlags;

  typedef std::map<const hkStringPtr, AssetDeletion, hkvStringSafeLessI<hkStringPtr> > AssetDeletionMap;
  AssetDeletionMap m_deletedAssets;

  hkRefPtr<hkvAssetTrackedDirectory> m_rootDirectory;
  hkArray<hkStringPtr> m_ignoreList;


  // LUT caching
  mutable hkStringPtr m_lutEntriesProfile;
  mutable std::vector< std::vector<hkvLookupTableEntry> > m_lutEntriesEngine;
  mutable std::vector< std::vector<std::string> > m_lutStringsEngine;
  mutable hkBitField m_validLutEntriesEngine;

  mutable std::vector< std::vector<hkvLookupTableEntry> > m_lutEntriesEditor;
  mutable std::vector< std::vector<std::string> > m_lutStringsEditor;
  mutable hkBitField m_validLutEntriesEditor;
  mutable bool m_LutEntriesEditorAssetPreviewMode;

public:
  ASSETFRAMEWORK_IMPEXP static const char* s_assetDataDirectory;
  ASSETFRAMEWORK_IMPEXP static const char* s_assetCacheDirectory;
  ASSETFRAMEWORK_IMPEXP static const char* s_thumbnailCacheDirectory;
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
