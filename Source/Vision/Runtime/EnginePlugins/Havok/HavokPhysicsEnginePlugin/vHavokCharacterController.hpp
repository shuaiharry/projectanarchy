/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file vHavokCharacterController.hpp

#ifndef vHavokCharacterController_HPP_INCLUDED
#define vHavokCharacterController_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokPhysicsModule.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokPhysicsIncludes.hpp>

#include <Physics2012/Dynamics/Phantom/hkpSimpleShapePhantom.h>
#include <Physics2012/Utilities/CharacterControl/CharacterProxy/hkpCharacterProxy.h>
#include <Physics2012/Utilities/CharacterControl/StateMachine/hkpDefaultCharacterStates.h>

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/CharacterControlStates/vCharacterInput.h>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/CharacterControlStates/vCharacterStateClimbing.h>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/CharacterControlStates/vCharacterStateFlying.h>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/CharacterControlStates/vCharacterStateInAir.h>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/CharacterControlStates/vCharacterStateJumping.h>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/CharacterControlStates/vCharacterStateOnGround.h>

class VTransitionStateMachine;
class vHavokCharacterController;

#if defined(HAVOK_SDK_VERSION_MAJOR) && (HAVOK_SDK_VERSION_MAJOR >= 2012)
typedef hkSimplexSolverInput hkpSimplexSolverInput;
#endif

#if !defined(_VISION_DOC)

#if defined (__SNC__)
#pragma diag_push
#pragma diag_suppress=1011
#endif

class vHavokCharacterPushableProxy : public hkpCharacterProxy, public hkpCharacterProxyListener
{
public:

	HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CHARACTER);
	
	vHavokCharacterPushableProxy( const hkpCharacterProxyCinfo& info, const vHavokCharacterController *pOwner, hkReal strength = 1.0f );

	virtual ~vHavokCharacterPushableProxy();

	virtual void processConstraintsCallback( const hkpCharacterProxy* proxy, const hkArray<hkpRootCdPoint>& manifold, hkpSimplexSolverInput& input );

	bool handleSteps( const vHavokCharacterController* character, const hkpCharacterInput& input, hkpCharacterOutput& output );
	bool isOnStep() const { return m_onStepCnt > 0; }

	void drawDebug( const vHavokCharacterController* character ) const;

	hkReal m_strength;	// Relative push strength
	int m_onStepCnt;	// indicates if handleStep() projected the controller onto a step
	class DebugHavokCharacterController* m_visDebug;
};

#if defined (__SNC__)
#pragma diag_pop
#endif

#endif // !_VISION_DOC


///
/// \brief
///   Havok Character Controller
///
class vHavokCharacterController : public IVObjectComponent
{
public:

  ///
  /// @name Constructor / Destructor
  /// @{
  ///

  ///
  /// \brief
  ///   Default constructor.
  ///
  /// The actual initialisation of the component happens in the ::SetOwner function
  ///
  VHAVOK_IMPEXP vHavokCharacterController();

  ///
  /// \brief
  ///   Destructor
  ///
  VHAVOK_IMPEXP virtual ~vHavokCharacterController();

  ///
  /// @}
  ///

  ///
  /// @name IVObjectComponent Virtual Overrides
  /// @{
  ///

  ///
  /// \brief
  ///   Overridden function to respond to owner changes.
  ///
  /// By setting the owner of this component the character controller object will be 
  /// added to the Havok World and gets registered in the Havok module. 
  ///
  /// \param pOwner
  ///   The owner of this component.
  /// 
  /// \remarks
  ///   SetOwner(NULL) removes the character controller object from the Havok World and
  ///   gets unregistered in the Havok module.
  /// 
  VHAVOK_IMPEXP virtual void SetOwner(VisTypedEngineObject_cl *pOwner) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Overridden function to determine if this component can be attached to a given object.
  ///
  /// The character controller component can be attached to VisObject3D_cl instances.
  /// 
  /// \param pObject
  ///   Possible owner candidate.
  /// 
  /// \param sErrorMsgOut
  ///   Reference to error message string.
  /// 
  /// \returns
  ///   TRUE if this component can be attached to the given object, FALSE otherwise.
  /// 
  VHAVOK_IMPEXP virtual BOOL CanAttachToObject(VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut) HKV_OVERRIDE;


  ///
  /// \brief
  ///   Overridden function to respond to variable changes.
  /// 
  /// \param pVar
  ///   Pointer to the variable object to identify the variable.
  /// 
  /// \param value
  ///   New value of the variable
  /// 
  VHAVOK_IMPEXP virtual void OnVariableValueChanged(VisVariable_cl *pVar, const char * value) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name VTypedObject Virtual Overrides
  /// @{
  ///

  #ifdef WIN32

  ///
  /// \brief
  ///   Overridable that is called by the editor to retrieve per-instance variable display hints. 
  ///
  /// \param pVariable
  ///   Variable to retrieve dynamic display hints for.
  ///
  /// \param destInfo
  ///   Structure that can be modified to affect the editor's displaying 
  ///   of the respective variable (read-only, hidden).
  ///
  VHAVOK_IMPEXP virtual void GetVariableAttributes(VisVariable_cl *pVariable, VVariableAttributeInfo &destInfo) HKV_OVERRIDE;
  
