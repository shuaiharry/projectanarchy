/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#include <Vision/Samples/Engine/RPGPlugin/Precompiled.h>

#include <Vision/Samples/Engine/RPGPlugin/AiControllerComponent.h>
#include <Vision/Samples/Engine/RPGPlugin/Character.h>
#include <Vision/Samples/Engine/RPGPlugin/GameManager.h>
#include <Vision/Samples/Engine/RPGPlugin/PlayerCharacter.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionHavokConversion.h>

#include <Vision/Runtime/Engine/Physics/IVisApiPhysicsModule.hpp>

namespace
{
  class RPG_ClosestRaycastResultIgnoreSourceTarget : public VisPhysicsRaycastBase_cl
  {
  public:
    RPG_ClosestRaycastResultIgnoreSourceTarget(VisTypedEngineObject_cl const* self, VisTypedEngineObject_cl const* target)
    : VisPhysicsRaycastBase_cl()
    , bHit(false)
    , pSelf(self)
    , pTarget(target)
    {
      iCollisionBitmask = hkpGroupFilter::calcFilterInfo(0, 0, 0, 0);
    }

    virtual void Reset()
    {
      VisPhysicsRaycastBase_cl::Reset();
      bHit = false;
      hit.Reset();
      iCollisionBitmask = hkpGroupFilter::calcFilterInfo(0, 0, 0, 0);
    }

    virtual bool onHit(VisPhysicsHit_t &hit)
    {
      if(hit.pHitObject == pSelf || hit.pHitObject == pTarget)
      {
        return true;
      }

      bHit = true;
      this->hit = hit;
      return false;
    }

    virtual bool allHits(){ return false; }

    VisPhysicsHit_t hit;
    bool bHit;
    VisTypedEngineObject_cl const* pSelf;
    VisTypedEngineObject_cl const* pTarget;
  };
}


// RPG_AiControllerComponent
RPG_AiControllerComponent::RPG_AiControllerComponent()
  : RPG_ControllerComponent(),
  m_lastAttackTime(0.0f),
  m_minAttackInterval(3.0f),
  m_fleeRange(300.0f)
{
}

V_IMPLEMENT_SERIAL(RPG_AiControllerComponent, RPG_ControllerComponent, 0, &g_RPGPluginModule);

START_VAR_TABLE(RPG_AiControllerComponent, RPG_ControllerComponent, "(RPG) AI Controller", 0, "(RPG) AI Controller")
END_VAR_TABLE

bool RPG_AiControllerComponent::CanAttack() const
{
  return ((Vision::GetTimer()->GetTime() - m_lastAttackTime) > m_minAttackInterval);
}

bool RPG_AiControllerComponent::TryMeleeAttack()
{
  if(CanAttack())
  {
    m_lastAttackTime = Vision::GetTimer()->GetTime();
    SetState(RPG_ControllerStateId::kMeleeAttacking);
    return true;
  }

  return false;
}

bool RPG_AiControllerComponent::TryRangedAttack()
{
  if(CanAttack())
  {
    m_lastAttackTime = Vision::GetTimer()->GetTime();
    SetState(RPG_ControllerStateId::kRangedAttacking);
    return true;
  }

  return false;
}

bool RPG_AiControllerComponent::TryAoeAttack()
{
  if(CanAttack())
  {
    m_lastAttackTime = Vision::GetTimer()->GetTime();
    return RPG_ControllerComponent::TryAoeAttack();
  }

  return false;
}

/// Finds and sets a target within range to interact with (usually to attack).
/// @TODO: Implement teams and team relationships, as target must currently be a player.
bool RPG_AiControllerComponent::AcquireTarget()
{
  VArray<RPG_Character*> const& characters = RPG_GameManager::s_instance.GetCharacters();

  hkvVec3 const& currentPosition = GetCharacter()->GetPosition();

  bool returnVal = false;
  for(int index = 0; index < characters.GetSize(); ++index)
  {
    RPG_Character* character = characters.GetAt(index);

    // can't target yourself
    if(character == GetCharacter())
    {
      continue;
    }

    // only target apparently alive players
    if(!character->IsOfType(V_RUNTIME_CLASS(RPG_PlayerCharacter)) ||
      character->IsDead() ||
      character->IsFeigningDeath())
    {
      continue;
    }

    float const aggroRadius = GetCharacter()->GetAggroRadius();
    hkvVec3 const& targetPosition = character->GetPosition();

    float const currentRangeSquared = (currentPosition - targetPosition).getLengthSquared();

    // check if they are within range
    if(currentRangeSquared <= aggroRadius * aggroRadius)
    {
      SetTarget(character);
      returnVal = true;
    }
  }
  return returnVal;
}

