/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Tools/vSceneViewer/vSceneViewerPCH.h>

#include <Vision/Tools/vSceneViewer/VFileServeDaemon.hpp>

#include <Vision/Tools/vSceneViewer/VNativeMobileDialog.hpp>
#include <Vision/Runtime/Base/System/IO/Stream/VFileServeStreamManager.hpp>

/// \brief Background thread broadcasting packets to announce the presence of this application.
class VFileServeBroadcaster : public VBackgroundThread
{
public:
  VFileServeBroadcaster() : VBackgroundThread(THREADPRIORITY_LOW, "FileServe Broadcaster"), m_bRunning(true)
  {
  }

  virtual ~VFileServeBroadcaster()
  {
    Stop();
  }

  virtual void Run() HKV_OVERRIDE
  {
    VMessage msgTemplate('HERE');

#if defined(_VISION_ANDROID)
    msgTemplate.Write(g_szDeviceModel);
#elif defined(_VISION_IOS)
    msgTemplate.Write(g_szDeviceName);
#elif defined(_VISION_TIZEN)
    Tizen::Base::String key(L"http://tizen.org/system/model_name");
    Tizen::Base::String modelName;
    Tizen::System::SystemInfo::GetValue(key, modelName);

    char szModelName[256];
    V_WCHAR_TO_UTF8(modelName.GetPointer(), szModelName, sizeof(szModelName));
    msgTemplate.Write(szModelName);
#else
#error Undefined platform!
#endif

    msgTemplate.Write(VTargetDeviceName[TARGETDEVICE_THIS]);

    int iBroadcastSocket = VTarget::CreateUDPBroadcastSocket();

    if(!VTarget::IsValidSocket(iBroadcastSocket))
    {
      m_bRunning = false;
    }

    int iFailedSubsequentBroadcasts = 0;
    
    while(m_bRunning)
    {
      {
        VMutexLocker lock(m_mutex);

        // Copy the template
        VMessage message(msgTemplate);

        // Announce if there is already someone serving us (this may change every time)
        message.Write(m_servedFrom.AsChar());

        if(!VTarget::BroadcastMessage(iBroadcastSocket, 4227, &message))
        {
          iFailedSubsequentBroadcasts++;
        }
        else
        {
          iFailedSubsequentBroadcasts = 0;
        }

        if(iFailedSubsequentBroadcasts >= 3)
        {
          break;
        }
      }
      Sleep(1000);
    }

    m_bRunning = false;

    VTarget::CloseSocket(iBroadcastSocket);
    m_stopEvent.Signal();
  }

  void Stop()
  {
    if(m_bRunning)
    {
      m_bRunning = false;
      m_stopEvent.Wait();
    }
  }

  bool IsRunning()
  {
    return m_bRunning;
  }

  void SetServedFrom(const char* pString)
  {
    VMutexLocker lock(m_mutex);
    m_servedFrom = pString;
  }

private:
  volatile bool m_bRunning;
  VMutex m_mutex;
  VString m_servedFrom;
  VEvent m_stopEvent;
};



VFileServeDaemon::VFileServeDaemon() :
  m_initState(FIRST_RUN), m_pDialog(NULL), m_pBroadcaster(NULL),
  m_bHasRemoteOptions(false),
  m_pRemoteConnection(NULL),
  m_iUseCacheButton(-1), m_iUseLastButton(-1), m_iQuitButton(-1)
{
  VTarget::OnConnection += this;
  VTarget::OnDisconnection += this;

#ifdef _VISION_IOS
  char szAlternativeCacheDir[FS_MAX_PATH];
  VPathHelper::CombineDirAndDir(szAlternativeCacheDir, VisSampleApp::GetDocumentsDirectory(), "FileServeCache");
  strcat_s(szAlternativeCacheDir, "/");
  m_szCacheDir = szAlternativeCacheDir;
#elif defined( _VISION_MOBILE )
  m_szCacheDir = VisSampleApp::GetCacheDirectory();
#else
#error "Platform not supported!"
#endif

#if defined( _VISION_IOS )
  m_szSettingsFile = VisSampleApp::GetDocumentsDirectory();
#elif defined( _VISION_ANDROID ) || defined( _VISION_APOLLO )
  m_szSettingsFile = VisSampleApp::GetSDCardDirectory();
#elif defined( _VISION_TIZEN )
  // TODO: sdcard directory not working because of insufficient permission (should use /opt/storage/sdcard/)
  m_szSettingsFile = "/opt/media";
#else
  #error "Platform not supported!"
#endif

  m_szSettingsFile += "/vFileServeHost.txt";
}

