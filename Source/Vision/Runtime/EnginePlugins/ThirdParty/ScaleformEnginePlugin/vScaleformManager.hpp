/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file vScaleformManager.hpp
#ifndef VSCALEFORMMANAGER_HPP_INCLUDED
#define VSCALEFORMMANAGER_HPP_INCLUDED

#if defined(WIN32) && !defined(HK_ANARCHY)
//remove USE_SF_IME and recompile the vScaleForm plugin if you haven't purchased ScaleformIME (included in VisionPlus)
//FIXME Bug HS#11455 #define USE_SF_IME
#endif

#if defined(VISIONDLL_ISPLUGIN) || defined(SCALEFORMPLUGIN_IMPORTS) || defined(SCALEFORMPLUGIN_EXPORTS)
  #include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformImpExp.hpp>
#else
  #define SCALEFORM_IMPEXP
#endif

//used file manager for IO stuff
extern IVFileStreamManager *g_pFileManager;

enum SF_CONSTANTS
{
  SF_MOVIE_SIZE_RENDERCONTEXT = -1,
  SF_MOVIE_SIZE_AUTHORED = -2,

  //this constant defines the max length of the cursor position array
  //edit for your needs!
  SF_MAX_CURSOR_NUM = 10,
  SF_CURSOR_SENSITIVITY = 550, // may range from 100 - 1000
  SF_WHEEL_SENSITIVITY = 1000 // may range from 1000 - 2000
};

//vision forward decls
class VLocalLoader;
class VScaleformMovieInstance;
class VScaleformCommandQueue;
#if defined _VISION_PS3
  class VScaleformMemoryManager;
#endif

#ifdef SF_AMP_SERVER
  class VScaleformAmpAppController;
#endif

//scaleform forward decls
namespace Scaleform
{
  namespace GFx
  {
    class Loader;
    class Event;
    class MouseEvent;
#ifdef _VISION_MOBILE
    class TouchEvent;
#elif defined(WIN32)
    class FontConfigSet;
#endif
  }

  namespace Render
  {
    class TextureManager;
    class TextureImage;
    class ThreadCommand;
    class Renderer2D;
    class ImageBase;

#if defined (WIN32) && defined(_VR_DX9)
    namespace D3D9 { class HAL; }
#elif defined(WIN32) && defined(_VR_DX11)
    namespace D3D1x { class HAL; }
#elif defined (_VISION_XENON)
    namespace X360 { class HAL; }
#elif defined (_VISION_PS3)
    namespace PS3 { class HAL; }
#elif defined (_VISION_MOBILE)
    namespace GL { class HAL; }
#endif
  }

  template<class C> class Ptr;
}

/// \class  VScaleformManager
///
/// \brief  Manages all instances of VScalefomMovieInstance and holds common used functions.
class VScaleformManager : public IVisCallbackHandler_cl
{
public:

  /// @name Init and DeInit methods
  /// @{

  /// \brief 
  ///   Initialization function, called when the first movie will be loaded (lazy loading)
  void Init(IVFileStreamManager *pManager = NULL);

  /// \brief 
  ///   Deinitialization function.
  void DeInit();

  /// @}

  /// @name Manager Methods
  /// @{

  /// \brief Static function to access the global instance of the Scaleform manager
  SCALEFORM_IMPEXP static VScaleformManager& GlobalManager();

  ///\brief
  /// Returns the collection of all scaleform movie instances
  SCALEFORM_IMPEXP VRefCountedCollection<VScaleformMovieInstance> &Instances();

  /// \brief  Enables or disables the multithreaded advance.
  ///         You can just change this setting this before the first movie is loaded!
  /// \param  bEnable true to enable, false to disable.
  SCALEFORM_IMPEXP void EnableMultithreadedAdvance(bool bEnable);

  /// \brief  Queries if a multithreaded advance is enabled.
  /// \return true if a multithreaded advance is enabled, false if not.
  SCALEFORM_IMPEXP bool IsMultithreadedAdvanceEnabled();

  /// \brief  Wait for all advance tasks to be finished.
  /// \note If you use multithreaded advance and want to call a function or access a data in AS
  ///       while advance tasks of movies that are running,
  ///       you should wait until the advance tasks are finished.
  SCALEFORM_IMPEXP void WaitForAllTasks();

  /// @}
  /// @name Movie Loading/Unloading
  /// @{


