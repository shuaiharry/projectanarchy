/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#include <Vision/Samples/Engine/RPGPlugin/Precompiled.h>

#include <Vision/Samples/Engine/RPGPlugin/PlayerCharacter.h>

#include <Vision/Samples/Engine/RPGPlugin/DependencyHelper.h>
#include <Vision/Samples/Engine/RPGPlugin/GameManager.h>
#include <Vision/Samples/Engine/RPGPlugin/PlayerControllerComponent.h>
#include <Vision/Samples/Engine/RPGPlugin/LevelInfo.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionEffectHelper.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionHavokBehaviorHelper.h>

#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorComponent.hpp>

#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/VFmodManager.hpp>


V_IMPLEMENT_SERIAL(RPG_PlayerCharacter, RPG_Character, 0, &g_RPGPluginModule);

START_VAR_TABLE(RPG_PlayerCharacter, RPG_Character, "Player Character Entity", 0, "")  
END_VAR_TABLE

  /// Constructor
RPG_PlayerCharacter::RPG_PlayerCharacter()
  : RPG_Character()
  , m_feignDeathDuration(3.8f)
{
  m_defaultTeam = TEAM_Blue;
}

void RPG_PlayerCharacter::InitializeCharacterEffects()
{
  const VString& fmodEventGroup = "Characters/Barbarian";
  RPG_CharacterEffect_e effectType;

  // basics ///////////////////////////////////////////////////////////////////
  effectType = FX_Spawn;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Respawn_mobile.xml";
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Spawn";
    m_characterEffectDefinitions[effectType].m_attachEffect = false;
  }

  effectType = FX_Moving;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Movement.xml";
  }

  effectType = FX_Footstep;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Footstep_mobile.xml";
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Footstep_Run_Stone";
  }

  // melee attack /////////////////////////////////////////////////////////////
  effectType = FX_MeleeBasicAttackSwing;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Basic_Melee_Swing_Charge_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxBoneName = "right_attachment_jnt";
  }

  effectType = FX_MeleeBasicAttack;
  {
    //m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Basic_Melee_Swing_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxBoneName = "right_attachment_jnt";
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Melee_Basic_Swing";
  }

  // ranged attack ////////////////////////////////////////////////////////////
  effectType = FX_RangedAttackChargeLoop;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Ranged_Attack_Charge_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxBoneName = "right_attachment_jnt";
  }

  effectType = FX_RangedAttackFire;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Ranged_Attack_Fire_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxBoneName = "right_attachment_jnt";
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Ranged_Attack_Fire";
  }

  effectType = FX_RangedAttackProjectile;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Ranged_Attack_Proj_mobile.xml";
  }

  effectType = FX_RangedAttackImpact;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Ranged_Attack_Impact.xml";
    m_characterEffectDefinitions[effectType].m_cameraShakeAmplitude = 50.f; 
    m_characterEffectDefinitions[effectType].m_cameraShakeDuration = 0.08f;
  }

  // power attack /////////////////////////////////////////////////////////////
  effectType = FX_PowerAttackSwing;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Power_Attack_Swing_Charge_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxBoneName = "right_attachment_jnt";
  }

  effectType = FX_PowerAttack;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Power_Attack_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxPositionOffset = hkvVec3(150.f, 0.f, 0.f);
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Power_Attack_Swing";
    m_characterEffectDefinitions[effectType].m_attachEffect = false;
    //m_characterEffectDefinitions[effectType].m_displayDebugInfo = true;
  }

  // aoe attack ///////////////////////////////////////////////////////////////
  effectType = FX_AoeAttackBegin;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Aoe_Attack_Swing_Charge_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxBoneName = "right_attachment_jnt";
  }

  effectType = FX_AoeAttackFire;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Aoe_Attack_mobile.xml";
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Aoe_Attack_Swing";
    m_characterEffectDefinitions[effectType].m_attachEffect = false;
  }

  // healing and damage ///////////////////////////////////////////////////////
  effectType = FX_ReceiveHeal;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Receive_Heal_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxBoneName = "spine_jnt";
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Receive_Heal";
    m_characterEffectDefinitions[effectType].m_shaderFlashDuration = 1.f;
    m_characterEffectDefinitions[effectType].m_shaderFlashColor = VColorRef(208, 24, 24);
  }

  effectType = FX_ReceiveMana;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Receive_Mana_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxBoneName = "spine_jnt";
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Receive_Heal";
    m_characterEffectDefinitions[effectType].m_shaderFlashDuration = 1.f;
    m_characterEffectDefinitions[effectType].m_shaderFlashColor = VColorRef(208, 208, 24);
  }

  effectType = FX_TakeHit;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_TakeHit_mobile.xml";
    m_characterEffectDefinitions[effectType].m_vfxBoneName = "spine_jnt";
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Take_Hit";
    m_characterEffectDefinitions[effectType].m_shaderFlashDuration = 1.f;
    m_characterEffectDefinitions[effectType].m_shaderFlashColor = VColorRef(150, 200, 255);
  }

  effectType = FX_Die;
  {
    m_characterEffectDefinitions[effectType].m_vfxFilename = "Particles/CharacterEffects/Barbarian_Die_mobile.xml";
    m_characterEffectDefinitions[effectType].m_fmodEventGroupName = fmodEventGroup;
    m_characterEffectDefinitions[effectType].m_fmodEventName = "Take_Hit";
  }
}

/// Returns type type of AiControllerComponent to create
VType* RPG_PlayerCharacter::GetControllerComponentType()
{
  return V_RUNTIME_CLASS(RPG_PlayerControllerComponent);
}

void RPG_PlayerCharacter::PostInitialize()
{
  RPG_Character::PostInitialize();

  VisObject3D_cl* audioListener = VFmodManager::GlobalManager().GetListenerObject();

  if(audioListener)
  {
    audioListener->SetPosition(GetPosition());

    if(Vision::Camera.GetMainCamera())
    {
      audioListener->SetOrientation(Vision::Camera.GetMainCamera()->GetOrientation());
    }
  }
}

void RPG_PlayerCharacter::ServerTick(float const deltaTime)
{
  VisObject3D_cl* audioListener = VFmodManager::GlobalManager().GetListenerObject();
  
  if(audioListener)
  {
    // set the audio listener to use the player character position and camera orientation, so sounds feel like they're coming from the right place, but aren't too quiet.
    audioListener->SetPosition(GetPosition());
    audioListener->SetOrientation(Vision::Camera.GetMainCamera()->GetOrientation());
  }

  RPG_Character::ServerTick(deltaTime);
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
