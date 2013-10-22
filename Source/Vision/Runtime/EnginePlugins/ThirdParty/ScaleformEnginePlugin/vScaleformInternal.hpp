/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef __VSCALEFORM_INTERNAL_HPP
#define __VSCALEFORM_INTERNAL_HPP

#include "GFx.h"

// GFx includes
#include "GFx/GFx_Player.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Log.h"
#include "Render/Renderer2D.h"

#ifdef USE_SF_IME
#include "GFxIME/GFx_IMEManagerWin32.h"
#endif

#ifdef SF_AMP_SERVER
#include "GFx_AMP.h"
#endif

class VScaleformMovieInstance;

/// \brief  
///   The advance task class allows multithreaded Scaleform movie execution. Used internally only.
class VScaleformAdvanceTask : public VThreadedTask
{
public:
  friend class VScaleformMovieInstance;

  VScaleformAdvanceTask(VScaleformMovieInstance* pMovieInst);

  /// \brief
  ///   Initializes the advance task.
  void Init();

  /// \brief
  ///   Adds the task to the thread manager.
  ///
  /// Always use this function instead of manually scheduling the task via
  /// the thread manager. This ensures that the movie instance access rights
  /// are set properly.
  ///
  /// \param fTimeDelta
  ///   The time delta used for the next advance step.
  void Schedule(float fTimeDelta);

  /// \brief
  ///   Wait until the advance task has finished.
  ///
  /// Call this function to gain access rights from the current thread to
  /// the movie instance.
  void WaitUntilFinished();

  virtual void Run(VManagedThread *pThread) HKV_OVERRIDE;

private:
  bool m_bInitInProgress;
  float m_fTimeDelta;
  VScaleformMovieInstance* m_pMovieInst;
};

/// \class  VScaleformCommandQueue
/// \brief  Internal command queue executor required for multithreaded Scaleform execution.
class VScaleformCommandQueue : public Scaleform::Render::ThreadCommandQueue
{
public:

  /// \brief  Default constructor.
  VScaleformCommandQueue();

  /// \brief  Destructor.
  ~VScaleformCommandQueue();

  void Start();

  /// @name Scaleform::Render::ThreadCommandQueue related methods
  /// @{

  /// \brief Add a command to the renderer queue. Implementation of Scaleform::Render::ThreadCommandQueue.
  void PushThreadCommand(Scaleform::Render::ThreadCommand* pCommand);

  void Run();

  /// @}

private:
  VThread* m_pThread;

  volatile bool m_bCancel;  
  VMutex m_commandMutex;    
  int m_iQueueLength;
  DynObjArray_cl<Scaleform::Render::ThreadCommand *> m_commandQueue;  
};

/// \class  VScaleformExternalInterfaceHandler
/// \brief  External interface handler to notify on script callbacks (successor interface to the FS Command).
class VScaleformExternalInterfaceHandler : public Scaleform::GFx::ExternalInterface
{
public:
  VOVERRIDE void Callback(Scaleform::GFx::Movie* pMovie, const char* pszMethodName,
    const Scaleform::GFx::Value* pArgs, unsigned int uiArgCount);
};


/// \class  VScaleformFSCommandHandler
/// \brief  FSCommand handler to notify on script callbacks (already deprecated in the Scaleform API).
class VScaleformFSCommandHandler : public Scaleform::GFx::FSCommandHandler
{
public:
    VOVERRIDE void Callback(Scaleform::GFx::Movie* pMovie, const char* szCommand, const char* szArgs);
};

/// \brief  Internal wrapper class which allows file opening in Scaleform via Vision file manager.
class VGFxFileOpener : public Scaleform::GFx::FileOpener
{
public: 

  // Override to opens a file using user-defined function and/or GFile class.
  // The default implementation uses buffer-wrapped GSysFile, but only
  // if GFC_USE_SYSFILE is defined.
  VOVERRIDE Scaleform::File* OpenFile(const char* purl, 
    int flags = Scaleform::FileConstants::Open_Read|Scaleform::FileConstants::Open_Buffered, 
    int mode = Scaleform::FileConstants::Mode_ReadWrite);

  // Returns last modified date/time required for file change detection.
  // Can be implemented to return 0 if no change detection is desired.
  // Default implementation checks file time if GFC_USE_SYSFILE is defined.
  VOVERRIDE Scaleform::SInt64 GetFileModifyTime(const char *purl) {return 0;}
};


/// \brief  Internal wrapper class for the Scaleform log.
class VScaleformLog : public Scaleform::GFx::Log
{
public: 
  // We override this function in order to do custom logging.
  VOVERRIDE void LogMessageVarg(Scaleform::LogMessageType messageType, const char* pfmt, va_list argList);
};


#ifdef SF_AMP_SERVER

  /// \brief Amp application controller implementation (to support callbacks from the Amp client).
  class VScaleformAmpAppController : public Scaleform::GFx::AMP::AppControlInterface
  {
  public:
    VScaleformAmpAppController(VScaleformManager *pManager);
    virtual ~VScaleformAmpAppController();
    virtual bool HandleAmpRequest(const Scaleform::GFx::AMP::MessageAppControl* pMessage);
    Scaleform::GFx::AMP::MessageAppControl * GetCaps();

  private:
    VScaleformManager *m_pManager;
    Scaleform::GFx::AMP::MessageAppControl m_caps;
  };

#endif // SF_AMP_SERVER

#endif // __VSCALEFORM_INTERNAL_HPP

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
