/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#include <Vision/Samples/Engine/RPGPlugin/Precompiled.h>

#include <Vision/Samples/Engine/RPGPlugin/Character.h>

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/Effects/BlobShadow.hpp>

#include <Behavior/Behavior/Event/hkbEventQueue.h>
#include <Behavior/Behavior/World/hkbWorld.h>

#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorComponent.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokCharacterController.hpp>

#include <Vision/Samples/Engine/RPGPlugin/Attachment.h>
#include <Vision/Samples/Engine/RPGPlugin/AttackableComponent.h>
#include <Vision/Samples/Engine/RPGPlugin/DependencyHelper.h>
#include <Vision/Samples/Engine/RPGPlugin/DestructibleEntity.h>
#include <Vision/Samples/Engine/RPGPlugin/GameManager.h>
#include <Vision/Samples/Engine/RPGPlugin/InventoryPickup.h>
#include <Vision/Samples/Engine/RPGPlugin/MeshTrailEffectComponent.h>
#include <Vision/Samples/Engine/RPGPlugin/PlayerControllerComponent.h>
#include <Vision/Samples/Engine/RPGPlugin/PlayerUI.h>
#include <Vision/Samples/Engine/RPGPlugin/Projectile.h>
#include <Vision/Samples/Engine/RPGPlugin/ProjectileHealing.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionEffectHelper.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionGameDebug.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionHavokBehaviorHelper.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionHavokConversion.h>

V_IMPLEMENT_SERIAL(RPG_Character, RPG_DamageableEntity, 0, &g_RPGPluginModule);

START_VAR_TABLE(RPG_Character, RPG_DamageableEntity, "Character Entity", 0, "")
  // entity properties
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_eyeHeight, "Eye Height", "Height of this character's eyes", "150", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_sensorSize, "Ai SensorSize", "Character Sensor Size", "128", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_desiredSpeed, "Ai DesiredSpeed", "Character Desired Speed", "200", 0, 0);

  // base character stats
  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_stats.m_strength, "Strength", "Strength augments weapon damage and critical damage.", "10", 0, 0);
  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_stats.m_dexterity, "Dexterity", "Dexterity determines critical and dodge chances.", "10", 0, 0);
  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_stats.m_spirit, "Spirit", "Spirit determines the character's max Mana and Spirit attack damage.", "10", 0, 0);
  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_stats.m_vitality, "Vitality", "Vitality determines the character's max Health, armor bonus, and block chance.", "10", 0, 0);

  // health
  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_stats.m_baseHealth, "Base Health", "Character's healthMax is calculated from Vitality. This is the base to which vitality bonus is added.", "200", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_stats.m_healthRegenerationEnabled, "Regenerate Health", "Does this character's health regenerate?", "TRUE", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_stats.m_healthRegenerationRate, "Health Regeneration Rate", "How many points per second does this character regenerate?", "1.5", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_stats.m_unlimitedHealth, "Unlimited Health", "If true, this character's health resets to max when reduced to 0, and doesn't trigger death.", "FALSE", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_displayHealth, "Display Health", "Enables/Disables Character Health display.", "TRUE", 0, 0);

  // mana
  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_stats.m_baseMana, "Base Mana", "ManaMax is calculated from Spirit. This is the base to which the derived bonus is added.", "30", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_stats.m_manaRegenerationEnabled, "Regenerate Mana", "Does this character's mana regenerate?", "TRUE", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_stats.m_manaRegenerationRate, "Mana Regeneration Rate", "How many points per second does this character regenerate?", "1.5", 0, 0);

  // actions
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_spawnAnimationDelay, "Spawn Animation Delay", "How many seconds should this character delay playing the spawn animation?", "0", 0, 0);

  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_healProjectileSpeed, "Heal Projectile Speed", "How fast does the heal projectile move?", "1000", 0, 0);
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_actionData.m_healProjectileLaunchBone, "Heal Projectile Launch Bone", "From which bone is the heal projectile spawned?", "", 0, 0, 0);

  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_rangedAttackProjectileSpeed, "Ranged Attack Projectile Speed", "How fast does the ranged attack projectile move?", "1000", 0, 0);
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_actionData.m_rangedAttackProjectileLaunchBone, "Ranged Attack Projectile Launch Bone", "From which bone is the ranged attck projectile spawned?", "", 0, 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_rangedAttackImpactSpeedMultiplier, "Ranged Attack Impact Speed Multiplier", "How fast does the enemy ragdoll move when killed by a projectile", "1", 0, 0);

  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_stats.m_baseDamage, "Base Damage", "Every character has a base damage stat, which is then modified by the equipped weapon and the character's strength", "20", 0, 0);
  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_stats.m_baseAttackSpeed, "Base Attack Speed", "How many miliseconds does an attack take? Equipped weapon can modify this speed.", "400", 0, 0);

  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_aoeAttackDamageMultiplier, "AoE Attack Damage Multiplier", "Weapon damage is multiplied by this factor in an AoE attack.", "2", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_aoeAttackRange, "AoE Attack Range", "Enemies up to this range will be hit by an AoE attack.", "250", 0, 0);
  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_actionData.m_aoeAttackManaCost, "AoE Attack Mana Cost", "Mana required to execute an AoE attack.", "5", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_aoeAttackImpactSpeedMultiplier, "AoE Attack Impact Speed Multiplier", "How fast does the enemy ragdoll move when killed by an AoE attack", "1.5", 0, 0);

  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_powerAttackDamageMultiplier, "Power Attack Damage Multiplier", "Weapon damage is multiplied by this factor in a Power attack.", "3", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_powerAttackRange, "Power Attack Range", "Enemies up to this range and within the angle will be hit by a Power attack.", "550", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_powerAttackAngle, "Power Attack Angle", "Enemies within this angle and in range will be hit by a Power attack.", "10", 0, 0);
  DEFINE_VAR_INT_AND_NAME(RPG_Character, m_actionData.m_powerAttackManaCost, "Power Attack Mana Cost", "Mana required to execute a Power attack.", "5", 0, 0);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_powerAttackImpactSpeedMultiplier, "Power Attack Impact Speed Multiplier", "How fast does the enemy ragdoll move when killed by a power attack", "2", 0, 0);
  
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_actionData.m_meleeAttackImpactSpeedMultiplier, "Melee Attack Impact Speed Multiplier", "How fast does the enemy ragdoll move when killed by a melee attack", "1", 0, 0);
  
  // ai
  DEFINE_VAR_FLOAT_AND_NAME(RPG_Character, m_aggroRadius, "Aggro Radius", "The Range at which the character can detect an enemy when not player controlled", "600", 0, 0);

  // inventory & equipment
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_projectileMeshFilename, "Projectile Model", "Projectile model spawned by this character's ranged attack", "", 0, 0, "filepicker(.model)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_equipmentAttachmentBoneNames[ES_Helmet], "Bone Name: Helmet", "Bone Name for the Helmet Attachment", "", 0, 0, 0);
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_equipmentAttachmentBoneNames[ES_Chest], "Bone Name: Chest", "Bone Name for the Chest Attachment", "", 0, 0, 0);
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_equipmentAttachmentBoneNames[ES_Gloves], "Bone Name: Gloves", "Bone Name for the Gloves Attachment", "", 0, 0, 0);
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_equipmentAttachmentBoneNames[ES_Shoulder], "Bone Name: Shoulder", "Bone Name for the Weapon Shoulder", "", 0, 0, 0);
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_equipmentAttachmentBoneNames[ES_Belt], "Bone Name: Belt", "Bone Name for the Belt Attachment", "", 0, 0, 0);
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_equipmentAttachmentBoneNames[ES_Boots], "Bone Name: Boots", "Bone Name for the Boots Attachment", "", 0, 0, 0);
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_equipmentAttachmentBoneNames[ES_Weapon], "Bone Name: Weapon", "Bone Name for the Weapon Attachment", "", 0, 0, 0);
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipment[ES_Weapon], "Default Weapon", "Default Weapon Pickup Prefab", "", 0, 0, "filepicker(.vPrefab)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipment[ES_Helmet], "Default Helmet", "Default Helmet Pickup Prefab", "", 0, 0, "filepicker(.vPrefab)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipment[ES_Chest], "Default Chest", "Default Chest Pickup Prefab", "", 0, 0, "filepicker(.vPrefab)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipment[ES_Gloves], "Default Gloves", "Default Gloves Pickup Prefab", "", 0, 0, "filepicker(.vPrefab)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipment[ES_Shoulder], "Default Shoulder", "Default Shoulder Pickup Prefab", "", 0, 0, "filepicker(.vPrefab)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipment[ES_Belt], "Default Belt", "Default Belt Prefab", "", 0, 0, "filepicker(.vPrefab)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipment[ES_Boots], "Default Boots", "Default Boots Prefab", "", 0, 0, "filepicker(.vPrefab)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipmentScripts[ES_Weapon], "Default Weapon Script", "Default Weapon Pickup Script", "", 0, 0, "filepicker(.lua)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipmentScripts[ES_Helmet], "Default Helmet Script", "Default Helmet Pickup Script", "", 0, 0, "filepicker(.lua)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipmentScripts[ES_Chest], "Default Chest Script", "Default Chest Pickup Script", "", 0, 0, "filepicker(.lua)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipmentScripts[ES_Gloves], "Default Gloves Script", "Default Gloves Pickup Script", "", 0, 0, "filepicker(.lua)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipmentScripts[ES_Shoulder], "Default Shoulder Script", "Default Shoulder Pickup Script", "", 0, 0, "filepicker(.lua)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipmentScripts[ES_Belt], "Default Belt Script", "Default Belt Script", "", 0, 0, "filepicker(.lua)");
  DEFINE_VAR_VSTRING_AND_NAME(RPG_Character, m_defaultEquipmentScripts[ES_Boots], "Default Boots Script", "Default Boots Script", "", 0, 0, "filepicker(.lua)");

  // death
  DEFINE_VAR_VSTRING(RPG_Character, m_healthPickupScriptName, "Script used to spawn health pickups on death.", "Scripts\\Pickups\\RPG_HealthPickup.lua", 0, 0, "filepicker(.lua)");
  DEFINE_VAR_INT(RPG_Character, m_numHealthPickupsDroppedOnDeathMin, "Minimum Health Pickups Dropped on death.", "3", 0, 0);
  DEFINE_VAR_INT(RPG_Character, m_numHealthPickupsDroppedOnDeathMax, "Maximum Health Pickups Dropped on death.", "5", 0, 0);
  DEFINE_VAR_VSTRING(RPG_Character, m_manaPickupScriptName, "Script used to spawn mana pickups on death.", "Scripts\\Pickups\\RPG_ManaPickup.lua", 0, 0, "filepicker(.lua)");
  DEFINE_VAR_INT(RPG_Character, m_numManaPickupsDroppedOnDeathMin, "Minimum Mana Pickups Dropped on death.", "3", 0, 0);
  DEFINE_VAR_INT(RPG_Character, m_numManaPickupsDroppedOnDeathMax, "Maximum Mana Pickups Dropped on death.", "5", 0, 0);

  // debugging
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_debugDisplayStats, "Display Debug Stats", "Enables/Disables Character Debug Rendering.", "FALSE", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_debugDisplayMovementInfo, "Display Debug Movement Info", "Enables/Disables Character Movement Debug Rendering.", "FALSE", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_debugDisplayInventory, "Display Inventory", "Enables/Disables Character Inventory Debug Rendering.", "FALSE", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_debugDisplayBehaviorEvents, "Display Debug Behavior Events", "Enables/Disables Character Debug Rendering.", "FALSE", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_debugDisplayNavmeshStatus, "Display Debug Navmesh Status", "Enables/Disables Character Debug Rendering.", "FALSE", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_debugDisplayEffectInformation, "Display Effect Information", "Prints the filenames of effects called by this character.", "FALSE", 0, 0);
  DEFINE_VAR_BOOL_AND_NAME(RPG_Character, m_debugDisplayStateInformation, "Display State Information", "Prints information about this character's AI state.", "FALSE", 0, 0);
