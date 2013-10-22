/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/ScaleformEnginePlugin.hpp>

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scripting/VScriptIncludes.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformManager.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformMovie.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformUtil.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformInternal.hpp>

//we do not use namespace Scaleform since it contains ambiguous symbols like Ptr, Value, etc...
using namespace Scaleform::Render;
using namespace Scaleform::GFx;

//global switches
static const bool s_bAntiAlias = true;

#ifdef _VISION_MOBILE

class VScaleformMultitouch : public Scaleform::GFx::MultitouchInterface
{
protected:
  int m_iMaxTouchPoints;
public:
  VScaleformMultitouch(int iMaxTouchPoints) : m_iMaxTouchPoints(iMaxTouchPoints) {}

  virtual unsigned GetMaxTouchPoints() const HKV_OVERRIDE { return m_iMaxTouchPoints; }
  
  virtual Scaleform::UInt32   GetSupportedGesturesMask() const HKV_OVERRIDE { return 0; }
  
  virtual bool     SetMultitouchInputMode(Scaleform::GFx::MultitouchInterface::MultitouchInputMode eMode) HKV_OVERRIDE { return true; }
};

/*
 class VScaleformVirtualKeyboard : public Scaleform::GFx::VirtualKeyboardInterface
 {
 public:
 // Invoked when an input textfield recieves focus
 virtual void    OnInputTextfieldFocusIn(bool multiline, const Scaleform::GFx::Render::RectF& textBox)
 {
 //open virtual KB
 }
 
 // Invoked when an input textfield loses focus
 virtual void    OnInputTextfieldFocusOut()
 {
 //close virtual KB
 }
 };
 */

#endif

////////////////////////////////////////////
//          VScaleformAdvanceTask         //
//////////////////////////////////////////// 

VScaleformAdvanceTask::VScaleformAdvanceTask(VScaleformMovieInstance* pMovieInst)
  : VThreadedTask()
  , m_bInitInProgress(false)
  , m_fTimeDelta(0.0f)
  , m_pMovieInst(pMovieInst)
{
}

void VScaleformAdvanceTask::Init()
{
  // Set init to true in order to perform initialization during Run().
  m_bInitInProgress = true;
  Schedule(0.0f);
  WaitUntilFinished();
  m_bInitInProgress = false;
}

void VScaleformAdvanceTask::Schedule(float fTimeDelta)
{
  m_fTimeDelta = fTimeDelta;

  if (VScaleformManager::GlobalManager().IsMultithreadedAdvanceEnabled())
  {
    Vision::GetThreadManager()->ScheduleTask(this);
  }
  else
  {
    Run(NULL);
  }
}

void VScaleformAdvanceTask::WaitUntilFinished()
{
  if (!VScaleformManager::GlobalManager().IsMultithreadedAdvanceEnabled())
    return;

  Vision::GetThreadManager()->WaitForTask(this, true);

  // Movie instance can now be accessed from the current thread.
  m_pMovieInst->GetGFxMovieInstance()->SetCaptureThread(Scaleform::GetCurrentThreadId());
}

void VScaleformAdvanceTask::Run(VManagedThread *pThread)
{
  VASSERT_MSG(m_pMovieInst != NULL, "No scaleform movie instance present!");
  m_pMovieInst->GetGFxMovieInstance()->SetCaptureThread(Scaleform::GetCurrentThreadId());

  // Initialization
  if (m_bInitInProgress)
  {
    m_pMovieInst->GetGFxMovieInstance()->HandleEvent(Event::SetFocus);
    m_pMovieInst->GetGFxMovieInstance()->Advance(0.0f, 0); // load first frame
  }
  // Advance Movie
  else
  {
    VASSERT(m_pMovieInst != NULL && m_pMovieInst->GetGFxMovieInstance() != NULL);
    m_pMovieInst->GetGFxMovieInstance()->Advance(m_fTimeDelta);
  }
}

//////////////////////////////////////////////
//          VScaleformMovieInstance         //
//////////////////////////////////////////////