bool RPG_AiControllerComponent::HasValidTarget() const
{
  if (m_target)
  {
    VArray<RPG_DamageableEntity*> const& attackableEntities = RPG_GameManager::s_instance.GetAttackableEntities();
    if(attackableEntities.Find(GetTarget()) < 0)
    {
      return false;
    }

    if(m_target->IsFrom(RPG_Character))
    {
      float const aggroRadius = GetCharacter()->GetAggroRadius();
      hkvVec3 const& currentPosition = GetCharacter()->GetPosition();

      RPG_Character* targetCharacter = static_cast<RPG_Character*>(GetTarget());
      hkvVec3 const& targetPosition = targetCharacter->GetPosition();

      float const currentRangeSquared = (currentPosition - targetPosition).getLengthSquared();

      // check if target is still within range
      if(currentRangeSquared > aggroRadius * aggroRadius)
      {
        return false;
      }
      else
      {
        return !targetCharacter->IsDead() && !targetCharacter->IsFeigningDeath();
      }
    }
    else
    {
      return true;
    }
  }
  return false;
}

bool RPG_AiControllerComponent::HasLineOfSightToTarget() const
{
  bool left, center, right;
  return HasLineOfSightToTarget(left, center, right);
}

bool RPG_AiControllerComponent::HasLineOfSightToTarget(bool& left, bool& center, bool& right) const
{
  VASSERT(GetCharacter());
  VASSERT(m_target);

  RPG_ClosestRaycastResultIgnoreSourceTarget raycastResult(GetCharacter(), m_target);

  hkvVec3 rayStart;
  GetCharacter()->CalcPositionForTargeting(rayStart);

  hkvVec3 rayEnd;
  GetTarget()->CalcPositionForTargeting(rayEnd);

  hkvVec3 directionToTarget = rayEnd - rayStart;
  directionToTarget.normalizeIfNotZero();

  // check from the center
  center = true;
  raycastResult.Reset();
  raycastResult.vRayStart = rayStart;
  raycastResult.vRayEnd = rayEnd;
  Vision::GetApplication()->GetPhysicsModule()->PerformRaycast(&raycastResult);
  if (GetCharacter()->ShouldDisplayDebugStateInformation())
  {
    if(raycastResult.bHit)
    {
      Vision::Game.DrawSingleLine(raycastResult.vRayStart, raycastResult.vRayEnd, VColorRef(208, 24, 24), 3.0f);
    }
    else
    {
      Vision::Game.DrawSingleLine(raycastResult.vRayStart, raycastResult.vRayEnd, VColorRef(24, 208, 24), 3.0f);
    }
  }
  if(raycastResult.bHit)
  {
    center = false;
  }

  // check from the left side
  left = true;
  raycastResult.Reset();
  raycastResult.vRayStart = rayStart + hkvVec3(-directionToTarget.y, directionToTarget.x, 0.0f) * GetCharacter()->GetCollisionRadius();
  raycastResult.vRayEnd = rayEnd + hkvVec3(-directionToTarget.y, directionToTarget.x, 0.0f) * GetCharacter()->GetCollisionRadius();
  Vision::GetApplication()->GetPhysicsModule()->PerformRaycast(&raycastResult);
  if (GetCharacter()->ShouldDisplayDebugStateInformation())
  {
    if(raycastResult.bHit)
    {
      Vision::Game.DrawSingleLine(raycastResult.vRayStart, raycastResult.vRayEnd, VColorRef(208, 24, 24), 3.0f);
    }
    else
    {
      Vision::Game.DrawSingleLine(raycastResult.vRayStart, raycastResult.vRayEnd, VColorRef(24, 208, 24), 3.0f);
    }
  }
  if(raycastResult.bHit)
  {
    left = false;
  }

  // check from the right side
  right = true;
  raycastResult.Reset();
  raycastResult.vRayStart = rayStart + hkvVec3(directionToTarget.y, -directionToTarget.x, 0.0f) * GetCharacter()->GetCollisionRadius();
  raycastResult.vRayEnd = rayEnd + hkvVec3(directionToTarget.y, -directionToTarget.x, 0.0f) * GetCharacter()->GetCollisionRadius();
  Vision::GetApplication()->GetPhysicsModule()->PerformRaycast(&raycastResult);
  if (GetCharacter()->ShouldDisplayDebugStateInformation())
  {
    if(raycastResult.bHit)
    {
      Vision::Game.DrawSingleLine(raycastResult.vRayStart, raycastResult.vRayEnd, VColorRef(208, 24, 24), 3.0f);
    }
    else
    {
      Vision::Game.DrawSingleLine(raycastResult.vRayStart, raycastResult.vRayEnd, VColorRef(24, 208, 24), 3.0f);
    }
  }
  if(raycastResult.bHit)
  {
    right = false;
  }

  return left && center && right;
}