END_VAR_TABLE


/// Constructor
RPG_Character::RPG_Character()
  : RPG_DamageableEntity()
  , m_eyeHeight(150.0f)
  , m_sensorSize(128.0f)
  , m_desiredSpeed(200.0f)
  , m_aggroRadius(500.0f)
  , m_knockBackStartTime(0.0f)
  , m_knockBackDuration(0.0f)
  , m_knockBackVector(0.0f, 0.0f, 0.0f)
  , m_dead(false)
  , m_feigningDeath(false)
  , m_timeOfDeath(0.0f)
  , m_deathDissolveDelay(5.0f)
  , m_deathImpulse(0.0f, 0.0f, 0.0f)
  , m_actionData()
  , m_inventoryHandler()
  , m_stats()
  , m_projectileMeshFilename("")
  , m_displayHealth(FALSE)
  , m_equipmentAttachments()
  , m_defaultTeam(TEAM_None)
  , m_team(TEAM_None)
  , m_healthPickupScriptName()
  , m_numHealthPickupsDroppedOnDeathMin(3)
  , m_numHealthPickupsDroppedOnDeathMax(5)
  , m_manaPickupScriptName()
  , m_numManaPickupsDroppedOnDeathMin(3)
  , m_numManaPickupsDroppedOnDeathMax(5)
  , m_debugDisplayStats(FALSE)
  , m_debugDisplayMovementInfo(FALSE)
  , m_debugDisplayInventory(FALSE)
  , m_debugDisplayBehaviorEvents(FALSE)
  , m_debugDisplayNavmeshStatus(FALSE)
  , m_debugDisplayEffectInformation(FALSE)
  , m_debugDisplayStateInformation(FALSE)
{
  m_collisionRadius = 40.0f;
  m_collisionHeight = 160.0f;

  for(int index = 0; index < ES_Count; ++index)
  {
    m_equipmentAttachmentBoneNames[index] = "";
    m_defaultEquipment[index] = "";
    m_defaultEquipmentScripts[index] = "";
  }
  
  for (int i = 0; i < FX_Count; ++i)
  {
    m_characterEffectDefinitions[i] = RPG_EffectDefinition();
    m_characterEffects[i] = NULL;
  }

  for(int i = 0; i < RPG_CharacterAnimationEvent::kNumCharacterAnimationEvents; ++i)
    m_animationEventIds[i] = -1;
}

