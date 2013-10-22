/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/ScaleformEnginePlugin.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/vScaleformManager.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformUtil.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/ScaleformEnginePlugin/VScaleformMovie.hpp>

#include "GFx/GFx_Player.h"

using namespace Scaleform::GFx;

//-----------------------------------------------------------------------------------
// VScaleformValueConst

VScaleformValueConst::VScaleformValueConst(const Scaleform::GFx::Value* pConstValue) 
  : m_pDisplayInfo(NULL)
  , m_sToString()
  , m_pConstValue(pConstValue)
{
  VASSERT_MSG(pConstValue!=NULL, "NULL initialization is not allowed!");
}

VScaleformValueConst::~VScaleformValueConst()
{
  delete static_cast<Value::DisplayInfo*>(m_pDisplayInfo);
  delete m_pConstValue;
}

const Value* VScaleformValueConst::GetGFxValueConst() const
{
  return m_pConstValue;
}

const VString VScaleformValueConst::ToString() const
{
  return m_sToString;
}

//-----------------------------------------------------------------------------------
// Getters

bool VScaleformValueConst::GetBool() const
{ 
  VASSERT(IsBool());
  return m_pConstValue->GetBool();
}

int VScaleformValueConst::GetInt() const
{ 
  VASSERT(IsInt());
  return m_pConstValue->GetInt();
}

unsigned int VScaleformValueConst::GetUInt() const
{
  VASSERT(IsUInt());
  return m_pConstValue->GetUInt();
}

float VScaleformValueConst::GetNumber() const
{
  VASSERT(IsNumber());
  return static_cast<float>(m_pConstValue->GetNumber());
}

const char* VScaleformValueConst::GetString() const 
{
  VASSERT(IsString());
  return m_pConstValue->GetString();
}

//-----------------------------------------------------------------------------------
// Type Checks

bool VScaleformValueConst::IsUndefined() const
{
  return m_pConstValue->IsUndefined();
}

bool VScaleformValueConst::IsNull() const
{
  return m_pConstValue->IsNull();
}

bool VScaleformValueConst::IsBool() const
{
  return m_pConstValue->IsBool();
}

bool VScaleformValueConst::IsInt() const
{
  return m_pConstValue->IsInt();
}

bool VScaleformValueConst::IsUInt() const
{
  return m_pConstValue->IsUInt();
}

bool VScaleformValueConst::IsNumber() const 
{
  return m_pConstValue->IsNumber();
}

bool VScaleformValueConst::IsNumeric() const
{
  return m_pConstValue->IsNumeric();
}

bool VScaleformValueConst::IsString() const
{
  return m_pConstValue->IsString();
}

bool VScaleformValueConst::IsDisplayObject() const
{
  return m_pConstValue->IsDisplayObject();
}

//-----------------------------------------------------------------------------------
// Display Info Getters

bool VScaleformValueConst::Display_GetVisible() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetVisible();
}

float VScaleformValueConst::Display_GetX() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetX());
}

float VScaleformValueConst::Display_GetY() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetY());
}

float VScaleformValueConst::Display_GetZ() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetZ());
}

float VScaleformValueConst::Display_GetXRotation() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetXRotation());
}

float VScaleformValueConst::Display_GetYRotation() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetYRotation());
}

float VScaleformValueConst::Display_GetRotation() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetRotation());
}

float VScaleformValueConst::Display_GetXScale() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetXScale());
}

float VScaleformValueConst::Display_GetYScale() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetYScale());
}

float VScaleformValueConst::Display_GetZScale() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetZScale());
}

float VScaleformValueConst::Display_GetAlpha() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetAlpha());
}

float VScaleformValueConst::Display_GetFOV() const
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);
  return static_cast<float>(static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->GetFOV());
}

//-----------------------------------------------------------------------------------

void VScaleformValueConst::SyncReferenceValueWithScaleform()
{
  switch(m_pConstValue->GetType())
  {
  case Value::VT_DisplayObject:
    // Ensure buffered display info object is available.
    if (m_pDisplayInfo == NULL)
      m_pDisplayInfo = new Value::DisplayInfo();

    m_pConstValue->GetDisplayInfo(static_cast<Value::DisplayInfo*>(m_pDisplayInfo));
    break;
  }

  // Save ToString conversion.
  m_sToString = m_pConstValue->ToString();
}

