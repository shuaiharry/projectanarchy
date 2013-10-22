/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file vScaleformManager.hpp
#ifndef VSCALEFORMMOVIE_HPP_INCLUDED
#define VSCALEFORMMOVIE_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformManager.hpp>

//Vision forward decls
class VScaleformValue;
class VScaleformValueConst;
class VScaleformMovieInstance;
class VScaleformAdvanceTask;
typedef VSmartPtr<VScaleformMovieInstance> VScaleformMovieInstancePtr;

//Scaleform forwards decls
namespace Scaleform
{
  namespace Render
  {
    namespace ContextImpl
    {
      template<class C> class DisplayHandle;
    }
    class TreeRoot;
  }

  namespace GFx 
  {
    class Value;
    class Movie;
    class MovieDef;
    typedef Render::ContextImpl::DisplayHandle<Render::TreeRoot> MovieDisplayHandle;
  }

  class Key;
  class KeyModifiers;
  template<class C> class Ptr;

  // Thread::GetThreadId.
  #if defined(_VISION_PS3)
    typedef sys_ppu_thread_t ThreadId;
  #else
    typedef void* ThreadId;
  #endif
}

/// \class  VOnFSCommand
/// \brief  Data object that gets send by the OnCommandCallback. Provides information about the movie and the command event.
class VOnFSCommand : public IVisCallbackDataObject_cl
{
public:
  SCALEFORM_IMPEXP VOnFSCommand(){}

  /// \brief  Internal constructor for FS commands.
  SCALEFORM_IMPEXP VOnFSCommand(VScaleformMovieInstance *pMovie, const char* szCommand, const char* szArgs);

  /// \brief  Internal destructor.
  SCALEFORM_IMPEXP virtual ~VOnFSCommand();

  VScaleformMovieInstance *m_pMovie;  ///< the movie that triggered the event
  VString m_sCommand;                 ///< the command string as set by the artist
  VString m_sArgs;                    ///< the command argument as set by the artist

  SCALEFORM_IMPEXP static VisCallback_cl OnFSCallback; 
};

/// \class  VOnExternalInterfaceCall
/// \brief  Data object that gets send by the OnCommandCallback. Provides information about the movie and the command event.
class VOnExternalInterfaceCall : public IVisCallbackDataObject_cl
{
public:
  SCALEFORM_IMPEXP VOnExternalInterfaceCall(){}

  /// \brief  Internal constructor.
  SCALEFORM_IMPEXP VOnExternalInterfaceCall(
    VScaleformMovieInstance *pMovie, const char* szMethodName, const Scaleform::GFx::Value* pArgs, unsigned int uiArgCount);

  /// \brief  Internal destructor.
  SCALEFORM_IMPEXP virtual ~VOnExternalInterfaceCall();

  VScaleformMovieInstance *m_pMovie;      ///< the movie that triggered the event
  VString m_sMethodName;                  ///< the method name that has been called (comparable to the former command string)
  VScaleformValueConst** m_ppArgs;        ///< the array of argument pointer
  unsigned char m_uiArgCount;             ///< the number of passed arguments

  SCALEFORM_IMPEXP static VisCallback_cl OnExternalInterfaceCallback; 
};

/// \class  VScaleformMovieInstance
///
/// \brief  The Scaleform movie instance is a wrapper around the Scaleform::GFx::Movie, handling initialization
/// 				of the Scaleform sub system, FSCallbacks, ExternalInterface calls, memorizing the Scaleform::Loader
/// 				object, etc...
class VScaleformMovieInstance : public VRefCounter
{
  friend class VScaleformTexture;
  friend class VScaleformAdvanceTask;
  friend class VScaleformManager;
  friend class VScaleformValue;
  friend class VScaleformFSCommandHandler;
  friend class VScaleformExternalInterfaceHandler;
  friend class VOnExternalInterfaceCall;
  friend class VScaleformAmpAppController;

protected:
  /// \brief  Constructor. Creates an new class instance.
  /// \note A width or height = SF_MOVIE_SIZE_RENDERCONTEXT will lead to the usage of the width and height of the render,
  ///   a width or height = SF_MOVIE_SIZE_AUTHORED will lead to the usage of the size information from the movie (e.g. the size in Flash).
  ///
  /// \param  szFileName            File name of the swf or gfx movie.
  /// \param  pLoader               (\b optional) the loader to use for this movie (when setting to NULL, the movie will create an instance on demand).
  /// \param  szCandidateMovie      (\b optional - \b for \b ScaleformIME \b only) the movie file for the candidate window - path relative to main movie file.
  /// \param  szImeXml              (\b optional - \b for \b ScaleformIME \b only) the xml file with your IME configuration - please specify an absolute path.
  /// \param  iPosX                 (\b optional) the x position coordinate.
  /// \param  iPosY                 (\b optional) the y position coordinate.
  /// \param  iWidth                (\b optional) the width.
  /// \param  iHeight               (\b optional) the height.
  VScaleformMovieInstance(const char *szFileName, Scaleform::GFx::Loader* pLoader = NULL, const char *szCandidateMovie = NULL,
    const char *szImeXml = NULL, int iPosX = 0, int iPosY = 0, int iWidth = -1, int iHeight = -1);

