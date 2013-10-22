/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef __VTIMED_VALUE_COMPONENT_HPP
#define __VTIMED_VALUE_COMPONENT_HPP
#ifndef _VISION_DOC
//since this component is used only internally
//for the scripting system we do not generate a C++ documentation for it

#include <Vision/Runtime/Engine/SceneElements/VisApiObjectComponent.hpp>

/// \brief Lua component for timed values. Not intended to be used in C++.
/// \par Example
///   \code
///     function OnAfterSceneLoaded(self)
///       self:AddComponentOfType("VTimedValueComponent", "Timer")
///       self.Timer:SetDuration(10)              -- it will take 10 sec
///       self.Timer:SetRange(0,10)               -- and the value will start with 0 up to 10 during that time (=sec in this case)
///       self.Timer:AddCallback("MyCallback", 3) -- call 'MyCallback' after 3 sec
///       self.Timer:Resume()                     -- start now
///     end
///
///     function MyCallback(self)
///       Debug:PrintLine("MyCallback has been called when the value reached " .. self.Timer:GetValue())
///     end
///   \endcode
class VTimedValueComponent : public IVObjectComponent, public IVisCallbackHandler_cl
{
public: 

  ///
  /// @name Constructor
  /// @{
  ///

  ///
  /// \brief
  ///   Component Constructor
  /// 
  /// \param iComponentFlags
  ///   See IVObjectComponent
  /// 
  /// \see
  ///   IVObjectComponent
  /// 
  SCRIPT_IMPEXP VTimedValueComponent(int iComponentFlags=VIS_OBJECTCOMPONENTFLAG_NOSERIALIZE);

  /// 
  /// \brief
  ///   Component Destructor
  /// 
  SCRIPT_IMPEXP virtual ~VTimedValueComponent();

  /// @}
  /// @name Timer Control
  /// @{

  /// \brief
  ///   Continue from last value if paused.
  SCRIPT_IMPEXP void Resume();

  /// \brief Stops (pause) the component and resets the value to the start value.
  SCRIPT_IMPEXP void Stop();

  /// \brief Pause the component, you can continue from the current value with resume.
  SCRIPT_IMPEXP void Pause();

  /// \brief Reset the value to the start value without pausing.
  SCRIPT_IMPEXP void Reset();

  /// \brief
  ///   Returns true if the component has been paused or never has been started.
  SCRIPT_IMPEXP inline bool IsPaused()
  { 
    return m_bPaused;
  }

  /// @}
  /// @name Timer Value And Range
  /// @{

  /// \brief Set the value range perambulated in the specified duration. When the end value (fToValue) is reached, it will start over.
  ///        Registered Lua callbacks will adjust accordingly.
  SCRIPT_IMPEXP void SetRange(float fFromValue, float fToValue, bool bReset = false);

  /// \brief Get the start value
  inline float GetFromValue()
  { 
    return m_fFromValue;
  };

  /// \brief Get the timer internal current time.
  inline float GetTime()
  {
    return m_fCurrentTime;
  };

  /// \brief Get the stop value (reached after defined duration time).
  inline float GetToValue()
  { 
    return m_fToValue;
  };

  /// \brief Get the time in sec in which the value progresses from the start (from) to the stop (to) value.
  inline float GetDuration()
  {
    return m_fDuration;
  };

  /// \brief Set the time required to interpolate from the start value to the stop value. Registered Lua callbacks will adjust accordingly.
  SCRIPT_IMPEXP void SetDuration(float fDuration);

  /// \brief Get the current value of the timer (if the duration is as long as the defined value range the value represents the time).
  SCRIPT_IMPEXP float GetValue();

  /// @}
  /// @name Lua Callback Handling
  /// @{
  
  /// \brief
  ///   Get the number of registered callbacks
  SCRIPT_IMPEXP unsigned int GetNumCallbacks();

  /// \brief Trigger component to re-evaluate the owners attached scripts (so that all scripts with matching callbacks get invoked)
  /// \param bUseCachingHeuristic Use heuristic to evaluate if all attached script components are know by this component (will memorize the number of attached scripts)
  /// \return Returns true when all attached scripts evaluated successfully, otherwise false.
  SCRIPT_IMPEXP bool EvaluateAttachedScripts(bool bUseCachingHeuristic = false);

  /// \brief
  ///   Add a Lua callback to be invoked by the timer
  /// \param szCallbackName The name of thee Lua callback
  /// \param [\b optional] fCallTime The time when this callback will be invoke.
  ///                                Invoked at the end of the time span if not explicitly set.
  SCRIPT_IMPEXP void AddCallback(const char * szCallbackName, float fCallTime = -1);

  /// @}
  /// @name IVObjectComponent Overrides
  /// @{

  /// \brief
  ///   RTTI macro
  V_DECLARE_SERIAL(VTimedValueComponent, SCRIPT_IMPEXP)

  /// \brief
  ///   RTTI macro to add a variable table
  V_DECLARE_VARTABLE(VTimedValueComponent, SCRIPT_IMPEXP)

  /// \brief Serialization
  SCRIPT_IMPEXP virtual void Serialize( VArchive &ar ) HKV_OVERRIDE;

  /// \brief Custom callback handling
  virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

  ///
  /// @}
  ///

protected:

  void Init();
  void DeInit();

  float m_fFromValue;
  float m_fToValue;
  float m_fCurrentTime;
  float m_fCurrentTimeFrom0To1;
  float m_fDuration;
  bool m_bPaused;
  bool m_bSceneRunning;
  int m_iCachedComponentCount;

  class VTimedLuaCallback;
  DynObjArray_cl<VTimedLuaCallback*> m_luaCallbacks;
  DynObjArray_cl<VScriptComponent*> m_parentScripts;
};

#endif // _VISION_DOC
#endif // __VTIMED_VALUE_COMPONENT_HPP

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
