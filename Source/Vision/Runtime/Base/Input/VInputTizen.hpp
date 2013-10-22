/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VInputTizen.hpp

#ifndef VINPUTTIZEN_HPP_INCLUDED
#define VINPUTTIZEN_HPP_INCLUDED

#include <FUix.h>

class VInputManagerTizen;
typedef VInputManagerTizen VInputManager;

// Compatibility typedefs
HKV_DEPRECATED_2012_2 typedef VInputManagerTizen VInputDeviceManager;
HKV_DEPRECATED_2012_2 typedef VInputManagerTizen VInputDeviceManagerTizen;

#define V_TIZEN_MAX_TOUCH_POINTS 10
#define V_TIZEN_TOUCH_UPDATE_QUEUE_SIZE 16


/// \brief
///   Enumeration for the model of the InputDevice, see IVInputDevice::GetModel
enum VInputDeviceModel 
{
  //name ::= "INPUT_DEVICE_" platform "_" device
  INPUT_DEVICE_NONE,

  INPUT_DEVICE_EMULATED_TOUCHSCREEN, ///< emulated touchscreen by vRemoteInput plugin
  INPUT_DEVICE_EMULATED_MOTIONSENSOR, ///< emulated motion sensor by vRemoteInput plugin
  INPUT_DEVICE_EMULATED_JOYSTICK,     ///< emulated joystick

  INPUT_DEVICE_TIZEN_TOUCHSCREEN,
  INPUT_DEVICE_TIZEN_MOTIONSENSOR,
  INPUT_DEVICE_TIZEN_KEY_INPUT
};

// name convention:
// ================
// name       ::= "CT_" device [ "_" platform ] "_" control
// 
// platform   ::= ( "PC" | "PS3" | "PSP2" | "WIIU" | "ANDROID" | "TIZEN" )
// device     ::= ( "KB" | "PAD" | "POINTER" | "MOUSE" | "BALANCEBOARD" | "NUNCHUCK" | "MOTIONPLUS" | "MOTION" | "TOUCH" | "JOYSTICK" )
// control    ::= [ "LEFT_" | "RIGHT_" | "MIDDLE_" ] identifier [ "_DELTA" ] [ "_X" | "_Y" | "_Z" ] [ "_NEG" | "_POS" ]
// identifier ::= { "A".."Z" | "0".."9" | "_" }*
/// \brief Platform specific input control (buttons, keys, touch points, etc) names and values.
enum VInputControlTizen
{
  CT_KB_TIZEN_MENU = CT_PLATFORM_FIRST,
  CT_KB_TIZEN_BACK,

  CT_PLATFORM_LENGTH
};

/// \class  VMotionInputTizen
///
/// \brief  Input device handling motion input by the user. 
///
class VMotionInputTizen : public IVInputDevice, public Tizen::Uix::Sensor::ISensorEventListener
{
  public:

    VMotionInputTizen();

    ~VMotionInputTizen();

    /// \brief
    ///   See IVInputDevice::Reset().
    VOVERRIDE void Reset();

    /// \brief
    ///   See IVInputDevice::Update(float fTimeDiff = 0.0f).
    VOVERRIDE void Update(float fTimeDiff = 0.0f);

    /// \brief
    ///   See IVInputDevice::GetControlName(unsigned int device).
    HKV_DEPRECATED_2012_2 virtual const char* GetControlName(unsigned int control) HKV_OVERRIDE;

    /// \fn void SetEnabled(bool bEnabled)
    ///
    /// \brief  Sets the status of the multitouch input. Since Tizen devices are mobile devices certain kinds of input (motion sensor, touch screen) can be activated as needed to conserve memory.
    ///
    /// \param  bEnabled  true to enable, false to disable. 
    ///
    VBASE_IMPEXP void SetEnabled(bool bEnabled);

    /// \fn VBASE_IMPEXP bool IsEnabled()
    ///
    /// \brief  Query if this multitouch input is enabled. 
    ///
    /// \return true if enabled, false if not. 
    ///
    VBASE_IMPEXP bool IsEnabled()
    {
      return m_bEnabled;
    }

