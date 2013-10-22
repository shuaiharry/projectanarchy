/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef RPG_PLUGIN_AI_CONTROLLER_COMPONENT_H__
#define RPG_PLUGIN_AI_CONTROLLER_COMPONENT_H__

#include <Vision/Samples/Engine/RPGPlugin/ControllerComponent.h>

class RPG_DamageableEntity;

enum RPG_AIState_e
{
  ST_None=-1,
  ST_Spawning=0,
  ST_Wandering,
  ST_Challenging,
  ST_MovingToPosition,
  ST_MeleeAttacking,
  ST_RangedAttacking,
  ST_AoeAttacking,
  ST_Fleeing,
  ST_Healing,
  ST_Count
};

// AI controller component
class RPG_AiControllerComponent : public RPG_ControllerComponent
{
public:
  // Constructor needs to be public for FORCE_LINKDYNCLASS on mobile
  RPG_AiControllerComponent();

  bool CanAttack() const;

  void SetLastAttackTime(float lastAttackTime) { m_lastAttackTime = lastAttackTime; }

  virtual bool TryMeleeAttack();

  virtual bool TryRangedAttack();

  // RPG_ControllerComponent
  bool TryAoeAttack() HKV_OVERRIDE;

//protected:
  //@{
  // Common utility methods
  bool HasValidTarget() const;
  bool HasLineOfSightToTarget() const;
  bool HasLineOfSightToTarget(bool& left, bool& center, bool& right) const;
  //@}

  //@{
  // Common utility methods
  bool AcquireTarget();
  //@}

  float m_lastAttackTime;
  float m_minAttackInterval;

  float m_fleeRange;

private:
  V_DECLARE_SERIAL_DLLEXP(RPG_AiControllerComponent, RPG_PLUGIN_IMPEXP);
  V_DECLARE_VARTABLE(RPG_AiControllerComponent, RPG_PLUGIN_IMPEXP);
};

namespace RPG_AiControllerState
{
  // MeleeAttacking
  class MeleeAttacking : public RPG_ControllerStateBase
  {
    void OnEnterState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent) HKV_OVERRIDE;

    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "AiController::MeleeAttacking"; }
  };

  // RangedAttacking
  class RangedAttacking : public RPG_ControllerStateBase
  {
    void OnEnterState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnExitState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent) HKV_OVERRIDE;

    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "AiController::RangedAttacking"; }
  };

  // AoeAttacking
  class AoeAttacking : public RPG_ControllerStateBase
  {
    void OnEnterState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnExitState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "AiController::AoeAttacking"; }
  };

  // Challenging
  class Challenging : public RPG_ControllerStateBase
  {
    void OnEnterState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent) HKV_OVERRIDE;

    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "AiController::Challenging"; }
  };
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