  /// \brief  Destructor.
  virtual ~VScaleformMovieInstance();

public:
  /// @name Render Setup
  /// @{

  /// \brief  Sets the background opacity for this instance.
  /// \param  fAlpha  The alpha value - 0.0 means the scene background shines through.
  SCALEFORM_IMPEXP void SetOpacity(float fAlpha);

  /// \brief  Sets new dimensions for the movie.
  /// \note A width or height = SF_MOVIE_SIZE_RENDERCONTEXT will lead to the usage of the width and height of the render,
  ///   a width or height = SF_MOVIE_SIZE_AUTHORED will lead to the usage of the size information from the movie (e.g. the size in Flash).
  ///
  /// \param  iPosX   (\b optional) the x position coordinate.
  /// \param  iPosY   (\b optional) the y position coordinate.
  /// \param  iWidth  (\b optional) the width.
  /// \param  iHeight (\b optional) the height.
  SCALEFORM_IMPEXP void SetDimensions(int iPosX = 0, int iPosY = 0, int iWidth = SF_MOVIE_SIZE_RENDERCONTEXT, int iHeight = SF_MOVIE_SIZE_RENDERCONTEXT);

  /// \brief  Gets the current dimensions of the movie.
  ///
  /// \param [out] iPosX   the x position.
  /// \param [out] iPosY   the y position.
  /// \param [out] iWidth  the width.
  /// \param [out] iHeight the height.
  SCALEFORM_IMPEXP void GetDimensions(int& iPosX, int& iPosY, int& iWidth, int& iHeight);

  /// \brief  Sets the position of the movie.
  /// \param  iPosX  the x position coordinate.
  /// \param  iPosY  the y position coordinate.
  SCALEFORM_IMPEXP void SetPosition(int iPosX, int iPosY);

  /// \brief  Gets the current position of the movie.
  /// \param [out] iPosX   the x position.
  /// \param [out] iPosY   the y position.
  SCALEFORM_IMPEXP void GetPosition(int& iPosX, int& iPosY) const;

  /// \brief  Set the size of the movie.
  /// \param  iWidth  the width.
  /// \param  iHeight the height.
  SCALEFORM_IMPEXP void SetSize(int iWidth, int iHeight);

  /// \brief  Gets the current size of the movie.
  /// \param [out] iWidth  the width.
  /// \param [out] iHeight the height.
  SCALEFORM_IMPEXP void GetSize(int& iWidth, int& iHeight) const;

  /// \brief  Gets the size which was used when authoring the movie.
  /// \param [out] iWidth  the authoring width.
  /// \param [out] iHeight the authoring height.
  SCALEFORM_IMPEXP void GetAuthoredSize(int& iWidth, int& iHeight) const;

  /// \brief Sets the movie to the authored size.
  SCALEFORM_IMPEXP void SetAuthoredSize();

  /// @}
  /// @name Movie Control / Properties
  /// @{

  /// \brief  Sets the visibility bitmask for this movie instance.
  ///
  /// It will only be rendered into render contexts, if the logical AND result of this bitmask and 
  /// the context's bitmask (see VisRenderContext_cl::SetRenderFilterMask) is not zero.
  /// To hide an object completely in all contexts, use SetVisibleBitmask(0).
  /// By default, only the first bit is set.
  ///
  /// \param iMask
  ///   the visibility bitmask to use
  SCALEFORM_IMPEXP void SetVisibleBitmask(unsigned int iMask);

  /// \brief  Returns the previously set visibility bitmask for this movie instance.
  SCALEFORM_IMPEXP unsigned int GetVisibleBitmask() const;

  /// \brief  Query if this movie is visible in any render context.
  SCALEFORM_IMPEXP bool IsVisibleInAnyContext() const;

  /// \brief Get the file name of the movie
  /// \return The name of the movie, including the path.
  SCALEFORM_IMPEXP const char * GetFileName() const;

  /// \brief  Gets the GFx movie instance.
  /// \return NULL if no movie has been loaded, otherwise the instance.
  /// \see IsValid
  SCALEFORM_IMPEXP Scaleform::GFx::Movie* GetGFxMovieInstance() const;

  /// \brief 
  ///   Waits until the current advance step has finished.
  ///
  /// This should always be called before modifying the scaleform movie instance
  /// in order to synchronize with a multithreaded advance task.
  SCALEFORM_IMPEXP void WaitForAdvanceFinished() const;

  /// \brief  Query if the movie is valid (pointer to Scaleform movie instance is not NULL).
  /// \return true if valid, false if not.
  /// \see GetGFxMovieInstance
  SCALEFORM_IMPEXP bool IsValid() const;