VFileServeDaemon::~VFileServeDaemon()
{
  VTarget::OnDisconnection -= this;
  VTarget::OnConnection -= this;
  V_SAFE_DELETE(m_pDialog);
  V_SAFE_DELETE(m_pBroadcaster);
}

void VFileServeDaemon::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
  if(pData->m_pSender == &VTarget::OnConnection)
  {
    VTargetConnectionCallbackItem* pConnectionData = static_cast<VTargetConnectionCallbackItem*>(pData);
    if(pConnectionData->pConnection && pConnectionData->pConnection->IsFor("FSIP"))
    {
      Vision::Error.SystemMessage("Received FSIP connection");
      m_pRemoteConnection = pConnectionData->pConnection;
    }
  }
  else if(pData->m_pSender == &VTarget::OnDisconnection)
  {
    VTargetConnectionCallbackItem* pConnectionData = static_cast<VTargetConnectionCallbackItem*>(pData);
    if(pConnectionData->pConnection == m_pRemoteConnection)
    {
      // Process any remaining messages
      ProcessNetworkMessages();
      m_pRemoteConnection = NULL;
    }
  }
}

void VFileServeDaemon::ProcessNetworkMessages()
{
  if(m_pRemoteConnection)
  {
    while(VMessage* pMsg = m_pRemoteConnection->GetNextASyncReceivedMessage())
    {
      if(pMsg->GetMessageType() == 'COTO' && m_szRemoteHost.IsEmpty())
      {
        char* pAddress = NULL;
        if(pMsg->ReadString(&pAddress))
        {
          Vision::Error.SystemMessage("Received FileServe connection message from %s", pAddress);
          m_szRemoteHost = pAddress;
          m_bHasRemoteOptions = true;
          break;
        }
      }
      delete pMsg;
    }
  }
}

bool VFileServeDaemon::SetupIsFinished()
{
  return m_initState == SETUP_FINISHED;
}