  /// \brief  Creates an new movie instance and loads a movie from .SWF or .GFX file (Allows
  /// 				specification of the IME configuration and candidate window).
  /// \note If you would like to customize your loader, call CreateLoader before call this function.
  /// 			A width or height = SF_MOVIE_SIZE_RENDERCONTEXT will lead to the usage of the width and
  /// 			height of the render, a width or height = SF_MOVIE_SIZE_AUTHORED will lead to the usage
  /// 			of the size information from the movie (e.g. the size in Flash).
  ///
  /// \param  szFileName        File name of the swf or gfx movie.
  /// \param  szCandidateMovie  the movie file for the candidate window - path relative to main movie file.
  /// \param  szImeXml          the xml file with your IME configuration - please specify an absolute path.
  /// \param  iPosX             (\b optional) the x position coordinate.
  /// \param  iPosY             (\b optional) the y position coordinate.
  /// \param  iWidth            (\b optional) the width.
  /// \param  iHeight           (\b optional) the height.
  ///
  /// \return NULL if the movie creation fails, otherwise an instance.
  /// \see CreateLoader
  SCALEFORM_IMPEXP VScaleformMovieInstance * LoadMovie(const char *szFileName,
    const char *szCandidateMovie, const char *szImeXml, int iPosX = 0, int iPosY = 0,
    int iWidth = SF_MOVIE_SIZE_RENDERCONTEXT, int iHeight = SF_MOVIE_SIZE_RENDERCONTEXT);

  /// \brief  Creates an new movie instance and loads a movie from .SWF or .GFX file.
  /// \note If you would like to customize your loader, call CreateLoader before call this function.
  /// 			A width or height = SF_MOVIE_SIZE_RENDERCONTEXT will lead to the usage of the width and
  /// 			height of the render, a width or height = SF_MOVIE_SIZE_AUTHORED will lead to the usage
  /// 			of the size information from the movie (e.g. the size in Flash).
  ///
  /// \param  szFileName        File name of the swf or gfx movie.
  /// \param  iPosX             (\b optional) the x position coordinate.
  /// \param  iPosY             (\b optional) the y position coordinate.
  /// \param  iWidth            (\b optional) the width.
  /// \param  iHeight           (\b optional) the height.
  ///
  /// \return NULL if the movie creation fails, otherwise an instance.
  /// \see CreateLoader
  SCALEFORM_IMPEXP VScaleformMovieInstance * LoadMovie(const char *szFileName, int iPosX = 0, int iPosY = 0,
    int iWidth = SF_MOVIE_SIZE_RENDERCONTEXT, int iHeight = SF_MOVIE_SIZE_RENDERCONTEXT);

  /// \brief  Get an already loaded movie instance.
  /// \param szFileName The file name of the movie (string has to match with the loading string).
  /// \return The instance if present, otherwise NULL.
  SCALEFORM_IMPEXP VScaleformMovieInstance * GetMovie(const char *szFileName) const;

  /// \brief  Unload a specified movie (delete the instance and remove it from the collection of the GlobalManager instance).
  /// \param  pMovie  The movie to delete.
  /// \return true if it succeeds, false if it fails.
  SCALEFORM_IMPEXP bool UnloadMovie(VScaleformMovieInstance *pMovie);

  /// \brief  Unload all Scaleform movie instances.
  SCALEFORM_IMPEXP void UnloadAllMovies();

  /// @}
  /// @name Input Handling
  /// @{
 
  /// \brief  Enables/disables (mouse) cursor input handling (enabled by default).
  /// \note   This just enables or disables the (mouse) cursor handling.
  /// 				When you turn on SetHandleWindowsInput, key input works independent of this switch.
  /// \param bEnable  true if the instance perform the input handling by itself.
  /// \see SetHandleWindowsInput
  SCALEFORM_IMPEXP void SetHandleCursorInput(bool bEnable);

  /// \brief  Checks if (mouse) cursor input handling is enabled or not
  /// \return true if CursorInput is enabled.
  /// \see SetHandleCursorInput
  inline bool GetHandleCursorInput() {return m_bHandleInput;}

#ifdef WIN32
  /// \brief Allows to control weather the Scaleform Manager should handle windows input and IME
  /// 			(key input and windows messages excluding mouse messages) or not.
  /// \note The windows input handling can be enabled additionally to the cursor (mouse)
  /// 			input handling, done via the SetHandleInput method. Windows input key handling
  /// 			is disabled by default.
  /// \param  bEnable true to enable, false to disable.
  /// \see SetHandleInput
  SCALEFORM_IMPEXP void SetHandleWindowsInput(bool bEnable);

