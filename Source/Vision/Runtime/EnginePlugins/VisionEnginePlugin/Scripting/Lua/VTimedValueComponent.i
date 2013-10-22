/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef VLUA_APIDOC

%nodefaultctor VTimedValueComponent;
%nodefaultdtor VTimedValueComponent;

class VTimedValueComponent : public IVObjectComponent
{
public: 

  void Resume();
  void Stop();
  void Reset();
  bool IsPaused();

  void SetRange(float fFromValue, float fToValue, bool bReset = false);
  void SetDuration(float fDuration);
  float GetValue();
  float GetTime();
  inline float GetFromValue();
  inline float GetToValue();
  inline float GetDuration();

  unsigned int GetNumCallbacks();
  bool EvaluateAttachedScripts();
  void AddCallback(const char * szCallbackName, float callTime = -1);
  
  %extend{
    VSWIG_CREATE_CAST(VTimedValueComponent)
  }
};

//add lua tostring and concat operators
VSWIG_CREATE_CONCAT(VTimedValueComponent, 256, "%s [%f - %f: %f sec %s]", self->GetComponentName()==NULL?self->GetClassTypeId()->m_lpszClassName:self->GetComponentName(), self->GetFromValue(), self->GetToValue(), self->GetDuration(), self->IsPaused()?"paused":"running")
VSWIG_CREATE_TOSTRING(VTimedValueComponent, "%s: %s [%f - %f: %f sec %s]", self->GetClassTypeId()->m_lpszClassName, self->GetComponentName(), self->GetFromValue(), self->GetToValue(), self->GetDuration(), self->IsPaused()?"paused":"running")

#else

/// \brief Lua component for timed values.
/// \see VisGame_cl::CreateComponent
/// \see VisTypedEngineObject_cl::AddComponentOfType
/// \see VisTypedEngineObject_cl::GetComponentOfType
/// \see VisTypedEngineObject_cl::GetComponentOfBaseType
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
class VTimedValueComponent : public IVObjectComponent
{
public: 

  /// @}
  /// @name Timer Control
  /// @{

  /// \brief Continue from last value if paused. Always call it when created in OnAfterSceneLoaded.
  /// \note You should call this function when the component has been created
  ///   in or after the OnAfterSceneLoaded callback, otherwise the timer will not start. There is no need to call that function when created in OnCreate or OnSerialize.
  void Resume();

  /// \brief Stops (pauses) the component and resets the value to the start value.
  void Stop();

  /// \brief Pause the component, you can continue from the current value with resume.
  /// \par Example
  ///   \code
  ///     function OnAfterSceneLoaded(self)
  ///       self:AddComponentOfType("VTimedValueComponent", "Timer")
  ///       self.Timer:SetDuration(30)  -- 30 sec
  ///       self.Timer:SetRange(1,5)    -- increase from 1 to 5 over 30 sec
  ///       self.Timer:AddCallback("MyCallback", 3) -- call 'MyCallback' after 3 sec
  ///       self.Timer:Resume()
  ///     end
  ///
  ///     function OnThink(self)
  ///       ...
  ///       self.Timer:Pause()
  ///       ...
  ///     end
  ///   \endcode
  void Pause();

  /// \brief Reset the value (and timer) to the start value without pausing.
  void Reset();

  /// \brief
  ///   Returns true if the component has been paused or never has been started.
  /// \brief Pause the component, you can continue from the current value with resume.
  /// \par Example
  ///   \code
  ///     function OnAfterSceneLoaded(self)
  ///       self:AddComponentOfType("VTimedValueComponent", "Timer")
  ///       self.Timer:SetDuration(15)     -- 15 sec
  ///       self.Timer:SetRange(-10,10)    -- start from -10 until 10 (in 15 sec)
  ///       self.Timer:AddCallback("MyCallback", 3) -- call 'MyCallback' afers 3 sec
  ///     end
  ///
  ///     function OnThink(self)
  ///       ...
  ///       if self.Timer:IsPaused() then
  ///         self.Timer:Resume()
  ///       end
  ///       ...
  ///     end
  ///   \endcode
  boolean IsPaused();

  /// @}
  /// @name Timer Value And Range
  /// @{

  /// \brief Set the range for values generated during the specified duration. When the end value (fToValue) is reached, it will start over.
  /// \param fromValue The start value when the timer is 0.
  /// \param toValue The end value reached after the set duration.
  /// \param reset [\b optional] Performs a reset (timer immediatly starts with 0, even when running) if set to true.
  /// \par Example
  ///   \code
  ///     function OnAfterSceneLoaded(self)
  ///       self:AddComponentOfType("VTimedValueComponent", "Timer")
  ///       self.Timer:SetDuration(2)               -- it takes 2 sec
  ///       self.Timer:SetRange(0,1)                -- during that time the value will go from 0 to 1.
  ///       self.Timer:AddCallback("MyCallback")    -- call 'MyCallback' at the end (after 2 sec, value==1)
  ///       self.Timer:Resume()
  ///     end
  ///   \endcode
  /// \see SetDuration
  void SetRange(number fromValue, number toValue, boolean reset = false);