void RPG_Character::CalcPositionForTargeting(hkvVec3& targetOut) const
{
  targetOut = 0.5f * (GetEyePosition() + GetPosition());
}

#ifdef SUPPORTS_SNAPSHOT_CREATION
void RPG_Character::GetDependencies(VResourceSnapshot &snapshot)
{
  RPG_DamageableEntity::GetDependencies(snapshot);

  RPG_DependencyHelper::AddModelDependency(m_projectileMeshFilename, snapshot);

  for(int index = 0; index < ES_Count; ++index)
  {
    RPG_DependencyHelper::AddPrefabDependency(m_defaultEquipment[index], snapshot);
    RPG_DependencyHelper::AddScriptEntityDependency(m_defaultEquipmentScripts[index], snapshot);
  }

  for(int index = 0; index < FX_Count; ++index)
  {
    RPG_DependencyHelper::AddParticleEffectDependency(m_characterEffectDefinitions[index].m_vfxFilename, snapshot);
    RPG_DependencyHelper::AddTextureDependency(m_characterEffectDefinitions[index].m_wallmarkTextureFilename, snapshot);
  }

  RPG_DependencyHelper::AddScriptEntityDependency(m_healthPickupScriptName, snapshot);
  RPG_DependencyHelper::AddScriptEntityDependency(m_manaPickupScriptName, snapshot);
}
#endif

void RPG_Character::Initialize()
{
  RPG_DamageableEntity::Initialize();

  // initialize this character's team to the default for its class
  SetTeam(m_defaultTeam);
}

void RPG_Character::InitializeProperties()
{
  InitializeCharacterEffects();
  RPG_DamageableEntity::InitializeProperties();
}

/// Called to shutdown the character
void RPG_Character::DisposeObject()
{
  if(Vision::Editor.IsPlaying())
  {
    RPG_GameManager::s_instance.RemoveCharacter(this);
  }

  m_characterController = NULL;
  m_havokBehavior = NULL;
  m_controller = NULL;

  RPG_DamageableEntity::DisposeObject();
}

/// Tick / Entity update
void RPG_Character::ThinkFunction()
{
  const float deltaTime = Vision::GetTimer()->GetTimeDifference();

  // dead characters don't do anything but decompose.
  if (IsDead())
  {
    UpdateDeathState(deltaTime);
    return;
  }

  // TODO - implement both server and client tick
  ServerTick(deltaTime);
  UpdateBehaviorWorldFromModel();
}

void RPG_Character::ServerTick(float deltaTime)
{
  // @TODO: dispose entity when dead after a certain amount of time.
  if (!IsDead())
  {
    m_stats.Tick(deltaTime);

    if (m_displayHealth)
    {
      RPG_VisionGameDebug::DisplayCharacterHealth(*this);
      RPG_VisionGameDebug::UpdateEventDisplay(*this);
    }
  }

  if (!IsDead())
  {
    ProcessAnimationEvents();
    m_inventoryHandler.Tick(deltaTime);
  }

  if (m_debugDisplayStats && !IsDead())
  {
    RPG_VisionGameDebug::DebugDisplayCharacterStats(*this);
  }
  if (m_debugDisplayMovementInfo && !IsDead())
  {
    RPG_VisionGameDebug::DebugDisplayMovementInfo(*this);
  }
  if (m_debugDisplayInventory && !IsDead())
  {
    m_inventoryHandler.DebugDisplayInventory();
  }
  if (m_debugDisplayNavmeshStatus && !IsDead())
  {
    RPG_VisionGameDebug::DebugDisplayNavmeshStatus(*this);
  }

  if(m_controller)
  {
    m_controller->ServerTick(deltaTime);

    SetAnimationVariable(RPG_CharacterAnimationVariable::kMoveSpeed, m_controller->GetSpeed());
  }
  else
  {
    if(m_characterController)
    {
      RemoveComponent(m_characterController);
      m_characterController = NULL;
    }
  }
}

//void RPG_Character::ClientTick(float const deltaTime)
//{
//}

hkvVec3 const RPG_Character::GetEyePosition() const
{
  hkvVec3 position = GetPosition();
  position.z += m_eyeHeight;

  return position;
}

bool RPG_Character::GetLocallyControlled() const
{
  // True if this controller is the same one that is set in the UI
  RPG_ControllerComponent *const uiController = RPG_PlayerUI::s_instance.GetController();
  return GetController() == uiController;
}

const RPG_InventoryHandler& RPG_Character::GetInventoryHandler() const
{
  return m_inventoryHandler;
}

RPG_InventoryHandler& RPG_Character::GetInventoryHandler()
{
  return m_inventoryHandler;
}

const RPG_CharacterStats& RPG_Character::GetCharacterStats() const
{
  return m_stats;
}

RPG_CharacterStats& RPG_Character::GetCharacterStats()
{
  return m_stats;
}

const RPG_CharacterActionData& RPG_Character::GetCharacterActionData() const
{
  return m_actionData;
}

RPG_CharacterActionData& RPG_Character::GetCharacterActionData()
{
  return m_actionData;
}

void RPG_Character::DoMeleeAttack(RPG_DamageableEntity *targetEntity)
{
  CreateCharacterEffect(FX_MeleeBasicAttack);

  if(targetEntity)
  {
    hkvVec3 targetToCharacterProjectedDir;
    float targetToCharacterProjectedDist;
    RPG_ControllerUtil::GetProjectedDirAndDistanceFromTarget(this, targetEntity, targetToCharacterProjectedDir, targetToCharacterProjectedDist);

    float const minDist = RPG_ControllerUtil::GetMinimumDistanceToAttack(this, targetEntity);

    if(targetToCharacterProjectedDist < minDist)
    {
      hkvVec3 const impactDirection = (targetEntity->GetPosition() - GetPosition()).getNormalized();
      targetEntity->TakeDamage(RPG_CharacterUtil::CalcOutgoingDamage(this), impactDirection * RPG_CharacterUtil::CalcImpactSpeed(this));
    }
  }
}

void RPG_Character::DoRangedAttack(hkvVec3 const& targetPoint)
{
  RPG_Projectile *projectile = static_cast<RPG_Projectile *>(Vision::Game.CreateEntity("RPG_Projectile", GetPosition()));
  {
    projectile->SetCharacterOwner(this);
    projectile->SetProjectileMesh(GetProjectileMeshFilename());
    projectile->SetProjectileEffect(PFX_Inflight, GetCharacterEffectDefinition(FX_RangedAttackProjectile));
    projectile->SetProjectileEffect(PFX_Impact, GetCharacterEffectDefinition(FX_RangedAttackImpact));
  }

  hkvVec3 firePosition = GetPosition();
  {
    // TODO cache this
    int boneIndex = -1;
    {
      VString const& boneName = GetCharacterActionData().GetRangedAttackProjectileLaunchBone();

      if(!boneName.IsEmpty())
      {
        boneIndex = GetMesh()->GetSkeleton()->GetBoneIndexByName(boneName.AsChar());

        if(boneIndex == -1)
        {
          Vision::Error.Warning("RPG_Action_RangedAttack::FireAttack - Supplied bone name doesn't exist on this skeleton: %s", boneName.AsChar());
          //VASSERT_MSG(boneIndex != -1, "Supplied bone name doesn't exist on this skeleton.");
        }
      }
    }

    if(boneIndex != -1)
    {
      // find the projectile launch bone's position in worldspace
      hkvVec3 boneWorldSpaceTranslation;
      hkvQuat boneWorldSpaceRotation;
      VVERIFY(GetBoneCurrentWorldSpaceTransformation(boneIndex, boneWorldSpaceTranslation, boneWorldSpaceRotation));
      firePosition = boneWorldSpaceTranslation;
    }
    else
    {
      // no bone identified - just raise the projectile up a bit
      firePosition.z += 100.0f;
    }
  }
  
  float const speed = GetCharacterActionData().GetRangedAttackProjectileSpeed();

  hkvVec3 fireDirection = targetPoint - firePosition; 
  fireDirection.z = 0.0f;

  if(fireDirection.normalizeIfNotZero() == HKV_SUCCESS)
  {
    projectile->Fire(firePosition, fireDirection, speed);
  }
}

