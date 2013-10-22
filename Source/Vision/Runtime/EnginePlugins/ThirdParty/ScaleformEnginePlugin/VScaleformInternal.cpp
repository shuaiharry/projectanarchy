/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/ScaleformEnginePlugin.hpp>

// GFx includes
#include "GFx/GFx_Player.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Log.h"
#include "Render/Renderer2D.h"
#include "Kernel/SF_Threads.h"

#if defined (WIN32) && defined(_VR_DX9)
#include "GFx_Renderer_D3D9.h"
#elif defined(WIN32) && defined(_VR_DX11)
#include "GFx_Renderer_D3D1x.h"
#elif defined (_VISION_XENON)
#include "Render/X360/X360_HAL.h"
#elif defined (_VISION_PS3)
#include "GFx_Renderer_PS3.h"
#elif defined (_VISION_MOBILE)
#include "GFx_Renderer_GL.h"
#endif

#ifdef SF_AMP_SERVER
#include "GFx_AMP.h"
#endif

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scripting/VScriptIncludes.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformUtil.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformMovie.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformInternal.hpp>

VisCallback_cl VOnExternalInterfaceCall::OnExternalInterfaceCallback;
VisCallback_cl VOnFSCommand::OnFSCallback;

IVFileStreamManager *g_pFileManager;

extern int PROFILING_FSCOMMAND;
extern int PROFILING_EXT_INTERFACE;

//command queue related methods
//=============================


#if defined(WIN32)  || defined(_VISION_XENON) 
DWORD __stdcall CommandQueueThreadFunc(LPVOID arg)
#define RETURN_VAL return 0;
#elif defined(_VISION_PS3)
void CommandQueueThreadFunc(uint64_t arg)
#define RETURN_VAL 
#elif defined(_VISION_POSIX)
void* CommandQueueThreadFunc(void* arg)
#define RETURN_VAL return 0;
#elif defined(_VISION_PSP2)
SceInt32 CommandQueueThreadFunc(SceSize, void* arg);
#define RETURN_VAL return 0;
#else
#error "Missing Platform!"
#endif
{
#if defined _VISION_PS3
  VScaleformCommandQueue *pQueue = (VScaleformCommandQueue *)((int)arg);
#else
  VScaleformCommandQueue *pQueue = (VScaleformCommandQueue *)arg;
#endif

  pQueue->Run();
  RETURN_VAL
}


VScaleformCommandQueue::VScaleformCommandQueue() :
  m_bCancel(false),
  m_iQueueLength(0),
  m_commandMutex(),
  m_commandQueue(8, NULL)
{
  m_pThread = new VThread(CommandQueueThreadFunc, this, -1, THREADPRIORITY_LOW);
}

VScaleformCommandQueue::~VScaleformCommandQueue()
{
  m_bCancel = true;
  m_pThread->Join();
  delete m_pThread;
}

void VScaleformCommandQueue::Start()
{
  m_pThread->Start();
}


void VScaleformCommandQueue::PushThreadCommand(Scaleform::Render::ThreadCommand* command)
{
  m_commandMutex.Lock();
  m_commandQueue[m_iQueueLength++] = command;
  m_commandMutex.Unlock();
}


void VScaleformCommandQueue::Run()
{
  do 
  {
    m_commandMutex.Lock();
    for(int i=0;i<m_iQueueLength;i++)
    {
      Scaleform::Render::ThreadCommand *pCommand = m_commandQueue.GetDataPtr()[i];
      if(pCommand!=NULL)
      {
        pCommand->Execute();
      }
    }
    m_iQueueLength = 0;
    m_commandMutex.Unlock();
    Sleep(1);
  } while (!m_bCancel);
}

// FSCommand callback. Used to handle video mode change requests from the SWF file.
void VScaleformFSCommandHandler::Callback(Scaleform::GFx::Movie* pMovie, const char* szCommand, const char* szArgString)
{
  VISION_PROFILE_FUNCTION(PROFILING_FSCOMMAND);

  VScaleformMovieInstance *pInst = static_cast<VScaleformMovieInstance*>(pMovie->GetUserData());
  VASSERT(pInst && "The user data is reserved for storing the pointer to the Vision object");

  pInst->m_queuedFSCommands.SetAtGrow(pInst->m_queuedFSCommands.GetSize(), 
    new VOnFSCommand(pInst, szCommand, szArgString));
}

// External interface callback. Used to handle video mode change requests from the SWF file.
void VScaleformExternalInterfaceHandler::Callback(
  Scaleform::GFx::Movie* pMovie, const char* szMethodName, const Scaleform::GFx::Value* pArgs, unsigned int uiArgCount)
{
  VISION_PROFILE_FUNCTION(PROFILING_EXT_INTERFACE);
  VScaleformMovieInstance *pInst = (VScaleformMovieInstance *)pMovie->GetUserData();
  VASSERT(pInst && "The user data is reserved for storing the pointer to the Vision object");

  pInst->m_queuedExternalCalls.SetAtGrow(pInst->m_queuedExternalCalls.GetSize(), 
    new VOnExternalInterfaceCall(pInst, szMethodName, pArgs, uiArgCount));
}


