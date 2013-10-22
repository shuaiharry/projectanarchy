/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef __VSCALEFORM_UTIL_HPP
#define __VSCALEFORM_UTIL_HPP

class VScaleformMovieInstance;

namespace Scaleform
{
  namespace GFx
  {
    class Value;
    class Movie;
  }
}

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformManager.hpp>

/// \brief  Wrapper class for const Scaleform::GFx::Value
/// 				(Get and Set functions work according to the original Scaleform implementation)
/// \sa     VScaleformValue
class VScaleformValueConst
{
  friend class VScaleformMovieInstance;
  friend class VOnExternalInterfaceCall;

  // Non-copyable
  VScaleformValueConst(const VScaleformValueConst&);
  VScaleformValueConst& operator=(const VScaleformValueConst&);

protected:
  /// \brief Constructor - Create a new wrapper of a const Scaleform::GFx::Value. See VScaleformValue for non-const.
  /// \param pConstValue An instance of a const Scaleform::GFx::Value * which is going to be wrapped.
  /// \sa VScaleformValue
  SCALEFORM_IMPEXP VScaleformValueConst(const Scaleform::GFx::Value* pConstValue);

public:
  /// \brief Destructor.
  SCALEFORM_IMPEXP virtual ~VScaleformValueConst();

  /// \brief Get the wrapped object.
  SCALEFORM_IMPEXP const Scaleform::GFx::Value* GetGFxValueConst() const;

  SCALEFORM_IMPEXP const VString ToString() const;

  // Getters
  SCALEFORM_IMPEXP bool         GetBool() const;
  SCALEFORM_IMPEXP int          GetInt() const;
  SCALEFORM_IMPEXP unsigned int GetUInt() const;  
  SCALEFORM_IMPEXP const char*  GetString() const;
  SCALEFORM_IMPEXP float        GetNumber() const;

  // Types
  SCALEFORM_IMPEXP bool IsUndefined() const;
  SCALEFORM_IMPEXP bool IsNull() const;
  SCALEFORM_IMPEXP bool IsBool() const;
  SCALEFORM_IMPEXP bool IsInt() const;
  SCALEFORM_IMPEXP bool IsUInt() const;
  SCALEFORM_IMPEXP bool IsNumber() const;
  SCALEFORM_IMPEXP bool IsNumeric() const;
  SCALEFORM_IMPEXP bool IsString() const;
  SCALEFORM_IMPEXP bool IsDisplayObject() const;

  // Display Info Getters
  SCALEFORM_IMPEXP bool  Display_GetVisible() const;
  SCALEFORM_IMPEXP float Display_GetX() const;
  SCALEFORM_IMPEXP float Display_GetY() const;
  SCALEFORM_IMPEXP float Display_GetZ() const;
  SCALEFORM_IMPEXP float Display_GetXRotation() const;
  SCALEFORM_IMPEXP float Display_GetYRotation() const;
  SCALEFORM_IMPEXP float Display_GetRotation() const;
  SCALEFORM_IMPEXP float Display_GetXScale() const;
  SCALEFORM_IMPEXP float Display_GetYScale() const;
  SCALEFORM_IMPEXP float Display_GetZScale() const;
  SCALEFORM_IMPEXP float Display_GetAlpha() const;
  SCALEFORM_IMPEXP float Display_GetFOV() const;

protected:
  // Called by the movie instance object from the capture thread.
  void SyncReferenceValueWithScaleform();

  // Buffered values for main thread access.
  void *m_pDisplayInfo; ///< display info objects.

  VString m_sToString; ///< Keeps the string that is returned when calling ToString.

private:
  const Scaleform::GFx::Value* m_pConstValue;
};

/// \brief  Wrapper class for Scaleform::GFx::Value (see VScaleformValueConst for const values)
/// 				(Get and Set functions work according to the original Scaleform implementation)
/// \note		Instance created via VScaleformMovieInstance::GetVariable();
/// \sa     VScaleformMovieInstance::GetVariable()
/// \sa     VScaleformValueConst
class VScaleformValue : public VScaleformValueConst
{
  friend class VScaleformMovieInstance;