void RPG_Character::DoAoeAttack()
{
  // TODO utility function? Populate target array
  VArray<RPG_DamageableEntity*> targets;
  {
    float const aoeRadius = GetCharacterActionData().GetAoeAttackRange();
    hkvVec3 const& currentPosition = GetPosition();

    // find potential targets in range
    VArray<RPG_DamageableEntity*> const& attackableEntities = RPG_GameManager::s_instance.GetAttackableEntities();
    for(int i = 0; i < attackableEntities.GetSize(); ++i)
    {
      RPG_DamageableEntity* attackableEntity = attackableEntities.GetAt(i);

      // ignore self
      if (attackableEntity == this)
        continue;

      // find range to target character
      hkvVec3 const& targetPosition = attackableEntity->GetPosition();
      float currentRangeSquared = (currentPosition - targetPosition).getLengthSquared();
      if (currentRangeSquared <= aoeRadius * aoeRadius)
      {
        targets.Add(attackableEntity);
      }
    }
  }

  // Deal damage
  int const outgoingDamage = RPG_CharacterUtil::CalcOutgoingDamage(this, GetCharacterActionData().GetAoeAttackDamageMultiplier());
  float const impactSpeed = RPG_CharacterUtil::CalcImpactSpeed(this, GetCharacterActionData().GetAoeAttackImpactSpeedMultiplier());

  for (int i = 0; i < targets.GetSize(); ++i)
  {
    RPG_DamageableEntity* attackableEntity = targets.GetAt(i);
    VASSERT(attackableEntity);

    hkvVec3 const impactDirection = (attackableEntity->GetPosition() - GetPosition()).getNormalized();
    attackableEntity->TakeDamage(outgoingDamage, impactDirection * impactSpeed);
  }
}

void RPG_Character::DoPowerAttack()
{
  // TODO utility function? Populate target array
  hkvVec3 const& characterPosition = GetPosition();
  VArray<RPG_DamageableEntity*> targets;
  {
    float const attackRange = GetCharacterActionData().GetPowerAttackRange();
    float const attackAngle = GetCharacterActionData().GetPowerAttackAngle();
    hkvVec3 const& characterDirection = GetDirection();

    // find potential targets in range
    VArray<RPG_DamageableEntity*> const& attackableEntities = RPG_GameManager::s_instance.GetAttackableEntities();
    for(int i = 0; i < attackableEntities.GetSize(); ++i)
    {
      RPG_DamageableEntity *target = attackableEntities.GetAt(i);

      // ignore self
      if (target == this)
        continue;

      hkvVec3 const& targetPosition = target->GetPosition();

      // find range to target character
      float const currentRangeSquared = (characterPosition - targetPosition).getLengthSquared();
      if (currentRangeSquared <= attackRange * attackRange)
      {
        // if target is in range, is target within angle of a frustum offset behind the character owner?
        float const frustumBaseOffset = GetCollisionRadius() / hkvMath::tanDeg(attackAngle);  // move the frustum base behind the character such that the walls of the frustum will align to the character's width
        hkvVec3 const frustumBase = characterPosition - frustumBaseOffset * characterDirection;
        hkvVec3 frustumBaseToTarget = targetPosition - frustumBase;
        frustumBaseToTarget.z = 0.f; // get rid of the up component
        frustumBaseToTarget.normalizeIfNotZero();

        if (GetDirection().dot(frustumBaseToTarget) >= hkvMath::cosDeg(attackAngle))
        {
          // target is within the angle of the frustum base, now is the target in front of my character owner?
          hkvVec3 characterOwnerToTarget = targetPosition - characterPosition;
          characterOwnerToTarget.z = 0.f;
          if (characterDirection.dot(characterOwnerToTarget) > 0) // no need to normalize, since we only care about the sign of the dot.
          {
            targets.Add(target);
          }
        }
      }
    }
  }

  // Deal damage
  int const outgoingDamage = RPG_CharacterUtil::CalcOutgoingDamage(this, GetCharacterActionData().GetPowerAttackDamageMultiplier());
  float const impactSpeed = RPG_CharacterUtil::CalcImpactSpeed(this, GetCharacterActionData().GetPowerAttackImpactSpeedMultiplier());

  for (int i = 0; i < targets.GetSize(); ++i)
  {
    RPG_DamageableEntity *target = targets.GetAt(i);

    hkvVec3 attackLine = GetDirection();
    attackLine.z = 0.0f; attackLine.normalize();

    float const lineProjection = target->GetPosition().dot(attackLine) - characterPosition.dot(attackLine);
    hkvVec3 const closestPointOnAttackLine = characterPosition + lineProjection * attackLine;
    hkvVec3 impactDirection = (target->GetPosition() - closestPointOnAttackLine); impactDirection.z = 0.0f;

    if(impactDirection.normalizeIfNotZero() != HKV_SUCCESS)
    {
      if(Vision::Game.GetFloatRand() > 0.5f)
      {
        impactDirection.x = attackLine.y;
        impactDirection.y = -attackLine.x;
      }
      else
      {
        impactDirection.x = -attackLine.y;
        impactDirection.y = -attackLine.x;
      }
    }
    impactDirection += attackLine;
    impactDirection.normalize();

    target->TakeDamage(outgoingDamage, impactDirection * impactSpeed);
  }
}

