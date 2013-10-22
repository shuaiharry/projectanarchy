/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#include <Vision/Samples/Engine/RPGPlugin/Precompiled.h>

#include <Vision/Samples/Engine/RPGPlugin/RangedGuardianAiControllerComponent.h>

#include <Vision/Samples/Engine/RPGPlugin/GameManager.h>
#include <Vision/Samples/Engine/RPGPlugin/PlayerCharacter.h>
#include <Vision/Samples/Engine/RPGPlugin/RangedGuardian.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionHavokConversion.h>

V_IMPLEMENT_SERIAL(RPG_RangedGuardianAiControllerComponent, RPG_AiControllerComponent, 0, &g_RPGPluginModule);

START_VAR_TABLE(RPG_RangedGuardianAiControllerComponent, RPG_AiControllerComponent, "(RPG) Ranged Guardian AI Controller", 0, "(RPG) Ranged Guardian AI Controller")
END_VAR_TABLE

  // RPG_GuardianAiControllerComponent
RPG_RangedGuardianAiControllerComponent::RPG_RangedGuardianAiControllerComponent()
  : RPG_AiControllerComponent()
{
  //m_debugDisplayStateInformation = true;
}

void RPG_RangedGuardianAiControllerComponent::SetOwner(VisTypedEngineObject_cl *newOwner)
{
  RPG_AiControllerComponent::SetOwner(newOwner);

  if(newOwner)
  {
    // Create states
    m_states[RPG_ControllerStateId::kNotBeingControlled] = new RPG_ControllerState::NotBeingControlled();
    m_states[RPG_ControllerStateId::kIdling] = new RPG_RangedGuardianAiControllerState::Idling();
    m_states[RPG_ControllerStateId::kMoving] = new RPG_RangedGuardianAiControllerState::Moving();
    m_states[RPG_ControllerStateId::kRangedAttacking] = new RPG_AiControllerState::RangedAttacking();
    m_states[RPG_ControllerStateId::kChallenging] = new RPG_AiControllerState::Challenging();
  }
}

void RPG_RangedGuardianAiControllerState::Idling::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
  RPG_ControllerStateBase::OnTick(controller, deltaTime);

  RPG_RangedGuardianAiControllerComponent *const rangedController = vstatic_cast<RPG_RangedGuardianAiControllerComponent *>(controller);
  RPG_Character *const character = controller->GetCharacter();

  if(rangedController->HasValidTarget())
  {
    controller->SetState(RPG_ControllerStateId::kMoving);
    return;
  }
  else if(rangedController->AcquireTarget())
  {
    controller->SetState(RPG_ControllerStateId::kChallenging);
    return;
  }
}

void RPG_RangedGuardianAiControllerState::Moving::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
  RPG_ControllerState::Moving::OnTick(controller, deltaTime);

  RPG_AiControllerComponent *aiController = vstatic_cast<RPG_AiControllerComponent*>(controller);
  RPG_Character *const character = aiController->GetCharacter();

  if(aiController->HasValidTarget())
  {
    RPG_DamageableEntity *const target = aiController->GetTarget();

    hkvVec3 targetToCharacterProjectedDir;
    float targetToCharacterProjectedDist;
    RPG_ControllerUtil::GetProjectedDirAndDistanceFromTarget(character, target, targetToCharacterProjectedDir, targetToCharacterProjectedDist);

    float const fleeDist = aiController->m_fleeRange;
    float const minDist =  RPG_ControllerUtil::GetMinimumDistanceToAttack(character, target);
    
    if(targetToCharacterProjectedDist < fleeDist)
    {
      // Flee
      controller->RequestPath(character->GetPosition() + fleeDist * targetToCharacterProjectedDir);
    }
    else if(targetToCharacterProjectedDist > minDist)
    {
      // MoveTo
      controller->RequestPath(target->GetPosition() + (target->GetCollisionRadius() + character->GetCollisionRadius()) * targetToCharacterProjectedDir);
    }
    else
    {
      // Attack
      aiController->TryRangedAttack();
      return;
    }
  }
  else
  {
    controller->SetState(RPG_ControllerStateId::kIdling);
    return;
  }

  hkvVec3 dir;
  RPG_ControllerUtil::CalcDirection(dir, controller->GetCharacter()->GetDirection(), controller->GetDirection(), 0.25f);
  controller->GetCharacter()->SetDirection(dir);
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