    /// \brief
    ///   See IVInputDevice::GetControlValue.
    /// 
    /// \param control
    ///   Integer value of the requested control. The following controls are supported: CT_MOTION_ACCELERATION_X/Y/Z.
    ///
    /// \param fDeadZone
    ///   Within the dead zone the return value is always 0.
    ///
    /// \param bTimeScaled
    ///   If true the control value will be scaled with the time difference from the previous frame.
    ///
    VBASE_IMPEXP virtual float GetControlValue(unsigned int control, float fDeadZone, bool bTimeScaled = false) HKV_OVERRIDE;


    /// \brief
    ///   See IVInputDevice::GetRawControlValue(unsigned int control).
    /// 
    /// \param control
    ///   Integer value of the requested control. This implementation doesn't support any raw control values.
    virtual int GetRawControlValue(unsigned int control) HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetModel().
    virtual int GetModel() HKV_OVERRIDE;
    
    /// \brief
    ///   See IVInputDevice::GetName().
    virtual const char* GetName() HKV_OVERRIDE;

    /// \fn hkvVec3 GetAcceleration()
    ///
    /// \brief  Gets the acceleration. 
    ///
    /// \return The acceleration. 
    ///
    hkvVec3 GetAcceleration();
  
  
    VBASE_IMPEXP void SetAcceleration(float fAccelerationX, float fAccelerationY, float fAccelerationZ);

    /// \brief
    ///   Sensor data callback.
    virtual void OnDataReceived(Tizen::Uix::Sensor::SensorType sensorType, Tizen::Uix::Sensor::SensorData& sensorData, unsigned long result);
    
  private:

    float m_fAccelerationX, m_fAccelerationY, m_fAccelerationZ;
    float m_fLastAccelerationX, m_fLastAccelerationY, m_fLastAccelerationZ;
  
    float m_fTimeDiff;

    bool m_bEnabled;
};

/// \class  VMultiTouchInputTizen
///
/// \brief  Multi touch device for Tizen. 
///
class VMultiTouchInputTizen : public IVMultiTouchInput, public Tizen::Ui::ITouchEventListener
{
  public:

    /// \fn VMultiTouchInputTizen()
    ///
    /// \brief  Constructor. 
    ///
    VMultiTouchInputTizen();

    /// \fn ~VMultiTouchInputTizen()
    ///
    /// \brief  Destructor. 
    ///
    ~VMultiTouchInputTizen();

    /// \brief
    ///   See IVInputDevice::Reset().
    virtual void Reset() HKV_OVERRIDE;


    /// \brief
    ///   See IVInputDevice::Update(float fTimeDiff = 0.0f).
    virtual void Update(float fTimeDiff = 0.0f) HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetControlName(unsigned int device).
    virtual const char* GetControlName(unsigned int control) HKV_OVERRIDE;
    
    /// \brief
    ////  See IVMultiTouchInput::GetTouchPointValue(int index, unsigned int control, bool bTimeScaled).
    virtual float GetTouchPointValue(int index, unsigned int control, bool bTimeScaled = false) HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetControlValue.
    /// 
    /// \param control
    ///   Integer value of the requested control. The following controls are supported: CT_TOUCH_ANY, CT_TOUCH_POINT_n_X/Y for n in [0, 9]
    ///
    /// \param fDeadZone
    ///   Within the dead zone the return value is always 0.
    ///
    /// \param bTimeScaled
    ///   If true the control value will be scaled with the time difference from the previous frame.
    ///
    VBASE_IMPEXP virtual float GetControlValue(unsigned int control, float fDeadZone, bool bTimeScaled = false) HKV_OVERRIDE;


    /// \brief
    ///   See IVInputDevice::GetRawControlValue(unsigned int control).
    /// 
    /// \param control
    ///   Integer value of the requested control. The following controls are supported: CT_TOUCH_ANY, CT_TOUCH_POINT_n_X/Y for n in [0, 9]
    virtual int GetRawControlValue(unsigned int control) HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetModel().
    virtual int GetModel() HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetName().
    virtual const char* GetName() HKV_OVERRIDE;

    /// \fn VBASE_IMPEXP virtual int GetMaximumNumberOfTouchPoints()
    ///
    /// \brief  Gets the maximum number of touch points. 
    ///
    /// \return The maximum number of touch points. 
    ///
    VBASE_IMPEXP virtual int GetMaximumNumberOfTouchPoints() HKV_OVERRIDE
    {
      return V_TIZEN_MAX_TOUCH_POINTS;
    }