  /// \brief  Checks if windows input handling is enabled or not
  /// \return true if windows input is enabled.
  /// \see SetHandleWindowsInput
  inline bool GetHandleWindowsInput() {return m_bHandlesWindowsInput;}

  /// \brief Set the positioning mode of the cursor to absolute or relative positioning.
  /// \param bAbsolute true for absolute, false for relative (incremental) mode.
  SCALEFORM_IMPEXP void SetAbsoluteCursorPositioning(bool bAbsolute);

#endif
  
#ifdef _VISION_MOBILE
  
  /// \brief Get the 0 based index of the primary touch point.
  /// \return A non-negative value if the primary touch point is set.
  ///         Any negative value means that there is no primary touch point (no touch point is interpretated as mouse)
  /// \see SetPrimaryTouchPoint
  SCALEFORM_IMPEXP int GetPrimaryTouchPoint();
  
  /// \brief Set the primary touch point for the Scaleform API.
  /// \param iTouchPoint The 0 based index of the primary touch point. 0 by default.
  ///                    A negative value will disable the primary touch point.
  ///                    If the max touch point index is exceeded the last possible value will be used.
  /// \see GetPrimaryTouchPoint
  SCALEFORM_IMPEXP void SetPrimaryTouchPoint(int iTouchPoint);
  
  /// \brief If enabled (default behavior) additional mouse events will be generated (beside the touch events).
  /// \param bEnable True to generate mouse events, otherwise false.
  /// \see   GetSimulateMouseEvents
  inline void SetSimulateMouseEvents(bool bEnable) { m_bSimulateMouseEvents = bEnable; }
  
  /// \brief Check if simulated mouse events are enabled.
  /// \return True if additional mouse events are generated apart from the touch events, otherwise false.
  /// \see   SetSimulateMouseEvents
  inline bool GetSimulateMouseEvents() { return m_bSimulateMouseEvents; }

  /// \brief Get the max number of touch points (minimum of the OS/device and the defined cursor number).
  static inline int GetMaxTouchPoints() { return s_iMaxTouchPoints; }
  
#endif

  /// \brief  Allows to associate a subset of trigger IDs with Scleform's keyboard events.
  ///
  /// \param  pInputMap The used input map.
  /// \param  iTriggerCount Number of present trigger in the map.
  /// \param  pTriggerID The trigger to map with a Scaleform key.
  /// \param  pGfXKeyMap The corresponding Scaleform key code.
  SCALEFORM_IMPEXP void SetKeyMapping(VInputMap *pInputMap, int iTriggerCount, const int *pTriggerID, const int *pGfXKeyMap);


  /// \brief You can set SetHandleInput to false and manually pass the cursor coordinates here (otherwise performed internally).
  /// 			 
  /// \param fCursorPosX Absolute X cursor position 
  /// \param fCursorPosY Absolute Y cursor position 
  /// \param fScrollDelta [\b optional] Scroll delta (e.g. from the mouse wheel)
  /// \param bLeftBtn [\b optional] Set to true if the left mouse button or equivalent (simple tap on mobile) is pressed
  /// \param bMiddleBtn [\b optional] Set to true if the middle mouse button or equivalent (drag on mobile) is pressed
  /// \param bRightBtn [\b optional] Set to true if the right mouse button or equivalent (unused on mobile) is pressed
  /// \param iMouseNum [\b optional] Index of the mouse (for multiple cursors / multi touch)
  SCALEFORM_IMPEXP void SetCursorPos(float fCursorPosX, float fCursorPosY, float fScrollDelta = 0, bool bLeftBtn = false, bool bMiddleBtn = false, bool bRightBtn = false, int iMouseNum = 0);