/// Applies damage to this character
///
/// param[in] damageAmount    amount of damage to apply, before armor / resistances
///
/// @return int   the amount of damage actually applied
int RPG_Character::TakeDamage(const int damageAmount, const hkvVec3& impactVelocity /*= hkvVec3(0,0,0)*/)
// XXX TODO -
// If knockback is not implemented, it is misleading to take in impactVelocity here,
// as impactVelocity is currently ignored except when the character is killed.
{
  // ignore damage if dead
  if (IsDead())
  {
    return 0;
  }

  // subtract armor from the damage dealt
  int appliedDamage = damageAmount - GetArmor();
  
  // check for dodge or block
  const float damageRoll = Vision::Game.GetFloatRand();

  // test dodge first - dodge is preferable, as it evades all damage
  if (damageRoll <= GetCharacterStats().GetDodgeChance())
  {
    // dodged the attack. Take no damage.
#ifdef _DEBUG
    const VString msg = "Dodged Attack";
    Vision::Error.SystemMessage(msg.AsChar());
    Vision::Message.Add(1, msg.AsChar());
#endif
    m_stats.LogCharacterEvent(CE_Dodge, Vision::GetTimer()->GetTime());
    return 0;
  }
  // if a dodge didn't happen, see whether we're able to block
  else if (IsShieldEquipped() && damageRoll <= GetCharacterStats().GetBlockChance())
  {
    // blocked the attack. Reduce damage.
#ifdef _DEBUG
    const VString msg = "Blocked Attack";
    Vision::Error.SystemMessage(msg.AsChar());
    Vision::Message.Add(1, msg.AsChar());
#endif
    const float blockDamageReduction = 0.5f;  // @todo: damage reduction factor may be better implemented as a factor of the equipped shield item instead.
    appliedDamage -= static_cast<int>(static_cast<float>(appliedDamage) * blockDamageReduction);  // subtracting applied factor, rather than simply multiplying, allows us to specify block factor as the % reduced
    m_stats.LogCharacterEvent(CE_Block, Vision::GetTimer()->GetTime());
  }
  
  appliedDamage = hkvMath::Max(appliedDamage, 0);

  // set a new health value based on the damage applied
  m_stats.SetHealth(m_stats.GetHealth() - appliedDamage);

  // record applied damage
  m_stats.LogCharacterEvent(CE_Damage, Vision::GetTimer()->GetTime(), appliedDamage);

  //DisplayDamage(appliedDamage);
  CreateCharacterEffect(FX_TakeHit);

  // should this character die?
  if (m_stats.GetHealth() <= 0)
  {
    if (m_stats.HasUnlimitedHealth())
    {
      m_stats.SetHealth(m_stats.GetHealthMax());
    }
    else
    {
      m_deathImpulse = impactVelocity;
      Die();
    }
  }

  return appliedDamage;
}

void RPG_Character::ApplyKnockBack( const hkvVec3& knockBackVector, const float duration )
{
  // This intentionally overwrites any previous knockback, as stacking knockbacks when
  // multiple enemies are attacking the player renders the game unplayable.
  m_knockBackStartTime = Vision::GetTimer()->GetCurrentTime();
  m_knockBackDuration = duration;
  m_knockBackVector = knockBackVector;
}

/// Applies healing to this character
///
/// param[in] healingAmount    amount of healing to apply
///
/// @return int   the amount of healing actually applied
int RPG_Character::AddHealth(const int healthAmount)
{
  // ignore healing if dead
  if (IsDead())
  {
    return 0;
  }

  // SetHealth will automatically account for healing greater than max health, so we can keep it simple here.
  m_stats.SetHealth(m_stats.GetHealth() + healthAmount);

  // green flash & other effects
  CreateCharacterEffect(FX_ReceiveHeal);

  // just return the amount we wanted to heal.
  return healthAmount;
}

void RPG_Character::AddMana(const int manaAmount)
{
  // ignore if dead
  if (IsDead())
  {
    return;
  }

  m_stats.SetMana(m_stats.GetMana() + manaAmount);

  // orange flash & other effects
  CreateCharacterEffect(FX_ReceiveMana);
}

void RPG_Character::Die()
{
  m_dead = true;
  m_timeOfDeath = Vision::GetTimer()->GetTime();

  StopAllPersistentCharacterEffects();
  SetWeaponTrailEnabledForEquippedWeapon(false);
  DetachEquipmentOnDeath();

  RaiseAnimationEvent(RPG_CharacterAnimationEvent::kDie);

  // remove the blob shadow component
  VBlobShadow *blobShadow = static_cast<VBlobShadow *>(Components().GetComponentOfType(V_RUNTIME_CLASS(VBlobShadow)));
  if(blobShadow)
  {
    RemoveComponent(blobShadow);
  }

  // remove the attackable component
  RPG_AttackableComponent* attackableComponent = static_cast<RPG_AttackableComponent*>(Components().GetComponentOfType(V_RUNTIME_CLASS(RPG_AttackableComponent)));
  if(attackableComponent)
  {
    RemoveComponent(attackableComponent);
  }

  // remove the controller component
  SetController(NULL);

  // remove the character controller component
  if(m_characterController)
  {
    RemoveComponent(m_characterController);
  }

  // remove the collision mesh
  SetCollisionMesh(NULL);

  // play any visual and/or sound effects associated with this action
  CreateCharacterEffect(FX_Die);

  // spawn death pickups for this character (health and mana globes)
  SpawnDeathPickups();
}

bool RPG_Character::IsDead() const
{
  return m_dead;
}

bool RPG_Character::IsFeigningDeath() const
{
  return m_feigningDeath;
}

void RPG_Character::SpawnDeathPickups()
{
}

const hkvVec3& RPG_Character::GetDeathImpulse() const
{
  return m_deathImpulse;
}

vHavokBehaviorComponent *RPG_Character::GetBehaviorComponent()
{
  return m_havokBehavior;
}

vHavokBehaviorComponent const *RPG_Character::GetBehaviorComponent() const
{
  return m_havokBehavior;
}

void RPG_Character::RaiseAnimationEvent(RPG_CharacterAnimationEvent::Enum animationEvent)
{
  VASSERT(m_havokBehavior);
  int const eventId = m_animationEventIds[animationEvent];
  if(m_havokBehavior && eventId >= 0)
    m_havokBehavior->m_character->getEventQueue()->enqueueWithExternalId(eventId);
}

void RPG_Character::SetAnimationVariable(RPG_CharacterAnimationVariable::Enum const animationVariable, float const value)
{
  VASSERT(m_havokBehavior);
  int const variableId = m_animationVariableIds[animationVariable];
  if(m_havokBehavior && variableId >= 0)
    m_havokBehavior->m_character->getBehavior()->setVariableValueWord(variableId, value);
}

// Note: potentially called from Havok worker thread
void RPG_Character::InitAnimationEventIds()
{
  hkStringMap<int> const& eventNameToIdMap = vHavokBehaviorModule::GetInstance()->getBehaviorWorld()->accessEventLinker().m_nameToIdMap;
  {
    m_animationEventIds[RPG_CharacterAnimationEvent::kDie] = eventNameToIdMap.getWithDefault("Die", -1);

    m_animationEventIds[RPG_CharacterAnimationEvent::kTakeControl] = eventNameToIdMap.getWithDefault("TakeControl", -1);

    m_animationEventIds[RPG_CharacterAnimationEvent::kMove] = eventNameToIdMap.getWithDefault("Move", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kMoveEnd] = eventNameToIdMap.getWithDefault("MoveEnd", -1);

    m_animationEventIds[RPG_CharacterAnimationEvent::kMeleeAttack] = eventNameToIdMap.getWithDefault("MeleeAttack", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kMeleeAttackEnd] = eventNameToIdMap.getWithDefault("MeleeAttackEnd", -1);

    m_animationEventIds[RPG_CharacterAnimationEvent::kRangedAttack] = eventNameToIdMap.getWithDefault("RangedAttack", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kRangedAttackEnd] = eventNameToIdMap.getWithDefault("RangedAttackEnd", -1);

    m_animationEventIds[RPG_CharacterAnimationEvent::kAoeAttack] = eventNameToIdMap.getWithDefault("AoeAttack", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kAoeAttackEnd] = eventNameToIdMap.getWithDefault("AoeAttackEnd", -1);

    m_animationEventIds[RPG_CharacterAnimationEvent::kPowerAttack] = eventNameToIdMap.getWithDefault("PowerAttack", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kPowerAttackEnd] = eventNameToIdMap.getWithDefault("PowerAttackEnd", -1);

    m_animationEventIds[RPG_CharacterAnimationEvent::kHeal] = eventNameToIdMap.getWithDefault("Heal", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kHealEnd] = eventNameToIdMap.getWithDefault("HealEnd", -1);

    m_animationEventIds[RPG_CharacterAnimationEvent::kSpawnEffect] = eventNameToIdMap.getWithDefault("SpawnEffect", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kFootStepEffect] = eventNameToIdMap.getWithDefault("FootStepEffect", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kMeleeAttackFire] = eventNameToIdMap.getWithDefault("MeleeAttackFire", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kRangedAttackFire] = eventNameToIdMap.getWithDefault("RangedAttackFire", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kAoeAttackFire] = eventNameToIdMap.getWithDefault("AoeAttackFire", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kPowerAttackFire] = eventNameToIdMap.getWithDefault("PowerAttackFire", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kHealFire] = eventNameToIdMap.getWithDefault("HealFire", -1);

    m_animationEventIds[RPG_CharacterAnimationEvent::kChallenge] = eventNameToIdMap.getWithDefault("Challenge", -1);
    m_animationEventIds[RPG_CharacterAnimationEvent::kChallengeEnd] = eventNameToIdMap.getWithDefault("ChallengeEnd", -1);
  }
}

