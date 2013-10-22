/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#include <Vision/Samples/Engine/RPGPlugin/Precompiled.h>

#include <Vision/Samples/Engine/RPGPlugin/PlayerControllerComponent.h>

#include <Vision/Samples/Engine/RPGPlugin/AttackableComponent.h>
#include <Vision/Samples/Engine/RPGPlugin/Character.h>
#include <Vision/Samples/Engine/RPGPlugin/PlayerUI.h>

RPG_PlayerControllerInput::RPG_PlayerControllerInput()
  : m_buttons(0),
  m_targetEntity(NULL),
  m_targetPoint()
{
}

// RPG_PlayerControllerComponent
RPG_PlayerControllerComponent::RPG_PlayerControllerComponent()
  : RPG_ControllerComponent()
{
}

void RPG_PlayerControllerComponent::ServerTick(float deltaTime)
{
  RPG_ControllerComponent::ServerTick(deltaTime);

  m_prevInput = m_input;
}

void RPG_PlayerControllerComponent::SetOwner(VisTypedEngineObject_cl *newOwner)
{
  RPG_ControllerComponent::SetOwner(newOwner);

  if(newOwner)
  {
    // Create states
    m_states[RPG_ControllerStateId::kNotBeingControlled] = new RPG_ControllerState::NotBeingControlled();
    m_states[RPG_ControllerStateId::kIdling] = new RPG_PlayerControllerState::Idling();
    m_states[RPG_ControllerStateId::kMoving] = new RPG_PlayerControllerState::Moving();
    m_states[RPG_ControllerStateId::kMeleeAttacking] = new RPG_PlayerControllerState::MeleeAttacking();
    m_states[RPG_ControllerStateId::kRangedAttacking] = new RPG_PlayerControllerState::RangedAttacking();
    m_states[RPG_ControllerStateId::kAoeAttacking] = new RPG_PlayerControllerState::AoeAttacking();
    m_states[RPG_ControllerStateId::kPowerAttacking] = new RPG_PlayerControllerState::PowerAttacking();
  }
}

V_IMPLEMENT_SERIAL(RPG_PlayerControllerComponent, RPG_ControllerComponent, 0, &g_RPGPluginModule);

START_VAR_TABLE(RPG_PlayerControllerComponent, RPG_ControllerComponent, "(RPG) Player Controller", 0, "(RPG) Player Controller")
END_VAR_TABLE

// Common utility functions
bool RPG_PlayerControllerState::OnTickSpecialAttacks(RPG_PlayerControllerComponent *const playerController, RPG_PlayerControllerInput const& input, RPG_PlayerControllerInput const& prevInput)
{
  // Ranged attack
  bool const rangedDown = input.IsDown(RPG_PlayerControllerInput::B_RANGED, prevInput);
  if(rangedDown)
  {
    playerController->SetTargetPoint(input.m_targetPoint);
    playerController->SetState(RPG_ControllerStateId::kRangedAttacking);
    return true;
  }

  // AOE attack
  bool const aoeDown = input.IsDown(RPG_PlayerControllerInput::B_AOE, prevInput);
  if(aoeDown && playerController->TryAoeAttack())
  {
    return true;
  }

  // Power attack
  bool const powerDown = input.IsDown(RPG_PlayerControllerInput::B_POWER, prevInput);
  if(powerDown && playerController->TryPowerAttack())
  {
    playerController->SetTargetPoint(input.m_targetPoint);
    return true;
  }

  return false;
}

// Idling
void RPG_PlayerControllerState::Idling::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
  RPG_ControllerStateBase::OnTick(controller, deltaTime);

  RPG_PlayerControllerComponent *playerController = vstatic_cast<RPG_PlayerControllerComponent*>(controller);

  RPG_PlayerControllerInput const& input = playerController->GetInput();
  RPG_PlayerControllerInput const& prevInput = playerController->GetPrevInput();

  // Tick special attacks
  if(RPG_PlayerControllerState::OnTickSpecialAttacks(playerController, input, prevInput))
  {
    return;
  }

  // Move
  bool const primaryDown = input.IsDown(RPG_PlayerControllerInput::B_PRIMARY, prevInput);
  bool const primaryHeld = input.IsPressed(RPG_PlayerControllerInput::B_PRIMARY) && prevInput.IsPressed(RPG_PlayerControllerInput::B_PRIMARY);

  if(primaryDown || primaryHeld)
  {
    if(primaryDown && input.m_targetEntity)
    {
      // Move to target
      controller->SetTarget(input.m_targetEntity);
      controller->SetTargetPoint(input.m_targetEntity->GetPosition());
    }
    else
    {
      // Move to point
      controller->SetTarget(NULL);
      controller->SetTargetPoint(input.m_targetPoint);
    }

    hkvVec3 const targetPoint = controller->GetTargetPoint();
    hkvVec3 const targetPointToCharacter(controller->GetCharacter()->GetPosition() - targetPoint);
    float const targetPointToCharactedDistSqr = targetPointToCharacter.getLengthSquared();
    float const distTolerance = hkvMath::Max(60.0f, controller->GetPathGoalReachedTolerance());
    float const distToleranceSqr = distTolerance * distTolerance;

    if(targetPointToCharactedDistSqr > distToleranceSqr)
    {
      controller->SetState(RPG_ControllerStateId::kMoving);
      return;
    }
  }
}

