
#ifndef VLUA_APIDOC

%nodefaultctor VScaleformManager;
%nodefaultdtor VScaleformManager;

class VScaleformManager
{
public:

  void EnableMultithreadedAdvance(bool bEnable);

  bool IsMultithreadedAdvanceEnabled();

  VScaleformMovieInstance * LoadMovie(const char *szFileName,
    const char *szCandidateMovie, const char *szImeXml, int iPosX = 0, int iPosY = 0,
    int iWidth = SF_MOVIE_SIZE_RENDERCONTEXT, int iHeight = SF_MOVIE_SIZE_RENDERCONTEXT);

  VScaleformMovieInstance * LoadMovie(const char *szFileName, int iPosX = 0, int iPosY = 0,
    int iWidth = SF_MOVIE_SIZE_RENDERCONTEXT, int iHeight = SF_MOVIE_SIZE_RENDERCONTEXT);

  VScaleformMovieInstance * GetMovie(const char *szFileName) const;

  bool UnloadMovie(VScaleformMovieInstance *pMovie);
  
  void UnloadAllMovies();

  void SetHandleCursorInput(bool bEnable);
  
  %extend{
    void SetAbsoluteCursorPositioning(bool bAbsolutePos)
    {
      %#ifdef WIN32
        self->SetAbsoluteCursorPositioning(bAbsolutePos);
      %#else
        Vision::Error.SystemMessage("Warning: Calling unsupported method 'SetAbsoluteCursorPositioning'");
      %#endif
    }
    
    int GetMovieRenderContextSize()
    {
      return SF_MOVIE_SIZE_RENDERCONTEXT;
    }
  }

  void SetCursorPos(float fMouseX, float fMouseY, float fWheelDelta = 0, bool bLeftBtn = false, bool bMiddleBtn = false, bool bRightBtn = false, int iMouseNum = 0);

  void IncCursorPos(float fMouseXDelta, float fMouseYDelta, float fWheelDelta = 0, bool bLeftBtn = false, bool bMiddleBtn = false, bool bRightBtn = false, int iMouseNum = 0);
  
  %extend{
    VSWIG_CREATE_CAST_UNSAFE(VScaleformManager);
  }
};

#else

/// \brief 
///   The \b Scaleform global is a wrapped instance of the global VScaleformManager.
/// \note
///   The Lua module containing all wrappes is called GFx, however there is no need to access the module directly.
/// \par Example for a scene or game script:
///   \code
///     function OnAfterSceneLoaded()
///       -- load the main menu at position 50,50
///       mainMovie = Scaleform:LoadMovie("mainmenu.swf", 50, 50)
///  
///       --load a resource file with a defined mouse cursor and make it visible
///       GUI:LoadResourceFile("GUI/MenuSystem.xml")
///       GUI:SetCursorVisible(true)
///     end
///
///     function OnAfterSceneUnloaded()
///       --unload all movies
///	      Scaleform:UnloadAllMovies()
///
///       --hide the cursor again
///       GUI:SetCursorVisible(false)
///     end
///
///   \endcode
class VScaleformManager 
{
public:
  
  ///
  /// @name Movie Handling
  /// @{
  
  /// \brief  Enables or disables the multithreaded advance.
  ///         You can just change this setting this before the first movie is loaded!
  /// \note The setting will persist until the vScaleformPlugin is unloaded!
  /// \param  enable true to enable, false to disable.
  void EnableMultithreadedAdvance(boolean enable);

  /// \brief  Queries if a multithreaded advance is enabled.
  /// \return true if a multithreaded advance is enabled, false if not.
  boolean IsMultithreadedAdvanceEnabled();

  /// \brief  Creates an new movie instance and loads a movie from .SWF or .GFX file (Allows
  /// 				specification of the IME configuration and candidate window).
  /// \param  fileName        File name of the swf or gfx movie.
  /// \param  candidateMovie  the movie file for the candidate window - path relative to main movie file.
  /// \param  imeXml          the xml file with your IME configuration - please specify an absolute path.
  /// \param  posX             (\b optional) the x position coordinate.
  /// \param  posY             (\b optional) the y position coordinate.
  /// \param  width            (\b optional) the width.
  /// \param  height           (\b optional) the height.
  ///
  /// \return nil if the movie creation fails, otherwise an instance.
  ///
  /// \par Example for a scene or game script:
  ///   \code
  ///     function OnAfterSceneLoaded()
  ///       -- load the main menu with IME support at position 50,50
  ///       mainMovie = Scaleform:LoadMovie("mainmenu.swf", "candidate.swf", "ime.xml", 50, 50)
  ///     end
  ///
  ///     function OnAfterSceneUnloaded()
  ///	      Scaleform:UnloadAllMovies()
  ///     end
  ///   \endcode
  /// \see UnloadMovie
  /// \see UnloadAllMovies
  /// \see GetMovieRenderContextSize
  VScaleformMovieInstance LoadMovie(string fileName, string candidateMovie = nil, string imeXml = nil,
    number posX = 0, number posY = 0, number width = SF_MOVIE_SIZE_RENDERCONTEXT, number height = SF_MOVIE_SIZE_RENDERCONTEXT);