VScaleformMovieInstance::VScaleformMovieInstance(const char *szFilename, Scaleform::GFx::Loader* pLoader /*= NULL */,
                                                 const char *szCandidateMovie /*= NULL */, const char *szImeXml /*= NULL */,
                                                 int iPosX /*= 0*/, int iPosY /*= 0*/, int iWidth /*= -1*/, int iHeight /*= -1*/)
  : m_pMovieInst(NULL)
  , m_phMovieDisplay(NULL)
  , m_referencedScaleformValues()
#ifdef USE_SF_IME
  , m_sCandidateMovie(szCandidateMovie)
  , m_sImeXml(szImeXml)
#endif
  , m_bInit(false)
  , m_bHandleInput(true)
  , m_bIsTexture(false)
  , m_iVisibleMask(VIS_ENTITY_VISIBLE_IN_WORLD)
  , m_pLastRenderContext(NULL)
  , m_iPosX(iPosX)
  , m_iPosY(iPosY)
  , m_iWidth(iWidth)
  , m_iHeight(iHeight)
  , m_iMovieAuthoredWidth(SF_MOVIE_SIZE_AUTHORED)
  , m_iMovieAuthoredHeight(SF_MOVIE_SIZE_AUTHORED)
  , m_sFileName(szFilename)
#if defined(WIN32)
  , m_pKeyModifiers(NULL)
#endif
  , m_pAdvanceTask(NULL)
{
  m_pAdvanceTask = new VScaleformAdvanceTask(this);

#ifdef WIN32
  m_pKeyModifiers = new Scaleform::KeyModifiers();
#endif
  Scaleform::Ptr<MovieDef> pMovieDef = *pLoader->CreateMovie(szFilename, Loader::LoadAll|Loader::LoadWaitCompletion);
  VASSERT_MSG(pMovieDef!=NULL, "Could not create Scaleform Movie Def!");

  //Scaleform::StringBuffer buffer;
  //Scaleform::Memory::pGlobalHeap->MemReport(buffer, Scaleform::MemoryHeap::MemReportFull);
  //Vision::Error.Warning("\n-----------------\nCreateMovie report:\n\n%s\n-----------------\n", buffer.ToCStr());

  // get the movie size which was used when authoring the movie
  m_iMovieAuthoredWidth = static_cast<int>(pMovieDef->GetWidth());
  m_iMovieAuthoredHeight = static_cast<int>(pMovieDef->GetHeight());

  m_pMovieInst = pMovieDef->CreateInstance(false /*initFirstFrame*/);
  VASSERT_MSG(m_pMovieInst!=NULL, "Could not create Scaleform Movie Instance!");

  //Scaleform::StringBuffer buffer2;
  //Scaleform::Memory::pGlobalHeap->MemReport(buffer2, Scaleform::MemoryHeap::MemReportFull);
  //Vision::Error.Warning("\n-----------------\nCreateInstance report:\n\n%s\n-----------------\n", buffer2.ToCStr());

  m_phMovieDisplay = new MovieDisplayHandle(m_pMovieInst->GetDisplayHandle());

  m_pMovieInst->SetEdgeAAMode(s_bAntiAlias ? EdgeAA_On : EdgeAA_Disable);

  m_pMovieInst->SetUserData(this);

  // enable one mouse
  m_pMovieInst->SetMouseCursorCount(1);

  // Background Alpha = 0 for transparent background
  m_pMovieInst->SetBackgroundAlpha(0.0f);

  // Update viewport:
  // The viewport of the Scaleform movie has to be set prior to the first advance since otherwise the movie will be not displayed 
  // correctly in the first rendered frame. Since at this point the actually used render-context that will be used to display the 
  // movie can not be always obtained, the current render-context will be used to update the viewport. Though in most cases this 
  // will work correctly, for cases in which the resolution of the current and actually used render-context differ, the first frame
  // will be displayed with the wrong viewport. Please note that m_pLastRenderContext will then be reset within the first invocation 
  // of VScaleformManager::RenderMovies().
  m_pLastRenderContext = Vision::Contexts.GetCurrentContext();
  UpdateViewport();

#ifdef _VISION_MOBILE
  //setup touch interface
  m_pMovieInst->SetMultitouchInterface(Scaleform::Ptr<MultitouchInterface>(*new VScaleformMultitouch(VScaleformManager::GlobalManager().GetMaxTouchPoints())));
  //setup virtual keyboard
  //m_pMovieInst->SetVirtualKeyboardInterface(Scaleform::Ptr<VirtualKeyboardInterface>(*new VScaleformVirtualKeyboard()));

#endif

#if defined(USE_SF_IME)
  
  InitImeManager();

  // Activate IME Manager if present.
  if (pLoader->GetIMEManager() != NULL)
  {
    pLoader->GetIMEManager()->SetActiveMovie(m_pMovieInst);
  }

#endif

  // Let the advance task do the initialization.
  m_pAdvanceTask->Init();
}


