#ifndef VLUA_APIDOC

%nodefaultctor VScaleformValue;
%nodefaultdtor VScaleformValue;

class VScaleformValue
{
public:

  bool GetBool() const;
  void SetBool(bool bValue);

  const char* GetString() const;
  void SetString(const char* szString);
  
  %extend{
    float GetNumber()
    {
      if (self->IsInt()) 
        return static_cast<float>(self->GetInt());
      if (self->GetUInt()) 
        return static_cast<float>(self->GetUInt());
      
      return self->GetNumber();
    }

	void SetNumber(float fValue)
	{
	  if (self->IsInt())
	    self->SetInt(static_cast<int>(fValue));
	  else if (self->IsUInt())
	    self->SetUInt(static_cast<unsigned int>(fValue));
	  else
	    self->SetNumber(fValue);
	}
  }

  bool IsUndefined() const;
  bool IsNull() const;
  bool IsBool() const;
  bool IsNumeric() const;
  bool IsString() const;

  float Display_GetX() const;
  float Display_GetY() const;
  float Display_GetRotation() const;
  float Display_GetXScale() const;
  float Display_GetYScale() const;
  float Display_GetAlpha() const;
  bool  Display_GetVisible() const;
  float Display_GetZ() const;
  float Display_GetXRotation() const;
  float Display_GetYRotation() const;
  float Display_GetZScale() const;
  float Display_GetFOV() const;

  void Display_SetX(float fX);
  void Display_SetY(float fY);
  void Display_SetXY(float fX, float fY);
  void Display_SetRotation(float fDeg);
  void Display_SetXScale(float fXScale);
  void Display_SetYScale(float fYScale);
  void Display_SetXYScale(float fXScale, float fYScale);
  void Display_SetAlpha(float fAlpha);
  void Display_SetVisible(bool fVisible);
  void Display_SetZ(float fZ);
  void Display_SetXRotation(float fDeg);
  void Display_SetYRotation(float fDeg);
  void Display_SetXYRotation(float fXDeg, float fYDeg);
  void Display_SetZScale(float fZScale);
  void Display_SetFOV(float fFov);

};

VSWIG_CREATE_CONCAT(VScaleformValue, 256, "%s", self->ToString().AsChar())
VSWIG_CREATE_TOSTRING(VScaleformValue, "VScaleformValue: '%s'", self->ToString().AsChar())

#else

/// \brief Wrapper class for VScaleformValue.
class VScaleformValue
{
public:

  ///
  /// @name Value Access
  /// @{
  
  /// \brief Get the boolean value of this variable.
  /// \return The boolean value (only valid if it is a boolean).
  /// \see IsBool
  boolean GetBool();

  /// \brief Set the boolean value of this variable.
  /// \param bValue The value to set.
  /// \sa IsBool
  void SetBool(boolean bValue);
  
  /// \brief Get the string value of this variable. Use tostring(var) if
  ///        you are not sure about the type or check with IsString() in advance!
  /// \return The string value (only valid if it is a string).
  /// \sa IsString
  string GetString();

  /// \brief Set the string value of this variable.
  /// \param sValue The string to set.
  /// \sa IsString
  void SetString(string sValue);
  
  /// \brief Get numeric value of this variable (check for internal type and returns int, uint or float).
  /// \return The numeric value of this variable (only valid if is a numeric value).
  /// \sa IsNumeric
  number GetNumber();

  /// \brief Set the numeric value of this variable.
  /// \param fValue The value to set.
  /// \sa IsNumeric
  void SetNumber(number fValue);

  /// @}
  /// @name Type Check
  /// @{
  
  /// \brief Check if the internal type is defined or not.
  /// \return true if the type is undefined otherwise false.
  boolean IsUndefined();
  
  /// \brief Check if the internal type is NULL/nil.
  /// \return true if the variable is NULL otherwise false.
  boolean IsNull();
  
  /// \brief Check if the internal type is a boolean.
  /// \return true if the variable contains a boolean otherwise false.
  boolean IsBool();
  
  /// \brief Check if the internal type has a numeric representation (int, uint or float).
  /// \return true if the variable contains a numeric value otherwise false.
  boolean IsNumeric();
  
  /// \brief Check if the internal type is a String (not StringW).
  /// \return true if the variable contains a string otherwise false.
  boolean IsString();

  /// @}
  /// @name Access To The Internal Display Object
  /// @{
  
  /// \brief Get the x position.
  number Display_GetX();
  /// \brief Get the y position.
  number Display_GetY();
  /// \brief Get the rotation on the XY plane.
  number Display_GetRotation();
  /// \brief Get the x scaling factor.
  number Display_GetXScale();
  /// \brief Get the y scaling factor.
  number Display_GetYScale();
  /// \brief Get alpha value.
  number Display_GetAlpha();
  /// \brief Check if it is visible.
  boolean  Display_GetVisible();
  /// \brief Get the z depth value.
  number Display_GetZ();
  /// \brief Get the rotation around the x axis.
  number Display_GetXRotation();
  /// \brief Get the rotation around the y axis.
  number Display_GetYRotation();
  /// \brief Get the z scaling value.
  number Display_GetZScale();
  /// \brief Get the current field of view.
  number Display_GetFOV();

  // \brief Set the x position.
  void Display_SetX(number x);
  // \brief Set the y position.
  void Display_SetY(number y);
  // \brief Set x and y position.
  void Display_SetXY(number x, number y);
  /// \brief Set the rotation on the XY plane.
  void Display_SetRotation(number degrees);
  /// \brief Set the x scaling factor.
  void Display_SetXScale(number scaleX);
  /// \brief Set the y scaling factor.
  void Display_SetYScale(number scaleY);
  /// \brief Set x and y scaling factor.
  void Display_SetXYScale(number scaleX, number scaleY);
  /// \brief Set alpha value.
  void Display_SetAlpha(number alpha);
  /// \brief Set visible or invisible.
  void Display_SetVisible(boolean visible);
  /// \brief Set the z depth value.
  void Display_SetZ(number z);
  /// \brief Set the rotation around the x axis.
  void Display_SetXRotation(number degX);
  /// \brief Set the rotation around the y axis.
  void Display_SetYRotation(number degY);
  /// \brief Set the rotation around the x and y axes.
  void Display_SetXYRotation(number degX, number degY);
  /// \brief Set the z scaling value.
  void Display_SetZScale(number scaleZ);
  /// \brief Set the current field of view.
  void Display_SetFOV(number fov);

  /// @}
  ///
};

#endif