void RPG_Character::InitAnimationVariableIds()
{
  hkStringMap<int> const& eventNameToIdMap = vHavokBehaviorModule::GetInstance()->getBehaviorWorld()->accessVariableLinker().m_nameToIdMap;
  {
    m_animationVariableIds[RPG_CharacterAnimationVariable::kMoveSpeed] = eventNameToIdMap.getWithDefault("MoveSpeed", -1);
  }
}

void RPG_Character::OnHavokAnimationEvent(hkbEvent const& behaviorEvent, bool raisedBySdk)
{
  if(behaviorEvent.getPayload())
  {
    // Explicit lock (not necessary if LOCK_MODE_AUTO is set, which it is by default, but play safe)
    hkReferencedObject::lockAll();
    {
      behaviorEvent.getPayload()->addReference();
    }
    hkReferencedObject::unlockAll();
  }
  m_queuedAnimationEvents.pushBack(behaviorEvent);
}

void RPG_Character::SetController(RPG_ControllerComponent *newController)
{
  IVObjectComponent *const controller = Components().GetComponentOfBaseType(V_RUNTIME_CLASS(RPG_ControllerComponent));
  if(controller)
  {
    RemoveComponent(controller);
  }

  if(newController)
  {
    AddComponent(newController);
  }

  m_controller = newController;
}

RPG_ControllerComponent* RPG_Character::GetController()
{
  return m_controller;
}

const RPG_ControllerComponent* RPG_Character::GetController() const
{
  return m_controller;
}

/// Returns the Aggro radius. Used for Ai characters not under player control. This could include a player under certain status effects.
float RPG_Character::GetAggroRadius() const
{
  return m_aggroRadius;
}

int RPG_Character::GetArmor() const
{
  // armor is calculated as a float, and fractional component is discarded for the return val
  float armor=0.0f;
  for (int i=ES_None+1; i<ES_Count; ++i)
  {
    const RPG_InventoryItem* equippedArmor = m_inventoryHandler.GetEquippedArmor(static_cast<RPG_EquipmentSlot_e>(i));
    if (equippedArmor)
    {
      armor += equippedArmor->GetArmorValue();
    }
  }
  armor += armor * m_stats.GetArmorBonus();
  return static_cast<int>(armor);
}

bool RPG_Character::IsShieldEquipped() const
{
  for (int i=ES_None+1; i<ES_Count; ++i)
  {
    const RPG_InventoryItem* equippedArmor = m_inventoryHandler.GetEquippedArmor(static_cast<RPG_EquipmentSlot_e>(i));
    if (equippedArmor)
    {
      if (equippedArmor->IsShieldType())
      {
        return true;
      }
    }
  }
  return false;
}

void RPG_Character::ProcessAnimationEvents()
{
  // Havok Behavior currently forces a synchronous update, so there should be no contention here; lock to be safe
  hkReferencedObject::lockAll();
  {
    for(hkArray<hkbEvent>::const_iterator it = m_queuedAnimationEvents.begin(), end = m_queuedAnimationEvents.end(); it != end; ++it)
    {
      OnProcessAnimationEvent(*it);

      VASSERT(m_controller);
      if(m_controller)
        m_controller->OnProcessAnimationEvent(this, *it);

      hkbEventPayload *const payload = (*it).getPayload();
      if(payload)
        payload->removeReference();
    }
  }
  hkReferencedObject::unlockAll();

  m_queuedAnimationEvents.clear();
}

void RPG_Character::OnProcessAnimationEvent(hkbEvent const& behaviorEvent)
{
  if(GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kFootStepEffect) == behaviorEvent.getId())
    CreateCharacterEffect(FX_Footstep);
}

void RPG_Character::UpdateBehaviorWorldFromModel()
{
  VASSERT(m_havokBehavior);
  VASSERT(m_havokBehavior->m_character);
  if(m_havokBehavior && m_havokBehavior->m_character)
  {
    // Behavior transform update
    hkQsTransform worldFromModel;
    {
      RPG_VisionHavokConversion::VisionToHavokRotationQuaternion(GetRotationMatrix(), worldFromModel.m_rotation);
      worldFromModel.setScale(hkVector4::getConstant<HK_QUADREAL_1>());
      RPG_VisionHavokConversion::VisionToHavokPoint(GetPosition(), worldFromModel.m_translation);
    }

    m_havokBehavior->m_character->setWorldFromModel(worldFromModel);
  }
}

void RPG_Character::MessageFunction(int id, INT_PTR paramA, INT_PTR paramB)
{
  RPG_DamageableEntity::MessageFunction(id, paramA, paramB);

  switch(id)
  {
  case RPG_VisionUserMessages::kHavokAnimationEvent:
    OnHavokAnimationEvent(*(hkbEvent*)(paramA), paramB != 0);
    break;
  }
}

