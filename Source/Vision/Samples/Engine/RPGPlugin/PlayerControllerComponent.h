/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef RPG_PLUGIN_PLAYER_CONTROLLER_COMPONENT_H__
#define RPG_PLUGIN_PLAYER_CONTROLLER_COMPONENT_H__

#include <Vision/Samples/Engine/RPGPlugin/ControllerComponent.h>

class RPG_DamageableEntity;

struct RPG_PlayerControllerInput
{
  enum Button
  {
    B_PRIMARY = (1u << 0),
    B_RANGED = (1u << 1),
    B_POWER = (1u << 2),
    B_AOE = (1u << 3),
  };

  RPG_PlayerControllerInput();

  bool IsPressed(Button button) const { return (m_buttons & button) != 0; }

  bool IsDown(Button button, RPG_PlayerControllerInput const& prev) const { return IsPressed(button) && !prev.IsPressed(button); }

  unsigned m_buttons;
  RPG_DamageableEntity *m_targetEntity;
  hkvVec3 m_targetPoint;
};

// Player controller component
class RPG_PlayerControllerComponent : public RPG_ControllerComponent
{
public:
  // Constructor needs to be public for FORCE_LINKDYNCLASS on mobile
  RPG_PlayerControllerComponent();

  void SetInput(RPG_PlayerControllerInput const& input) { m_input = input; }

  RPG_PlayerControllerInput const& GetInput() const { return m_input; }

  RPG_PlayerControllerInput const& GetPrevInput() const { return m_prevInput; }

  // RPG_ControllerComponent
  void ServerTick(float deltaTime) HKV_OVERRIDE;

private:
  // IVObjectComponent
  void SetOwner(VisTypedEngineObject_cl *newOwner) HKV_OVERRIDE;

  RPG_PlayerControllerInput m_input, m_prevInput;

  V_DECLARE_SERIAL_DLLEXP(RPG_PlayerControllerComponent, RPG_PLUGIN_IMPEXP);
  V_DECLARE_VARTABLE(RPG_PlayerControllerComponent, RPG_PLUGIN_IMPEXP);
};

namespace RPG_PlayerControllerState
{
  // Common
  bool OnTickSpecialAttacks(RPG_PlayerControllerComponent *playerController, RPG_PlayerControllerInput const& input, RPG_PlayerControllerInput const& prevInput);

  // Idling
  class Idling : public RPG_ControllerStateBase
  {
    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Player::Idling"; }
  };

  // Moving
  class Moving : public RPG_ControllerState::Moving
  {
    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Player::Moving"; }
  };

  // MeleeAttacking
  class MeleeAttacking : public RPG_ControllerStateBase
  {
    void OnEnterState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent) HKV_OVERRIDE;

    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Player::MeleeAttacking"; }
  };

  // AoeAttacking
  class AoeAttacking : public RPG_ControllerStateBase
  {
    void OnEnterState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Player::AoeAttacking"; }
  };

  // RangedAttacking
  class RangedAttacking : public RPG_ControllerStateBase
  {
    void OnEnterState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnExitState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent) HKV_OVERRIDE;

    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Player::RangedAttacking"; }
  };

  // PowerAttacking
  class PowerAttacking : public RPG_ControllerStateBase
  {
    void OnEnterState(RPG_ControllerComponent *controller) HKV_OVERRIDE;

    void OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent) HKV_OVERRIDE;

    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Player::PowerAttacking"; }
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
