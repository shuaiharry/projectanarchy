
#ifndef VLUA_APIDOC

%nodefaultctor VScaleformMovieInstance;
%nodefaultdtor VScaleformMovieInstance;

class VScaleformMovieInstance
{
public:
  void SetOpacity(float fAlpha);

  void SetDimensions(int iPosX = 0, int iPosY = 0, int iWidth = SF_MOVIE_SIZE_RENDERCONTEXT, int iHeight = SF_MOVIE_SIZE_RENDERCONTEXT);
  
  void SetPosition(int iPosX, int iPosY);

  void SetSize(int iWidth, int iHeight);

  void SetAuthoredSize();

  void SetVisibleBitmask(unsigned int iMask);

  unsigned int GetVisibleBitmask() const;

  bool IsVisibleInAnyContext() const;
  
  const char * GetFileName() const;

  void SetPaused(bool bPaused);

  bool IsPaused() const;

  bool IsFocused() const;

  void SetHandleInput(bool bEnable);
  
  VScaleformValue* GetVariable(const char * szVarName);
};

//add lua tostring and concat operators
VSWIG_CREATE_CONCAT(VScaleformMovieInstance, 256, "[%s]", self->GetFileName())
VSWIG_CREATE_TOSTRING(VScaleformMovieInstance, "VScaleformMovieInstance: '%s'", self->GetFileName())

//implement GetPosition native because we would like to return two values at once
%native(VScaleformMovieInstance_GetPosition) int VScaleformMovieInstance_GetPosition(lua_State *L);
%{
  SWIGINTERN int VScaleformMovieInstance_GetPosition(lua_State *L)
  {
    IS_MEMBER_OF(VScaleformMovieInstance) //this will move this function to the method table of the specified class
    
    SWIG_CONVERT_POINTER(L, 1, VScaleformMovieInstance, pSelf)
    
    int x, y;
    pSelf->GetPosition(x, y);
    
    lua_pushnumber(L, (lua_Number)x);
    lua_pushnumber(L, (lua_Number)y);
      
    return 2; //in every case we leave two values at the stack
  }
%}

//implement GetSize native because we would like to return two values at once
%native(VScaleformMovieInstance_GetSize) int VScaleformMovieInstance_GetSize(lua_State *L);
%{
  SWIGINTERN int VScaleformMovieInstance_GetSize(lua_State *L)
  {
    IS_MEMBER_OF(VScaleformMovieInstance) //this will move this function to the method table of the specified class
    
    SWIG_CONVERT_POINTER(L, 1, VScaleformMovieInstance, pSelf)
    
    int x, y;
    pSelf->GetSize(x, y);
    
    lua_pushnumber(L, (lua_Number)x);
    lua_pushnumber(L, (lua_Number)y);
      
    return 2; //in every case we leave two values at the stack
  }
%}

//implement GetAuthoredSize native because we would like to return two values at once
%native(VScaleformMovieInstance_GetAuthoredSize) int VScaleformMovieInstance_GetAuthoredSize(lua_State *L);
%{
  SWIGINTERN int VScaleformMovieInstance_GetAuthoredSize(lua_State *L)
  {
    IS_MEMBER_OF(VScaleformMovieInstance) //this will move this function to the method table of the specified class
    
    SWIG_CONVERT_POINTER(L, 1, VScaleformMovieInstance, pSelf)
    
    int x, y;
    pSelf->GetAuthoredSize(x, y);
    
    lua_pushnumber(L, (lua_Number)x);
    lua_pushnumber(L, (lua_Number)y);
      
    return 2; //in every case we leave two values at the stack
  }
%}

#else

/// \brief  Wrapper class for VScaleformMovieInstance.
/// \par Example
///   \code
///     ...
///     mainMovie = Scaleform:LoadMovie("pause_menu.swf")
///  
///     -- e.g. temporary deactivate a menu
///     mainMenu:SetVisible(false)
///     mainMenu:SetHandleInput(false)
///     ...
///
///   \endcode
class VScaleformMovieInstance
{
public:

  ///
  /// @name Render Setup
  /// @{

  /// \brief  Sets the background opacity for this instance.
  /// \param  fAlpha  The alpha value - 0.0 means the scene background shines through.
  void SetOpacity(float fAlpha);