    /// \fn void SetEnabled(bool bEnabled)
    ///
    /// \brief  Sets the status of the multitouch input.
    ///
    /// \param  bEnabled  true to enable, false to disable. 
    ///
    VBASE_IMPEXP void SetEnabled(bool bEnabled);

    /// \fn VBASE_IMPEXP bool IsEnabled()
    ///
    /// \brief  Query if this multitouch input is enabled. 
    ///
    /// \return true if enabled, false if not. 
    ///
    VBASE_IMPEXP bool IsEnabled()
    {
      return m_bEnabled;
    }

    #ifndef _VISION_DOC
    
    VBASE_IMPEXP void AddNewTouch(int iTouchId, int iXAbsolute, int iYAbsolute);
  
    VBASE_IMPEXP void UpdateTouch(int iTouchId, int iXAbsolute, int iYAbsolute);
  
    VBASE_IMPEXP void RemoveTouch(int iTouchId);
    
    #endif
  
    VBASE_IMPEXP virtual const VTouchPoint& GetTouch(unsigned int uiIndex) const HKV_OVERRIDE;
  
    VBASE_IMPEXP virtual bool IsActiveTouch(unsigned int uiIndex) const HKV_OVERRIDE;

    /// \brief
    ///   Called when an entity is touch moved.
    virtual void OnTouchMoved(const Tizen::Ui::Control& source, const Tizen::Graphics::Point& currentPosition, const Tizen::Ui::TouchEventInfo& touchInfo) HKV_OVERRIDE;

    /// \brief
    ///   Called when an entity is touch pressed.
    virtual void OnTouchPressed(const Tizen::Ui::Control& source, const Tizen::Graphics::Point& currentPosition, const Tizen::Ui::TouchEventInfo& touchInfo) HKV_OVERRIDE;

    /// \brief
    ///   Called when an entity is touch released.
    virtual void OnTouchReleased(const Tizen::Ui::Control& source, const Tizen::Graphics::Point& currentPosition, const Tizen::Ui::TouchEventInfo& touchInfo) HKV_OVERRIDE;

    /// \brief
    ///   Called when the touch is canceled.
    virtual void OnTouchCanceled(const Tizen::Ui::Control& source, const Tizen::Graphics::Point& currentPosition, const Tizen::Ui::TouchEventInfo& touchInfo) HKV_OVERRIDE;

    virtual void OnTouchDoublePressed(const Tizen::Ui::Control& source, const Tizen::Graphics::Point& currentPosition, const Tizen::Ui::TouchEventInfo& touchInfo) HKV_OVERRIDE {}
    virtual void OnTouchFocusIn(const Tizen::Ui::Control& source, const Tizen::Graphics::Point& currentPosition, const Tizen::Ui::TouchEventInfo& touchInfo) HKV_OVERRIDE {}
    virtual void OnTouchFocusOut(const Tizen::Ui::Control& source, const Tizen::Graphics::Point& currentPosition, const Tizen::Ui::TouchEventInfo& touchInfo) HKV_OVERRIDE {}
    virtual void OnTouchLongPressed (const Tizen::Ui::Control& source, const Tizen::Graphics::Point& currentPosition, const Tizen::Ui::TouchEventInfo& touchInfo) HKV_OVERRIDE {}
  
  private:

	/// \brief Internally used struct to keep track of asynchronous touch events
    struct VTizenMTUItem
    {
      float fX;
      float fY;
      int iID;
    };

    float m_fTimeDiff;

    float m_fInvX;
    float m_fInvY;

    int m_iXRatio;
    int m_iYRatio;
  
    VTouchPoint m_TouchPoints[V_TIZEN_MAX_TOUCH_POINTS];

    VTizenMTUItem m_TouchUpdateQueue[V_TIZEN_TOUCH_UPDATE_QUEUE_SIZE];

    unsigned int m_uiTouchUpdateQueueLength;

    VMutex m_TouchUpdateQueueMutex;
 
    bool m_bEnabled;
};

/// \class  VKeyInputTizen
///
/// \brief  Input device handling the input via hardware buttons on the Tizen device.
///
class VKeyInputTizen : public IVInputDevice, public Tizen::Ui::IKeyEventListener
{
  public:
  
    VKeyInputTizen();
    