  /// \brief  Creates an new movie instance and loads a movie from .SWF or .GFX file.
  /// \param  fileName        File name of the swf or gfx movie.
  /// \param  posX             (\b optional) the x position coordinate.
  /// \param  posY             (\b optional) the y position coordinate.
  /// \param  width            (\b optional) the width.
  /// \param  height           (\b optional) the height.
  ///
  /// \return nil if the movie creation fails, otherwise an instance.
  ///
  /// \par Example for a scene or game script:
  ///   \code
  ///     function OnAfterSceneLoaded()
  ///       -- load the main menu at position 0, 200 withj a size of 100, 100
  ///       mainMovie = Scaleform:LoadMovie("mainmenu.swf", 0, 200, 100, 100)
  ///     end
  ///
  ///     function OnAfterSceneUnloaded()
  ///	      Scaleform:UnloadAllMovies()
  ///     end
  ///   \endcode
  /// \see UnloadMovie
  /// \see UnloadAllMovies
  /// \see GetMovieRenderContextSize
  VScaleformMovieInstance LoadMovie(string fileName, number posX = 0, number posY = 0,
    number width = SF_MOVIE_SIZE_RENDERCONTEXT, number height = SF_MOVIE_SIZE_RENDERCONTEXT);

  /// \brief Get an already loaded movie instance.
  /// \param fileName The file name of the movie (string has to match with the loading string).
  /// \return The instance if present, otherwise nil.
  VScaleformMovieInstance GetMovie(string fileName);

  /// \brief  Unload a specified movie (delete the instance and remove it from the collection of the GlobalManager instance).
  /// \param  movie  The movie to delete.
  /// \return true if it succeeds, false if it fails.
  /// \see UnloadAllMovies
  boolean UnloadMovie(VScaleformMovieInstance movie);
  
  /// \brief  Unload all Scaleform movie instances.
  /// \par Example for a scene or game script:
  ///   \code
  ///     function OnAfterSceneUnloaded()
  ///	      Scaleform:UnloadAllMovies()
  ///     end
  ///   \endcode
  /// \see UnloadMovie
  void UnloadAllMovies();

  /// @}
  /// @name Utility Functions
  /// @{

  /// \brief Set the positioning mode of the cursor to absolute or relative positioning. For Win32 only!
  /// \note In vForge the positing mode automatically switches to absolute, since we do
  ///  not track the mouse all the time (e.g. when not running the game in the editor)
  /// \param absolutePositionMode true for absolute, false for relative (incremental) mode.
  void SetAbsoluteCursorPositioning(boolean absolutePositionMode);
  
  /// \brief Enables/disables (mouse) cursor input handling (enabled by default).
  /// \param enable Set to true to enable, false to disable.
  void SetHandleCursorInput(boolean enable);

  /// \brief Use this function to manipulate cursor position or click in your Lua script.
  /// \param x Absolute X cursor position 
  /// \param y Absolute Y cursor position 
  /// \param wheelDelta [\b optional] Mouse wheel delta
  /// \param left [\b optional] Set to true if the left mouse button is pressed
  /// \param middle [\b optional] Set to true if the middle mouse button is pressed
  /// \param right [\b optional] Set to true if the right mouse button is pressed
  /// \param cursorIndex [\b optional] Index of the mouse (for multiple mouse cursors)
  void SetCursorPos(number x, number y, number wheelDelta = 0, boolean left = false, boolean middle = false, boolean right = false, number cursorIndex = 0);

  /// \brief Use this function to manipulate cursor position or click in your Lua script.
  /// \param deltaX Delta X cursor position
  /// \param deltaY Delta Y cursor position
  /// \param wheelDelta [\b optional] Mouse wheel delta
  /// \param left [\b optional] Set to true if the left mouse button is pressed
  /// \param middle [\b optional] Set to true if the middle mouse button is pressed
  /// \param right [\b optional] Set to true if the right mouse button is pressed
  /// \param cursorIndex [\b optional] Index of the mouse (for multiple mouse cursors)
  void IncCursorPos(number deltaX, number deltaY, number wheelDelta = 0, boolean left = false, boolean middle = false, boolean right = false, number cursorIndex = 0);
  
  /// \brief Get the size of the render context of a video (constant SF_MOVIE_SIZE_RENDERCONTEXT).
  /// \return The one dimensional size of the render context of a movie.
  number GetMovieRenderContextSize();
  
  /// @}
  /// @name Callbacks
  /// @{
  
  /// \brief Lua callback function for scene or game scripts. Triggered by OnExternalInterfaceCall of loaded Scaleform movies.
  /// \param movie The filename of the movie performing the external interface call.
  /// \param command The name of the command / function to be executed.
  /// \param argTable A Lua table containing the parameters of the the command as strings (from index 1 to n - can be empty too).
  ///                 However Lua will interprete these string as number as well.
  /// \par Example for a scene or game script:
  ///   \code
  ///   function OnExternalInterfaceCall(movie, command, argTable)
  ///     Debug:PrintLine("External Interface Call: " .. command .. ": " .. #argTable .. " arguments")
  ///   end
  ///   \endcode
  void OnExternalInterfaceCall(string movie, string command, table argTable);
  
  /// \brief Lua callback function for scene or game scripts. Triggered by OnFsCommand of loaded Scaleform movies.
  /// \param movie The filename of the movie performing the external interface call.
  /// \param command The name of the command / function to be executed.
  /// \param argument The argument of the command as string.
  /// \par Example for a scene or game script:
  ///   \code
  ///   function OnFsCommand(movie, command, argument)
  ///     Debug:PrintLine("FS Command: " .. command .. ": " .. argument)
  ///   end
  ///   \endcode
  void OnFsCommand(string movie, string command, string argument);
  
  ///
  /// @}
  ///
};

#endif
