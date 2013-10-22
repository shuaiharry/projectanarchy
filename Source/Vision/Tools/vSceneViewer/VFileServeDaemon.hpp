/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

class IVNativeMobileDialog;
class VFileServeBroadcaster;

/// \brief A helper that runs in the background and takes care of setting up a FileServe connection.
class VFileServeDaemon : private IVisCallbackHandler_cl
{
public:
  /// \brief Constructor.
  VFileServeDaemon();

  /// \brief Destructor.
  virtual ~VFileServeDaemon();

  /// \brief Returns true if the FileServe connection was established by an external connection or user choice.
  bool SetupIsFinished();

  /// \brief Runs the setup, and returns false in case of failure or cancelling.
  ///
  /// Until the setup is finished, call this method in your main application loop, e.g.:
  /// \code
  ///   VISION_RUN
  ///   {
  ///     if(!g_pFileServeDaemon->SetupIsFinished())
  ///     {
  ///       return g_pFileServeDaemon->RunSetup();
  ///     }
  ///     
  ///     // Standard application code here...
  ///   }
  ///  \endcode
  bool RunSetup();

private:
  // Disable copying
  VFileServeDaemon(const VFileServeDaemon&);
  VFileServeDaemon& operator=(const VFileServeDaemon&);

  virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

  void ProcessNetworkMessages();

  enum InitState
  {
    FIRST_RUN,
    SHOWING_DIALOG,
    SETUP_FINISHED,
    FAILED,
  };

  InitState m_initState;
  IVNativeMobileDialog* m_pDialog;
  VFileServeBroadcaster* m_pBroadcaster;
  VConnection* m_pRemoteConnection;

  int m_iUseCacheButton;
  int m_iUseLastButton;
  int m_iQuitButton;

  VString m_szCacheDir;
  VString m_szHostFromSettings;
  VString m_szSettingsFile;

  VString m_szRemoteHost;
  IVFileStreamManagerPtr m_spFallbackManager;
  bool m_bHasRemoteOptions;
};

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