VScaleformMovieInstance::~VScaleformMovieInstance()
{
  Invalidate();
}

#ifdef USE_SF_IME

void VScaleformMovieInstance::InitImeManager()
{
  Loader* pLoader = VScaleformManager::GlobalManager().GetLoader();
  if (pLoader->GetIMEManager() != NULL)
    return;

  if (m_sCandidateMovie.IsEmpty() || m_sImeXml.IsEmpty())
    return;

  // Check if window is confgured properly.
  if (Vision::GetApplication()->m_appConfig.m_windowConfig.CheckWindowSettingForIME() == false)
  {
    Vision::Error.Warning("vScaleformPlugin: You are about to initialize the IME Manager with incorrect WindowConfig!");
  }

  Scaleform::Ptr<Scaleform::GFx::IMEManagerBase> pImeManager = *new Scaleform::GFx::IME::GFxIMEManagerWin32(Vision::Video.GetCurrentConfig()->m_hWnd);
  VASSERT_MSG(pImeManager, "IME Manager not initialized")

	/*  Parameters:
	1- Logger used to log IME messages. If not logging is desired, pass NULL
	2- Fileopener necessary to open the ime.xml file that contains the list of supported IME's. If NULL, a default
		list will be used instead
	3- Path for the ime.xml file that contains a list of supported IMEs - creaate an absolute path, otherwise
     the method will look in the scaleform SDK path for the file!
	4- Used to set if while loading the candidate list swf, IMEManager should check if the file exists in 
	   current directory (where the parent swf is located) or not. If false, IMEManager will not check
	   if the file exists, and if the file is eventually not found, an error message will be printed.
	   This is useful if you have you own file management system and you want FileOpener::OpenFile to be
	   called regardless. 
	*/
  pImeManager->Init(pLoader->GetLog(), (FileOpener *) pLoader->GetFileOpener().GetPtr(), m_sImeXml, true);

  pImeManager->SetIMEMoviePath(m_sCandidateMovie); //relative to your root movie (IME/IMESample.swf - IME.swf is in the same directory)
  
  pLoader->SetIMEManager(pImeManager);  // pLoader keeps the object from this point
}

#endif

void VScaleformMovieInstance::Invalidate()
{
  if (!IsValid())
    return;

  m_pAdvanceTask->WaitUntilFinished();
  V_SAFE_DELETE(m_pAdvanceTask);

  #ifdef WIN32
    V_SAFE_DELETE(m_pKeyModifiers);
  #endif

  for (int i = 0; i < m_referencedScaleformValues.GetSize(); i++)
  {
    delete m_referencedScaleformValues[i];
  }
  m_referencedScaleformValues.SetSize(0);

  V_SAFE_RELEASE(m_pMovieInst);

  V_SAFE_DELETE(m_phMovieDisplay); //no ref count for movie display handle
}

VScaleformValue* VScaleformMovieInstance::GetVariable(const char * szVarName)
{
  VASSERT_MSG(szVarName != NULL, "Specify a valid var!");
  VASSERT_MSG(m_pMovieInst != NULL, "Movie not loaded");

  // Check if variable object has already been created.
  for (int i = 0; i < m_referencedScaleformValues.GetSize(); i++)
  {
    VScaleformValue* pValue = m_referencedScaleformValues[i];
    if (pValue->m_sVarName == szVarName)
      return pValue;
  }

  // Wait until the advance task has finished before retrieving the variable.
  m_pAdvanceTask->WaitUntilFinished();

  // Create new Scaleform variable.
  Scaleform::GFx::Value* pValue = new Scaleform::GFx::Value();
  if (!m_pMovieInst->GetVariable(pValue, szVarName))
  {
    delete pValue;
    return NULL;
  }

  // Wrap SF object.
  VScaleformValue *pWrappedValue = new VScaleformValue(pValue, m_pMovieInst, szVarName);

  // Get current scaleform value from the movie.
  pWrappedValue->SyncValueWithScaleform();

  // Add to internal list in order to update scaleform values every frame.
  m_referencedScaleformValues.Add(pWrappedValue);

  return pWrappedValue;
}