void RPG_Character::PostInitialize()
{
  RPG_DamageableEntity::PostInitialize();

  if (m_debugDisplayEffectInformation)
  {
    ActivateCharacterEffectDebugDisplay();
  }

  if(!Vision::Editor.IsPlaying())
  {
    return;
  }

  // Behavior animation init
  m_havokBehavior = static_cast<vHavokBehaviorComponent *>(Components().GetComponentOfType(V_RUNTIME_CLASS(vHavokBehaviorComponent)));

  VASSERT(m_havokBehavior->m_character);
  if(!m_havokBehavior->m_character)
  {
    m_havokBehavior->InitVisionCharacter(this);
  }

  InitAnimationEventIds();
  InitAnimationVariableIds();

  UpdateBehaviorWorldFromModel();

  // AI controller init
  if(GetControllerComponentType())
  {
    m_controller =  static_cast<RPG_ControllerComponent*>(GetControllerComponentType()->CreateInstance());
    
    m_controller->SetSensorSize(m_sensorSize);
    m_controller->SetDesiredSpeed(m_desiredSpeed);

    AddComponent(m_controller);
  }
  else
  {
    m_controller = static_cast<RPG_ControllerComponent*>(Components().GetComponentOfBaseType(V_RUNTIME_CLASS(RPG_ControllerComponent)));
  }
  VASSERT_MSG(m_controller, "This character requires an AI component. Please put an (RPG) AI Controller or (RPG) Player AI Controller on it.");

  // create attachment proxies
  for(int index = 0; index < ES_Count; ++index)
  {
    m_equipmentAttachments.Add(new RPG_Attachment(this));
  }

  // Havok Character Proxy init
  m_characterController = new vHavokCharacterController;
  m_characterController->Capsule_Radius = m_collisionRadius;
  m_characterController->Character_Bottom = hkvVec3(0.0f, 0.0f, m_collisionRadius);
  m_characterController->Character_Top = hkvVec3(0.0f, 0.0f, m_collisionHeight - m_collisionRadius);
  m_characterController->Gravity_Scale = 3.0f;
  AddComponent(m_characterController);

  // Initialize health
  m_stats.SetHealthMax(m_stats.CalculateHealthMax());
  m_stats.SetHealth(m_stats.GetHealthMax());

  // Initialize mana
  m_stats.SetManaMax(m_stats.CalculateManaMax());
  m_stats.SetMana(m_stats.GetManaMax());

  // @temp: apply temporary experience values
  m_stats.SetExperienceLevelThreshold(1000);  // @temp: test value
  m_stats.SetExperience(386); // @temp: test value

  m_inventoryHandler.Initialize(this);
  //RPG_InventoryHandler::AddDefaultInventory(*this);

  RPG_GameManager::s_instance.AddCharacter(this);

  // setup default equipment
  SetUpDefaultEquipment();

  // if this character has any ambient effects defined, start them up now.
  CreateCharacterEffect(FX_Ambient);
}

/// Attaches an equipment prefab to the bone associated with the desired equipment slot
VisBaseEntity_cl* RPG_Character::AttachEquipmentPrefab(RPG_EquipmentSlot_e slot, const VString& prefabName)
{
  RPG_Attachment* attachment = m_equipmentAttachments.GetAt(slot);

  VisBaseEntity_cl* entity = RPG_GameManager::s_instance.CreateEntityFromPrefab(prefabName,  hkvVec3(0.0f, 0.0f, 0.0f),  hkvVec3(0.0f, 0.0f, 0.0f));

  if (entity)
  {
  attachment->Attach(entity, m_equipmentAttachmentBoneNames[slot], hkvVec3(0.0f, 0.0f, 0.0f));
  }
  return entity;
}

/// Attaches an equipment model to the bone associated with the desired equipment slot
VisBaseEntity_cl* RPG_Character::AttachEquipmentModel(RPG_EquipmentSlot_e slot, const VString& modelName)
{
  RPG_Attachment* attachment = m_equipmentAttachments.GetAt(slot);

  VisBaseEntity_cl* entity = RPG_GameManager::s_instance.CreateEntity("VisBaseEntity_cl", hkvVec3(0.0f, 0.0f, 0.0f));
  entity->SetMesh(modelName);
  if (entity)
  {
  attachment->Attach(entity, m_equipmentAttachmentBoneNames[slot], hkvVec3(0.0f, 0.0f, 0.0f));
  }
  return entity;
}

/// Detaches anything attached to the bone associated with the desired equipment slot
void RPG_Character::DetachEquipment(RPG_EquipmentSlot_e slot, bool disposeObject /*= true*/)
{
  RPG_Attachment* attachment = m_equipmentAttachments.GetAt(slot);

  attachment->Detach(disposeObject);
}

void RPG_Character::DetachEquipmentOnDeath()
{
  for(int i = 0; i < m_equipmentAttachments.Count(); ++i)
  {
    RPG_InventoryItem* inventoryItem = m_inventoryHandler.GetEquippedItem(static_cast<RPG_EquipmentSlot_e>(i));
    if (inventoryItem)
    {
      if (inventoryItem->GetDetachOnDeath())
      {
        DetachEquipment(static_cast<RPG_EquipmentSlot_e>(i));
        inventoryItem->SetEquippedEntity(NULL);
      }
    }
  }
}

void RPG_Character::CreateCharacterEffect(RPG_CharacterEffect_e const effectType)
{
  RPG_EffectDefinition const& effectDef = GetCharacterEffectDefinition(effectType);
  RPG_Effect *&effect = m_characterEffects[effectType];

  // create an entry for this instantiated effect, if needed
  if (!effect)
  {
    effect = vstatic_cast<RPG_Effect *>(Vision::Game.CreateEntity("RPG_Effect", GetPosition()));
  }
  VASSERT(effect);

  // whether this effect has been newly created, or is being recycled, its position and orientation need to be aligned with the character.
  effect->SetPosition(GetPosition());
  effect->SetOrientation(GetOrientation());

  // create this effect's visual and audio components
  if (effectDef.m_attachEffect)
  {
    effect->Create(effectDef, this); // attached effect
  }
  else
  {
    // if we're not attached, but we've been given a bone name, spawn unattached at that bone's current location.
    int boneIndex = -1;
    if (!effectDef.m_vfxBoneName.IsEmpty())
    {
      boneIndex = GetMesh()->GetSkeleton()->GetBoneIndexByName(effectDef.m_vfxBoneName.AsChar());
      if(boneIndex == -1)
      {
        Vision::Error.Warning("RPG_Character::CreateCharacterEffect - Supplied bone name doesn't exist on this skeleton: %s", effectDef.m_vfxBoneName.AsChar());
      }
    }

    if(boneIndex != -1)
    {
      // find the projectile launch bone's position in worldspace
      hkvVec3 boneWorldSpaceTranslation;
      hkvQuat boneWorldSpaceRotation;
      VVERIFY(GetBoneCurrentWorldSpaceTransformation(boneIndex, boneWorldSpaceTranslation, boneWorldSpaceRotation));
      hkvVec3 boneWorldSpaceOrientation;
      boneWorldSpaceRotation.getAsEulerAngles(boneWorldSpaceOrientation.z, boneWorldSpaceOrientation.y, boneWorldSpaceOrientation.x); // zyx. Because fancy, that's why.
      effect->Create(effectDef, boneWorldSpaceTranslation, boneWorldSpaceOrientation);  // unattached effect at this character's location
    }
    else
    {
      effect->Create(effectDef, GetPosition(), GetOrientation());  // unattached effect at this character's location
    }
  }
}

VisParticleEffect_cl* RPG_Character::GetPersistentCharacterEffect(RPG_CharacterEffect_e const effectType) const
{
  if (m_characterEffects[effectType])
  {
    return m_characterEffects[effectType]->GetPersistentVisualEffect();
  }
  return NULL;
}

void RPG_Character::PauseCharacterEffect(RPG_CharacterEffect_e const effectType)
{
  if (m_characterEffects[effectType])
  {
    m_characterEffects[effectType]->Pause();
  }
}

void RPG_Character::FinishCharacterEffect(RPG_CharacterEffect_e const effectType)
{
  if (m_characterEffects[effectType])
  {
    m_characterEffects[effectType]->Finish();

    // clear the effect slot reference
    m_characterEffects[effectType] = NULL;
  }
}

