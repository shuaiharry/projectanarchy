/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VFileServeStreamManager.hpp

#ifndef DC_VBASE_VFILESERVESTREAMMANAGER_HPP
#define DC_VBASE_VFILESERVESTREAMMANAGER_HPP

#include <Vision/Runtime/Base/System/IO/Stream/IVFileStream.hpp>
#include <Vision/Runtime/Base/System/Threading/SyncPrimitive/VMutex.hpp>

#if defined( SUPPORTS_FILESERVE_CLIENT ) || defined( _VISION_DOC )

//forward declarations
typedef void CURL;
class VDiskFileStreamManager;
class VFileServeStreamManager;

/// \brief data directory used by the vFileServe Client
class VFileServerDataDirectory : public IVFileStreamManager
{
public:
  /// \brief constructor
	VFileServerDataDirectory(VFileServeStreamManager& owner);
  
private:
	VFileServerDataDirectory(const VFileServerDataDirectory&);
	VFileServerDataDirectory& operator=(const VFileServerDataDirectory&);

public:
	virtual const char* GetName() HKV_OVERRIDE;
	virtual IVFileOutStream* Create(const char* szFileName, int iFlags = 0) HKV_OVERRIDE;
	virtual int GetNumDataDirectories() HKV_OVERRIDE;
	virtual const char* GetDataDirectory(int idx = 0) HKV_OVERRIDE;
  /// \brief returns the hash of the relative part of the path stored
  inline char* GetRelativePartHash() { return m_szRelativePartHash; }

protected:
	virtual IVFileInStream* InternalOpen(const char* szFileName, int iFlags, const InternalOpenContext& ioc) HKV_OVERRIDE;
	virtual BOOL InternalExists(const char* szFileName) HKV_OVERRIDE;
	virtual BOOL InternalGetTimeStamp(const char *szFileName, VFileTime &destTime) HKV_OVERRIDE;
	virtual bool InternalSetDataDirectory(int idx, const char* szPath)  HKV_OVERRIDE;

private:
	VFileServeStreamManager& m_owner;
	VString m_sRelativePath;
  char m_szRelativePartHash[16];
};

typedef VSmartPtr<VFileServerDataDirectory> VFileServerDataDirectoryPtr;

/// \brief 
///  Client side of the vFileServe system, downloads files from the http server
class VFileServeStreamManager : public IVFileStreamManager
{
  V_DECLARE_DYNAMIC_DLLEXP(VFileServeStreamManager, VBASE_IMPEXP);

  friend class VFileServerDataDirectory;

public:
  enum HashSeed
  {
    HASH_SEED = 0x539
  };

	/// \brief constructor
	VBASE_IMPEXP VFileServeStreamManager(const char* pszHostUrl, const char* pszCacheDirectory, IVFileStreamManager* pFallbackManager);

	/// \brief destructor
	~VFileServeStreamManager();

	VBASE_IMPEXP void SendSceneChangedEvent();

	/// \brief
	///   Initializes the VFileServeStreamManager connection
	///
	/// \return
	///   returns true on success, false on error
	bool Init();

	/// \return
	///   "VFileServeStreamManager"
	virtual const char* GetName() HKV_OVERRIDE;

	virtual IVFileInStream* InternalOpen(const char* pszFileName, int iFlags, const InternalOpenContext& ioc) HKV_OVERRIDE;
	virtual IVFileOutStream* Create(const char* pszFileName, int iFlags = 0) HKV_OVERRIDE;
	virtual BOOL InternalExists(const char* pszFileName) HKV_OVERRIDE;
	virtual BOOL InternalGetTimeStamp(const char *pszFileName, VFileTime &destTime) HKV_OVERRIDE;
	virtual int GetNumDataDirectories() HKV_OVERRIDE;
	virtual bool InternalSetDataDirectory(int idx, const char* pszPath) HKV_OVERRIDE;
	virtual const char* GetDataDirectory(int idx = 0) HKV_OVERRIDE;
	virtual void SetEnableDirectoryCaching(bool bStatus, int idx = 0) HKV_OVERRIDE;
	virtual bool IsDirectoryCachingEnabled(int idx = 0) HKV_OVERRIDE;

	/// \brief gets the host url
	inline const char* GetHostUrl() { return m_sHostUrl.AsChar(); }

	/// \brief Returns whether the server can be connected to.
	inline bool IsConnected() { return m_bConnected; }