bool VFileServeDaemon::RunSetup()
{
  // Already done
  if(m_initState == SETUP_FINISHED)
  {
    return true;
  }

  // Currently showing failure dialog
  if(m_initState == FAILED)
  {
    // Continue running until the dialog is dismissed
    Sleep(100);
    return m_pDialog->GetState() == IVNativeMobileDialog::WAITING;
  }

  // Setup dialog and background broadcaster
  if(m_initState == FIRST_RUN)
  {
    // Need this for the background connection
    VBaseInit();

    // Try to load the last host from the settings file
    if(IVFileInStream *pStream = VBase_GetFileStreamManager()->Open(m_szSettingsFile))
    {
      char szHostUrl[1024];
      size_t uiLen = pStream->Read(szHostUrl, V_ARRAY_SIZE(szHostUrl)-1);
      pStream->Close();
      pStream = NULL;
      szHostUrl[uiLen] = '\0';
      m_szHostFromSettings = szHostUrl;
    }
    
    m_pDialog = IVNativeMobileDialog::CreateInstance();

    m_pDialog->SetTitle("vFileServe");
    m_pDialog->SetText("Waiting for vFileServe connection...");

    if(VFileHelper::ExistsDir(m_szCacheDir))
    {
      m_iUseCacheButton = m_pDialog->AddButton("Use Cache");
    }
  
    if(!m_szHostFromSettings.IsEmpty())
    {
      VString text;
      text.Format("Use %s", m_szHostFromSettings.AsChar());
      m_iUseLastButton = m_pDialog->AddButton(text.AsChar());
    }

    m_iQuitButton = m_pDialog->AddButton("Quit");

    m_pDialog->Show();  

    m_pBroadcaster = new VFileServeBroadcaster();
    m_pBroadcaster->Start();

    m_initState = SHOWING_DIALOG;

#if defined(_VISION_ANDROID)
    // Allow running the app while the file serve dialog is shown in the foreground
    VVideo::GetVideoConfig()->bRunWhileSleeping = true;
#endif

    return true;
  }

  // E.g. when pressing the back button
  if(m_pDialog->GetState() == IVNativeMobileDialog::CANCELLED)
  {
    V_SAFE_DELETE(m_pDialog);
    return false;
  }

  // E.g. when socket couldn't be created
  if(!m_pBroadcaster->IsRunning())
  { 
      m_pDialog->SetText("Broadcast failed. Check network connection.");
  }

  // When a button was pressed, take according action
  if(m_pDialog->GetState() != IVNativeMobileDialog::WAITING)
  {
    if(m_pDialog->GetState() == m_iUseLastButton)
    {
      m_szRemoteHost = m_szHostFromSettings;
      m_bHasRemoteOptions = true;
    }
    else if(m_pDialog->GetState() == m_iUseCacheButton)
    {
      // Set up fileserve without IP
      m_spFallbackManager = Vision::File.GetManager();
      m_bHasRemoteOptions = true;
    }
    else if(m_pDialog->GetState() == m_iQuitButton)
    {
      return false;
    }
  }
  else
  {
    // Process network messages
    ProcessNetworkMessages();
  }

  // Connect to host
  if(m_bHasRemoteOptions)
  {
    V_SAFE_DELETE(m_pDialog);

    Vision::Error.SystemMessage("Connecting to %s now", m_szRemoteHost.AsChar());

    if(!VFileHelper::ExistsDir(m_szCacheDir.AsChar()))
    {
      VFileHelper::MkDirRecursive(m_szCacheDir.AsChar());
    }

    VFileServeStreamManager* pFSSM = new VFileServeStreamManager(m_szRemoteHost, m_szCacheDir, m_spFallbackManager);
    if(!m_szRemoteHost.IsEmpty())
    {
      if(!pFSSM->IsConnected())
      {
        V_SAFE_DELETE(m_pDialog);

        // Show a new dialog with an error message
        m_pDialog = IVNativeMobileDialog::CreateInstance();
        m_pDialog->SetTitle("Connection failed");

        VString errorMsg;
        errorMsg.Format("Failed to connect to remote host '%s'", m_szRemoteHost.AsChar());
        m_pDialog->SetText(errorMsg.AsChar());
        m_pDialog->AddButton("Quit");

        m_pDialog->Show();

        m_initState = FAILED;
        return true;
      }

      // Write new IP to settings file
      if(IVFileOutStream* pSettings = VBase_GetFileStreamManager()->Create(m_szSettingsFile))
      {
        pSettings->WriteText(m_szRemoteHost);
        pSettings->Close();
      }
      else
      {
        // Our file stream doesn't provide error information, but since it's a disk file stream at this point errno will contain the correct information.
        Vision::Error.Warning("Could not open '%s' for writing: %s. Unable to save the last used IP.", m_szSettingsFile.AsChar(), strerror(errno));
      }
    }
    
    VisSampleApp::s_bIsFileserverInstalled = true;

    pFSSM->SetDataRoot(VISION_ROOT_DATA);
    Vision::File.SetManager(pFSSM);
    Vision::File.SetUseAbsolutePaths(false);

    m_initState = SETUP_FINISHED;

#if defined(_VISION_ANDROID)
    VVideo::GetVideoConfig()->bRunWhileSleeping = false;
#endif

    if(m_szRemoteHost.IsEmpty())
    {
      m_pBroadcaster->SetServedFrom("Cache");
    }
    else
    {
      m_pBroadcaster->SetServedFrom(m_szRemoteHost);
    }

    return true;
  }

  // Just sleep a while until something happens
  Sleep(100);
  return true;
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