//-----------------------------------------------------------------------------------
// VScaleformValue

VScaleformValue::VScaleformValue(Scaleform::GFx::Value* pValue, Scaleform::GFx::Movie* pMovieInstance, const char* szVarName) 
  : VScaleformValueConst(pValue)
  , m_uiDirtyFlags(0)
  , m_pMovieInstance(pMovieInstance)
  , m_sVarName(szVarName)
  , m_pValue(pValue)
{
}

VScaleformValue::~VScaleformValue()
{
}

Value* VScaleformValue::GetGFxValue() const
{
  return m_pValue;
}

//-----------------------------------------------------------------------------------
// Setters

void VScaleformValue::SetNull()
{
  m_uiDirtyFlags = DIRTY_VALUE;
  m_pValue->SetNull();
}

void VScaleformValue::SetBool(bool bValue)
{
  m_uiDirtyFlags = DIRTY_VALUE;
  m_pValue->SetBoolean(bValue);
}

void VScaleformValue::SetInt(int iValue)
{
  m_uiDirtyFlags = DIRTY_VALUE;
  m_pValue->SetInt(iValue);
}

void VScaleformValue::SetUInt(unsigned int uiValue)
{
  m_uiDirtyFlags = DIRTY_VALUE;
  m_pValue->SetUInt(uiValue);
}

void VScaleformValue::SetNumber(float fValue)
{
  m_uiDirtyFlags = DIRTY_VALUE;
  m_pValue->SetNumber(fValue);
}

void VScaleformValue::SetString(const char* szString)
{
  m_uiDirtyFlags = DIRTY_VALUE;
  
  // Tell the movie to allocate the data for the string.
  m_pMovieInstance->CreateString(m_pValue, szString);
}

//-----------------------------------------------------------------------------------
// Display Info Setters

void VScaleformValue::Display_SetVisible(bool bVisible)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_VISIBLE;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetVisible(bVisible);
}

void VScaleformValue::Display_SetX(float fX)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_POS_X;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetX(fX);
}

void VScaleformValue::Display_SetY(float fY)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_POS_Y;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetY(fY);
}

void VScaleformValue::Display_SetXY(float fX, float fY)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_POS_X | DIRTY_DISPLAY_POS_Y;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetX(fX);
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetY(fY);
}

void VScaleformValue::Display_SetZ(float fZ)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_POS_Z;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetZ(fZ);
}

void VScaleformValue::Display_SetXRotation(float fDeg)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_ROT_X;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetXRotation(fDeg);
}

void VScaleformValue::Display_SetYRotation(float fDeg)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_ROT_Y;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetYRotation(fDeg);
}

void VScaleformValue::Display_SetXYRotation(float fXDeg, float fYDeg)
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_ROT_X | DIRTY_DISPLAY_ROT_Y;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetXRotation(fXDeg);
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetYRotation(fYDeg);
}

void VScaleformValue::Display_SetRotation(float fDeg)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_ROT;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetRotation(fDeg);
}

void VScaleformValue::Display_SetXScale(float fXScale)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_SCALE_X;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetXScale(fXScale);
}

void VScaleformValue::Display_SetYScale(float fYScale)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_SCALE_Y;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetYScale(fYScale);
}

void VScaleformValue::Display_SetXYScale(float fXScale, float fYScale)
{
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_SCALE_X | DIRTY_DISPLAY_SCALE_Y;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetXScale(fXScale);
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetYScale(fYScale);
}

void VScaleformValue::Display_SetZScale(float fZScale)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_SCALE_Z;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetZScale(fZScale);
}

void VScaleformValue::Display_SetAlpha(float fAlpha)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_ALPHA;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetAlpha(fAlpha);
}