void RPG_Character::StopAllPersistentCharacterEffects()
{
  for(int i = 0; i < FX_Count; ++i)
    FinishCharacterEffect(static_cast<RPG_CharacterEffect_e>(i));
}

// TODO modifying definitions?
void RPG_Character::ActivateCharacterEffectDebugDisplay()
{
  for (int i = 0; i < FX_Count; ++i)
  {
    RPG_CharacterEffect_e const effectType = static_cast<RPG_CharacterEffect_e>(i);
    if (!m_characterEffectDefinitions[effectType].m_vfxFilename.IsEmpty())
    {
      m_characterEffectDefinitions[effectType].m_displayDebugInfo = true;
    }
  }
}

void RPG_Character::UpdateDeathState(float const deltaTime)
{
  if (Vision::GetTimer()->GetTime() - m_timeOfDeath >= m_deathDissolveDelay)
  {
    hkvVec3 effectPosition = GetPosition();
    
    int const boneIndex = hkvMath::Min(1, GetMesh()->GetSkeleton()->GetBoneCount() - 1); // Attempt to grab the character's root (bone 1). Otherwise, grab the origin (bone 0).
    if(GetMesh()->GetSkeleton()->GetBone(boneIndex))
    {
      // find the location of the character's root bone in worldspace
      hkvVec3 boneWorldSpaceTranslation;
      hkvQuat boneWorldSpaceRotation;
      if (GetBoneCurrentWorldSpaceTransformation(boneIndex, boneWorldSpaceTranslation, boneWorldSpaceRotation))
      {
        effectPosition = boneWorldSpaceTranslation;
      }
    }

    // spawn the death dissolve effect
    RPG_GameManager::s_instance.CreateEffect(m_characterEffectDefinitions[FX_DeathDissolve], effectPosition);

    // clear me out.
    DisposeObject();
  }
}

bool RPG_Character::IsOnNavMesh(float const testRadius /*= 1.f*/) const
{
  return RPG_ControllerUtil::IsPointOnNavmesh(GetPosition(), testRadius);
}

void RPG_Character::Teleport(hkvVec3 const& position)
{
  m_characterController->SetPosition(position);
  SetPosition(position);
  UpdateBinding();
}

VString const& RPG_Character::GetProjectileMeshFilename() const
{
  return m_projectileMeshFilename;
}

void RPG_Character::SetTeam(RPG_Team_e const& team)
{
  m_team = team;
}

RPG_Team_e RPG_Character::GetTeam() const
{
  return m_team;
}

void RPG_Character::SetWeaponTrailEnabledForEquippedWeapon(bool const enabled)
{
  const RPG_InventoryItem* equippedWeapon = GetInventoryHandler().GetEquippedWeapon();
  if (equippedWeapon)
  {
    VisBaseEntity_cl* eqippedWeaponEntity = equippedWeapon->GetEquippedEntity();
    if (eqippedWeaponEntity)
    {
      RPG_MeshTrailEffectComponent* meshTrailComponent = static_cast<RPG_MeshTrailEffectComponent*>(eqippedWeaponEntity->Components().GetComponentOfType(V_RUNTIME_CLASS(RPG_MeshTrailEffectComponent)));
      if (meshTrailComponent)
      {
        meshTrailComponent->SetEnabled(enabled);
      }
    }
  }
}

void RPG_Character::SetUpDefaultEquipment()
{
  for(int index = 0; index < ES_Count; ++index)
  {
    // check for a bone name for this slot
    if(!m_equipmentAttachmentBoneNames[index].IsEmpty())
    {
      VisBaseEntity_cl* baseEntity = NULL;

      if(!m_defaultEquipment[index].IsEmpty())
      {
        // create the attachment from a prefab if we have one
        baseEntity = RPG_GameManager::s_instance.CreateEntityFromPrefab(m_defaultEquipment[index], hkvVec3(0.0f, 0.0f, 0.0f), hkvVec3(0.0f, 0.0f, 0.0f));
      }
      else if(!m_defaultEquipmentScripts[index].IsEmpty())
      {
        // if we have no prefab, create a script entity if we have one
        baseEntity = RPG_GameManager::s_instance.CreateEntityFromScript(m_defaultEquipmentScripts[index], hkvVec3(0.0f, 0.0f, 0.0f), hkvVec3(0.0f, 0.0f, 0.0f));
      }

      // attach the entity if it's an inventory pickup
      if(baseEntity &&
        baseEntity->IsFrom(RPG_InventoryPickup))
      {
        RPG_InventoryPickup* defaultWeaponPickup = static_cast<RPG_InventoryPickup*>(baseEntity);

        defaultWeaponPickup->GivePickupItem(this);
        defaultWeaponPickup->DisposeObject();
        GetInventoryHandler().EquipItem(static_cast<RPG_EquipmentSlot_e>(index), GetInventoryHandler().GetInventoryCollection().Count() - 1); // bails gracefully if supplied item is already equipped.
        SetWeaponTrailEnabledForEquippedWeapon(false);  // begin with weapon trails disabled
      }
    }
  }
}

void RPG_Character::SetVisible(bool visible)
{
  unsigned int visibleBitmask;
  if (visible)
  {
    visibleBitmask = VIS_ENTITY_VISIBLE;
  }
  else
  {
    visibleBitmask = VIS_ENTITY_INVISIBLE;
  }

  // set the character's visibility
  SetVisibleBitmask(visibleBitmask);

  // set inventory visibility
  for(int i = 0; i < m_equipmentAttachments.Count(); ++i)
  {
    RPG_InventoryItem* inventoryItem = m_inventoryHandler.GetEquippedItem(static_cast<RPG_EquipmentSlot_e>(i));
    if (inventoryItem)
    {
      VisBaseEntity_cl* equippedEntity = inventoryItem->GetEquippedEntity();
      if (equippedEntity)
      {
        equippedEntity->SetVisibleBitmask(visibleBitmask);
      }
    }
  }
}

int RPG_CharacterUtil::CalcOutgoingDamage(RPG_Character const *character, float const damageMultiplier)
{
  int outgoingDamage = 0;
  {
    RPG_InventoryItem const *const equippedWeapon = character->GetInventoryHandler().GetEquippedWeapon();
    if(equippedWeapon)
    {
      int const characterBaseDamage = character->GetCharacterStats().GetBaseDamage();
      float const damageMin = characterBaseDamage * equippedWeapon->GetMinDamage();
      float const damageMax = characterBaseDamage * equippedWeapon->GetMaxDamage();
      outgoingDamage = static_cast<int>((damageMin + Vision::Game.GetFloatRand() * (damageMax - damageMin)) * damageMultiplier + 0.5f);
    }
  }

  return outgoingDamage;
}

float RPG_CharacterUtil::CalcImpactSpeed(RPG_Character const *character, float const impactSpeedMultiplier)
{
  float impactSpeed = 0;
  RPG_InventoryItem const *const equippedWeapon = character->GetInventoryHandler().GetEquippedWeapon();
  if(equippedWeapon)
  {
    impactSpeed = equippedWeapon->GetImpactSpeed() * impactSpeedMultiplier;
  }
  return impactSpeed;
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