// Moving
void RPG_PlayerControllerState::Moving::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
  RPG_ControllerStateBase::OnTick(controller, deltaTime);

  RPG_PlayerControllerComponent *playerController = vstatic_cast<RPG_PlayerControllerComponent*>(controller);

  RPG_PlayerControllerInput const& input = playerController->GetInput();
  RPG_PlayerControllerInput const& prevInput = playerController->GetPrevInput();

  // Tick special attacks
  if(RPG_PlayerControllerState::OnTickSpecialAttacks(playerController, input, prevInput))
  {
    return;
  }

  // Move or melee
  bool const primaryDown = input.IsDown(RPG_PlayerControllerInput::B_PRIMARY, prevInput);
  bool const primaryHeld = input.IsPressed(RPG_PlayerControllerInput::B_PRIMARY) && prevInput.IsPressed(RPG_PlayerControllerInput::B_PRIMARY);

  // Update target on click
  if(primaryDown)
  {
    playerController->SetTarget(input.m_targetEntity);
  }

  RPG_Character *const character = playerController->GetCharacter();
  RPG_DamageableEntity *const target = playerController->GetTarget();

  if(target)
  {
    hkvVec3 targetToCharacterProjectedDir;
    float targetToCharacterProjectedDist;
    RPG_ControllerUtil::GetProjectedDirAndDistanceFromTarget(character, target, targetToCharacterProjectedDir, targetToCharacterProjectedDist);

    float const minDist = RPG_ControllerUtil::GetMinimumDistanceToAttack(character, target);

    if(targetToCharacterProjectedDist > minDist)
    {
      controller->SetTargetPoint(target->GetPosition() + (target->GetCollisionRadius() + character->GetCollisionRadius()) * targetToCharacterProjectedDir);
    }
    else
    {
      controller->GetCharacter()->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kMeleeAttack);
      controller->SetState(RPG_ControllerStateId::kMeleeAttacking);
      return;
    }
  }
  else if(primaryDown || primaryHeld)
  {
    controller->SetTargetPoint(input.m_targetPoint);
  }

  // Handle target reached or target update
  hkvVec3 const targetPoint = controller->GetTargetPoint();
  hkvVec3 const targetPointToCharacter(character->GetPosition() - targetPoint);
  float const targetPointToCharactedDistSqr = targetPointToCharacter.getLengthSquared();
  float const pathGoaltoleranceSqr = controller->GetPathGoalReachedTolerance() * controller->GetPathGoalReachedTolerance();

  if(targetPointToCharactedDistSqr <= pathGoaltoleranceSqr)
  {
    controller->SetState(RPG_ControllerStateId::kIdling);
    return;
  }
  else if(primaryDown || primaryHeld)
  {
    controller->RequestPath(targetPoint);
  }

  // Reorient
  hkvVec3 dir;
  RPG_ControllerUtil::CalcDirection(dir, controller->GetCharacter()->GetDirection(), controller->GetDirection(), 0.25f);
  controller->GetCharacter()->SetDirection(dir);
}

// MeleeAttacking
void RPG_PlayerControllerState::MeleeAttacking::OnEnterState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnEnterState(controller);

  controller->GetCharacter()->CreateCharacterEffect(FX_MeleeBasicAttackSwing);
}

void RPG_PlayerControllerState::MeleeAttacking::OnProcessAnimationEvent(RPG_ControllerComponent *controller, hkbEvent const& animationEvent)
{
  RPG_ControllerStateBase::OnProcessAnimationEvent(controller, animationEvent);

  RPG_Character *const character = controller->GetCharacter();

  if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kMeleeAttackEnd) == animationEvent.getId())
  {
    controller->SetState(RPG_ControllerStateId::kIdling);
  }
  else if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kMeleeAttackFire) == animationEvent.getId())
  {
    // Do attack
    character->DoMeleeAttack(vstatic_cast<RPG_DamageableEntity*>(controller->GetTarget()));
  }
}