  /// \brief You can set SetHandleInput to false and manually increment cursor coordinates here (otherwise performed internally).
  /// \param fCursorDeltaX X mouse position change
  /// \param fCursorDeltaY Y mouse position change
  /// \param fScrollDelta [\b optional] Scroll delta
  /// \param bLeftBtn [\b optional] Set to true if the left mouse button or equivalent (simple tap on mobile) is pressed
  /// \param bMiddleBtn [\b optional] Set to true if the middle mouse button or equivalent (drag on mobile) is pressed
  /// \param bRightBtn [\b optional] Set to true if the right mouse button or equivalent (unused on mobile) is pressed
  /// \param iCursorNum [\b optional] Index of the mouse (for multiple cursors / multi touch)
  SCALEFORM_IMPEXP void IncCursorPos(float fCursorDeltaX, float fCursorDeltaY, float fScrollDelta = 0, bool bLeftBtn = false, bool bMiddleBtn = false, bool bRightBtn = false, int iCursorNum = 0)
  {
    SetCursorPos(m_pfCursorPosX[iCursorNum]+fCursorDeltaX, m_pfCursorPosY[iCursorNum]+fCursorDeltaY, fScrollDelta, bLeftBtn, bMiddleBtn, bRightBtn, iCursorNum);
  }

  /// \brief  Gets the cursor position of the specified mouse.
  /// \note   You can adjust SF_MAX_CURSOR_NUM to your needs.
  /// \param [out] fCursorPosX The mouse x coordinate.
  /// \param [out] fCursorPosY The mouse y coordinate.
  /// \param  iCursorNum  (\b optional) zero-based index of the mouse number (0 to SF_MAX_CURSOR_NUM).
  ///                     Default is -1 which will use the last active cursor.
  SCALEFORM_IMPEXP void GetCursorPos(float &fCursorPosX, float &fCursorPosY, int iCursorNum = -1) const;
  
  /// \brief Returns the 0-based index of the last active cursor.
  /// \return The cursor index from 0 to SF_MAX_CURSOR_NUM-1.
  inline int GetActiveCursor() { return m_iLastActiveCursor; }

#ifdef WIN32
  
  /// @}
  /// @name Font Configuration
  /// @{
  
  /// \brief  Applies the specified font to the loader. If there is no present loader object a new one will be created.
  /// \param  szFontFilename  File name of the font.
  /// \param  szConfigName [\b optional] FontConfig name to use, using first font config if unspecified.
  /// \return true if it succeeds, false if it fails.
  /// \see CreateLoader
  SCALEFORM_IMPEXP bool ApplyFontToTheLoader(const char *szFontFilename,const char *szConfigName = NULL);
  
  /// \brief  Applies the specified FontConfig to the loader. 
  /// \param  szConfigName  FontConfig name to use.
  /// \return true if it succeeds, false if it fails.
  /// \see ApplyFontToTheLoader
  SCALEFORM_IMPEXP bool ApplyFontConfig(const char *szConfigName);
  
#endif
  
  /// @}
  /// @name Loader Methods (only use result pointers when linking statically!)
  /// @{

  /// \brief Get the current loader instance.
  /// \return The used loader instance or NULL if not present.
  SCALEFORM_IMPEXP Scaleform::GFx::Loader * GetLoader() const;

  /// \brief  Create a new loader. This call will replace the current loader (if present).
  /// \param pManager The file manager, which should be used by the loader.
  /// \return NULL if it fails, otherwise a pointer to the created loader.
  SCALEFORM_IMPEXP Scaleform::GFx::Loader* CreateLoader(IVFileStreamManager *pManager = NULL);

  /// @}
  /// @name Rendering Related Methods
  /// @{

  /// \brief  Set the vision specific render order for the OnRenderHook callback.
  /// \param eRenderHook  
  ///   The value for the render order, default is SCALEFORM_RENDERHOOK_ORDERCONST (= VRH_PRE_SCREENMASKS).
  SCALEFORM_IMPEXP void SetRenderOrder(VRenderHook_e eRenderHook);

  /// \brief Renders the given movie instances.
  SCALEFORM_IMPEXP void RenderMovies(VScaleformMovieInstance** ppInstances, unsigned int iCount);

  /// @}
  /// @name Rendering Related Methods (only use result pointers when linking statically!)
  /// @{

  /// \brief  Returns a pointer to the used Scaleform texture manager.
  /// \return NULL if no movie and therefore no texture manager is present.
  SCALEFORM_IMPEXP Scaleform::Render::TextureManager * GetTextureManager() const;