void VScaleformMovieInstance::SetOpacity(float fAlpha)
{
  VASSERT_MSG(m_pMovieInst!=NULL, "Movie not loaded");
  m_pAdvanceTask->WaitUntilFinished();

  m_pMovieInst->SetBackgroundAlpha(fAlpha);
}

#ifdef WIN32

void VScaleformMovieInstance::HandlePreTranslatedIMEInputMessage(VWindowsMessageDataObject* pObj)
{
  if ((pObj->m_Msg.message == WM_KEYDOWN) || (pObj->m_Msg.message == WM_KEYUP) ||
    ImmIsUIMessage(NULL, pObj->m_Msg.message, pObj->m_Msg.wParam, pObj->m_Msg.lParam))
  {
    m_pAdvanceTask->WaitUntilFinished();

    IMEWin32Event ev(IMEEvent::IME_PreProcessKeyboard,
      (Scaleform::UPInt)pObj->m_Hwnd, pObj->m_Msg.message, pObj->m_Msg.wParam, pObj->m_Msg.lParam);
	  
    m_pMovieInst->HandleEvent(ev);
  }
}

static  struct
{
  WPARAM  winKey;
  Scaleform::KeyCode appKey;
} 
keyCodeMap [] =
{
  {VK_BACK,    Scaleform::Key::Backspace},
  {VK_TAB,     Scaleform::Key::Tab},
  {VK_CLEAR,   Scaleform::Key::Clear},
  {VK_RETURN,  Scaleform::Key::Return},
  {VK_SHIFT,   Scaleform::Key::Shift},
  {VK_CONTROL, Scaleform::Key::Control},
  {VK_MENU,    Scaleform::Key::Alt},
  {VK_PAUSE,   Scaleform::Key::Pause},
  {VK_CAPITAL, Scaleform::Key::CapsLock},
  {VK_ESCAPE,  Scaleform::Key::Escape},
  {VK_SPACE,   Scaleform::Key::Space},
  {VK_PRIOR,   Scaleform::Key::PageUp},
  {VK_NEXT,    Scaleform::Key::PageDown},
  {VK_END,     Scaleform::Key::End},
  {VK_HOME,    Scaleform::Key::Home},
  {VK_LEFT,    Scaleform::Key::Left},
  {VK_UP,      Scaleform::Key::Up},
  {VK_RIGHT,   Scaleform::Key::Right},
  {VK_DOWN,    Scaleform::Key::Down},
  {VK_INSERT,  Scaleform::Key::Insert},
  {VK_DELETE,  Scaleform::Key::Delete},
  {VK_HELP,    Scaleform::Key::Help},
  {VK_NUMLOCK, Scaleform::Key::NumLock},
  {VK_SCROLL,  Scaleform::Key::ScrollLock},

  {VK_OEM_1,     Scaleform::Key::Semicolon},
  {VK_OEM_PLUS,  Scaleform::Key::Equal},
  {VK_OEM_COMMA, Scaleform::Key::Comma},
  {VK_OEM_MINUS, Scaleform::Key::Minus},
  {VK_OEM_PERIOD,Scaleform::Key::Period},
  {VK_OEM_2,     Scaleform::Key::Slash},
  {VK_OEM_3,     Scaleform::Key::Bar},
  {VK_OEM_4,     Scaleform::Key::BracketLeft},
  {VK_OEM_5,     Scaleform::Key::Backslash},
  {VK_OEM_6,     Scaleform::Key::BracketRight},
  {VK_OEM_7,     Scaleform::Key::Quote},
  {VK_OEM_AX,	   Scaleform::Key::OEM_AX},   //  'AX' key on Japanese AX kbd
  {VK_OEM_102,   Scaleform::Key::OEM_102},  //  "<>" or "\|" on RT 102-key kbd.
  {VK_ICO_HELP,  Scaleform::Key::ICO_HELP}, //  Help key on ICO
  {VK_ICO_00,	   Scaleform::Key::ICO_00},	 //  00 key on ICO

  {0,            Scaleform::Key::None }     // Terminator
};