  /// \brief Sets new dimensions for the movie.
  /// \note Call parameterless to reset to it's defaults.
  /// \param  x   (\b optional) the x position coordinate.
  /// \param  y   (\b optional) the y position coordinate.
  /// \param  width  (\b optional) the width.
  /// \param  height (\b optional) the height.
  /// \see VScaleformManager::GetMovieRenderContextSize
  void SetDimensions(number x = 0, number y = 0, number width = SF_MOVIE_SIZE_RENDERCONTEXT, number height = SF_MOVIE_SIZE_RENDERCONTEXT);
  
  /// \brief Set the position of the movie.
  /// \param  x  the x position coordinate.
  /// \param  y  the y position coordinate.
  void SetPosition(number x, number y);

  /// \brief  Set the size of the movie.
  /// \param  width  the width.
  /// \param  height the height.
  void SetSize(number width, number height);

  /// \brief Sets the movie to the authored size.
  void SetAuthoredSize();
  
  /// \brief  Gets the current size of the movie.
  /// \return Two numbers defining the current size.
  /// \par Example
  ///   \code
  ///     ...
  ///     mainMovie = Scaleform:LoadMovie("menu.swf")
  ///     local width, height = mainMenu:GetSize()
  ///     ...
  ///   \endcode
  multiple GetSize();
  
  /// \brief  Gets the current position of the movie.
  /// \return Two numbers defining the current x and y position.
  /// \par Example
  ///   \code
  ///     ...
  ///     mainMovie = Scaleform:LoadMovie("menu.swf", 30, 90)
  ///     local x, y = mainMenu:GetPosition()
  ///     Vision.Assert(x==30, "The x position should be 30!")
  ///     Vision.Assert(y==90, "The y position should be 90!")
  ///     ...
  ///   \endcode
  multiple GetPosition();
  
  /// \brief  Gets the size which was used when authoring the movie.
  /// \return Two numbers defining the authored size.
  /// \par Example
  ///   \code
  ///     ...
  ///     mainMovie = Scaleform:LoadMovie("menu.swf")
  ///     local width, height = mainMenu:GetAuthoredSize()
  ///     ...
  ///   \endcode
  multiple GetAuthoredSize();
  
  /// @}
  /// @name Movie Control
  /// @{
  
  /// \brief  Query if this movie is visible.
  /// \return true if visible, false if not.
  string GetFileName();

  /// \brief  Sets the visibility bitmask for this movie instance.
  ///
  /// It will only be rendered into render contexts, if the logical AND result of this bitmask and 
  /// the context's bitmask (see VisRenderContext_cl::SetRenderFilterMask) is not zero.
  /// To hide an object completely in all contexts, use SetVisibleBitmask(0).
  /// By default, only the first bit is set.
  ///
  /// \param iMask
  ///   the visibility bitmask to use
  void SetVisibleBitmask(unsigned int iMask);

  /// \brief  Returns the previously set visibility bitmask for this movie instance.
  unsigned int GetVisibleBitmask() const;

  /// \brief  Query if this movie is visible in any render context.
  bool IsVisibleInAnyContext() const;

  /// \brief  Pauses the playback of the movie.
  /// \param paused set to true for pause, false for play.
  void SetPaused(boolean paused);

  /// \brief  Query if this object is paused.
  /// \return true if paused, false if not.
  boolean IsPaused();

  /// \brief  Query if this object is focused.
  /// \return true if focused, false if not.
  boolean IsFocused();

  /// \brief Enable or disable input handling for this movie instance.
  /// \param enable Set to false to stop further input handling. Default is true.
  void SetHandleInput(boolean enable);
  
  /// \brief  Gets a variable from the Scaleform movie.
  /// \param  varName  The name of the variable including path in the movie (e.g. '_root/MyVar').
  /// \return null if it fails, else the wrapped variable.
  /// \par Example
  ///   \code
  ///     ...
  ///     if var==nil then
  ///       var = movie:GetVariable("_root.mainMenu")
  ///     else
  ///       var:Display_SetXYRotation(x, y); -- create a 3Di effect
  ///     end
  ///     ...
  ///   \endcode
  VScaleformValue GetVariable(string varName);
  
  /// @}
  ///

};

#endif