  #if defined(_VISION_XENON) || defined(WIN32) || defined(_VISION_PS3)
    /// \brief Convert a vision texture to a Scaleform in order to be used inside a SWF movie.
    /// \param pTexture  The texture to convert.
    /// \return NULL if it fails, else the Scaleform texture.
    SCALEFORM_IMPEXP Scaleform::Render::TextureImage * ConvertTexture(VTextureObject *pTexture);
  #endif

  /// @}
  /// @name Internal Methods
  /// @{

  ///\brief Constructor
  VScaleformManager();

  ///\brief Destructor
  virtual ~VScaleformManager();

  /// @}
  /// @name Scaleform Key Codes (for input mapping)
  /// @{

  SCALEFORM_IMPEXP static const int SF_KEY_BACKSPACE;
  SCALEFORM_IMPEXP static const int SF_KEY_TAB;
  SCALEFORM_IMPEXP static const int SF_KEY_CLEAR;
  SCALEFORM_IMPEXP static const int SF_KEY_RETURN;
  SCALEFORM_IMPEXP static const int SF_KEY_SHIFT;
  SCALEFORM_IMPEXP static const int SF_KEY_CONTROL;
  SCALEFORM_IMPEXP static const int SF_KEY_ALT;
  SCALEFORM_IMPEXP static const int SF_KEY_PAUSE;
  SCALEFORM_IMPEXP static const int SF_KEY_CAPSLOCK;
  SCALEFORM_IMPEXP static const int SF_KEY_ESCAPE;
  SCALEFORM_IMPEXP static const int SF_KEY_SPACE;
  SCALEFORM_IMPEXP static const int SF_KEY_PAGEUP;
  SCALEFORM_IMPEXP static const int SF_KEY_PAGEDOWN;
  SCALEFORM_IMPEXP static const int SF_KEY_END;
  SCALEFORM_IMPEXP static const int SF_KEY_HOME;
  SCALEFORM_IMPEXP static const int SF_KEY_LEFT;
  SCALEFORM_IMPEXP static const int SF_KEY_UP;
  SCALEFORM_IMPEXP static const int SF_KEY_RIGHT;
  SCALEFORM_IMPEXP static const int SF_KEY_DOWN;
  SCALEFORM_IMPEXP static const int SF_KEY_INSERT;
  SCALEFORM_IMPEXP static const int SF_KEY_DELETE;
  SCALEFORM_IMPEXP static const int SF_KEY_HELP;
  SCALEFORM_IMPEXP static const int SF_KEY_NUMLOCK;
  SCALEFORM_IMPEXP static const int SF_KEY_SCROLLLOCK;

  SCALEFORM_IMPEXP static const int SF_KEY_SEMICOLON;
  SCALEFORM_IMPEXP static const int SF_KEY_EQUAL;
  SCALEFORM_IMPEXP static const int SF_KEY_COMMA;
  SCALEFORM_IMPEXP static const int SF_KEY_MINUS;
  SCALEFORM_IMPEXP static const int SF_KEY_PERIOD;
  SCALEFORM_IMPEXP static const int SF_KEY_SLASH;
  SCALEFORM_IMPEXP static const int SF_KEY_BAR;
  SCALEFORM_IMPEXP static const int SF_KEY_BRACKETLEFT;
  SCALEFORM_IMPEXP static const int SF_KEY_BACKSLASH;
  SCALEFORM_IMPEXP static const int SF_KEY_BRACKETRIGHT;
  SCALEFORM_IMPEXP static const int SF_KEY_QUOTE;
  SCALEFORM_IMPEXP static const int SF_KEY_OEM_AX;   //  'AX' key on Japanese AX kbd
  SCALEFORM_IMPEXP static const int SF_KEY_OEM_102;  //  "<>" or "\|" on RT 102-key kbd.
  SCALEFORM_IMPEXP static const int SF_KEY_ICO_HELP; //  Help key on ICO
  SCALEFORM_IMPEXP static const int SF_KEY_ICO_00;	 //  00 key on ICO
  
  /// @}
protected:
  /// @name Callback Handling
  /// @{

  /// \brief Executes the callback actions.
  virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

  /// @}
  /// @name Methods operating on all movie instances
  /// @{

  /// \brief Broadcasts the validate focus method to all movie instances
  void ValidateFocus(float x, float y);

  /// \brief broadcast the event to all movies
  void HandleEvent(Scaleform::GFx::Event * pEvent);