  /// \brief sets the data root directory
  inline void SetDataRoot(const char* pszDataRoot) { m_sDataRoot = pszDataRoot; }

	/// \brief 
	///   Tries to reconnect to the server.
	bool Reconnect();

	virtual int GetNumSubManagerSlots() const HKV_OVERRIDE;

	virtual IVFileStreamManager* GetSubManager(int iSlot) const HKV_OVERRIDE;

private:

	enum VBufferedFileState_e
	{
		VBFS_MISSING,
		VBFS_UP_TO_DATE,
		VBFS_DOWNLOADED
	};

	enum Constants
	{
		NumDataDirectories = 32
	};

  struct DownloadResult
  {
    bool bSuccessful;
    double lfDownloadSize;
    int iHttpResponseCode;
    time_t serverLastModified;

    DownloadResult() :
      bSuccessful(false),
      lfDownloadSize(0.0),
      iHttpResponseCode(404)
    {}
  };

	bool DetermineServerTime(time_t &serverTime);

	void GetCachedFilePath(char* szDest, size_t uiDestSize, const char* szFilename, const char* szRelativePartHash);

	bool GetUpToDateFile(const char* pszFileName, const char* pszDataDir = NULL, const char* szOriginalFile = NULL);
	DownloadResult TryDownloadingFile(const char* pszFileName, bool bCached, time_t lastModified);

	// copies the last downloaded file (if available) to its cached file path
	void CopyDownloadedFile();

  void RemoveDataDir(CURL* pCurl, int idx);

  void HandleHeader(const VString& name, const VString& value);

	static size_t CurlWriteCallback(char* data, size_t numElements, size_t elementSize, void* userData);
  static size_t CurlHeaderCallback(void* ptr, size_t numElements, size_t elementSize, void* userData);
	struct CurlBuffer
	{
		char data[128];
		unsigned int pos;
		CurlBuffer() : pos(0) {}
	};

	bool CheckForSuccessfulConnection(int iErrorCode, const char* szUrl);
	void DisplayConnectionWarning();

	void MakeCombinedPath(const char* szFileName, const char* szDataDir, VStaticString<FS_MAX_PATH>& sFinalPath);
	IVFileInStream* InternalOpen(const char* pszFileName, const char* pszDataDir, int iFlags, const InternalOpenContext& ioc);
	BOOL InternalExists(const char* pszFileName, const char* pszDataDir);
	BOOL InternalGetTimeStamp(const char *pszFileName, const char* pszDataDir, VFileTime &destTime);

	//escapes the string, copies the escaped contents to dst
	static void EscapePathForHttp(const char* pszSrc, char* pszDst, size_t uiDstLength);

private:
	bool m_bInitialized;
	bool m_bDoDirectoryCaching;
	bool m_bConnected;
  VStaticString<FS_MAX_PATH> m_sCachePath;
	VStaticString<FS_MAX_PATH> m_sTempFilePath;
	VStaticString<FS_MAX_PATH> m_sBufferedFile;
	VStaticString<FS_MAX_PATH> m_sBufferedCachedFilePath;
	VBufferedFileState_e m_BufferedFileState;
	VFileTime m_BufferedFileModifiedTime;
  // The hash of the relative part of the directory the currently buffered file is in.
  // If the file is in the path Vision/Data/Samples/FileServe and the relative directory given was "FileServe" the
  // base directory will be Vision/Data/Samples and the hash will be computed from "FileServe" on the server side
  VStaticString<16> m_sRelativePartHash;

	VStaticString<128> m_sHostUrl;
	VStaticString<FS_MAX_PATH> m_sCacheSubDir; // the currently used cache directory for the last file
  VStaticString<FS_MAX_PATH> m_sSubDir; // the currently used sub directory for the last file
	VStaticString<128> m_sServerPrefixDir; // the sub-directory in the cache folder specified by the server prefix
  VStaticString<FS_MAX_PATH> m_sDataRoot; // the Havok root directory in which the "Data" folder can be found in

	CURL* m_pCurl;
	VSmartPtr<VDiskFileStreamManager> m_spDiskFileStreamManager;
  VSmartPtr<IVFileStreamManager> m_spFallbackManager;
	VFileServerDataDirectoryPtr m_DataDirectories[NumDataDirectories]; //data directories mirrored from the server.
	VMutex m_Mutex;
	time_t m_timeOffset;
};



#endif // defined( SUPPORTS_FILESERVE_CLIENT )

#endif // DC_VBASE_VFILESERVESTREAMMANAGER_HPP

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