    virtual ~VKeyInputTizen();
    
    /// \brief
    ///   See IVInputDevice::Reset().
    virtual void Reset() HKV_OVERRIDE;


    /// \brief
    ///   See IVInputDevice::Update(float fTimeDiff = 0.0f).
    virtual void Update(float fTimeDiff = 0.0f) HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetControlName(unsigned int device).
    virtual const char* GetControlName(unsigned int control) HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetControlValue.
    /// 
    /// \param control
    ///   Integer value of the requested control. The following controls are supported: CT_KB_TIZEN_MENU, CT_KB_TIZEN_BACK.
    ///
    /// \param fDeadZone
    ///   Within the dead zone the return value is always 0.
    ///
    /// \param bTimeScaled
    ///   If true, scales the difference from the dead zone by the frame time.
    ///
    VBASE_IMPEXP virtual float GetControlValue(unsigned int control, float fDeadZone, bool bTimeScaled = false) HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetRawControlValue(unsigned int control).
    /// 
    /// \param control
    ///   Integer value of the requested control. The following controls are supported: CT_KB_TIZEN_MENU, CT_KB_TIZEN_BACK.
    virtual int GetRawControlValue(unsigned int control) HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetModel().
    virtual int GetModel() HKV_OVERRIDE;

    /// \brief
    ///   See IVInputDevice::GetName().
    virtual const char* GetName() HKV_OVERRIDE;
    
    /// \brief
    /// Called when a key is pressed and held down for some time.
    virtual void OnKeyLongPressed( const Tizen::Ui::Control& source, Tizen::Ui::KeyCode keyCode ) HKV_OVERRIDE;

    /// \brief
    /// Called when a key is pressed.
    virtual void OnKeyPressed( const Tizen::Ui::Control& source, Tizen::Ui::KeyCode keyCode ) HKV_OVERRIDE;

    /// \brief
    /// Called when a key is release.
    virtual void OnKeyReleased( const Tizen::Ui::Control& source, Tizen::Ui::KeyCode keyCode ) HKV_OVERRIDE;

  private:
  
    unsigned int m_uiKeyState;
    
};

/// \class  VInputManagerTizen
///
/// \brief  The input device manager for Tizen. 
///
class VInputManagerTizen : public VInputManagerBase
{
  public:  
    /// \brief
    ///   Initialize the Tizen device manager.
    static void Init();


    /// \brief
    ///   De-initialize the Tizen device manager.
    static void DeInit();


    /// \brief
    ///   Get the initialization status of the Tizen device manager.
    static bool IsInitialized();

    static void Update(float timeDiff = 0.0f);

    /// \brief
    ///   Gets an input device of the specified type.
    /// 
    /// \param type
    ///   The type of the demanded input device.
    /// 
    /// \return
    ///   IVInputDevice& device: Reference to the requested input device. If the requested type was
    ///   not available the method returns a reference to the field s_NoInputDevice of
    ///   VInputManagerTizen
    static IVInputDevice& GetInputDevice(VInputDeviceType type);
	
	/// \brief
    ///   Gets an input device of the specified type.
    /// 
    /// \param szDeviceName
    ///   The name of the demanded input device, which can be "touch" (or "touchscreen"), "pad" and "motion" (or "motionsensor");
    /// 
    /// \return
    ///   IVInputDevice &device: Reference to the requested input device. If the requested type was
    ///   not available the method returns a reference to VInputManagerTizen::s_NoInputDevice.
    static IVInputDevice& GetInputDevice(const char * szDeviceName);
    
    static VMultiTouchInputTizen& GetTouchScreen();
  
    static VMotionInputTizen& GetMotionSensor();
    
    static VKeyInputTizen& GetKeyInput();

    static IVSoftKeyboardAdapter& GetSoftkeyboardAdapter();

    static IVInputDevice s_NoInputDevice;
	
    static int s_iFrameCounter;

  private:

    static VMultiTouchInputTizen* s_pMultiTouchInput;

    static VMotionInputTizen* s_pMotionInput;
    
    static VKeyInputTizen* s_pKeyInput;

    static Tizen::Uix::Sensor::SensorManager* s_pSensorManager;

    static IVSoftKeyboardAdapter* s_pSoftkeyboardAdapter;

    static int s_iInitCount;
    // Here stuff like touch input can be added

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
