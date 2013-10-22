/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HKV_BACKGROUND_ASSET_HANDLING_HPP
#define HKV_BACKGROUND_ASSET_HANDLING_HPP


#include <Common/Base/Object/hkReferencedObject.h>

#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvAssetLibrary.hpp>

class hkvSubtreeMonitor;
class hkvAssetUpdateBackgroundProcessor;
class hkvTransformationBackgroundProcessor;
struct hkvBackgroundTransformationOutput;

class hkvBackgroundAssetHandling : public hkReferencedObject
{
public:
  HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_TOOLS);

public:
  /// \brief
  ///   Settings for initializing a new instance of the class hkvBackgroundAssetHandling.
  struct Config
  {
    Config()
    {
      m_numUpdateThreads = 0;
      m_numTransformThreads = 0;
      m_enableMonitoring = true;
      m_addTransformedToRCS = false;
      m_addThumbnailsToRCS = false;
    }

    ///   The number of threads for the asset updating background tasks. If a value < 1 is passed,
    ///   the number of threads will be set to the number of physical processor cores - 1 (but at
    ///   least 1).
    int m_numUpdateThreads;

    ///   Same as m_numUpdateThreads, but for asset transformation tasks.
    int m_numTransformThreads;

    ///   Whether to enable directory monitoring (either change monitoring or timed scans of the
    ///   directory tree if the former is not possible). If disabled, only explicitly queued
    ///   scans are executed.
    bool m_enableMonitoring;

    ///   Whether to add newly transformed assets to RCS.
    bool m_addTransformedToRCS;

    ///   Whether to add newly generated asset thumbnails to RCS.
    bool m_addThumbnailsToRCS;
  };

  /// \brief
  ///   Initializes a new background asset handling subsystem (file system scans, asset updates,
  ///   asset transforms).
  hkvBackgroundAssetHandling(const Config& config);

private:
  hkvBackgroundAssetHandling(const hkvBackgroundAssetHandling&);
  hkvBackgroundAssetHandling& operator=(const hkvBackgroundAssetHandling&);
public:
  ~hkvBackgroundAssetHandling();

public:
  bool getScanAllFoldersOnStart() const;
  void setScanAllFoldersOnStart(bool scan);

  void setAddTransformedToRCS(bool addTransformed);
  void setAddThumbnailsToRCS(bool addThumbnails);

  void start();
  void stop(bool keepCurrentWork);

  bool isRunning() const;

  void queueScan(hkvAssetLibrary& library, const char* relativePath = NULL);
  void queryScanProgress(hkUint32& out_scansRemaining, hkUint32& out_changesRemaining);

  void queueTransformation(hkvAsset& asset);
  void clearPendingTransformations();
  void getTransformationOutputs(hkArray<hkvBackgroundTransformationOutput>& out_outputs);
  void queryTransformationProgress(hkUint32& out_count, hkUint32& out_remaining) const;

  void queueUpdate(hkvAsset& asset);
  void queryUpdateProgress(hkUint32& out_count, hkUint32& out_remaining) const;

private:
  hkvSubtreeMonitor* m_subtreeMonitor;
  hkvAssetUpdateBackgroundProcessor* m_updateProcessor;
  hkvTransformationBackgroundProcessor* m_transformProcessor;

  bool m_scanAllFoldersOnStart;
  bool m_wasKeepingCurrentWork;
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