void VScaleformMovieInstance::HandleWindowsInputMessage(VWindowsMessageDataObject* pObj)
{
  m_pAdvanceTask->WaitUntilFinished();

  // mouse buttons are not handled here because of the non uniform input handling across all platforms
  switch(pObj->m_Msg.message)
  {

#ifdef USE_SF_IME
    case WM_IME_STARTCOMPOSITION:
    case WM_IME_KEYDOWN:
    case WM_IME_COMPOSITION:   
    case WM_IME_ENDCOMPOSITION:
    case WM_IME_NOTIFY:
      //case WM_INPUTLANGCHANGE:
    case WM_IME_CHAR:
      {
        //Vision::Error.Warning("WM_IME_CHAR (%u) W: 0x%x L: 0x%x", pObj->m_Msg.message, pObj->m_Msg.wParam, pObj->m_Msg.lParam);
        IMEWin32Event ev(IMEWin32Event::IME_Default,
          (Scaleform::UPInt)pObj->m_Msg.hwnd, pObj->m_Msg.message, pObj->m_Msg.wParam, pObj->m_Msg.lParam);

        Scaleform::UInt32 handleEvtRetVal = m_pMovieInst->HandleEvent(ev);
        pObj->m_bProcessed = (handleEvtRetVal & Movie::HE_NoDefaultAction) != 0;
      }
      break;
#endif

    case WM_KILLFOCUS:
      m_pMovieInst->HandleEvent(Event::KillFocus);
      break;

    case WM_KEYDOWN: // fall through
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
      {
        //helpers
        unsigned int uiMessage = pObj->m_Msg.message;
        WPARAM wParam = pObj->m_Msg.wParam;

        //we have to convert the char to SF KeyCode enum
        Event::EventType eventType = (uiMessage == WM_KEYDOWN || uiMessage == WM_SYSKEYDOWN) ? Event::KeyDown : Event::KeyUp;

        Scaleform::KeyCode keyCode = Scaleform::Key::None;

        if (wParam >= 'A' && wParam <= 'Z')                   keyCode = (Scaleform::KeyCode) ((wParam - 'A') + Scaleform::Key::A);
        else if (wParam >= VK_F1 && wParam <= VK_F15)         keyCode = (Scaleform::KeyCode) ((wParam - VK_F1) + Scaleform::Key::F1);
        else if (wParam >= '0' && wParam <= '9')              keyCode = (Scaleform::KeyCode) ((wParam - '0') + Scaleform::Key::Num0);
        else if (wParam >= VK_NUMPAD0 && wParam <= VK_DIVIDE) keyCode = (Scaleform::KeyCode) ((wParam - VK_NUMPAD0) + Scaleform::Key::KP_0);
        else
        {
          for (int i = 0; keyCodeMap[i].winKey != 0; i++)
          {
            if (wParam == keyCodeMap[i].winKey)
            {
              keyCode = keyCodeMap[i].appKey;
              break;
            }
          }
        }

        //forward the event
        if(keyCode!=Scaleform::Key::None)
        { 
          Scaleform::UInt8 scanCode = (pObj->m_Msg.lParam >> 16) & 0xff;
          bool  extendedKeyFlag = (pObj->m_Msg.lParam & (1 << 24)) != 0;
          // we need a scanCode to distinguish between left and right shifts
          if (wParam == VK_SHIFT && scanCode == 54) // right shift code
          {
            extendedKeyFlag = true;
          }

          m_pMovieInst->HandleEvent(KeyEvent(eventType, keyCode, 0, 0, *updateModifiers(extendedKeyFlag)));
        }
        break; //do not mark msg as handled, otherwise WM_CHAR will miss the character
      }

    case WM_CHAR:
      m_pMovieInst->HandleEvent(CharEvent((Scaleform::UInt32)pObj->m_Msg.wParam));
      break;

    default:
      break;
  }
}