  /// \brief
  ///   Clears the movie instance.
  ///
  /// After calling this function, IsValid() will return false.
  ///
  SCALEFORM_IMPEXP void Invalidate();

  /// \brief  Pauses the playback of the movie.
  /// \param bPaused set to true for pause, false for play
  SCALEFORM_IMPEXP void SetPaused(bool bPaused);

  /// \brief  Query if this object is paused.
  /// \return true if paused, false if not.
  SCALEFORM_IMPEXP bool IsPaused() const;

  /// \brief  Query if this object is focused.
  /// \return true if focused, false if not.
  SCALEFORM_IMPEXP bool IsFocused() const;

  /// \brief  
  ///   Gets a variable from the Scaleform movie.
  /// 
  /// The lifetime of the object returned is managed by this Scaleform movie. It
  /// will get destroyed when the movie instance is destroyed.
  ///
  /// \param  szVarName  
  ///   The name of the variable including path specification (e.g. '_root/MyVar').
  /// \return 
  ///   null if variable is not present, else the wrapped variable.
  SCALEFORM_IMPEXP VScaleformValue* GetVariable(const char * szVarName);

  /// @}
  /// @name Input Handling
  /// @{

  /// \brief Enable or disable input handling for this movie instance.
  /// \param bEnable Set to false to stop further input handling. Default is true.
  SCALEFORM_IMPEXP void SetHandleInput(bool bEnable);

#ifdef WIN32
  /// \brief  Handle the specified windows input message (if possible).
  SCALEFORM_IMPEXP void HandleWindowsInputMessage(VWindowsMessageDataObject* pObj);

  /// \brief  Handle the specified IME message before message translation.
  SCALEFORM_IMPEXP void HandlePreTranslatedIMEInputMessage(VWindowsMessageDataObject* pObj);
#endif

  /// @}
  /// @name Callbacks
  /// @{
  
  /// \brief Overridable to get movie event notifications. Custom movie classes can override this function
  /// or alternatively register a listener to the public OnCommandCallback callback
  ///
  /// \param  szCommand The command executed in the movie.
  /// \param  szArgs    The command's arguments.
  SCALEFORM_IMPEXP virtual void OnCommand(const char* szCommand, const char* szArgs)
  {
  }

  //
  // Overridable to get movie event notifications. Custom movie classes can override this function
  // or alternatively register a listener to the public OnExternalInterfaceCall callback
  SCALEFORM_IMPEXP virtual void OnExternalInterfaceCall(const char* szMethodName, VScaleformValueConst** ppArgs, unsigned int uiArgCount)
  {
  }

  /// @}

protected:
  /// \brief  Movie evaluates focus for a mouse click event at x,y.
  ///
  /// \param  fMouseX The mouse x coordinate.
  /// \param  fMouseY The mouse y coordinate.
  /// \return true if the movie newly received a focus event, false otherwise.
  bool ValidateFocus(float fMouseX, float fMouseY);

  //update movie viewport
  void UpdateViewport();

  //used in the input handling system (as proposed by SF)
  Scaleform::KeyModifiers * updateModifiers(bool extendedKeyFlag = false);

  ///
  /// \brief
  ///   Synchronizes the scaleform variable values with the movie.
  ///
  /// This can only be called when the advance task  isn't currently running.
  void SyncScaleformVariables();

  /// \brief
  ///   Process queued scaleform callbacks in main thread.
  void HandleScaleformCallbacks();

  /// \brief
  ///   Initialized the IME manager.
  void InitImeManager();

private:
  // GFx members:
  Scaleform::GFx::Movie * m_pMovieInst;
  Scaleform::GFx::MovieDisplayHandle * m_phMovieDisplay;

#if defined(USE_SF_IME)
  //vars required for IME init
  VString m_sCandidateMovie;
  VString m_sImeXml;
#endif

  bool m_bInit; ///< Used for initializing the advance thread.
  bool m_bHandleInput;
  bool m_bIsTexture;

  unsigned int m_iVisibleMask;

  VisRenderContext_cl* m_pLastRenderContext;

  int m_iPosX, m_iPosY;
  int m_iWidth, m_iHeight;
  int m_iMovieAuthoredWidth, m_iMovieAuthoredHeight;

  VString m_sFileName;

#ifdef WIN32
  Scaleform::KeyModifiers *m_pKeyModifiers;
#endif

  VScaleformAdvanceTask* m_pAdvanceTask;
  VArray<VScaleformValue*> m_referencedScaleformValues;

  // Queued callback objects send from the advance thread.
  // (Can be accessed when the advance thread is not running)
  VArray<VOnFSCommand*> m_queuedFSCommands;
  VArray<VOnExternalInterfaceCall*> m_queuedExternalCalls;
};

#endif // VSCALEFORMMOVIE_HPP_INCLUDED

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