  /// \brief broadcast mouse events (here the relative movie position will be subtracted from the cursor position).
  void HandleEvent(Scaleform::GFx::MouseEvent * pMouseEvent);
  
#ifdef _VISION_MOBILE
  /// \brief broadcast touch events (here the relative movie position will be subtracted from the cursor position).
  void HandleEvent(Scaleform::GFx::TouchEvent * pTouchEvent);
#endif

  /// @}
  /// @name Internal (cursor) input handling
  /// @{

  //optionally handles mouse input
  void HandleInput();
  
  //handle the customized input map
  void HandleInputMap();

  /// @}

#ifdef WIN32
  /// \brief Get Font config index.
  int GetFontConfigIndexByName(const char *szConfigName);

  Scaleform::GFx::FontConfigSet* m_pFontConfigSet;
#endif

  //for one time init of the manager
  bool m_bInitialized;

  //collection of all loaded movie instances
  VRefCountedCollection<VScaleformMovieInstance> m_Instances;

  //the default loader for movie files
  VLocalLoader * m_pLoader;

  // Scaleform renderer HAL (we cannot use Scaleform smart pointers here,
  // because it looks like an instance for the compiler - and not like a pointer)
  #if defined (WIN32) && defined(_VR_DX9)
    Scaleform::Render::D3D9::HAL *  m_pRenderHal;
  #elif defined(WIN32) && defined(_VR_DX11)
    Scaleform::Render::D3D1x::HAL * m_pRenderHal; 
  #elif defined (_VISION_XENON)
    Scaleform::Render::X360::HAL * m_pRenderHal;
  #elif defined (_VISION_PS3)
    Scaleform::Render::PS3::HAL * m_pRenderHal; 
  #elif defined (_VISION_MOBILE)
    Scaleform::Render::GL::HAL * m_pRenderHal;
  #endif

  //2D renderer
  Scaleform::Render::Renderer2D * m_pRenderer;

  //set the render order for this movie
  int m_RenderOrder;

  //D3D device pointers
  #if defined (_VR_DX9) || defined (_VISION_XENON) 
    LPDIRECT3DDEVICE9   m_pd3dDevice;
  #elif defined(_VR_DX11)
    ID3D11Device*  m_pd3dDevice;
    ID3D11DeviceContext* m_pd3dContext;
  #endif

  #if defined(_VISION_MOBILE)
    static const int s_iMaxTouchPoints;
    int m_iPrimaryTouchPoint;
    bool m_bSimulateMouseEvents;
  #endif
  
  VScaleformCommandQueue * m_pCommandQueue;

  //input handling vars
  VInputMap *m_pCursorInputMap;

  bool m_bHandleInput;
  bool m_bHandlesWindowsInput;
  bool m_bUseAbsoluteCursorPosition;
  bool m_bMultithreadedAdvance;

  float m_pfCursorPosX[SF_MAX_CURSOR_NUM];
  float m_pfCursorPosY[SF_MAX_CURSOR_NUM];
  int m_iButtonMask[SF_MAX_CURSOR_NUM];
  int m_iLastActiveCursor;

  int m_iTriggerCount;
  DynArray_cl<int> m_pTriggerID;
  DynArray_cl<int> m_pGfXKeyMap;
  DynArray_cl<bool> m_bTriggerIDOldState;

  VInputMap *m_pInputMap;

#ifdef SF_AMP_SERVER
  friend class VScaleformAmpAppController;
  VScaleformAmpAppController * m_pAmpAppController;
#endif

private:
  static VScaleformManager s_globalInstance;
};


// defines the entry point inside a render loop (pre screenmasks is best to render custom mouse cursors on top)
const int SCALEFORM_RENDERHOOK_ORDERCONST = VRH_PRE_SCREENMASKS;


/// \brief Helper-Renderloop that displays nothing but the Scaleform movie
class VScaleformMovieExclusiveRenderLoop : public IVisRenderLoop_cl
{
public:
  SCALEFORM_IMPEXP VScaleformMovieExclusiveRenderLoop(VScaleformMovieInstance* pMovieInstance);

  /// \brief Implementation of IVisRenderLoop_cl
  SCALEFORM_IMPEXP virtual void OnDoRenderLoop(void *pUserData) HKV_OVERRIDE;

private:
  VScaleformMovieInstance* m_pMovieInstance;
};


#endif // VSCALEFORMMANAGER_HPP_INCLUDED

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