//keep track of the pressed modifiers like CTRL, SHIRT, etc..
Scaleform::KeyModifiers * VScaleformMovieInstance::updateModifiers(bool extendedKeyFlag)
{
  Scaleform::KeyModifiers mods;
  if (::GetKeyState(VK_SHIFT) & 0x8000)
    mods.SetShiftPressed(true);
  if (::GetKeyState(VK_CONTROL) & 0x8000)
    mods.SetCtrlPressed(true);
  if (::GetKeyState(VK_MENU) & 0x8000)
    mods.SetAltPressed(true);
  if (::GetKeyState(VK_NUMLOCK) & 0x1)
    mods.SetNumToggled(true);
  if (::GetKeyState(VK_CAPITAL) & 0x1)
    mods.SetCapsToggled(true);
  if (::GetKeyState(VK_SCROLL) & 0x1)
    mods.SetScrollToggled(true);
  if (extendedKeyFlag)
    mods.SetExtendedKey(true);
  *m_pKeyModifiers = mods;
  return m_pKeyModifiers;
}

#endif

bool VScaleformMovieInstance::IsValid() const
{
  return (m_pMovieInst != NULL);
}

void VScaleformMovieInstance::SetVisibleBitmask(unsigned int iMask) 
{ 
  m_iVisibleMask = iMask; 
}

unsigned int VScaleformMovieInstance::GetVisibleBitmask() const 
{ 
  return m_iVisibleMask; 
}

bool VScaleformMovieInstance::IsVisibleInAnyContext() const
{
  return m_iVisibleMask > 0;
}

void VScaleformMovieInstance::SetPaused(bool bPaused)
{
  m_pAdvanceTask->WaitUntilFinished();

  m_pMovieInst->SetPause(bPaused);
}

bool VScaleformMovieInstance::IsPaused() const
{
  m_pAdvanceTask->WaitUntilFinished();

  // 0 playback
  // 1 stopped
  return (m_pMovieInst->GetPlayState() != 0);
}

bool VScaleformMovieInstance::IsFocused() const
{
  if(m_pMovieInst == NULL) 
    return false;

  m_pAdvanceTask->WaitUntilFinished();
  return m_pMovieInst->IsMovieFocused();
}

void VScaleformMovieInstance::SetHandleInput(bool bEnabled)
{
  m_bHandleInput = bEnabled;
}

const char * VScaleformMovieInstance::GetFileName() const
{
  return m_sFileName.AsChar();
}

Scaleform::GFx::Movie* VScaleformMovieInstance::GetGFxMovieInstance() const
{
  return m_pMovieInst;
}

void VScaleformMovieInstance::WaitForAdvanceFinished() const
{
  m_pAdvanceTask->WaitUntilFinished();
}

void VScaleformMovieInstance::SetDimensions(int iPosX /*= 0*/, int iPosY /*= 0*/, int iWidth /*= -1*/, int iHeight /*= -1*/)
{
  m_iPosX = iPosX;
  m_iPosY = iPosY;
  m_iWidth = iWidth;
  m_iHeight = iHeight;

  UpdateViewport();
}

void VScaleformMovieInstance::GetDimensions(int& iPosX, int& iPosY, int& iWidth, int& iHeight)
{
  iPosX = m_iPosX;
  iPosY = m_iPosY;

  iWidth = m_iWidth;
  iHeight = m_iHeight;
}


void VScaleformMovieInstance::SetPosition(int iPosX, int iPosY)
{
  SetDimensions(iPosX, iPosY, m_iWidth, m_iHeight);
}

// gets the position
void VScaleformMovieInstance::GetPosition(int& iPosX, int& iPosY) const
{
  iPosX = m_iPosX;
  iPosY = m_iPosY;
}

// sets the size of the movie
void VScaleformMovieInstance::SetSize(int iWidth, int iHeight)
{
  SetDimensions(m_iPosX, m_iPosY, iWidth, iHeight);
}

// gets the currently set size of the movie
void VScaleformMovieInstance::GetSize(int& iWidth, int& iHeight) const
{
  iWidth = m_iWidth;
  iHeight = m_iHeight;
}

// gets the size which was used when authoring the movie
void VScaleformMovieInstance::GetAuthoredSize(int& iWidth, int& iHeight) const
{
  iWidth = m_iMovieAuthoredWidth;
  iHeight = m_iMovieAuthoredHeight;
}