  // Non-copyable
  VScaleformValue(const VScaleformValue&);
  VScaleformValue& operator=(const VScaleformValue&);

private:
  /// \brief Constructor - Create a new wrapper of a Scaleform::GFx::Value.
  /// \param pValue The Scaleform variable to wrap.
  /// \param bDestroyOnDelete (\b optional) Delete the internal pointer when deleting this object.
  /// \sa GetVariable
  /// \sa VScaleformValueConst
  SCALEFORM_IMPEXP VScaleformValue(Scaleform::GFx::Value* pValue,
    Scaleform::GFx::Movie* pMovieInstance, const char* szVarName);

public:
  /// \brief Destructor.
  SCALEFORM_IMPEXP virtual ~VScaleformValue();

  /// \brief Get the wrapped object.
  SCALEFORM_IMPEXP Scaleform::GFx::Value* GetGFxValue() const;

  // Setters
  SCALEFORM_IMPEXP void SetNull();
  SCALEFORM_IMPEXP void SetBool(bool bValue);
  SCALEFORM_IMPEXP void SetInt(int iValue);
  SCALEFORM_IMPEXP void SetUInt(unsigned int uiValue);  
  SCALEFORM_IMPEXP void SetNumber(float fValue);
  SCALEFORM_IMPEXP void SetString(const char* szString);

  // Display Info Setters
  SCALEFORM_IMPEXP void Display_SetVisible(bool fVisible);
  SCALEFORM_IMPEXP void Display_SetX(float fX);
  SCALEFORM_IMPEXP void Display_SetY(float fY);
  SCALEFORM_IMPEXP void Display_SetXY(float fX, float fY);
  SCALEFORM_IMPEXP void Display_SetZ(float fZ);
  SCALEFORM_IMPEXP void Display_SetXRotation(float fDeg);
  SCALEFORM_IMPEXP void Display_SetYRotation(float fDeg);
  SCALEFORM_IMPEXP void Display_SetXYRotation(float fXDeg, float fYDeg);
  SCALEFORM_IMPEXP void Display_SetRotation(float fDeg);
  SCALEFORM_IMPEXP void Display_SetXScale(float fXScale);
  SCALEFORM_IMPEXP void Display_SetYScale(float fYScale);
  SCALEFORM_IMPEXP void Display_SetXYScale(float fXScale, float fYScale);
  SCALEFORM_IMPEXP void Display_SetZScale(float fZScale);
  SCALEFORM_IMPEXP void Display_SetAlpha(float fAlpha);
  SCALEFORM_IMPEXP void Display_SetFOV(float fFov);

protected:
  // Called by the movie instance object from the capture thread.
  void SyncValueWithScaleform();

private:
  // Maintain dirty state to be able to sync with the scaleform movie.
  enum DirtyFlags
  {
    DIRTY_VALUE           = V_BIT(1),

    // DisplayInfo specific flags
    DIRTY_DISPLAY_VISIBLE = V_BIT(2),
    DIRTY_DISPLAY_POS_X   = V_BIT(3),
    DIRTY_DISPLAY_POS_Y   = V_BIT(4),
    DIRTY_DISPLAY_POS_Z   = V_BIT(5),
    DIRTY_DISPLAY_ROT_X   = V_BIT(6),
    DIRTY_DISPLAY_ROT_Y   = V_BIT(7),
    DIRTY_DISPLAY_ROT     = V_BIT(8),
    DIRTY_DISPLAY_SCALE_X = V_BIT(9),
    DIRTY_DISPLAY_SCALE_Y = V_BIT(10),
    DIRTY_DISPLAY_SCALE_Z = V_BIT(11),
    DIRTY_DISPLAY_ALPHA   = V_BIT(12),
    DIRTY_DISPLAY_FOV     = V_BIT(13)
  };
  unsigned int m_uiDirtyFlags;

  Scaleform::GFx::Movie* m_pMovieInstance;
  VString m_sVarName;

  Scaleform::GFx::Value* m_pValue;
};

#endif // __VSCALEFORM_UTIL_HPP

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