  #endif

  ///
  /// \brief
  ///   The message function has been overridden to forward collision events to a script 
  VHAVOK_IMPEXP virtual void MessageFunction(int iID, INT_PTR iParamA, INT_PTR iParamB) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name Serialization
  /// @{
  ///

  V_DECLARE_SERIAL_DLLEXP(vHavokCharacterController, VHAVOK_IMPEXP)

  V_DECLARE_VARTABLE(vHavokCharacterController, VHAVOK_IMPEXP)

  VHAVOK_IMPEXP virtual void Serialize(VArchive &ar) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual void OnDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual VBool WantsDeserializationCallback(const VSerializationContext &context) HKV_OVERRIDE
  {
    return TRUE;
  }

  ///
  /// @}
  ///

  ///
  /// @name Debug Rendering
  /// @{
  ///

  ///
  /// \brief
  ///   Enable / Disable debug rendering for this rigid body's shape.
  ///
  /// \param bEnable
  ///   If TRUE, the shape of this rigid body will be displayed in Vision.
  ///
  VHAVOK_IMPEXP void SetDebugRendering(BOOL bEnable);

  ///
  /// \brief 
  ///   Returns whether debug rendering is enabled.
  ///
  inline bool GetDebugRenderEnabled () const 
  { 
    return (Debug != FALSE); 
  }

  ///
  /// \brief
  ///   Set the color of the debug rendering representation of this rigid body's shape.
  ///
  /// \param color
  ///   New Color of the debug rendering representation.
  ///
  VHAVOK_IMPEXP void SetDebugColor(VColorRef color);

  ///
  /// @}
  ///

  ///
  /// @name Simulation
  /// @{
  ///

  ///
  /// \brief
  ///   Enables / Disables simulation.
  ///
  /// \param bEnabled
  ///   Enabled state to set.
  ///
  VHAVOK_IMPEXP void SetEnabled(BOOL bEnabled);

  ///
  /// \brief
  ///   Returns whether simulation is enabled;
  inline bool IsEnabled() const
  {
    return (m_bEnabled == TRUE);
  }
  
  ///
  /// \brief
  ///   After step, get the actual linear velocity achieved.
  ///
  /// This includes effect of gravity if unsupported.
  ///
  /// \param currentVelocity
  ///   Reference to velocity vector.
  ///
  VHAVOK_IMPEXP void GetCurrentLinearVelocity(hkvVec3& currentVelocity) const;

  ///
  /// \brief
  ///   Directly sets a position
  ///
  /// \param x
  ///   New position
  ///
  VHAVOK_IMPEXP void SetPosition(const hkvVec3& x);

  ///
  /// \brief
  ///   Returns the character's position
  ///
  VHAVOK_IMPEXP hkvVec3 GetPosition() const;

  ///
  /// \brief
  ///   Checks if the character is standing on the ground
  ///
  VHAVOK_IMPEXP bool IsStanding() const;

  ///
  /// \brief
  ///   Checks if the character is supported by world geometry in a given direction
  ///
  VHAVOK_IMPEXP bool CheckSupport(const hkvVec3& v) const;

  ///
  /// \brief
  ///   Sets whether the character is able to jump.
  ///
  VHAVOK_IMPEXP void SetWantJump(bool wantJump);

  ///
  /// \brief
  ///   Sets whether the character is able to fly.
  ///
  VHAVOK_IMPEXP void SetFlyState(bool bIsFlying);

  ///
  /// \brief
  ///   Step the character (note this locks the world, so should only do this outside of physics step).
  ///
  /// Will not update the owner, as otherwise this could not be executed in the background.
  /// UpdateOwner will be called by the Physics Module in the main thread of physics handling.
  ///
  /// \param fTimeStep
  ///   Fixed physics time step.
  ///
  /// \param iNumSteps
  ///   Number of steps to integrate.
  ///
  /// \param fDuration
  ///   Frame-interval of application.
  ///
  VHAVOK_IMPEXP virtual void Step(float fTimeStep, int iNumSteps, float fDuration);

  /// \brief
  ///   Updates the owner of this component with the position and rotation calculated
  ///   by the animation and physics systems.
  VHAVOK_IMPEXP void UpdateOwner();

  /// \brief
  ///   Returns a pointer to the internal \c hkpCharacterProxy object.
  ///
  inline const vHavokCharacterPushableProxy* GetCharacterProxy() const 
  { 
    return m_pCharacterProxy; 
  }

  ///
  /// \brief
  ///   Returns the internal \c hkpCharacterProxy object.
  ///
  inline vHavokCharacterPushableProxy* GetCharacterProxy()
  {
    return m_pCharacterProxy;
  }

  /// \brief
  ///   Returns a const pointer to the \c hkpCharacterContext object used internally.
  ///
  inline const hkpCharacterContext* GetCharacterContext() const 
  { 
    return m_pCharacterContext; 
  }