void VScaleformMovieInstance::SetAuthoredSize()
{
  SetDimensions(m_iPosX, m_iPosY, SF_MOVIE_SIZE_AUTHORED, SF_MOVIE_SIZE_AUTHORED);
}

void VScaleformMovieInstance::UpdateViewport()
{
  if (m_pMovieInst == NULL || m_pLastRenderContext == NULL)
    return;

  // if the height or width is undefined use the render context size it is attached to
  int iRenderContextWidth, iRenderContextHeight;
  m_pLastRenderContext->GetSize(iRenderContextWidth, iRenderContextHeight);

  if(m_iWidth == SF_MOVIE_SIZE_RENDERCONTEXT)
    m_iWidth = iRenderContextWidth;
  else if(m_iWidth == SF_MOVIE_SIZE_AUTHORED)
    m_iWidth = m_iMovieAuthoredWidth;

  if(m_iHeight == SF_MOVIE_SIZE_RENDERCONTEXT)
    m_iHeight = iRenderContextHeight;
  else if(m_iHeight == SF_MOVIE_SIZE_AUTHORED)
    m_iHeight = m_iMovieAuthoredHeight;

  m_pAdvanceTask->WaitUntilFinished();

  unsigned int iFlags = 0;
  if (m_bIsTexture)
  {
    iFlags |= Scaleform::Render::Viewport::View_IsRenderTexture;
  }

  m_pMovieInst->SetViewport(iRenderContextWidth, iRenderContextHeight, m_iPosX, m_iPosY, m_iWidth, m_iHeight, iFlags);
}

bool VScaleformMovieInstance::ValidateFocus(float fX, float fY)
{
  VASSERT_MSG(m_pMovieInst!=NULL, "GFx Movie not present");
  m_pAdvanceTask->WaitUntilFinished();

  // focus will be restored when clicking anywhere inside the scaleform movie and the movie is visible
  if( IsVisibleInAnyContext() && (fX>=m_iPosX && fX<=(m_iPosX+m_iWidth)) && (fY>=m_iPosY && fY<=(m_iPosY+m_iHeight)) )
  {
    if(m_pMovieInst->IsMovieFocused()) return false;

    m_pMovieInst->HandleEvent(Event::SetFocus);
    return true;
  }

  //outside click
  m_pMovieInst->HandleEvent(Event::KillFocus);
  return false;
}

void VScaleformMovieInstance::SyncScaleformVariables()
{
  for (int i = 0; i < m_referencedScaleformValues.GetSize(); i++)
  {
    VScaleformValue* pValue = m_referencedScaleformValues.GetAt(i);

    pValue->SyncValueWithScaleform();
  }
}