  /// \brief Set the time required to interpolate from the start value to the stop value. Already registered
  ///        Lua callback timestamps will be streched in proportion to the previous duration!
  ///        So it is easier to setup the callbacks after you defined the duration.
  /// \param duration The duration in seconds
  /// \par Example
  ///   \code
  ///     function OnAfterSceneLoaded(self)
  ///       self:AddComponentOfType("VTimedValueComponent", "Timer")
  ///       self.Timer:SetDuration(60)           -- it takes 60 sec
  ///       self.Timer:SetRange(0,10)            -- during that time the value will go from 0 to 10.
  ///       self.Timer:AddCallback("MyCallback") -- call 'MyCallback' at the end (after 60 sec, value==10)
  ///       self.Timer:Resume()
  ///     end
  ///   \endcode
  /// \see SetRange
  void SetDuration(number duration);

  /// \brief Get the current value.
  /// \return Returns the current value.
  /// \par Example
  ///   \code
  ///     function OnAfterSceneLoaded(self)
  ///       self:AddComponentOfType("VTimedValueComponent", "Timer")
  ///       self.Timer:SetDuration(10)           -- it will take 10 sec
  ///       self.Timer:SetRange(0,10)            -- and the value will start with 0 up to 10
  ///       self.Timer:AddCallback("MyCallback") -- call 'MyCallback' at the end (after 10 sec, value==10)
  ///       self.Timer:Resume()                  -- start now
  ///     end
  ///
  ///     function MyCallback(self)
  ///       Debug:PrintLine("Value: " .. self.Timer:GetValue())
  ///     end
  ///   \endcode
  number GetValue();

  /// \brief Get the current internal time of the component.
  /// \return Returns the current timer value.
  /// \par Example
  ///   \code
  ///     function OnAfterSceneLoaded(self)
  ///       self:AddComponentOfType("VTimedValueComponent", "Timer")
  ///       self.Timer:SetDuration(10)           -- it will take 10 sec
  ///       self.Timer:SetRange(0,20)            -- and the value will start with 0 up to 10
  ///       self.Timer:AddCallback("MyCallback") -- call 'MyCallback' at the end (after 10 sec, value==20)
  ///       self.Timer:Resume()                  -- start now
  ///     end
  ///
  ///     function MyCallback(self)
  ///       Debug:PrintLine("Called after " .. self.Timer:GetTime() .. " sec with a value of " ..self.Timer:GetValue())
  ///     end
  ///   \endcode
  number GetTime();

  /// \brief Get the start value (the value is progress from the start to the stop value).
  /// \return Returns the the start value.
  number GetFromValue();

  /// \brief Get the stop value (reached after defined duration time).
  /// \return Returns the the stop value.
  number GetToValue();

  /// \brief Get the time in sec in which the value progresses from the start (from) to the stop (to) value.
  /// \return The duration in seconds.
  number GetDuration();
  
  /// @}
  /// @name Lua Callback Handling
  /// @{
  
  /// \brief
  ///   Get the number of registered callbacks
  number GetNumCallbacks();
  
  /// \brief Trigger component to re-evaluate script components attached to the owner.
  /// \note This is only required when changing attached Lua scripts at runtime.
  /// \return Returns true when an owner with a script component has been found.
  boolean EvaluateAttachedScripts();

  /// \brief Add a Lua callback to be invoked by the timer. Note that the timer cannot guarantee to call the callback at the exact defined value (depends on the frame rate).
  /// \param callbackName The name of thee Lua callback
  /// \param callTime [\b optional] The time when this callback will be invoked. By default -1 which will set it to the end of the time span.
  /// \par Example
  ///   \code
  ///     function OnAfterSceneLoaded(self)
  ///       self:AddComponentOfType("VTimedValueComponent", "Timer")
  ///       self.Timer:SetDuration(15)                   -- it will take 15 sec
  ///       self.Timer:SetRange(-20,20)                  -- and the value will start with -20 up to 20
  ///       self.Timer:AddCallback("TimerCallback1",  1) -- call 'TimerCallback1' after 1sec
  ///       self.Timer:AddCallback("TimerCallback2",  5) -- call 'TimerCallback2' after 5sec
  ///       self.Timer:AddCallback("TimerCallback3", 15) -- call 'TimerCallback3' after 15sec
  ///       self.Timer:Resume()                          -- start now
  ///     end
  ///
  ///     function TimerCallback1(self)
  ///       Debug:PrintLine("1 - Called after " .. self.Timer:GetTime() .. " sec with a value of " ..self.Timer:GetValue())
  ///     end
  ///     function TimerCallback2(self)
  ///       Debug:PrintLine("2 - Called after " .. self.Timer:GetTime() .. " sec with a value of " ..self.Timer:GetValue())
  ///     end
  ///     function TimerCallback3(self)
  ///       Debug:PrintLine("3 - Called after " .. self.Timer:GetTime() .. " sec with a value of " ..self.Timer:GetValue())
  ///     end
  ///   \endcode
  void AddCallback(string callbackName, number callTime = -1);

};

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