  /// \brief
  ///   Updates the collision capsule based on the Character_Top, Character_Bottom and Capsule_Radius 
  ///   members.
  VHAVOK_IMPEXP void UpdateBoundingVolume();

  /// \brief
  ///   Tries to set the collision capsule to the given endpoints. 
  ///
  /// The function succeeds whenever the resulting capsule doesn't intersect the world.
  ///
  /// \param capTop
  ///   First capsule axis endpoint
  ///
  /// \param capBottom
  ///   Second capsule axis endpoint
  VHAVOK_IMPEXP bool TryUpdateBoundingVolume(const hkvVec3& capTop, const hkvVec3& capBottom);

  /// \brief
  ///   Returns the current gravity scaling factor. 
  ///
  /// The default value is 1.0f (normal gravity of the world)
  inline float GetGravityScaling() const 
  { 
    return Gravity_Scale; 
  }

  /// \brief
  ///   Sets the scaling of the world's gravity vector. 
  ///
  /// \param fValue
  ///   The value used for scaling.
  ///
  /// The default value is 1.0f (normal gravity of the world). Passing 0.0f disables the gravity.
  inline void SetGravityScaling(float fValue)
  {
    Gravity_Scale = fValue;
  }

  /// \brief
  ///   Sets the collision parameters of this character controller with the parameters of a hkpGroupFilter.
  /// \param iLayer
  ///   the collision layer
  /// \param iGroup
  ///   the collision group
  /// \param iSubsystem
  ///   the collision subsystem
  /// \param iSubsystemDontCollideWith
  ///   the collision subsystem this body shouldn't collide with
  /// \note
  ///   See the Havok documentation on rigid body collisions for more 
  ///   information on how to specify values for these parameters.
  VHAVOK_IMPEXP void SetCollisionInfo(int iLayer, int iGroup, int iSubsystem, int iSubsystemDontCollideWith);

  /// \brief
  ///   Sets the collision filter of this character controller.
  /// \param iCollisionFilter
  ///   The collision filter as computed by hkpGroupFilter::calcFilterInfo.
  /// \note
  ///   See the Havok documentation on rigid body collisions for more 
  ///   information on how to specify values for the parameters of hkpGroupFilter::calcFilterInfo.
  VHAVOK_IMPEXP void SetCollisionInfo(int iCollisionFilter);

  ///
  /// \brief
  ///   Creates the actual Havok character controller and adds it to the simulation.
  ///
  VHAVOK_IMPEXP void CreateHavokController();

  ///
  /// \brief
  ///   Removes internal character object from the simulation and releases the reference.
  ///
  VHAVOK_IMPEXP void DeleteHavokController();

  ///
  /// \brief
  ///   Creates state machine for character controller, You can create new state machine by overriding the method.
  ///
  VHAVOK_IMPEXP virtual void CreateStateMachineAndContext();

  ///
  /// @}
  ///

  ///
  /// @name Members exposed to vForge:
  /// @{
  ///

  float Capsule_Radius;                   ///< Radius of the character controller capsule.
  hkvVec3 Character_Top;                  ///< Top position of the character controller capsule.
  hkvVec3 Character_Bottom;               ///< Bottom position of the character controller capsule.
  float Static_Friction;                  ///< Default static friction for surfaces.
  float Dynamic_Friction;                 ///< Default dynamic friction for surfaces.
  float Max_Slope;                        ///< Maximum slope that the character can walk upwards (in degrees, max 90 deg).
  float Character_Mass;                   ///< Mass of the character.
  float Character_Strength;               ///< Maximum force that the character controller can impart onto moving objects.
  float Gravity_Scale;                    ///< Scalar factor to scale the global gravity strength. Default is 1.0.
  float Jump_Height;                      ///< Jump Height of the character controller. Default is 1.5.
  float Step_Height;                      ///< Maximum height of step that can be climbed.
  float Step_Threshold;                   ///< Step identity threshold.
  float Max_Velocity;                     ///< Maximum velocity of the character.
  float Max_Acceleration;                 ///< Maximum acceleration of the character.
  float PenetrationRecoverySpeed;         ///< Defines how fast the character is pushed away from any penetrated geometry.

  BOOL m_bEnabled;                        ///< Determines if character controller is simulated.
  BOOL Debug;                             ///< Determines if debug rendering is active.
  VColorRef DebugColor;                   ///< Debug color of the debug geometry representation.
  BOOL Fly_State;                         ///< Determines if character controller is able to fly.

  ///
  /// @}
  ///

protected:
  vHavokCharacterPushableProxy* m_pCharacterProxy;  ///< Proxy of the Havok character controller
  hkpCharacterContext* m_pCharacterContext; 
  hkvVec3 m_currentVelocity;                        ///< Current velocity of the controller (in Vision units)
  bool m_wantJump;
  vCharacterInput m_characterInput;
  int m_iCollisionFilter;

private:
  // Applies entity scale to specified values, validates those values and returns the
  // worldTransform matrix of the entity.
  hkvMat4 ApplyEntityScale(hkVector4 &vTop, hkVector4 &vBottom, float &fRadius);
};

#endif // vHavokCharacterController_HPP_INCLUDED

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