void VScaleformMovieInstance::HandleScaleformCallbacks()
{
  // Process FS callbacks
  for (int i = 0; i < m_queuedFSCommands.GetSize(); i++)
  {
    VOnFSCommand* pData = m_queuedFSCommands[i];

    // call virtual function
    OnCommand(pData->m_sCommand, pData->m_sArgs); 

    // ..and also trigger the callback
    VOnFSCommand::OnFSCallback.TriggerCallbacks(pData);

    // inform scripting system about the callback...
    
    // call 'OnFsCommand' for the scene script (if callback is present)
    IVScriptInstance *pSceneInst = VScriptResourceManager::GlobalManager().GetSceneScript();
    if (pSceneInst != NULL)
      pSceneInst->ExecuteFunctionArg("OnFsCommand", "sss", GetFileName(), pData->m_sCommand.AsChar(), pData->m_sArgs.AsChar());

    // call 'OnFsCommand' for the game script (if callback is present)
    IVScriptInstance *pGameInst = VScriptResourceManager::GlobalManager().GetGameScript();
    if (pGameInst != NULL)
      pGameInst->ExecuteFunctionArg("OnFsCommand", "sss", GetFileName(), pData->m_sCommand.AsChar(), pData->m_sArgs.AsChar());

    delete pData;
  }

  m_queuedFSCommands.SetSize(0, -1, false /* Do not deallocate */);

  // Process External Interface Calls
  for (int i = 0; i < m_queuedExternalCalls.GetSize(); i++)
  {
    VOnExternalInterfaceCall* pData = m_queuedExternalCalls[i];

    // call virtual function
    OnExternalInterfaceCall(pData->m_sMethodName, pData->m_ppArgs, pData->m_uiArgCount);

    // ..and also trigger the callback
    VOnExternalInterfaceCall::OnExternalInterfaceCallback.TriggerCallbacks(pData);

    //inform scripting system about the callback...
    IVScriptInstance *pSceneInst = VScriptResourceManager::GlobalManager().GetSceneScript();
    IVScriptInstance *pGameInst = VScriptResourceManager::GlobalManager().GetGameScript();

    // call 'OnExternalInterfaceCall' for the scene script (if callback is present)
    if(pSceneInst != NULL || pGameInst != NULL)
    {
      VASSERT_MSG(pData->m_uiArgCount < 59, 
        "Exceeded max parameter count of 59 for VScaleformExternalInterfaceHandler in Lua (table truncated after parameter 5)");

      char** ppArray = new char* [pData->m_uiArgCount];
      VString* stringRepresentation = new VString[pData->m_uiArgCount];

      char szFormatStringBuffer[64] = "ss[";
      for(unsigned int i = 0; i < pData->m_uiArgCount; i++)
      {
        // this 'copy' is required since 'ToString()' will be
        // removed from the stack at the end of the loop
        stringRepresentation[i] = pData->m_ppArgs[i]->ToString().AsChar(); 

        // assign to void * array
        ppArray[i] = stringRepresentation[i].GetChar();

        // add the string parameter to the format string...
        szFormatStringBuffer[i+3] = 's';
      }

      // terminate the format string
      szFormatStringBuffer[pData->m_uiArgCount + 3] = ']';
      szFormatStringBuffer[pData->m_uiArgCount + 4] = 0;

      // scene script
      if (pSceneInst != NULL)
      {
        pSceneInst->ExecuteFunctionArg("OnExternalInterfaceCall", szFormatStringBuffer, 
          GetFileName(), pData->m_sMethodName.AsChar(), ppArray);
      }

      // game script
      if (pGameInst != NULL)
      {
        pGameInst->ExecuteFunctionArg("OnExternalInterfaceCall", szFormatStringBuffer, 
          GetFileName(), pData->m_sMethodName.AsChar(), ppArray);
      }

      V_SAFE_DELETE_ARRAY(ppArray);
      V_SAFE_DELETE_ARRAY(stringRepresentation);
    }

    delete pData;
  }

  m_queuedExternalCalls.SetSize(0, -1, false  /* Do not deallocate */);
}

VOnExternalInterfaceCall::VOnExternalInterfaceCall(
  VScaleformMovieInstance *pMovie, const char* szMethodName, const Scaleform::GFx::Value* pArgs, unsigned int uiArgCount) 
  : IVisCallbackDataObject_cl(&VOnExternalInterfaceCall::OnExternalInterfaceCallback)
  , m_pMovie(pMovie)
  , m_sMethodName(szMethodName)
  , m_ppArgs(NULL)
  , m_uiArgCount(uiArgCount)
{
  if(uiArgCount > 0)
  {
    m_ppArgs = new VScaleformValueConst*[uiArgCount];
    
    for(unsigned int i = 0; i < uiArgCount; i++)
    {
      // Create wrapped value and make copy of GFXValue.
      // Original value will not be accessible late on.
      m_ppArgs[i] = new VScaleformValueConst(new Scaleform::GFx::Value(pArgs[i]));

      // We're currently in the advance thread. So it is safe to sync the values.
      m_ppArgs[i]->SyncReferenceValueWithScaleform();
    }
  }
}

VOnExternalInterfaceCall::~VOnExternalInterfaceCall()
{
  for(unsigned int i = 0; i < m_uiArgCount; i++)
  {
    delete m_ppArgs[i];
  }
  V_SAFE_DELETE_ARRAY(m_ppArgs);
}

VOnFSCommand::VOnFSCommand(
  VScaleformMovieInstance *pMovie, const char* szCommand, const char* szArgs) : 
IVisCallbackDataObject_cl(&VOnFSCommand::OnFSCallback)
{
  m_pMovie = pMovie;
  m_sCommand = szCommand;
  m_sArgs = szArgs;
}

VOnFSCommand::~VOnFSCommand()
{
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
