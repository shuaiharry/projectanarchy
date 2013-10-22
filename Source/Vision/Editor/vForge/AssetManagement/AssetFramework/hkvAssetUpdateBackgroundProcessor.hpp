/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HKV_ASSET_UPDATE_BACKGROUND_PROCESSOR_HPP_INCLUDED
#define HKV_ASSET_UPDATE_BACKGROUND_PROCESSOR_HPP_INCLUDED

#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvAssetLibrary.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Assets/hkvAsset.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Base/hkvBackgroundProcessor.hpp>
#include <Vision/Runtime/Base/IPC/VIOLoop.hpp>

#include <vector>

class hkvMsgAssetDependency;
class hkvMsgAssetProcessed;
class hkvMsgAssetPropertyHint;

struct hkvAssetUpdateInput
{
  hkvAssetUpdateInput(hkvAsset* asset = NULL)
  : m_asset(asset)
  {
  }

  bool operator==(const hkvAssetUpdateInput& other) const
  {
    return (m_asset == other.m_asset);
  }

  bool operator<(const hkvAssetUpdateInput& other) const
  {
    return (m_asset < other.m_asset);
  }

  void operator=(const hkvAssetUpdateInput& other)
  {
    m_asset = other.m_asset;
  }

  hkvAsset::RefPtr m_asset;
};

struct hkvAssetUpdateOutput
{
};


class hkvAssetUpdateIPCIOLoop : public IOLoop
{
public:
  hkvAssetUpdateIPCIOLoop(UINT localId, UINT pipeId, HANDLE clientNotifyEvent,
    IChannelListener* listener, Channel** channel);
private:
  hkvAssetUpdateIPCIOLoop(const hkvAssetUpdateIPCIOLoop&);
  hkvAssetUpdateIPCIOLoop& operator=(const hkvAssetUpdateIPCIOLoop&);

public:
  virtual void OnMessageReceived(const Message &) HKV_OVERRIDE;

private:
  UINT m_localId;
  UINT m_pipeId;
  HANDLE m_clientNotifyEvent;
};


class hkvAssetUpdateIPCInterface : public IChannelListener
{
public:
  hkvAssetUpdateIPCInterface(hkUint32 localId, hkUint32 pipeId);
private:
  hkvAssetUpdateIPCInterface(const hkvAssetUpdateIPCInterface&);
  hkvAssetUpdateIPCInterface& operator=(const hkvAssetUpdateIPCInterface&);
public:
  ~hkvAssetUpdateIPCInterface();

public:
  enum ProcessResult {
    PROCESS_OK,
    PROCESS_FAILED,
    PROCESS_ABORTED,
    PROCESS_TIMED_OUT
  };

public:
  virtual void OnMessageReceived(const Message& msg) HKV_OVERRIDE;

  bool isValid() const;
  ProcessResult processAsset(hkvAsset& asset, bool createThumbnail, bool determineDependencies, 
    bool getPropertyHint, float timeoutSeconds = 0);

  void abort();
  bool isAborted() const;

  const std::vector<hkStringPtr>& getAssetDependencies() const;
  const char* getAssetPropertyHint() const;

private:
  void send(const Message& msg);

  bool startResourceHelper();

  void handleAssetDependency(const hkvMsgAssetDependency& msg);
  void handleAssetPropertyHint(const hkvMsgAssetPropertyHint& msg);
  void handleAssetProcessed(const hkvMsgAssetProcessed& msg);

private:
  hkUint32 m_localId;
  hkUint32 m_pipeId;
  Channel* m_localChannel;
  hkvAssetUpdateIPCIOLoop* m_ioLoop;
  HANDLE m_notifyEvent;

  bool m_valid;
  volatile bool m_aborted;

  std::vector<hkStringPtr> m_assetDependencies;
  hkStringPtr m_assetPropertyHint;
  bool m_processingFinished;
  bool m_processingSuccessful;
};


class hkvAssetUpdateBackgroundProcessor : public hkvBackgroundProcessor<hkvAssetUpdateInput, hkvAssetUpdateOutput>
{
  friend class hkvAssetUpdate;

public:
  hkvAssetUpdateBackgroundProcessor(hkUint32 numThreads = 1);
private:
  hkvAssetUpdateBackgroundProcessor(const hkvAssetUpdateBackgroundProcessor&);
  hkvAssetUpdateBackgroundProcessor& operator=(const hkvAssetUpdateBackgroundProcessor&);

public:
  void setAddThumbnailsToRCS(bool addThumbnailsToRCS);
  bool getAddThumbnailsToRCS() const;

protected:
  virtual void onRetryCountExceeded(const hkvAssetUpdateInput& input) HKV_OVERRIDE;

  virtual void beforeStart() HKV_OVERRIDE;
  virtual void onStopping() HKV_OVERRIDE;
  virtual void afterStop() HKV_OVERRIDE;
  virtual void beforeClearInputs() HKV_OVERRIDE;

  virtual hkvBackgroundProcessingResult process(const hkvAssetUpdateInput& input, hkvAssetUpdateOutput& output) HKV_OVERRIDE;

private:
  hkvAssetUpdateIPCInterface* ensureValidIPC();

private:
  hkCriticalSection m_protect;
  hkvAssetUpdateIPCInterface** m_ipcInterfaces;

  bool m_addThumbnailsToRCS;
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