///////////////////////////////////
//          VisionGFile          //
///////////////////////////////////

// Helper class to both implement the GFile interface and providing file access through IVFileInStream
class VisionGFile : public Scaleform::File
{
public:
  VisionGFile(IVFileStreamManager *pManager, const char *szFilename)
  {
    VASSERT(pManager);
    // Open file for reading
    m_pStream = pManager->Open(szFilename);
  }

  virtual ~VisionGFile() {Close();}

  IVFileInStream *m_pStream;

  // GFile implementations
  virtual const char* GetFilePath()                                       {return m_pStream->GetFileName();}                                  
  virtual bool        IsValid()                                           {return m_pStream!=NULL;}
  virtual bool        IsWritable()                                        {return false;}
  virtual int         Tell ()                                             {return (int)LTell();}
  virtual int64       LTell()                                             {return m_pStream->GetPos();}
  virtual int         GetLength ()                                        {return (int)LGetLength();}
  virtual int64       LGetLength ()                                       {return m_pStream->GetSize();}        
  virtual int         GetErrorCode()                                      {if (!m_pStream) return ENOENT;return 0;}        
  virtual int         Write(const Scaleform::UByte *pbufer, int numBytes) {VASSERT(!"Unsupported file operation");return 0;}
  virtual int         Read(Scaleform::UByte *pbufer, int numBytes)        {return (int)m_pStream->Read(pbufer,numBytes);}
  virtual int         SkipBytes(int numBytes)                             {Scaleform::UByte dummy;for (int i=0;i<numBytes;i++) Read(&dummy,1);return numBytes;}
  virtual int         BytesAvailable()                                    {return GetLength()-Tell();}                               
  virtual bool        Flush()                                             {return true;}
  virtual int         Seek(int offset, int origin=Seek_Set)               {return (int)LSeek(offset, origin);}
  virtual bool        ChangeSize(int newSize)                             {VASSERT(!"Unsupported file operation");return false;}
  virtual int         CopyFromStream(File *pstream, int byteSize)         {VASSERT(!"Unsupported file operation");return 0;}
  virtual bool        Close()                                             {if (!m_pStream) return false; m_pStream->Close();m_pStream=NULL;return true;}
  virtual int64       LSeek(int64 offset, int origin=Seek_Set)            {if (!m_pStream->SetPos((LONG)offset,origin)) return -1;return LTell();}
};


// File loader callback.
Scaleform::File* VGFxFileOpener::OpenFile(const char* purl, int flags, int mode)
{
  if (purl == NULL || strlen(purl) == 0)
    return NULL;

  VASSERT(g_pFileManager);
  return new VisionGFile(g_pFileManager, purl);
}


////////////////////////////////////////
//           VScaleformLog            //
////////////////////////////////////////

void VScaleformLog::LogMessageVarg(Scaleform::LogMessageType messageType, const char* pfmt, va_list argList )
{
  const int MSG_MAX = FS_MAX_PATH;
  char comp[MSG_MAX];
  vsprintf(comp, pfmt, argList);

#ifdef WIN32 
  VLocale::UTF8ToMultiByte(comp,-1,comp,MSG_MAX);
#endif

  // Output log to console
  Vision::Error.SystemMessage(comp);
}

#ifdef SF_AMP_SERVER

VScaleformAmpAppController::VScaleformAmpAppController(VScaleformManager *pManager) :
  m_pManager(pManager),
  m_caps()
{
  VASSERT_MSG(pManager!=NULL, "You have to supply a valid VScaleformManager");
  m_caps.SetTogglePause(true);
  m_caps.SetToggleWireframe(true);
}

VScaleformAmpAppController::~VScaleformAmpAppController()
{
}

Scaleform::GFx::AMP::MessageAppControl * VScaleformAmpAppController::GetCaps()
{
  return &m_caps;
}

bool VScaleformAmpAppController::HandleAmpRequest(const Scaleform::GFx::AMP::MessageAppControl* pMessage)
{
  if(pMessage==NULL)
    return false;

  //no action if there is no loaded movie...
  int iCount = m_pManager->m_Instances.Count();
  if(iCount<1)
    return false;

  if(pMessage->IsTogglePause())
  {
    //toggle pause for all movies 
    for(int i=0;i<iCount;i++)
    {
      VScaleformMovieInstance *pMovie = m_pManager->m_Instances.GetAt(i);

      pMovie->m_pAdvanceTask->WaitUntilFinished();

      pMovie->SetPaused(!pMovie->IsPaused());
    }

    return true;
  }
  else if(pMessage->IsToggleWireframe())
  {
    //toggle wireframe mode
    Vision::Renderer.SetWireframeMode(!Vision::Renderer.GetWireframeMode());
    return true;
  }

  return false;
}

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