void VScaleformValue::Display_SetFOV(float fFov)
{ 
  VASSERT(IsDisplayObject() && m_pDisplayInfo != NULL);

  m_uiDirtyFlags |= DIRTY_DISPLAY_FOV;
  static_cast<Value::DisplayInfo*>(m_pDisplayInfo)->SetFOV(fFov);
}

//-----------------------------------------------------------------------------------

void VScaleformValue::SyncValueWithScaleform()
{
  if (m_uiDirtyFlags != 0)
  {
    // POD typed values need to be re-applied to the scaleform movie.
    if ((m_uiDirtyFlags & DIRTY_VALUE) != 0)
    {
      m_pMovieInstance->SetVariable(m_sVarName, *m_pValue);
    }
    // Buffered values. These are references.
    else
    {
      // Only display object info supported for now.
      VASSERT(m_pValue->GetType() == Value::VT_DisplayObject);

      // First update values changed from the engine side
      Value::DisplayInfo displayInfo;
      m_pValue->GetDisplayInfo(&displayInfo);

      Value::DisplayInfo *pBufferedDisplayInfo = static_cast<Value::DisplayInfo*>(m_pDisplayInfo);

      if ((m_uiDirtyFlags & DIRTY_DISPLAY_VISIBLE) != 0)
        displayInfo.SetVisible(pBufferedDisplayInfo->GetVisible());

      if ((m_uiDirtyFlags & DIRTY_DISPLAY_POS_X) != 0)
        displayInfo.SetX(pBufferedDisplayInfo->GetX());
      if ((m_uiDirtyFlags & DIRTY_DISPLAY_POS_Y) != 0)
        displayInfo.SetY(pBufferedDisplayInfo->GetY());
      if ((m_uiDirtyFlags & DIRTY_DISPLAY_POS_Z) != 0)
        displayInfo.SetZ(pBufferedDisplayInfo->GetZ());

      if ((m_uiDirtyFlags & DIRTY_DISPLAY_ROT_X) != 0)
        displayInfo.SetXRotation(pBufferedDisplayInfo->GetXRotation());
      if ((m_uiDirtyFlags & DIRTY_DISPLAY_ROT_Y) != 0)
        displayInfo.SetYRotation(pBufferedDisplayInfo->GetYRotation());
      if ((m_uiDirtyFlags & DIRTY_DISPLAY_ROT) != 0)
        displayInfo.SetRotation(pBufferedDisplayInfo->GetRotation());

      if ((m_uiDirtyFlags & DIRTY_DISPLAY_SCALE_X) != 0)
        displayInfo.SetXScale(pBufferedDisplayInfo->GetXScale());
      if ((m_uiDirtyFlags & DIRTY_DISPLAY_SCALE_Y) != 0)
        displayInfo.SetYScale(pBufferedDisplayInfo->GetYScale());
      if ((m_uiDirtyFlags & DIRTY_DISPLAY_SCALE_Z) != 0)
        displayInfo.SetZScale(pBufferedDisplayInfo->GetZScale());

      if ((m_uiDirtyFlags & DIRTY_DISPLAY_ALPHA) != 0)
        displayInfo.SetAlpha(pBufferedDisplayInfo->GetAlpha());
      if ((m_uiDirtyFlags & DIRTY_DISPLAY_FOV) != 0)
        displayInfo.SetFOV(pBufferedDisplayInfo->GetFOV());

      m_pValue->SetDisplayInfo(displayInfo);
      *pBufferedDisplayInfo = displayInfo;
    }
    
    m_uiDirtyFlags = 0;

    // Save ToString conversion.
    m_sToString = m_pValue->ToString();
  }
  else
  {
    switch(m_pValue->GetType())
    {
    case Value::VT_Boolean:
    case Value::VT_Int:
    case Value::VT_UInt:
    case Value::VT_Number:
    case Value::VT_String:
      // Re-get value for pod types.
      m_pMovieInstance->GetVariable(m_pValue, m_sVarName);
      m_sToString = m_pValue->ToString();
      break;

    case Value::VT_DisplayObject:
      VScaleformValueConst::SyncReferenceValueWithScaleform();
      break;
    }
  }
}

//-----------------------------------------------------------------------------------

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