// RPG_AiControllerState::MeleeAttacking
void RPG_AiControllerState::MeleeAttacking::OnEnterState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnEnterState(controller);

  RPG_Character *const character = controller->GetCharacter();

  character->CreateCharacterEffect(FX_MeleeBasicAttackSwing);

  character->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kMeleeAttack);
}

void RPG_AiControllerState::MeleeAttacking::OnProcessAnimationEvent(RPG_ControllerComponent *const controller, hkbEvent const& animationEvent)
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
    character->DoMeleeAttack(controller->GetTarget());
  }
}

void RPG_AiControllerState::MeleeAttacking::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
  RPG_ControllerStateBase::OnTick(controller, deltaTime);

  RPG_DamageableEntity const *const targetEntity = controller->GetTarget();

  if(targetEntity)
  {
    RPG_ControllerUtil::FaceTowards(controller, targetEntity->GetPosition(), 0.25f);
  }
}

// RPG_AiControllerState::RangedAttacking
void RPG_AiControllerState::RangedAttacking::OnEnterState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnEnterState(controller);

  RPG_Character *const character = controller->GetCharacter();

  character->CreateCharacterEffect(FX_RangedAttackChargeLoop);

  character->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kRangedAttack);
}

void RPG_AiControllerState::RangedAttacking::OnExitState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnExitState(controller);

  controller->GetCharacter()->PauseCharacterEffect(FX_RangedAttackChargeLoop);
}

void RPG_AiControllerState::RangedAttacking::OnProcessAnimationEvent(RPG_ControllerComponent *const controller, hkbEvent const& animationEvent)
{
  RPG_ControllerStateBase::OnProcessAnimationEvent(controller, animationEvent);

  RPG_Character *const character = controller->GetCharacter();

  if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kRangedAttackEnd) == animationEvent.getId())
  {
    controller->SetState(RPG_ControllerStateId::kIdling);
  }
  else if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kRangedAttackFire) == animationEvent.getId())
  {
    character->PauseCharacterEffect(FX_RangedAttackChargeLoop);
    character->CreateCharacterEffect(FX_RangedAttackFire);

    character->DoRangedAttack(controller->GetTargetPoint());
  }
}

void RPG_AiControllerState::RangedAttacking::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
  RPG_ControllerStateBase::OnTick(controller, deltaTime);

  RPG_DamageableEntity const *const targetEntity = controller->GetTarget();

  if(targetEntity)
  {
    controller->SetTargetPoint(targetEntity->GetPosition());

    RPG_ControllerUtil::FaceTowards(controller, targetEntity->GetPosition(), 0.25f);
  }
}

// RPG_AiControllerState::AoeAttacking
void RPG_AiControllerState::AoeAttacking::OnEnterState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnEnterState(controller);

  RPG_Character *const character = controller->GetCharacter();

  character->CreateCharacterEffect(FX_AoeAttackBegin);
  character->CreateCharacterEffect(FX_AoeAttackChargeLoop);

  character->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kAoeAttack);
}

void RPG_AiControllerState::AoeAttacking::OnExitState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnExitState(controller);

  controller->GetCharacter()->PauseCharacterEffect(FX_AoeAttackChargeLoop);
}

void RPG_AiControllerState::AoeAttacking::OnProcessAnimationEvent(RPG_ControllerComponent *const controller, hkbEvent const& animationEvent)
{
  RPG_ControllerStateBase::OnProcessAnimationEvent(controller, animationEvent);

  RPG_Character *const character = controller->GetCharacter();

  if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kAoeAttackEnd) == animationEvent.getId())
  {
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

// RPG_AiControllerState::Challenging
void RPG_AiControllerState::Challenging::OnEnterState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnEnterState(controller);

  RPG_Character *const character = controller->GetCharacter();

  character->CreateCharacterEffect(FX_Challenge);

  character->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kChallenge);
}

void RPG_AiControllerState::Challenging::OnProcessAnimationEvent(RPG_ControllerComponent *const controller, hkbEvent const& animationEvent)
{
  RPG_ControllerStateBase::OnProcessAnimationEvent(controller, animationEvent);

  if(controller->GetCharacter()->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kChallengeEnd) == animationEvent.getId())
  {
    controller->SetState(RPG_ControllerStateId::kIdling);
  }
}

void RPG_AiControllerState::Challenging::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
  RPG_ControllerStateBase::OnTick(controller, deltaTime);

  RPG_DamageableEntity const *const targetEntity = controller->GetTarget();

  if(targetEntity)
  {
    RPG_ControllerUtil::FaceTowards(controller, targetEntity->GetPosition(), 0.25f);
  }
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
