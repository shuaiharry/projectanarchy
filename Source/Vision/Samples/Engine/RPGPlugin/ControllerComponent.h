/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef RPG_PLUGIN_CONTROLLER_COMPONENT_H__
#define RPG_PLUGIN_CONTROLLER_COMPONENT_H__

#include <Vision/Samples/Engine/RPGPlugin/BaseComponent.h>
#include <Vision/Samples/Engine/RPGPlugin/Character.h>

class hkaiCharacter;
class hkaiBehavior;
class hkbEvent;

class RPG_Character;
class RPG_DamageableEntity;

class RPG_ControllerStateBase;

namespace RPG_ControllerStateId
{
  enum Enum
  {
    // Common
    kNotBeingControlled,
    kIdling,
    kMoving,
    kMeleeAttacking,
    kRangedAttacking,
    kAoeAttacking,
    kPowerAttacking,
    kHealing,

    // AI
    kChallenging,
    kFleeing,

    kNumStates
  };
}

// Character component
class RPG_ControllerComponent : public RPG_BaseComponent
{
public:
  // Constructor needs to be public for FORCE_LINKDYNCLASS on mobile
  RPG_ControllerComponent();

  void SetState(RPG_ControllerStateId::Enum newStateId);

  RPG_Character *GetCharacter() const { return vstatic_cast<RPG_Character*>(GetOwner()); }

  // AI character
  void RequestPath(hkvVec3 const& dest);

  void CancelRequestedPath();

  float GetPathGoalReachedTolerance() const;

  hkvVec3 GetDirection() const { return m_cachedSpeed > 0.0f ? m_cachedDirection : GetCharacter()->GetDirection(); }

  float GetSpeed() const { return m_cachedSpeed; }

  void SetSensorSize(float sensorSize);

  void SetDesiredSpeed(float desiredSpeed);

  // Target entity
  void SetTarget(RPG_DamageableEntity* target) { m_target = (target ? target->GetWeakReference() : NULL); }

  RPG_DamageableEntity* GetTarget() const { return vstatic_cast<RPG_DamageableEntity*>(m_target); }

  // Target point
  void SetTargetPoint(hkvVec3 const& targetPoint) { m_targetPoint = targetPoint; }

  hkvVec3 const& GetTargetPoint() { return m_targetPoint; }

  // Update gameplay logic here. Called from entity updates, and only on the server.
  virtual void ServerTick(float deltaTime);

  virtual void OnProcessAnimationEvent(RPG_Character *characterOwner, hkbEvent const& animationEvent);

  // Attacks
  virtual bool TryAoeAttack();

  virtual bool TryPowerAttack();

  // Called by AI world step with AI step dt
  void OnAfterCharacterStep(float deltaTime);

protected:
  // IVObjectComponent
  BOOL CanAttachToObject(VisTypedEngineObject_cl *object, VString& errorOut) HKV_OVERRIDE;

  void SetOwner(VisTypedEngineObject_cl *newOwner) HKV_OVERRIDE;

protected:
  VSmartPtr<RPG_ControllerStateBase> m_states[RPG_ControllerStateId::kNumStates];
  RPG_ControllerStateId::Enum m_currentStateId;
  bool m_settingState;

  hkRefPtr<hkaiCharacter> m_aiCharacter;
  hkRefPtr<hkaiBehavior> m_aiBehavior;

  float m_sensorSize;
  float m_desiredSpeed;

  hkvVec3 m_cachedDirection;
  float m_cachedSpeed;

  VWeakPtr<VisBaseEntity_cl> m_target;
  hkvVec3 m_targetPoint;

private:
  V_DECLARE_SERIAL_DLLEXP(RPG_ControllerComponent, RPG_PLUGIN_IMPEXP);
  V_DECLARE_VARTABLE(RPG_ControllerComponent, RPG_PLUGIN_IMPEXP);
};

// RPG_ControllerStateBase: base of all controller states
class RPG_ControllerStateBase : public VRefCounter
{
public:
  virtual void OnEnterState(RPG_ControllerComponent *controller);

  virtual void OnExitState(RPG_ControllerComponent *controller);

  virtual void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent);

  virtual void OnTick(RPG_ControllerComponent *controller, float deltaTime);

  virtual char const *GetName() const = 0;
};

// RPG_ControllerState: common states
namespace RPG_ControllerState
{
  class NotBeingControlled : public RPG_ControllerStateBase
  {
    void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Controller::NotBeingControlled"; }
  };

  class Moving : public RPG_ControllerStateBase
  {
    void OnEnterState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnExitState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Controller::Moving"; }
  };
}

namespace RPG_ControllerUtil
{
  // Vector slerp, with linearly independent (current, desired) check and t clamped to (0, 1)
  void CalcDirection(hkvVec3& resultDir, hkvVec3 const& currentDir, hkvVec3 const& desiredDir, float t);

  bool GetClosestPointOnNavMesh(hkvVec3 const& position, float testRadius, hkvVec3& outPosition);

  bool IsPointOnNavmesh(hkvVec3 const& position, float const testRadius = 0.1f);

  // TODO remove? Only used by RPG_ControllerUtil::FaceTowards
  bool CalcFaceTowards(hkvVec3& resultDir, hkvVec3 const& fromDir, hkvVec3 const& fromPos, hkvVec3 const& toPos, float t);

  void FaceTowards(RPG_ControllerComponent *controller, hkvVec3 const& toPos, float t);

  void GetProjectedDirAndDistanceFromTarget(RPG_Character const *character, RPG_DamageableEntity *const target, hkvVec3& outDir, float& outDist);

  float GetMinimumDistanceToAttack(RPG_Character const *character, RPG_DamageableEntity *const target);
}

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