void RPG_PlayerControllerState::MeleeAttacking::OnTick(RPG_ControllerComponent *controller, float deltaTime)
{
  RPG_ControllerStateBase::OnTick(controller, deltaTime);

  RPG_DamageableEntity const *const target = controller->GetTarget();

  if(target)
  {
    RPG_ControllerUtil::FaceTowards(controller, target->GetPosition(), 0.25f);
  }
}

// AoeAttacking
void RPG_PlayerControllerState::AoeAttacking::OnEnterState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnEnterState(controller);

  RPG_Character *const character = controller->GetCharacter();

  character->CreateCharacterEffect(FX_AoeAttackBegin);
  character->CreateCharacterEffect(FX_AoeAttackChargeLoop);

  controller->GetCharacter()->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kAoeAttack);
}

void RPG_PlayerControllerState::AoeAttacking::OnProcessAnimationEvent(RPG_ControllerComponent *const controller, hkbEvent const& animationEvent)
{
  RPG_ControllerStateBase::OnProcessAnimationEvent(controller, animationEvent);

  RPG_Character *const character = controller->GetCharacter();

  if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kAoeAttackEnd) == animationEvent.getId())
  {
    character->PauseCharacterEffect(FX_AoeAttackChargeLoop);

    controller->SetState(RPG_ControllerStateId::kIdling);
  }
  else if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kAoeAttackFire) == animationEvent.getId())
  {
    // Do attack
    character->PauseCharacterEffect(FX_AoeAttackChargeLoop);
    character->CreateCharacterEffect(FX_AoeAttackFire);

    character->DoAoeAttack();
  }
}

// RangedAttacking
void RPG_PlayerControllerState::RangedAttacking::OnEnterState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnEnterState(controller);

  controller->GetCharacter()->CreateCharacterEffect(FX_RangedAttackChargeLoop);

  controller->GetCharacter()->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kRangedAttack);
}

void RPG_PlayerControllerState::RangedAttacking::OnExitState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnExitState(controller);

  controller->GetCharacter()->PauseCharacterEffect(FX_RangedAttackChargeLoop);
}

void RPG_PlayerControllerState::RangedAttacking::OnProcessAnimationEvent(RPG_ControllerComponent *const controller, hkbEvent const& animationEvent)
{
  RPG_ControllerStateBase::OnProcessAnimationEvent(controller, animationEvent);

  RPG_Character *const character = controller->GetCharacter();

  if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kRangedAttackEnd) == animationEvent.getId())
  {
    controller->SetState(RPG_ControllerStateId::kIdling);
  }
  else if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kRangedAttackFire) == animationEvent.getId())
  {
    // Do attack
    character->PauseCharacterEffect(FX_RangedAttackChargeLoop);
    character->CreateCharacterEffect(FX_RangedAttackFire);

    character->DoRangedAttack(controller->GetTargetPoint());
  }
}

void RPG_PlayerControllerState::RangedAttacking::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
  RPG_ControllerStateBase::OnTick(controller, deltaTime);

  RPG_ControllerUtil::FaceTowards(controller, controller->GetTargetPoint(), 0.25f);
}

// PowerAttacking
void RPG_PlayerControllerState::PowerAttacking::OnEnterState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnEnterState(controller);

  controller->GetCharacter()->CreateCharacterEffect(FX_PowerAttackSwing);

  controller->GetCharacter()->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kPowerAttack);
}

void RPG_PlayerControllerState::PowerAttacking::OnProcessAnimationEvent(RPG_ControllerComponent *const controller, hkbEvent const& animationEvent)
{
  RPG_ControllerStateBase::OnProcessAnimationEvent(controller, animationEvent);

  RPG_Character *const character = controller->GetCharacter();

  if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kPowerAttackEnd) == animationEvent.getId())
  {
    controller->SetState(RPG_ControllerStateId::kIdling);
  }
  else if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kPowerAttackFire) == animationEvent.getId())
  {
    character->CreateCharacterEffect(FX_PowerAttack);

    // Do attack
    character->DoPowerAttack();
  }
}

void RPG_PlayerControllerState::PowerAttacking::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
  RPG_ControllerStateBase::OnTick(controller, deltaTime);

  RPG_ControllerUtil::FaceTowards(controller, controller->GetTargetPoint(), 0.25f);
}

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
